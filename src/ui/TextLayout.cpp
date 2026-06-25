#include "ui/TextLayout.h"

#include <algorithm>
#include <cmath>

namespace visiform::ui {
namespace {

[[nodiscard]] float measuredTextWidth(const visage::Font& font, const std::u32string& text)
{
    if (text.empty() || font.packedFont() == nullptr) {
        return 0.0f;
    }
    return font.stringWidth(text);
}

[[nodiscard]] std::u32string toUtf32(const std::string& text)
{
    return visage::String::convertUtf8ToUtf32<std::u32string>(text);
}

[[nodiscard]] std::string toUtf8(const std::u32string& text)
{
    return visage::String::convertUtf32ToUtf8(text);
}

[[nodiscard]] bool isSpace(char32_t value)
{
    return value == U' ' || value == U'\t' || value == U'\r' || value == U'\v' || value == U'\f';
}

[[nodiscard]] std::u32string trimTrailingSpaces(std::u32string value)
{
    while (!value.empty() && isSpace(value.back())) {
        value.pop_back();
    }
    return value;
}

[[nodiscard]] std::size_t skipLeadingSpaces(const std::u32string& text, std::size_t index)
{
    while (index < text.size() && isSpace(text[index])) {
        ++index;
    }
    return index;
}

[[nodiscard]] std::u32string elideLine(const visage::Font& font, const std::u32string& text, float availableWidth)
{
    if (text.empty() || availableWidth <= 0.0f || measuredTextWidth(font, text) <= availableWidth) {
        return availableWidth <= 0.0f ? std::u32string{} : text;
    }

    const std::u32string ellipsis = U"\u2026";
    const float ellipsisWidth = measuredTextWidth(font, ellipsis);
    if (ellipsisWidth > availableWidth) {
        return {};
    }

    const int prefixLength = font.widthOverflowIndex(
        text.c_str(),
        static_cast<int>(text.size()),
        availableWidth - ellipsisWidth);
    std::u32string fitted = text.substr(0, static_cast<std::size_t>(std::max(0, prefixLength)));
    fitted = trimTrailingSpaces(std::move(fitted));
    fitted += ellipsis;
    return fitted;
}

void appendWrappedLine(const visage::Font& font,
    const std::u32string& source,
    float availableWidth,
    std::vector<std::u32string>& lines)
{
    if (source.empty()) {
        lines.emplace_back();
        return;
    }
    if (availableWidth <= 0.0f) {
        lines.push_back(trimTrailingSpaces(source));
        return;
    }

    std::size_t lineStart = skipLeadingSpaces(source, 0);
    while (lineStart < source.size()) {
        const std::u32string remaining = trimTrailingSpaces(source.substr(lineStart));
        if (remaining.empty()) {
            break;
        }
        if (measuredTextWidth(font, remaining) <= availableWidth) {
            lines.push_back(remaining);
            break;
        }

        std::size_t bestBreak = std::string::npos;
        std::size_t overflowIndex = lineStart + 1;
        for (std::size_t index = lineStart + 1; index <= source.size(); ++index) {
            if (index < source.size() && isSpace(source[index])) {
                bestBreak = index;
            }
            const std::u32string candidate = trimTrailingSpaces(source.substr(lineStart, index - lineStart));
            if (measuredTextWidth(font, candidate) > availableWidth) {
                overflowIndex = index;
                break;
            }
        }

        if (bestBreak != std::string::npos && bestBreak > lineStart) {
            lines.push_back(trimTrailingSpaces(source.substr(lineStart, bestBreak - lineStart)));
            lineStart = skipLeadingSpaces(source, bestBreak + 1);
            continue;
        }

        const std::size_t splitIndex = std::max<std::size_t>(lineStart + 1, overflowIndex - 1);
        lines.push_back(source.substr(lineStart, splitIndex - lineStart));
        lineStart = splitIndex;
    }

    if (lines.empty()) {
        lines.emplace_back();
    }
}

[[nodiscard]] std::vector<std::u32string> logicalLines(const std::u32string& text)
{
    std::vector<std::u32string> lines;
    std::size_t lineStart = 0;
    while (lineStart <= text.size()) {
        const std::size_t lineEnd = text.find(U'\n', lineStart);
        const std::size_t safeLineEnd = lineEnd == std::u32string::npos ? text.size() : lineEnd;
        lines.push_back(text.substr(lineStart, safeLineEnd - lineStart));
        if (lineEnd == std::u32string::npos) {
            break;
        }
        lineStart = lineEnd + 1;
    }
    return lines;
}

[[nodiscard]] float alignedX(const TextLayoutRect& bounds, float lineWidth, const std::string& alignment)
{
    if (alignment == "Center") {
        return bounds.x + std::max(0.0f, (bounds.width - lineWidth) * 0.5f);
    }
    if (alignment == "Right") {
        return bounds.x + std::max(0.0f, bounds.width - lineWidth);
    }
    return bounds.x;
}

[[nodiscard]] float alignedY(const TextLayoutRect& bounds, float totalHeight, const std::string& alignment)
{
    if (alignment == "Bottom") {
        return bounds.y + std::max(0.0f, bounds.height - totalHeight);
    }
    if (alignment == "Center") {
        return bounds.y + std::max(0.0f, (bounds.height - totalHeight) * 0.5f);
    }
    return bounds.y;
}

} // namespace

TextOverflowMode textOverflowModeFromString(const std::string& value)
{
    return value == "Ellipsis" ? TextOverflowMode::Ellipsis : TextOverflowMode::Clip;
}

std::string textOverflowModeToString(TextOverflowMode value)
{
    return value == TextOverflowMode::Ellipsis ? "Ellipsis" : "Clip";
}

TextLayoutResult layoutText(
    const std::string& text,
    const visage::Font& font,
    const TextLayoutRect& bounds,
    const TextLayoutOptions& options)
{
    TextLayoutResult result;
    result.bounds = bounds;
    result.lineHeight = std::max(1.0f, font.packedFont() != nullptr ? font.lineHeight() : 16.0f);

    const std::u32string utf32 = toUtf32(text);
    std::vector<std::u32string> layoutLines;
    if (!options.multiline) {
        std::u32string firstLine = logicalLines(utf32).front();
        if (options.overflowMode == TextOverflowMode::Ellipsis) {
            firstLine = elideLine(font, trimTrailingSpaces(firstLine), bounds.width);
        }
        layoutLines.push_back(trimTrailingSpaces(firstLine));
    }
    else {
        for (const auto& line : logicalLines(utf32)) {
            if (options.wordWrap) {
                appendWrappedLine(font, line, bounds.width, layoutLines);
            }
            else {
                layoutLines.push_back(trimTrailingSpaces(line));
            }
        }
    }

    result.totalTextHeight = result.lineHeight * static_cast<float>(layoutLines.size());
    const float textTop = alignedY(bounds, result.totalTextHeight, options.verticalAlignment);
    result.lines.reserve(layoutLines.size());
    for (std::size_t index = 0; index < layoutLines.size(); ++index) {
        const std::string lineText = toUtf8(layoutLines[index]);
        const float lineWidth = measuredTextWidth(font, layoutLines[index]);
        result.lines.push_back({
            lineText,
            lineWidth,
            alignedX(bounds, lineWidth, options.horizontalAlignment),
            textTop + result.lineHeight * static_cast<float>(index)
        });
    }
    return result;
}

void drawTextLayout(
    visage::Canvas& canvas,
    const visage::Font& font,
    const TextLayoutResult& layout)
{
    if (layout.bounds.width <= 0.0f || layout.bounds.height <= 0.0f) {
        return;
    }

    canvas.saveState();
    canvas.setClampBounds(layout.bounds.x, layout.bounds.y, layout.bounds.width, layout.bounds.height);
    for (const auto& line : layout.lines) {
        canvas.text(line.text, font, visage::Font::kTopLeft, line.x, line.y, std::max(layout.bounds.width, line.width), layout.lineHeight);
    }
    canvas.restoreState();
}

void drawLaidOutText(
    visage::Canvas& canvas,
    const std::string& text,
    const visage::Font& font,
    const TextLayoutRect& bounds,
    const TextLayoutOptions& options)
{
    drawTextLayout(canvas, font, layoutText(text, font, bounds, options));
}

} // namespace visiform::ui
