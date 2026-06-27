#include "ui/DesignerCanvas.h"

#include "model/BoxSizerLayout.h"
#include "model/LookAndFeelRegistry.h"
#include "model/WidgetItemUtils.h"
#include "model/WidgetRegistry.h"
#include "ui/TextLayout.h"
#include "ui/WidgetMetrics.h"
#include "ui/VisualStyleBaseline.h"
#include "ui/resources/ImageResourceCache.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iterator>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kPadding = 16.0f;
constexpr float kPreviewPadding = 18.0f;
constexpr float kTitleBarHeight = 28.0f;

struct PanelRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    [[nodiscard]] bool contains(float px, float py) const
    {
        return px >= x && py >= y && px <= x + width && py <= y + height;
    }

    [[nodiscard]] bool isValid() const
    {
        return width > 0.0f && height > 0.0f;
    }
};

void fillCircleApprox(visage::Canvas& canvas, float centerX, float centerY, float radius, int color);

void fillRoundedRect(visage::Canvas& canvas, float x, float y, float width, float height, float radius, int color)
{
    if (width <= 0.0f || height <= 0.0f) {
        return;
    }

    if (radius <= 1.0f) {
        canvas.setColor(color);
        canvas.fill(x, y, width, height);
        return;
    }

    const float r = std::min(radius, std::min(width, height) * 0.5f);
    canvas.setColor(color);
    // center rect
    canvas.fill(x + r, y, std::max(0.0f, width - r * 2.0f), height);
    // left and right rects
    canvas.fill(x, y + r, r, std::max(0.0f, height - r * 2.0f));
    canvas.fill(x + width - r, y + r, r, std::max(0.0f, height - r * 2.0f));
    // corner circles
    fillCircleApprox(canvas, x + r, y + r, r, color);
    fillCircleApprox(canvas, x + width - r, y + r, r, color);
    fillCircleApprox(canvas, x + r, y + height - r, r, color);
    fillCircleApprox(canvas, x + width - r, y + height - r, r, color);
}

void drawRoundedRectBorder(visage::Canvas& canvas, float x, float y, float width, float height, float radius, int borderColor, float thickness, int fillColor)
{
    if (thickness <= 0.0f) {
        return;
    }

    if (radius <= 1.0f) {
        // Fallback to rectangular border
        canvas.setColor(borderColor);
        canvas.fill(x, y, width, thickness);
        canvas.fill(x, y + height - thickness, width, thickness);
        canvas.fill(x, y, thickness, height);
        canvas.fill(x + width - thickness, y, thickness, height);
        return;
    }

    const float r = std::min(radius, std::min(width, height) * 0.5f);
    // draw edge bars
    canvas.setColor(borderColor);
    canvas.fill(x + r, y, std::max(0.0f, width - r * 2.0f), thickness); // top
    canvas.fill(x + r, y + height - thickness, std::max(0.0f, width - r * 2.0f), thickness); // bottom
    canvas.fill(x, y + r, thickness, std::max(0.0f, height - r * 2.0f)); // left
    canvas.fill(x + width - thickness, y + r, thickness, std::max(0.0f, height - r * 2.0f)); // right

    // draw corner caps as filled circles
    fillCircleApprox(canvas, x + r, y + r, r, borderColor);
    fillCircleApprox(canvas, x + width - r, y + r, r, borderColor);
    fillCircleApprox(canvas, x + r, y + height - r, r, borderColor);
    fillCircleApprox(canvas, x + width - r, y + height - r, r, borderColor);

    // carve inner area back out to create border thickness
    const float innerX = x + thickness;
    const float innerY = y + thickness;
    const float innerW = std::max(0.0f, width - thickness * 2.0f);
    const float innerH = std::max(0.0f, height - thickness * 2.0f);
    const float innerR = std::max(0.0f, r - thickness);
    fillRoundedRect(canvas, innerX, innerY, innerW, innerH, innerR, fillColor);
}

struct PreviewLayout {
    PanelRect preview{};
    PanelRect form{};
    float scale = 1.0f;
};

struct WidgetScreenInfo {
    const model::WidgetNode* widget = nullptr;
    PanelRect bounds{};
};

struct ResolvedWidgetStyle {
    int panelColor = 0xff1f242d;
    int fillColor = 0xff2b313d;
    int recessedColor = 0xff202630;
    int raisedColor = 0xff303744;
    int textColor = 0xffeef2f8;
    int secondaryTextColor = 0xffaeb8c8;
    int disabledTextColor = 0xff6c7788;
    int borderColor = 0xff97a3b7;
    int focusColor = 0xff2d7ff9;
    int accentColor = 0xff2d7ff9;
    int disabledColor = 0xff6c7788;
    int selectedColor = 0xff355382;
    int hoverColor = 0xff354052;
    int pressedColor = 0xff232a35;
    int checkedColor = 0xff355382;
    int highlightColor = 0xffc8d2e2;
    int shadowColor = 0xff11151c;
    float borderThickness = 1.0f;
    float cornerRadius = 0.0f;
    std::string fontFamily = "Default";
    float fontSize = 16.0f;
    int fontWeight = 400;
    bool italic = false;
    float controlPadding = 8.0f;
    float textPadding = 8.0f;
    std::string horizontalTextAlignment = "Default";
    std::string verticalTextAlignment = "Default";
    bool multiline = false;
    bool wordWrap = false;
    std::string overflowMode = "Clip";
};

struct GridColors {
    int minorLineColor = 0xff252a33;
    int majorLineColor = 0xff303744;
};

std::string imageWidgetPlaceholderText(const resources::ResolvedImageSource& resolvedSource,
    const resources::ImageResourceCache::CachedImageData* cachedImage = nullptr)
{
    if (resolvedSource.missingResource) {
        return "Missing image resource";
    }
    if (!resolvedSource.hasImage) {
        return "Image";
    }
    if (cachedImage == nullptr) {
        return resolvedSource.displayText.empty() ? std::string{ "Image" } : "Image: " + resolvedSource.displayText;
    }
    if (cachedImage->info.available && cachedImage->encodedBytes != nullptr && !cachedImage->encodedBytes->empty()) {
        return {};
    }
    if (cachedImage->info.error == "Image source file does not exist.") {
        return "Missing image file";
    }
    if (!cachedImage->info.error.empty()) {
        return "Image load failed";
    }
    return "Image";
}

PanelRect expandRect(const PanelRect& rect, float padding)
{
    return { rect.x - padding, rect.y - padding, rect.width + padding * 2.0f, rect.height + padding * 2.0f };
}

PanelRect selectionRectToScreenRect(const PreviewLayout& layout, const DesignerCanvas::SelectionRect& rect)
{
    return {
        layout.form.x + rect.x * layout.scale,
        layout.form.y + rect.y * layout.scale,
        rect.width * layout.scale,
        rect.height * layout.scale
    };
}

void drawBorder(visage::Canvas& canvas, const PanelRect& bounds, int color, float thickness = 1.0f)
{
    if (!bounds.isValid() || thickness <= 0.0f) {
        return;
    }

    canvas.setColor(color);
    canvas.fill(bounds.x, bounds.y, bounds.width, thickness);
    canvas.fill(bounds.x, bounds.y + bounds.height - thickness, bounds.width, thickness);
    canvas.fill(bounds.x, bounds.y, thickness, bounds.height);
    canvas.fill(bounds.x + bounds.width - thickness, bounds.y, thickness, bounds.height);
}

void drawRoundedBox(visage::Canvas& canvas, const PanelRect& bounds, const ResolvedWidgetStyle& style)
{
    if (!bounds.isValid()) {
        return;
    }

    const float radius = std::clamp(style.cornerRadius, 0.0f, std::min(bounds.width, bounds.height) * 0.5f);
    fillRoundedRect(canvas, bounds.x, bounds.y, bounds.width, bounds.height, radius, style.fillColor);
    drawRoundedRectBorder(canvas, bounds.x, bounds.y, bounds.width, bounds.height, radius, style.borderColor, style.borderThickness, style.fillColor);
}

void drawRoundedBox(visage::Canvas& canvas, const PanelRect& bounds, int fillColor, int borderColor, float borderThickness, float cornerRadius)
{
    if (!bounds.isValid()) {
        return;
    }

    const float radius = std::clamp(cornerRadius, 0.0f, std::min(bounds.width, bounds.height) * 0.5f);
    fillRoundedRect(canvas, bounds.x, bounds.y, bounds.width, bounds.height, radius, fillColor);
    drawRoundedRectBorder(canvas, bounds.x, bounds.y, bounds.width, bounds.height, radius, borderColor, borderThickness, fillColor);
}

enum class RoundedCorner {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

void fillRoundedCornerRing(visage::Canvas& canvas,
    float centerX,
    float centerY,
    float radius,
    float thickness,
    RoundedCorner corner,
    int color)
{
    const float outerRadius = std::max(0.0f, radius);
    const float innerRadius = std::max(0.0f, outerRadius - std::max(1.0f, thickness));
    if (outerRadius <= 0.0f) {
        return;
    }

    canvas.setColor(color);
    const int radiusPixels = std::max(1, static_cast<int>(std::ceil(outerRadius)));
    for (int offsetY = -radiusPixels; offsetY <= radiusPixels; ++offsetY) {
        const float dy = static_cast<float>(offsetY);
        const bool upper = dy <= 0.0f;
        if ((corner == RoundedCorner::TopLeft || corner == RoundedCorner::TopRight) != upper) {
            continue;
        }

        const float outerHalfWidth = std::sqrt(std::max(0.0f, outerRadius * outerRadius - dy * dy));
        const float innerHalfWidth = std::abs(dy) < innerRadius
            ? std::sqrt(std::max(0.0f, innerRadius * innerRadius - dy * dy))
            : 0.0f;
        const bool left = corner == RoundedCorner::TopLeft || corner == RoundedCorner::BottomLeft;
        const float startX = left ? centerX - outerHalfWidth : centerX + innerHalfWidth;
        const float endX = left ? centerX - innerHalfWidth : centerX + outerHalfWidth;
        canvas.fill(startX, centerY + dy, std::max(1.0f, endX - startX), 1.0f);
    }
}

void drawRoundedEdgeTreatment(visage::Canvas& canvas,
    const PanelRect& bounds,
    float cornerRadius,
    float thickness,
    int leadingColor,
    int trailingColor)
{
    if (!bounds.isValid() || thickness <= 0.0f || bounds.width < 2.0f || bounds.height < 2.0f) {
        return;
    }

    const float edge = std::max(1.0f, std::min(thickness, std::min(bounds.width, bounds.height) * 0.25f));
    const float radius = std::clamp(cornerRadius, 0.0f, std::min(bounds.width, bounds.height) * 0.5f);
    if (radius <= 1.0f) {
        canvas.setColor(leadingColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, edge);
        canvas.fill(bounds.x, bounds.y, edge, bounds.height);
        canvas.setColor(trailingColor);
        canvas.fill(bounds.x, bounds.y + bounds.height - edge, bounds.width, edge);
        canvas.fill(bounds.x + bounds.width - edge, bounds.y, edge, bounds.height);
        return;
    }

    canvas.setColor(leadingColor);
    canvas.fill(bounds.x + radius, bounds.y, std::max(0.0f, bounds.width - radius * 2.0f), edge);
    canvas.fill(bounds.x, bounds.y + radius, edge, std::max(0.0f, bounds.height - radius * 2.0f));
    fillRoundedCornerRing(canvas, bounds.x + radius, bounds.y + radius, radius, edge, RoundedCorner::TopLeft, leadingColor);
    fillRoundedCornerRing(canvas, bounds.x + radius, bounds.y + bounds.height - radius, radius, edge, RoundedCorner::BottomLeft, leadingColor);

    canvas.setColor(trailingColor);
    canvas.fill(bounds.x + radius, bounds.y + bounds.height - edge, std::max(0.0f, bounds.width - radius * 2.0f), edge);
    canvas.fill(bounds.x + bounds.width - edge, bounds.y + radius, edge, std::max(0.0f, bounds.height - radius * 2.0f));
    fillRoundedCornerRing(canvas, bounds.x + bounds.width - radius, bounds.y + radius, radius, edge, RoundedCorner::TopRight, trailingColor);
    fillRoundedCornerRing(canvas, bounds.x + bounds.width - radius, bounds.y + bounds.height - radius, radius, edge, RoundedCorner::BottomRight, trailingColor);
}

int resolvedStateFill(const ResolvedWidgetStyle& style, const visual_style::State& state, int fillColor)
{
    if (!state.enabled) {
        return visual_style::blend(fillColor, style.disabledColor, 0.55f);
    }

    switch (visual_style::resolveBaseState(state)) {
    case visual_style::BaseState::Pressed: return visual_style::blend(fillColor, 0xff000000, 0.20f);
    case visual_style::BaseState::Hovered: return visual_style::blend(fillColor, style.accentColor, 0.12f);
    case visual_style::BaseState::CheckedOrSelected: return visual_style::blend(fillColor, style.accentColor, 0.18f);
    case visual_style::BaseState::Disabled: return visual_style::blend(fillColor, style.disabledColor, 0.55f);
    case visual_style::BaseState::Normal: return fillColor;
    }

    return fillColor;
}

void drawRoundedBevel(visage::Canvas& canvas,
    const PanelRect& bounds,
    const ResolvedWidgetStyle& style,
    const visual_style::State& state,
    int fillColor)
{
    if (!bounds.isValid()) {
        return;
    }

    const bool pressed = visual_style::resolveBaseState(state) == visual_style::BaseState::Pressed;
    const int resolvedFill = resolvedStateFill(style, state, fillColor);
    const int resolvedBorder = state.enabled ? style.borderColor : visual_style::blend(style.borderColor, style.disabledColor, 0.65f);
    drawRoundedBox(canvas, bounds, resolvedFill, resolvedBorder, style.borderThickness, style.cornerRadius);

    if (bounds.width < 3.0f || bounds.height < 3.0f) {
        return;
    }

    const float inset = std::max(1.0f, std::min(style.borderThickness + 1.0f, std::min(bounds.width, bounds.height) * 0.25f));
    const int leading = pressed ? style.shadowColor : style.highlightColor;
    const int trailing = pressed ? style.highlightColor : style.shadowColor;
    const PanelRect edgeBounds{
        bounds.x + inset,
        bounds.y + inset,
        std::max(0.0f, bounds.width - inset * 2.0f),
        std::max(0.0f, bounds.height - inset * 2.0f)
    };
    drawRoundedEdgeTreatment(canvas, edgeBounds, std::max(0.0f, style.cornerRadius - inset), 1.0f, leading, trailing);
}

void drawRoundedRecessed(visage::Canvas& canvas,
    const PanelRect& bounds,
    const ResolvedWidgetStyle& style,
    const visual_style::State& state)
{
    if (!bounds.isValid()) {
        return;
    }

    int fillColor = style.recessedColor;
    if (state.readOnly) {
        fillColor = visual_style::blend(fillColor, style.disabledColor, 0.28f);
    }
    if (state.checkedOrSelected || state.active) {
        fillColor = visual_style::blend(fillColor, style.accentColor, 0.14f);
    }
    if (!state.enabled) {
        fillColor = visual_style::blend(fillColor, style.disabledColor, 0.55f);
    }

    drawRoundedBox(canvas,
        bounds,
        fillColor,
        state.focused && state.enabled ? style.focusColor : style.borderColor,
        style.borderThickness,
        style.cornerRadius);

    if (bounds.width < 3.0f || bounds.height < 3.0f) {
        return;
    }

    const float inset = std::max(1.0f, std::min(style.borderThickness + 1.0f, std::min(bounds.width, bounds.height) * 0.25f));
    const PanelRect edgeBounds{
        bounds.x + inset,
        bounds.y + inset,
        std::max(0.0f, bounds.width - inset * 2.0f),
        std::max(0.0f, bounds.height - inset * 2.0f)
    };
    drawRoundedEdgeTreatment(canvas, edgeBounds, std::max(0.0f, style.cornerRadius - inset), 1.0f, style.shadowColor, style.highlightColor);
}

int blendColor(int colorA, int colorB, float amount)
{
    const auto blendChannel = [amount](int first, int second) {
        return static_cast<int>(std::round(static_cast<float>(first) * (1.0f - amount) + static_cast<float>(second) * amount));
    };

    const int a = blendChannel((colorA >> 24) & 0xff, (colorB >> 24) & 0xff);
    const int r = blendChannel((colorA >> 16) & 0xff, (colorB >> 16) & 0xff);
    const int g = blendChannel((colorA >> 8) & 0xff, (colorB >> 8) & 0xff);
    const int b = blendChannel(colorA & 0xff, colorB & 0xff);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

GridColors computeGridColors(int backgroundColor)
{
    const float luminance = 0.2126f * static_cast<float>((backgroundColor >> 16) & 0xff)
        + 0.7152f * static_cast<float>((backgroundColor >> 8) & 0xff)
        + 0.0722f * static_cast<float>(backgroundColor & 0xff);
    const int contrastTarget = luminance < 128.0f ? 0xffffffff : 0xff000000;

    GridColors colors;
    colors.minorLineColor = blendColor(backgroundColor, contrastTarget, 0.08f);
    colors.majorLineColor = blendColor(backgroundColor, contrastTarget, 0.18f);
    if (colors.majorLineColor == colors.minorLineColor) {
        colors.majorLineColor = blendColor(backgroundColor, contrastTarget, 0.28f);
    }

    return colors;
}

void fillCircleApprox(visage::Canvas& canvas, float centerX, float centerY, float radius, int color)
{
    if (radius <= 0.0f) {
        return;
    }

    canvas.setColor(color);
    const int radiusPixels = std::max(1, static_cast<int>(std::ceil(radius)));
    for (int offsetY = -radiusPixels; offsetY <= radiusPixels; ++offsetY) {
        const float dy = static_cast<float>(offsetY);
        const float halfWidth = std::sqrt(std::max(0.0f, radius * radius - dy * dy));
        canvas.fill(centerX - halfWidth, centerY + dy, halfWidth * 2.0f + 1.0f, 1.0f);
    }
}

std::optional<std::string> resolveFontPath(const std::string& family, bool bold, bool italic)
{
    std::string normalizedFamily = family;
    std::transform(normalizedFamily.begin(), normalizedFamily.end(), normalizedFamily.begin(),
        [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

    std::vector<std::string> candidates;
    if (normalizedFamily == "arial") {
        if (bold && italic) {
            candidates.push_back("C:/Windows/Fonts/arialbi.ttf");
        }
        if (bold) {
            candidates.push_back("C:/Windows/Fonts/arialbd.ttf");
        }
        if (italic) {
            candidates.push_back("C:/Windows/Fonts/ariali.ttf");
        }
        candidates.push_back("C:/Windows/Fonts/arial.ttf");
    }
    else if (normalizedFamily == "tahoma") {
        if (bold) {
            candidates.push_back("C:/Windows/Fonts/tahomabd.ttf");
        }
        candidates.push_back("C:/Windows/Fonts/tahoma.ttf");
    }
    else {
        if (bold && italic) {
            candidates.push_back("C:/Windows/Fonts/segoeuiz.ttf");
        }
        if (bold) {
            candidates.push_back("C:/Windows/Fonts/segoeuib.ttf");
        }
        if (italic) {
            candidates.push_back("C:/Windows/Fonts/segoeuii.ttf");
        }
        candidates.push_back("C:/Windows/Fonts/segoeui.ttf");
        candidates.push_back("C:/Windows/Fonts/tahoma.ttf");
        candidates.push_back("C:/Windows/Fonts/arial.ttf");
    }

    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }

    return std::nullopt;
}

float resolvedFontSize(const model::WidgetNode& widget, const ResolvedWidgetStyle& style)
{
    return std::clamp(style.fontSize, 8.0f, 72.0f);
}

const visage::Font& resolvedWidgetFont(const model::WidgetNode& widget,
    const ResolvedWidgetStyle& style,
    const visage::Font& fallback,
    visage::Font& fontStorage)
{
    const float fontSize = resolvedFontSize(widget, style);
    const std::string fontFamily = style.fontFamily;
    const bool fontBold = style.fontWeight >= 600;
    const bool fontItalic = style.italic;

    const bool useFallbackFont = (fontFamily.empty() || fontFamily == "Default")
        && !fontBold
        && !fontItalic
        && std::abs(fontSize - defaultDesignerFontSize()) < 0.01f;
    if (useFallbackFont) {
        return fallback;
    }

    const auto fontPath = resolveFontPath(fontFamily, fontBold, fontItalic);
    if (!fontPath.has_value()) {
        return fallback;
    }

    fontStorage = visage::Font(fontSize, *fontPath);
    return fontStorage.packedFont() != nullptr ? fontStorage : fallback;
}

std::string widgetLabel(const model::WidgetNode& widget)
{
    if (!widget.name.empty()) {
        return widget.name;
    }

    if (!widget.id.empty()) {
        return widget.id;
    }

    return widget.typeName();
}

std::optional<float> tryParseFloatText(const std::string& text)
{
    if (text.empty() || text == "<unset>") {
        return std::nullopt;
    }

    std::istringstream stream(text);
    float value = 0.0f;
    char trailing = '\0';
    if (!(stream >> value)) {
        return std::nullopt;
    }
    if (stream >> trailing) {
        return std::nullopt;
    }
    return value;
}

float getNumericProperty(const model::WidgetNode& widget, const std::string& key, float defaultValue)
{
    const auto* property = widget.getProperty(key);
    if (property == nullptr) {
        return defaultValue;
    }

    if (property->isFloat()) {
        return property->asFloat(defaultValue);
    }
    if (property->isInt()) {
        return static_cast<float>(property->asInt(static_cast<int>(defaultValue)));
    }
    if (property->isString()) {
        if (const auto parsedValue = tryParseFloatText(property->asString({}))) {
            return *parsedValue;
        }
    }

    return defaultValue;
}

std::string getStringProperty(const model::WidgetNode& widget, const std::string& key, const std::string& defaultValue)
{
    return widget.getStringProperty(key, defaultValue);
}

std::string getDisplayTextOrFallback(const model::WidgetNode& widget, const std::string& key, const std::string& fallback)
{
    const std::string value = getStringProperty(widget, key, {});
    return value.empty() ? fallback : value;
}

auto textJustification(const ResolvedWidgetStyle& style, decltype(visage::Font::kTopLeft) fallback)
    -> decltype(visage::Font::kTopLeft)
{
    if (style.horizontalTextAlignment == "Center") {
        return visage::Font::kCenter;
    }
    if (style.horizontalTextAlignment == "Right") {
        return visage::Font::kTopRight;
    }
    if (style.horizontalTextAlignment == "Left") {
        return visage::Font::kTopLeft;
    }
    return fallback;
}

float alignedTextTop(float y, float height, float fontSize, const ResolvedWidgetStyle& style)
{
    if (style.verticalTextAlignment == "Top") {
        return y;
    }
    if (style.verticalTextAlignment == "Bottom") {
        return y + std::max(0.0f, height - fontSize * 1.35f);
    }
    return centeredTextTop(y, height, fontSize);
}

TextLayoutOptions textLayoutOptions(
    const ResolvedWidgetStyle& style,
    bool forceSingleLine = false,
    std::string defaultHorizontalAlignment = "Left",
    std::string defaultVerticalAlignment = "Top")
{
    return {
        forceSingleLine ? false : style.multiline,
        forceSingleLine ? false : style.wordWrap,
        textOverflowModeFromString(style.overflowMode),
        style.horizontalTextAlignment == "Default" ? std::move(defaultHorizontalAlignment) : style.horizontalTextAlignment,
        style.verticalTextAlignment == "Default" ? std::move(defaultVerticalAlignment) : style.verticalTextAlignment
    };
}

void drawWidgetText(visage::Canvas& canvas,
    const std::string& text,
    const visage::Font& font,
    const PanelRect& bounds,
    const ResolvedWidgetStyle& style,
    bool forceSingleLine = false,
    std::string defaultHorizontalAlignment = "Left",
    std::string defaultVerticalAlignment = "Top")
{
    drawLaidOutText(
        canvas,
        text,
        font,
        { bounds.x, bounds.y, std::max(0.0f, bounds.width), std::max(0.0f, bounds.height) },
        textLayoutOptions(style, forceSingleLine, std::move(defaultHorizontalAlignment), std::move(defaultVerticalAlignment)));
}

bool shouldDrawEditorLabel(bool showEditorDecorations)
{
    return showEditorDecorations;
}

std::string runtimeTextOrEditorLabel(
    const model::WidgetNode& widget,
    const std::string& propertyKey,
    bool showEditorDecorations)
{
    const std::string runtimeText = getStringProperty(widget, propertyKey, {});
    if (!runtimeText.empty()) {
        return runtimeText;
    }

    return shouldDrawEditorLabel(showEditorDecorations) ? widgetLabel(widget) : std::string{};
}

std::string editorOnlyLabel(const model::WidgetNode& widget, bool showEditorDecorations)
{
    return shouldDrawEditorLabel(showEditorDecorations) ? widgetLabel(widget) : std::string{};
}

bool getBoolProperty(const model::WidgetNode& widget, const std::string& key, bool defaultValue)
{
    return widget.getBoolProperty(key, defaultValue);
}

std::vector<std::string> splitCommaSeparatedValues(const std::string& text)
{
    std::vector<std::string> values;
    std::istringstream stream(text);
    std::string item;
    while (std::getline(stream, item, ',')) {
        const auto first = std::find_if_not(item.begin(), item.end(), [](unsigned char character) {
            return std::isspace(character) != 0;
        });
        const auto last = std::find_if_not(item.rbegin(), item.rend(), [](unsigned char character) {
            return std::isspace(character) != 0;
        }).base();
        if (first < last) {
            values.emplace_back(first, last);
        }
    }

    if (values.empty()) {
        values.push_back("OK");
    }

    return values;
}

std::vector<std::string> tabLabels(const model::WidgetNode& widget)
{
    if (widget.type == model::WidgetType::TabControl && widget.tabPageCount() > 0) {
        std::vector<std::string> labels;
        labels.reserve(widget.tabPageCount());
        for (const auto& child : widget.children) {
            if (child.type == model::WidgetType::TabPage) {
                labels.push_back(child.tabTitle());
            }
        }
        if (!labels.empty()) {
            return labels;
        }
    }

    std::vector<std::string> labels;
    std::istringstream stream(getStringProperty(widget, "tabs", "Tab 1,Tab 2"));
    std::string item;
    while (std::getline(stream, item, ',')) {
        const auto first = std::find_if_not(item.begin(), item.end(), [](unsigned char character) {
            return std::isspace(character) != 0;
        });
        const auto last = std::find_if_not(item.rbegin(), item.rend(), [](unsigned char character) {
            return std::isspace(character) != 0;
        }).base();
        if (first < last) {
            labels.emplace_back(first, last);
        }
    }

    if (labels.empty()) {
        labels.push_back("Tab 1");
    }

    return labels;
}

int selectedTabIndex(const model::WidgetNode& widget)
{
    if (widget.type == model::WidgetType::TabControl && widget.tabPageCount() > 0) {
        return widget.selectedTabIndex();
    }

    const std::vector<std::string> labels = tabLabels(widget);
    return std::clamp(widget.getIntProperty("selectedTabIndex", widget.getIntProperty("selectedTab", 0)), 0, std::max(0, static_cast<int>(labels.size()) - 1));
}

bool isChildVisibleInParent(const model::WidgetNode& parent, const model::WidgetNode& child)
{
    if (parent.type != model::WidgetType::TabControl) {
        return true;
    }

    if (parent.tabPageCount() > 0) {
        const auto* selectedPage = parent.tabPageAt(parent.selectedTabIndex());
        return child.type == model::WidgetType::TabPage && selectedPage != nullptr && child.id == selectedPage->id;
    }

    return child.getIntProperty("tabIndex", 0) == selectedTabIndex(parent);
}

float normalizedSliderValue(const model::WidgetNode& widget, float value)
{
    const float minimum = getNumericProperty(widget, "min", 0.0f);
    const float maximum = getNumericProperty(widget, "max", 100.0f);
    if (maximum <= minimum) {
        return 0.5f;
    }

    const float clampedValue = std::clamp(value, minimum, maximum);
    return std::clamp((clampedValue - minimum) / (maximum - minimum), 0.0f, 1.0f);
}

float normalizedSliderValue(const model::WidgetNode& widget)
{
    return normalizedSliderValue(widget, getNumericProperty(widget, "value", 50.0f));
}

float normalizedRangeValue(const model::WidgetNode& widget, float value)
{
    const float minimum = getNumericProperty(widget, "min", 0.0f);
    const float maximum = getNumericProperty(widget, "max", 100.0f);
    if (maximum <= minimum) {
        return 0.0f;
    }

    return std::clamp((value - minimum) / (maximum - minimum), 0.0f, 1.0f);
}

float normalizedRangeValue(const model::WidgetNode& widget, const std::string& valueKey = "value")
{
    return normalizedRangeValue(widget, getNumericProperty(widget, valueKey, getNumericProperty(widget, "min", 0.0f)));
}

std::string progressBarDisplayText(const model::WidgetNode& widget)
{
    if (!getBoolProperty(widget, "showText", true)) {
        return {};
    }

    const std::string explicitText = getDisplayTextOrFallback(widget, "text", {});
    if (!explicitText.empty()) {
        return explicitText;
    }

    const float minimum = getNumericProperty(widget, "min", 0.0f);
    const float maximum = std::max(minimum, getNumericProperty(widget, "max", 100.0f));
    const float safeMaximum = maximum <= minimum ? minimum + 1.0f : maximum;
    const float value = std::clamp(getNumericProperty(widget, "value", minimum), minimum, safeMaximum);
    const float normalized = safeMaximum <= minimum ? 0.0f : std::clamp((value - minimum) / (safeMaximum - minimum), 0.0f, 1.0f);
    return std::to_string(static_cast<int>(std::round(normalized * 100.0f))) + "%";
}

int parseColorOrDefault(const std::string& value, int defaultColor)
{
    if (value.empty() || value.front() != '#') {
        return defaultColor;
    }

    try {
        const std::string digits = value.substr(1);
        const std::uint32_t parsed = static_cast<std::uint32_t>(std::stoul(digits, nullptr, 16));
        if (digits.size() == 6) {
            return static_cast<int>(0xff000000u | parsed);
        }
        if (digits.size() == 8) {
            return static_cast<int>(parsed);
        }
    }
    catch (...) {
    }

    return defaultColor;
}

ResolvedWidgetStyle resolveWidgetStyle(const model::ProjectDocument& document, const model::WidgetNode& widget)
{
    const model::ResolvedLookAndFeelStyle resolved =
        model::LookAndFeelRegistry::instance().resolve(document, widget);

    ResolvedWidgetStyle style;
    style.panelColor = parseColorOrDefault(resolved.applicationSurfaceColor, style.panelColor);
    style.fillColor = parseColorOrDefault(resolved.controlSurfaceColor, style.fillColor);
    style.recessedColor = parseColorOrDefault(resolved.recessedSurfaceColor, style.recessedColor);
    style.raisedColor = parseColorOrDefault(resolved.raisedSurfaceColor, style.raisedColor);
    style.textColor = parseColorOrDefault(resolved.primaryTextColor, style.textColor);
    style.secondaryTextColor = parseColorOrDefault(resolved.secondaryTextColor, style.secondaryTextColor);
    style.disabledTextColor = parseColorOrDefault(resolved.disabledTextColor, style.disabledTextColor);
    style.borderColor = parseColorOrDefault(resolved.borderColor, style.borderColor);
    style.focusColor = parseColorOrDefault(resolved.focusOutlineColor, style.focusColor);
    style.accentColor = parseColorOrDefault(resolved.accentColor, style.accentColor);
    style.disabledColor = parseColorOrDefault(resolved.disabledSurfaceColor, style.disabledColor);
    style.selectedColor = parseColorOrDefault(resolved.selectedStateColor, style.selectedColor);
    style.hoverColor = parseColorOrDefault(resolved.hoverStateColor, style.hoverColor);
    style.pressedColor = parseColorOrDefault(resolved.pressedStateColor, style.pressedColor);
    style.checkedColor = parseColorOrDefault(resolved.checkedStateColor, style.checkedColor);
    style.highlightColor = parseColorOrDefault(resolved.highlightEdgeColor, style.highlightColor);
    style.shadowColor = parseColorOrDefault(resolved.shadowEdgeColor, style.shadowColor);
    style.borderThickness = resolved.borderThickness;
    style.cornerRadius = resolved.cornerRadius;
    style.fontFamily = resolved.fontFamily;
    style.fontSize = resolved.fontSize;
    style.fontWeight = resolved.fontWeight;
    style.italic = resolved.italic;
    style.controlPadding = resolved.controlPadding;
    style.textPadding = resolved.textPadding;
    style.horizontalTextAlignment = resolved.horizontalTextAlignment;
    style.verticalTextAlignment = resolved.verticalTextAlignment;
    style.multiline = resolved.multiline;
    style.wordWrap = resolved.wordWrap;
    style.overflowMode = resolved.overflowMode;

    return style;
}

model::WidgetAppearanceState appearanceStateFor(const visual_style::State& state)
{
    switch (visual_style::resolveBaseState(state)) {
    case visual_style::BaseState::Hovered:
        return model::WidgetAppearanceState::Hover;
    case visual_style::BaseState::CheckedOrSelected:
        return model::WidgetAppearanceState::CheckedOrSelected;
    case visual_style::BaseState::Pressed:
        return model::WidgetAppearanceState::Pressed;
    case visual_style::BaseState::Disabled:
        return model::WidgetAppearanceState::Disabled;
    case visual_style::BaseState::Normal:
        return model::WidgetAppearanceState::Normal;
    }
    return model::WidgetAppearanceState::Normal;
}

visual_style::State visualStateForAppearancePreview(model::WidgetAppearanceState state)
{
    visual_style::State visualState;
    visualState.enabled = true;
    switch (state) {
    case model::WidgetAppearanceState::Hover:
        visualState.hovered = true;
        break;
    case model::WidgetAppearanceState::Pressed:
        visualState.hovered = true;
        visualState.pressed = true;
        break;
    case model::WidgetAppearanceState::Focused:
        visualState.focused = true;
        break;
    case model::WidgetAppearanceState::CheckedOrSelected:
        visualState.checkedOrSelected = true;
        break;
    case model::WidgetAppearanceState::Disabled:
        visualState.enabled = false;
        break;
    case model::WidgetAppearanceState::Normal:
        break;
    }
    return visualState;
}

ResolvedWidgetStyle resolveWidgetStyle(
    const model::ProjectDocument& document,
    const model::WidgetNode& widget,
    const visual_style::State& state)
{
    const model::ResolvedLookAndFeelStyle resolved =
        model::LookAndFeelRegistry::instance().resolve(
            document,
            widget,
            appearanceStateFor(state),
            state.focused && state.enabled);

    ResolvedWidgetStyle style;
    style.panelColor = parseColorOrDefault(resolved.applicationSurfaceColor, style.panelColor);
    style.fillColor = parseColorOrDefault(resolved.controlSurfaceColor, style.fillColor);
    style.recessedColor = parseColorOrDefault(resolved.recessedSurfaceColor, style.recessedColor);
    style.raisedColor = parseColorOrDefault(resolved.raisedSurfaceColor, style.raisedColor);
    style.textColor = parseColorOrDefault(resolved.primaryTextColor, style.textColor);
    style.secondaryTextColor = parseColorOrDefault(resolved.secondaryTextColor, style.secondaryTextColor);
    style.disabledTextColor = parseColorOrDefault(resolved.disabledTextColor, style.disabledTextColor);
    style.borderColor = parseColorOrDefault(resolved.borderColor, style.borderColor);
    style.focusColor = parseColorOrDefault(resolved.focusOutlineColor, style.focusColor);
    style.accentColor = parseColorOrDefault(resolved.accentColor, style.accentColor);
    style.disabledColor = parseColorOrDefault(resolved.disabledSurfaceColor, style.disabledColor);
    style.selectedColor = parseColorOrDefault(resolved.selectedStateColor, style.selectedColor);
    style.hoverColor = parseColorOrDefault(resolved.hoverStateColor, style.hoverColor);
    style.pressedColor = parseColorOrDefault(resolved.pressedStateColor, style.pressedColor);
    style.checkedColor = parseColorOrDefault(resolved.checkedStateColor, style.checkedColor);
    style.highlightColor = parseColorOrDefault(resolved.highlightEdgeColor, style.highlightColor);
    style.shadowColor = parseColorOrDefault(resolved.shadowEdgeColor, style.shadowColor);
    style.borderThickness = resolved.borderThickness;
    style.cornerRadius = resolved.cornerRadius;
    style.fontFamily = resolved.fontFamily;
    style.fontSize = resolved.fontSize;
    style.fontWeight = resolved.fontWeight;
    style.italic = resolved.italic;
    style.controlPadding = resolved.controlPadding;
    style.textPadding = resolved.textPadding;
    style.horizontalTextAlignment = resolved.horizontalTextAlignment;
    style.verticalTextAlignment = resolved.verticalTextAlignment;
    style.multiline = resolved.multiline;
    style.wordWrap = resolved.wordWrap;
    style.overflowMode = resolved.overflowMode;
    return style;
}

PreviewLayout calculatePreviewLayout(float x,
    float y,
    float width,
    float height,
    const model::ProjectDocument& document,
    bool previewMode,
    float zoom,
    float panX,
    float panY)
{
    PreviewLayout layout;
    layout.preview = {
        x + kPadding,
        y + (previewMode ? kPadding : kHeaderHeight + 12.0f),
        std::max(0.0f, width - kPadding * 2.0f),
        std::max(0.0f, height - (previewMode ? kPadding * 2.0f : kHeaderHeight + 24.0f))
    };

    if (!layout.preview.isValid() || !document.root.bounds.isValid()) {
        return layout;
    }

    if (layout.preview.width <= 0.0f || layout.preview.height <= 0.0f || zoom <= 0.0f) {
        return layout;
    }

    layout.scale = zoom;
    const float formWidth = document.root.bounds.width * layout.scale;
    const float formHeight = document.root.bounds.height * layout.scale;
    layout.form = {
        layout.preview.x + (layout.preview.width - formWidth) * 0.5f + panX,
        layout.preview.y + (layout.preview.height - formHeight) * 0.5f + panY,
        formWidth,
        formHeight
    };
    return layout;
}

void drawSelectionOutline(visage::Canvas& canvas, const PanelRect& bounds)
{
    drawBorder(canvas, { bounds.x - 2.0f, bounds.y - 2.0f, bounds.width + 4.0f, bounds.height + 4.0f }, 0xff2d7ff9, 1.5f);
}

void drawSecondarySelectionOutline(visage::Canvas& canvas, const PanelRect& bounds)
{
    drawBorder(canvas, { bounds.x - 1.0f, bounds.y - 1.0f, bounds.width + 2.0f, bounds.height + 2.0f }, 0xffd9473f, 1.25f);
}

constexpr float kResizeHandleVisualSize = 10.0f;
constexpr float kResizeHandleHitSize = 16.0f;
constexpr float kBottomRightHitInward = 18.0f;
constexpr float kBottomRightHitOutward = 4.0f;
constexpr float kGripArcMinimumExtent = 18.0f;
constexpr float kSecondGripArcMinimumExtent = 28.0f;
constexpr std::array<float, 2> kGripArcRadii = { 8.0f, 13.0f };

struct ResizeHandleGeometry {
    PanelRect visualBounds{};
    PanelRect hitBounds{};
};

visual_style::Palette baselinePalette(const ResolvedWidgetStyle& style, int fillColor)
{
    visual_style::Palette palette =
        visual_style::makePalette(fillColor, style.borderColor, style.textColor, style.accentColor, style.disabledColor);
    palette.secondaryText = style.secondaryTextColor;
    palette.disabledText = style.disabledTextColor;
    palette.focus = style.focusColor;
    palette.selectedFill = style.selectedColor;
    palette.checkedFill = style.checkedColor;
    palette.highlight = style.highlightColor;
    palette.shadow = style.shadowColor;
    palette.hoverFill = style.hoverColor;
    palette.pressedFill = style.pressedColor;
    palette.recessedFill = style.recessedColor;
    palette.borderThickness = style.borderThickness;
    return palette;
}

visual_style::Rect baselineRect(const PanelRect& bounds)
{
    return { bounds.x, bounds.y, bounds.width, bounds.height };
}

bool widgetIsEnabled(const model::WidgetNode& widget)
{
    return widget.getBoolProperty("enabled", true) && !widget.getBoolProperty("disabled", false);
}

ResizeHandleGeometry resizeHandleGeometry(const PanelRect& bounds, DesignerCanvas::HitRegion region)
{
    const float halfVisualHandle = kResizeHandleVisualSize * 0.5f;
    const float halfHitHandle = kResizeHandleHitSize * 0.5f;
    ResizeHandleGeometry geometry;

    switch (region) {
    case DesignerCanvas::HitRegion::TopLeftHandle:
        geometry.visualBounds = { bounds.x - halfVisualHandle, bounds.y - halfVisualHandle, kResizeHandleVisualSize, kResizeHandleVisualSize };
        geometry.hitBounds = { bounds.x - halfHitHandle, bounds.y - halfHitHandle, kResizeHandleHitSize, kResizeHandleHitSize };
        break;
    case DesignerCanvas::HitRegion::TopRightHandle:
        geometry.visualBounds = { bounds.x + bounds.width - halfVisualHandle, bounds.y - halfVisualHandle, kResizeHandleVisualSize, kResizeHandleVisualSize };
        geometry.hitBounds = { bounds.x + bounds.width - halfHitHandle, bounds.y - halfHitHandle, kResizeHandleHitSize, kResizeHandleHitSize };
        break;
    case DesignerCanvas::HitRegion::BottomLeftHandle:
        geometry.visualBounds = { bounds.x - halfVisualHandle, bounds.y + bounds.height - halfVisualHandle, kResizeHandleVisualSize, kResizeHandleVisualSize };
        geometry.hitBounds = { bounds.x - halfHitHandle, bounds.y + bounds.height - halfHitHandle, kResizeHandleHitSize, kResizeHandleHitSize };
        break;
    case DesignerCanvas::HitRegion::BottomRightHandle:
        geometry.visualBounds = { bounds.x + bounds.width - halfVisualHandle, bounds.y + bounds.height - halfVisualHandle, kResizeHandleVisualSize, kResizeHandleVisualSize };
        geometry.hitBounds = {
            bounds.x + bounds.width - kBottomRightHitInward,
            bounds.y + bounds.height - kBottomRightHitInward,
            kBottomRightHitInward + kBottomRightHitOutward,
            kBottomRightHitInward + kBottomRightHitOutward
        };
        break;
    case DesignerCanvas::HitRegion::None:
    case DesignerCanvas::HitRegion::Body:
        break;
    }

    return geometry;
}

DesignerCanvas::HitRegion hitHandle(const PanelRect& bounds, float x, float y)
{
    constexpr std::array<DesignerCanvas::HitRegion, 4> kHandles = {
        DesignerCanvas::HitRegion::TopLeftHandle,
        DesignerCanvas::HitRegion::TopRightHandle,
        DesignerCanvas::HitRegion::BottomLeftHandle,
        DesignerCanvas::HitRegion::BottomRightHandle
    };

    DesignerCanvas::HitRegion nearestHandle = DesignerCanvas::HitRegion::None;
    float nearestDistanceSquared = std::numeric_limits<float>::max();

    for (DesignerCanvas::HitRegion handle : kHandles) {
        const ResizeHandleGeometry geometry = resizeHandleGeometry(bounds, handle);
        if (!geometry.hitBounds.contains(x, y)) {
            continue;
        }

        const float centerX = geometry.visualBounds.x + geometry.visualBounds.width * 0.5f;
        const float centerY = geometry.visualBounds.y + geometry.visualBounds.height * 0.5f;
        const float distanceX = x - centerX;
        const float distanceY = y - centerY;
        const float distanceSquared = distanceX * distanceX + distanceY * distanceY;
        if (distanceSquared < nearestDistanceSquared) {
            nearestDistanceSquared = distanceSquared;
            nearestHandle = handle;
        }
    }

    return nearestHandle;
}

bool showsDirectResizeAffordance(const model::WidgetNode& widget)
{
    return widget.type != model::WidgetType::FormWindow
        && widget.type != model::WidgetType::TabPage
        && widget.dockMode() == model::DockMode::None;
}

void drawBottomRightGripArcs(visage::Canvas& canvas, const PanelRect& bounds)
{
    const float availableExtent = std::min(bounds.width, bounds.height);
    if (availableExtent < kGripArcMinimumExtent) {
        return;
    }

    const int arcCount = availableExtent >= kSecondGripArcMinimumExtent ? 2 : 1;
    constexpr float kArcInset = 2.0f;
    const float right = bounds.x + bounds.width - kArcInset;
    const float bottom = bounds.y + bounds.height - kArcInset;

    canvas.setColor(0xff2d7ff9);
    for (int index = 0; index < arcCount; ++index) {
        const float radius = kGripArcRadii[static_cast<std::size_t>(index)];
        canvas.quadratic(right, bottom - radius, right - radius, bottom - radius, right - radius, bottom, 1.5f);
    }
}

bool hasAssignedCallbacks(const model::WidgetNode& widget)
{
    const auto* definition = model::WidgetRegistry::instance().find(widget.type);
    if (definition == nullptr) {
        return false;
    }

    return std::any_of(definition->events.begin(), definition->events.end(), [&widget](const model::WidgetEventDefinition& event) {
        return !widget.getStringProperty(event.key, {}).empty();
    });
}

void drawSelectionHandles(visage::Canvas& canvas, const PanelRect& bounds, bool callbackIndicator)
{
    constexpr std::array<DesignerCanvas::HitRegion, 4> kHandles = {
        DesignerCanvas::HitRegion::TopLeftHandle,
        DesignerCanvas::HitRegion::TopRightHandle,
        DesignerCanvas::HitRegion::BottomLeftHandle,
        DesignerCanvas::HitRegion::BottomRightHandle
    };

    drawBottomRightGripArcs(canvas, bounds);

    for (DesignerCanvas::HitRegion handle : kHandles) {
        const PanelRect handleBounds = resizeHandleGeometry(bounds, handle).visualBounds;
        canvas.setColor(0xffffffff);
        canvas.fill(handleBounds.x, handleBounds.y, handleBounds.width, handleBounds.height);
        drawBorder(canvas, handleBounds, 0xff2d7ff9);
        if (handle == DesignerCanvas::HitRegion::TopRightHandle) {
            canvas.setColor(callbackIndicator ? 0xfff4c84a : 0xff8fb8f4);
            const float dotSize = callbackIndicator ? 4.0f : 3.0f;
            canvas.fill(handleBounds.x + handleBounds.width - dotSize - 2.0f,
                handleBounds.y + 2.0f,
                dotSize,
                dotSize);
        }
    }
}

std::optional<std::string> hitTestWidgetScreenId(const model::WidgetNode& widget,
    float formScreenX,
    float formScreenY,
    float parentLocalX,
    float parentLocalY,
    float scale,
    float x,
    float y,
    float smallWidgetHitPadding)
{
    const float widgetLocalX = parentLocalX + widget.bounds.x;
    const float widgetLocalY = parentLocalY + widget.bounds.y;
    const PanelRect bounds{
        formScreenX + widgetLocalX * scale,
        formScreenY + widgetLocalY * scale,
        std::max(1.0f, widget.bounds.width * scale),
        std::max(1.0f, widget.bounds.height * scale)
    };

    // Z-order convention:
    // - children[0] is backmost
    // - children.back() is frontmost
    // Hit testing walks children from front to back so the topmost overlap wins.
    for (auto iterator = widget.children.rbegin(); iterator != widget.children.rend(); ++iterator) {
        if (!isChildVisibleInParent(widget, *iterator)) {
            continue;
        }
        if (auto match = hitTestWidgetScreenId(*iterator, formScreenX, formScreenY, widgetLocalX, widgetLocalY,
                scale, x, y, smallWidgetHitPadding)) {
            if (widget.type == model::WidgetType::TabControl
                && iterator->type == model::WidgetType::TabPage
                && *match == iterator->id) {
                continue;
            }
            return match;
        }
    }

    PanelRect hitBounds = bounds;
    if (bounds.width < 14.0f || bounds.height < 14.0f) {
        hitBounds = expandRect(bounds, smallWidgetHitPadding);
    }

    if (hitBounds.contains(x, y)) {
        return widget.id;
    }

    return std::nullopt;
}

void drawGrid(visage::Canvas& canvas, const PanelRect& bounds, float scale, bool showMinorGrid, int gridSize, int majorGridSize, int backgroundColor)
{
    const float scaledGridSize = gridSize * scale;
    if (scaledGridSize < 4.0f) {
        return;
    }

    const float scaledMajorGridSize = majorGridSize * scale;

    const float contentTop = bounds.y + std::min(kTitleBarHeight, bounds.height);
    const float contentHeight = bounds.height - std::min(kTitleBarHeight, bounds.height);
    if (contentHeight <= 0.0f) {
        return;
    }

    const GridColors gridColors = computeGridColors(backgroundColor);

    if (showMinorGrid) {
        canvas.setColor(gridColors.minorLineColor);
        for (float x = bounds.x + scaledGridSize; x < bounds.x + bounds.width; x += scaledGridSize) {
            canvas.fill(x, contentTop, 1.0f, contentHeight);
        }
        for (float y = contentTop + scaledGridSize; y < bounds.y + bounds.height; y += scaledGridSize) {
            canvas.fill(bounds.x, y, bounds.width, 1.0f);
        }
    }

    if (scaledMajorGridSize >= scaledGridSize + 1.0f) {
        canvas.setColor(gridColors.majorLineColor);
        for (float x = bounds.x + scaledMajorGridSize; x < bounds.x + bounds.width; x += scaledMajorGridSize) {
            canvas.fill(x, contentTop, 1.0f, contentHeight);
        }
        for (float y = contentTop + scaledMajorGridSize; y < bounds.y + bounds.height; y += scaledMajorGridSize) {
            canvas.fill(bounds.x, y, bounds.width, 1.0f);
        }
    }
}

std::optional<WidgetScreenInfo> findWidgetScreenInfo(const model::WidgetNode& widget,
    const std::string& widgetId,
    float formScreenX,
    float formScreenY,
    float parentLocalX,
    float parentLocalY,
    float scale)
{
    const float widgetLocalX = parentLocalX + widget.bounds.x;
    const float widgetLocalY = parentLocalY + widget.bounds.y;
    const PanelRect bounds{
        formScreenX + widgetLocalX * scale,
        formScreenY + widgetLocalY * scale,
        std::max(1.0f, widget.bounds.width * scale),
        std::max(1.0f, widget.bounds.height * scale)
    };

    if (widget.id == widgetId) {
        return WidgetScreenInfo{ &widget, bounds };
    }

    for (const auto& child : widget.children) {
        if (!isChildVisibleInParent(widget, child)) {
            continue;
        }
        if (auto match = findWidgetScreenInfo(child, widgetId, formScreenX, formScreenY, widgetLocalX, widgetLocalY, scale)) {
            return match;
        }
    }

    return std::nullopt;
}

void drawWidget(visage::Canvas& canvas,
    const visage::Font& font,
    bool drawText,
    const model::ProjectDocument& document,
    const DesignerCanvas& designerCanvas,
    resources::ImageResourceCache* imageCache,
    bool simplifySelectedImages,
    const model::WidgetNode& widget,
    float formScreenX,
    float formScreenY,
    float parentLocalX,
    float parentLocalY,
    float scale,
    const std::string& selectedWidgetId,
    bool showGrid,
    bool showMinorGrid,
    int gridSize,
    int majorGridSize,
    bool showEditorDecorations,
    const std::optional<model::WidgetAppearanceState>& appearancePreviewState)
{
    const float widgetLocalX = parentLocalX + widget.bounds.x;
    const float widgetLocalY = parentLocalY + widget.bounds.y;
    const PanelRect bounds{
        formScreenX + widgetLocalX * scale,
        formScreenY + widgetLocalY * scale,
        std::max(1.0f, widget.bounds.width * scale),
        std::max(1.0f, widget.bounds.height * scale)
    };
    const bool previewSelectedAppearance = designerCanvas.mode() == DesignerCanvas::Mode::Design
        && widget.id == selectedWidgetId
        && appearancePreviewState.has_value()
        && model::LookAndFeelRegistry::supportsWidgetState(widget.type, *appearancePreviewState);
    const visual_style::State visualState = previewSelectedAppearance
        ? visualStateForAppearancePreview(*appearancePreviewState)
        : designerCanvas.resolvedVisualState(widget);
    const ResolvedWidgetStyle style = designerCanvas.mode() == DesignerCanvas::Mode::Preview
        || previewSelectedAppearance
        ? resolveWidgetStyle(document, widget, visualState)
        : resolveWidgetStyle(document, widget);
    visage::Font widgetFontStorage{};
    const visage::Font& widgetFont = resolvedWidgetFont(widget, style, font, widgetFontStorage);
    const float fontSize = resolvedFontSize(widget, style);

    switch (widget.type) {
    case model::WidgetType::FormWindow: {
        drawRoundedBox(canvas, bounds, style);
        if (showGrid) {
            drawGrid(canvas, bounds, scale, showMinorGrid, gridSize, majorGridSize, style.fillColor);
        }
        canvas.setColor(style.panelColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, std::min(kTitleBarHeight, bounds.height));

        if (drawText) {
            const std::string title = runtimeTextOrEditorLabel(widget, "title", showEditorDecorations);
            if (!title.empty()) {
                canvas.setColor(style.textColor);
                canvas.text(title, widgetFont, visage::Font::kTopLeft,
                    bounds.x + 10.0f, bounds.y + 4.0f, std::max(0.0f, bounds.width - 20.0f), 22.0f);
            }
        }
        break;
    }
    case model::WidgetType::MenuBar: {
        const bool enabled = widgetIsEnabled(widget);
        const auto palette = baselinePalette(style, blendColor(style.panelColor, style.fillColor, 0.35f));
        const auto items = model::splitItems(getStringProperty(widget, "items", {}));
        const std::string selectedIndexKey = std::string(model::selectedItemIndexPropertyKey(widget.type));
        const int selectedIndex = model::sanitizeSelectedIndex(items,
            widget.getIntProperty(selectedIndexKey, items.empty() ? -1 : 0));
        const float itemHeight = std::max(0.0f, bounds.height - 8.0f);
        float itemLeft = bounds.x + 6.0f;
        visual_style::drawBevel(canvas, baselineRect(bounds), palette, visualState);
        for (std::size_t index = 0; index < items.size() && itemLeft < bounds.x + bounds.width - 6.0f; ++index) {
            const std::string& item = items[index];
            const float idealWidth = std::max(56.0f, estimateDesignerTextWidth(item, fontSize) + 12.0f);
            const float itemWidth = std::min(idealWidth, std::max(0.0f, bounds.x + bounds.width - 6.0f - itemLeft));
            const bool selected = static_cast<int>(index) == selectedIndex;
            if (selected) {
                canvas.setColor(blendColor(style.accentColor, style.fillColor, 0.32f));
                canvas.fill(itemLeft, bounds.y + 4.0f, itemWidth, itemHeight);
            }
            if (drawText) {
                canvas.setColor(visual_style::stateTextColor(palette, enabled));
                canvas.text(item, widgetFont, textJustification(style, visage::Font::kCenter),
                    itemLeft + 4.0f, bounds.y + 4.0f, std::max(0.0f, itemWidth - 8.0f), itemHeight);
            }
            itemLeft += itemWidth + 4.0f;
        }
        if (items.empty() && drawText && shouldDrawEditorLabel(showEditorDecorations)) {
            canvas.setColor(visual_style::stateTextColor(palette, enabled));
            canvas.text("<empty>", widgetFont, visage::Font::kCenter, bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }
    case model::WidgetType::ToolBar: {
        const auto items = model::splitItems(getStringProperty(widget, "items", {}));
        const std::string selectedIndexKey = std::string(model::selectedItemIndexPropertyKey(widget.type));
        const int selectedIndex = model::sanitizeSelectedIndex(items,
            widget.getIntProperty(selectedIndexKey, items.empty() ? -1 : 0));
        const float itemHeight = std::max(0.0f, bounds.height - 10.0f);
        float itemLeft = bounds.x + 6.0f;
        drawRoundedBox(canvas, bounds, blendColor(style.panelColor, style.fillColor, 0.22f), style.borderColor, style.borderThickness, style.cornerRadius);
        for (std::size_t index = 0; index < items.size() && itemLeft < bounds.x + bounds.width - 6.0f; ++index) {
            const std::string& item = items[index];
            const float idealWidth = std::max(64.0f, estimateDesignerTextWidth(item, fontSize) + 18.0f);
            const float itemWidth = std::min(idealWidth, std::max(0.0f, bounds.x + bounds.width - 6.0f - itemLeft));
            const bool selected = static_cast<int>(index) == selectedIndex;
            canvas.setColor(selected ? blendColor(style.accentColor, style.fillColor, 0.34f) : style.fillColor);
            canvas.fill(itemLeft, bounds.y + 5.0f, itemWidth, itemHeight);
            drawBorder(canvas, { itemLeft, bounds.y + 5.0f, itemWidth, itemHeight }, style.borderColor, style.borderThickness);
            if (drawText) {
                canvas.setColor(style.textColor);
                canvas.text(item, widgetFont, visage::Font::kCenter,
                    itemLeft + 4.0f, bounds.y + 5.0f, std::max(0.0f, itemWidth - 8.0f), itemHeight);
            }
            itemLeft += itemWidth + 6.0f;
        }
        if (items.empty() && drawText && shouldDrawEditorLabel(showEditorDecorations)) {
            canvas.setColor(style.textColor);
            canvas.text("<empty>", widgetFont, visage::Font::kCenter, bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }
    case model::WidgetType::StatusBar: {
        const bool enabled = widgetIsEnabled(widget);
        const auto palette = baselinePalette(style, style.fillColor);
        visual_style::drawRecessed(canvas, baselineRect(bounds), palette, false, enabled);

        int fields = static_cast<int>(getNumericProperty(widget, "fields", 1.0f));
        fields = std::clamp(fields, 1, 4);
        const float fieldWidth = bounds.width / static_cast<float>(fields);
        const float fieldInset = std::min(10.0f, std::max(6.0f, bounds.height * 0.16f));
        const float textHeight = std::max(0.0f, bounds.height - 4.0f);
        for (int i = 0; i < fields; ++i) {
            const float fx = bounds.x + fieldWidth * static_cast<float>(i);
            const float fw = fieldWidth;
            if (drawText) {
                const std::string key = std::string("text") + std::to_string(i);
                const std::string text = getStringProperty(widget, key, {});
                canvas.setColor(visual_style::stateTextColor(palette, enabled));
                drawWidgetText(canvas, text, widgetFont,
                    { fx + fieldInset, bounds.y + 2.0f, std::max(0.0f, fw - fieldInset * 2.0f), textHeight },
                    style,
                    false,
                    "Left",
                    "Center");
            }
            if (i + 1 < fields) {
                canvas.setColor(style.borderColor);
                canvas.fill(fx + fw - 1.0f, bounds.y + fieldInset * 0.5f, 1.0f, std::max(0.0f, bounds.height - fieldInset));
            }
        }
        break;
    }
    case model::WidgetType::ProgressBar: {
        const bool enabled = widgetIsEnabled(widget);
        const auto palette = baselinePalette(style, style.fillColor);
        const float normalized = normalizedRangeValue(widget, "value");
        auto progressState = visualState;
        progressState.enabled = enabled;
        drawRoundedRecessed(canvas, bounds, style, progressState);

        const float inset = std::min(3.0f, std::max(1.0f, style.borderThickness + 1.0f));
        const float trackWidth = std::max(0.0f, bounds.width - inset * 2.0f);
        const float trackHeight = std::max(0.0f, bounds.height - inset * 2.0f);
        const float fillWidth = trackWidth * normalized;
        const int progressColor = enabled ? style.accentColor : visual_style::blend(style.accentColor, style.disabledColor, 0.62f);
        fillRoundedRect(canvas, bounds.x + inset, bounds.y + inset, fillWidth, trackHeight,
            std::clamp(style.cornerRadius, 0.0f, std::min(fillWidth, trackHeight) * 0.5f), progressColor);

        const std::string text = progressBarDisplayText(widget);
        if (drawText && !text.empty()) {
            canvas.setColor(normalized >= 0.5f && enabled ? 0xfff8fbff : visual_style::stateTextColor(palette, enabled));
            canvas.text(text, widgetFont, visage::Font::kCenter, bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }
    case model::WidgetType::ColorPicker: {
        const bool enabled = widgetIsEnabled(widget);
        const auto palette = baselinePalette(style, style.fillColor);
        const std::string colorValue = getStringProperty(widget, "value", "#2D7DFF");
        const bool showText = getBoolProperty(widget, "showText", true);
        const float swatchSize = std::max(16.0f, bounds.height - 12.0f);
        const float swatchX = bounds.x + 6.0f;
        const float swatchY = bounds.y + (bounds.height - swatchSize) * 0.5f;
        const float textX = swatchX + swatchSize + 10.0f;
        auto pickerState = visualState;
        pickerState.enabled = enabled;
        drawRoundedBevel(canvas, bounds, style, pickerState, style.fillColor);
        canvas.setColor(enabled ? parseColorOrDefault(colorValue, style.accentColor)
                                : visual_style::blend(parseColorOrDefault(colorValue, style.accentColor), style.disabledColor, 0.58f));
        canvas.fill(swatchX, swatchY, swatchSize, swatchSize);
        drawBorder(canvas, { swatchX, swatchY, swatchSize, swatchSize }, style.borderColor, style.borderThickness);
        if (drawText) {
            canvas.setColor(visual_style::stateTextColor(palette, enabled));
            const std::string label = showText ? getStringProperty(widget, "text", {}) : std::string{};
            const std::string text = showText && !label.empty() ? label + "  " + colorValue : colorValue;
            canvas.text(text, font, visage::Font::kTopLeft,
                textX, bounds.y + bounds.height * 0.5f - 10.0f,
                std::max(0.0f, bounds.width - (textX - bounds.x) - 8.0f), std::max(0.0f, bounds.height - 8.0f));
        }
        break;
    }
    case model::WidgetType::ModalDialog: {
        canvas.setColor(0xff0f1318);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);

        const PanelRect dialogBounds{
            bounds.x + std::max(10.0f, bounds.width * 0.08f),
            bounds.y + std::max(10.0f, bounds.height * 0.08f),
            std::max(120.0f, bounds.width - std::max(20.0f, bounds.width * 0.16f)),
            std::max(80.0f, bounds.height - std::max(20.0f, bounds.height * 0.16f))
        };

        const auto palette = baselinePalette(style, style.fillColor);
        visual_style::drawBevel(canvas, baselineRect(dialogBounds), palette);
        canvas.setColor(style.panelColor);
        canvas.fill(dialogBounds.x + 2.0f, dialogBounds.y + 2.0f,
            std::max(0.0f, dialogBounds.width - 4.0f), std::min(28.0f, std::max(0.0f, dialogBounds.height - 4.0f)));

        if (drawText) {
            canvas.setColor(style.textColor);
            canvas.text(getStringProperty(widget, "title", "Dialog"), widgetFont, visage::Font::kTopLeft,
                dialogBounds.x + 8.0f, dialogBounds.y + 4.0f, std::max(0.0f, dialogBounds.width - 16.0f), 20.0f);
            canvas.text(getStringProperty(widget, "message", "Message text"), widgetFont, visage::Font::kTopLeft,
                dialogBounds.x + 10.0f, dialogBounds.y + 40.0f, std::max(0.0f, dialogBounds.width - 20.0f), std::max(20.0f, dialogBounds.height - 90.0f));
        }

        const auto buttons = splitCommaSeparatedValues(getStringProperty(widget, "buttons", "OK"));
        const float buttonWidth = std::max(56.0f, std::min(96.0f, dialogBounds.width * 0.22f));
        const float buttonHeight = 24.0f;
        const float buttonSpacing = 8.0f;
        const float totalButtonWidth = static_cast<float>(buttons.size()) * buttonWidth
            + static_cast<float>(std::max<std::size_t>(0, buttons.size() - 1)) * buttonSpacing;
        float buttonX = dialogBounds.x + std::max(8.0f, (dialogBounds.width - totalButtonWidth) * 0.5f);
        const float buttonY = dialogBounds.y + dialogBounds.height - buttonHeight - 10.0f;
        for (const auto& button : buttons) {
            auto buttonPalette = palette;
            buttonPalette.fill = style.accentColor;
            visual_style::drawBevel(canvas, { buttonX, buttonY, buttonWidth, buttonHeight }, buttonPalette);
            if (drawText) {
                canvas.setColor(style.textColor);
                canvas.text(button, widgetFont, visage::Font::kCenter, buttonX, buttonY, buttonWidth, buttonHeight);
            }
            buttonX += buttonWidth + buttonSpacing;
        }
        break;
    }
    case model::WidgetType::Frame: {
        const bool enabled = widgetIsEnabled(widget);
        const auto palette = baselinePalette(style, style.fillColor);
        auto frameState = visualState;
        frameState.enabled = enabled;
        drawRoundedBevel(canvas, bounds, style, frameState, style.fillColor);
        canvas.setColor(enabled ? style.panelColor : visual_style::blend(style.panelColor, style.disabledColor, 0.55f));
        canvas.fill(bounds.x + 2.0f, bounds.y + 2.0f, std::max(0.0f, bounds.width - 4.0f), std::min(24.0f, std::max(0.0f, bounds.height - 4.0f)));
        if (drawText) {
            const std::string title = runtimeTextOrEditorLabel(widget, "title", showEditorDecorations);
            if (!title.empty()) {
                canvas.setColor(visual_style::stateTextColor(palette, enabled));
                const float padding = std::clamp(style.textPadding, 0.0f, bounds.width * 0.35f);
                drawWidgetText(canvas, title, widgetFont,
                    { bounds.x + padding, bounds.y + 6.0f, std::max(0.0f, bounds.width - padding * 2.0f), 20.0f },
                    style,
                    true);
            }
        }
        break;
    }
    case model::WidgetType::GroupBox: {
        const bool enabled = widgetIsEnabled(widget);
        const auto palette = baselinePalette(style, style.fillColor);
        const PanelRect contentBounds{ bounds.x, bounds.y + 10.0f, bounds.width, std::max(0.0f, bounds.height - 10.0f) };
        visual_style::State groupState;
        groupState.enabled = enabled;
        drawRoundedBevel(canvas, contentBounds, style, groupState, style.fillColor);
        if (drawText) {
            const std::string title = runtimeTextOrEditorLabel(widget, "title", showEditorDecorations);
            if (!title.empty()) {
                const float padding = std::clamp(style.textPadding, 0.0f, bounds.width * 0.35f);
                const float titleWidth = std::min(bounds.width - padding * 2.0f, std::max(48.0f, estimateDesignerTextWidth(title, fontSize)));
                canvas.setColor(enabled ? style.fillColor : visual_style::blend(style.fillColor, style.disabledColor, 0.55f));
                canvas.fill(bounds.x + padding, bounds.y, titleWidth + 12.0f, 20.0f);
                canvas.setColor(visual_style::stateTextColor(palette, enabled));
                drawWidgetText(canvas, title, widgetFont,
                    { bounds.x + padding + 6.0f, bounds.y + 1.0f, std::max(0.0f, bounds.width - padding * 2.0f), 18.0f },
                    style,
                    true);
            }
        }
        break;
    }
    case model::WidgetType::Panel: {
        const bool enabled = widgetIsEnabled(widget);
        auto palette = baselinePalette(style, style.fillColor);
        palette.highlight = visual_style::blend(palette.highlight, palette.fill, 0.55f);
        palette.shadow = visual_style::blend(palette.shadow, palette.fill, 0.55f);
        if (style.borderThickness > 0.0f) {
            visual_style::State panelState;
            panelState.enabled = enabled;
            drawRoundedBevel(canvas, bounds, style, panelState, style.fillColor);
        }
        else {
            drawRoundedBox(canvas,
                bounds,
                enabled ? palette.fill : visual_style::blend(palette.fill, palette.disabled, 0.55f),
                style.borderColor,
                0.0f,
                style.cornerRadius);
        }
        break;
    }
    case model::WidgetType::Sizer:
        drawRoundedBox(canvas, bounds, style.fillColor, blendColor(style.borderColor, style.fillColor, 0.25f), style.borderThickness, style.cornerRadius);
        if (drawText && shouldDrawEditorLabel(showEditorDecorations)) {
            const std::string label = std::string(model::toString(model::parseSizerOrientation(widget))) + " Sizer";
            canvas.setColor(blendColor(style.textColor, style.fillColor, 0.35f));
            canvas.text(label, widgetFont, widget.children.empty() ? visage::Font::kCenter : visage::Font::kTopLeft,
                bounds.x + 6.0f, bounds.y + 4.0f, std::max(0.0f, bounds.width - 12.0f), widget.children.empty() ? bounds.height : 20.0f);
        }
        break;
    case model::WidgetType::TabControl: {
        const bool enabled = widgetIsEnabled(widget);
        const auto palette = baselinePalette(style, style.fillColor);
        const std::vector<std::string> labels = tabLabels(widget);
        const int selectedTab = designerCanvas.previewSelectedTab(widget, selectedTabIndex(widget));
        const float headerHeight = std::min(32.0f, std::max(24.0f, bounds.height * 0.18f));
        auto tabControlState = visualState;
        tabControlState.enabled = enabled;
        drawRoundedRecessed(canvas, bounds, style, tabControlState);
        const float tabWidth = bounds.width / static_cast<float>(std::max<std::size_t>(1, labels.size()));
        for (std::size_t index = 0; index < labels.size(); ++index) {
            const PanelRect tabBounds{
                bounds.x + tabWidth * static_cast<float>(index),
                bounds.y,
                tabWidth,
                headerHeight
            };
            auto tabPalette = palette;
            const bool selected = static_cast<int>(index) == selectedTab;
            tabPalette.fill = selected ? style.fillColor : blendColor(style.panelColor, style.fillColor, 0.22f);
            auto tabState = visualState;
            tabState.checkedOrSelected = selected;
            tabState.active = selected;
            tabState.pressed = selected && visualState.pressed;
            visual_style::drawBevel(canvas, baselineRect(tabBounds), tabPalette, tabState);
            if (drawText) {
                canvas.setColor(visual_style::stateTextColor(tabPalette, enabled));
                drawWidgetText(canvas, labels[index], widgetFont, tabBounds, style, true, "Center", "Center");
            }
        }
        canvas.setColor(enabled ? style.fillColor : visual_style::blend(style.fillColor, style.disabledColor, 0.55f));
        canvas.fill(bounds.x + 2.0f, bounds.y + headerHeight - 1.0f, std::max(0.0f, bounds.width - 4.0f), std::max(0.0f, bounds.height - headerHeight - 1.0f));
        break;
    }
    case model::WidgetType::TabPage: {
        const bool enabled = widgetIsEnabled(widget);
        auto palette = baselinePalette(style, style.fillColor);
        palette.border = blendColor(style.borderColor, style.fillColor, 0.20f);
        drawRoundedBox(canvas,
            bounds,
            enabled ? style.recessedColor : visual_style::blend(style.recessedColor, style.disabledColor, 0.55f),
            palette.border,
            style.borderThickness,
            style.cornerRadius);
        break;
    }
    case model::WidgetType::Label:
        if (drawText) {
            const std::string text = runtimeTextOrEditorLabel(widget, "text", showEditorDecorations);
            if (!text.empty()) {
                canvas.setColor(style.textColor);
                const float padding = std::clamp(style.textPadding, 0.0f, bounds.width * 0.45f);
                drawWidgetText(canvas, text, widgetFont,
                    { bounds.x + padding, bounds.y + padding, std::max(0.0f, bounds.width - padding * 2.0f), std::max(0.0f, bounds.height - padding * 2.0f) },
                    style);
            }
        }
        break;
    case model::WidgetType::Button:
    {
        const bool pressedState = visualState.pressed
            || (widget.getBoolProperty("toggleMode", false) && visualState.checkedOrSelected);
        const bool enabled = widgetIsEnabled(widget);
        const std::string text = getStringProperty(widget, "text", {});
        const std::string configuredNormalText = getStringProperty(widget, "normalText", {});
        const std::string configuredPressedText = getStringProperty(widget, "pressedText", {});
        const std::string normalText = !configuredNormalText.empty()
            ? configuredNormalText
            : (!text.empty() ? text : editorOnlyLabel(widget, showEditorDecorations));
        const std::string pressedText = !configuredPressedText.empty() ? configuredPressedText : normalText;
        const int normalFillColor = parseColorOrDefault(getStringProperty(widget, "normalFillColor", {}), style.fillColor);
        const int pressedFillColor = parseColorOrDefault(getStringProperty(widget, "pressedFillColor", {}), blendColor(style.fillColor, style.accentColor, 0.18f));
        const int stateFill = pressedState ? pressedFillColor : normalFillColor;
        const auto palette = baselinePalette(style, stateFill);
        auto buttonState = visualState;
        buttonState.pressed = pressedState;
        buttonState.enabled = enabled;
        drawRoundedBevel(canvas, bounds, style, buttonState, stateFill);
        if (drawText && !normalText.empty()) {
            const float padding = std::clamp(style.textPadding, 0.0f, std::min(bounds.width, bounds.height) * 0.45f);
            canvas.setColor(visual_style::stateTextColor(palette, enabled));
            drawWidgetText(canvas, pressedState ? pressedText : normalText, widgetFont,
                {
                    bounds.x + padding + (pressedState ? 1.0f : 0.0f),
                    bounds.y + padding + (pressedState ? 1.0f : 0.0f),
                    std::max(0.0f, bounds.width - padding * 2.0f),
                    std::max(0.0f, bounds.height - padding * 2.0f)
                },
                style,
                false,
                "Center",
                "Center");
        }
        break;
    }
    case model::WidgetType::TextBox: {
        const bool enabled = widgetIsEnabled(widget);
        auto palette = baselinePalette(style, style.fillColor);
        auto textBoxState = visualState;
        textBoxState.enabled = enabled;
        drawRoundedRecessed(canvas, bounds, style, textBoxState);
        if (drawText && !designerCanvas.isPreviewTextOverlayWidget(widget.id)) {
            const float padding = std::clamp(style.textPadding, 0.0f, bounds.width * 0.45f);
            canvas.setColor(visual_style::stateTextColor(palette, enabled));
            drawWidgetText(canvas, designerCanvas.previewText(widget, getStringProperty(widget, "text", "")), widgetFont,
                { bounds.x + padding, bounds.y + 4.0f, std::max(0.0f, bounds.width - padding * 2.0f), std::max(0.0f, bounds.height - 8.0f) },
                style,
                false,
                "Left",
                "Center");
        }
        break;
    }
    case model::WidgetType::ComboBox: {
        const bool enabled = widgetIsEnabled(widget);
        const auto items = model::splitItems(getStringProperty(widget, "items", {}));
        const int selectedIndex = designerCanvas.previewSelectedIndex(widget,
            model::sanitizeSelectedIndex(items, widget.getIntProperty("selectedIndex", items.empty() ? -1 : 0)));
        const std::string selectedText = model::getSelectedItemText(items, selectedIndex);
        const float arrowWidth = std::min(26.0f, std::max(20.0f, bounds.width * 0.18f));
        auto palette = baselinePalette(style, style.fillColor);
        auto comboState = visualState;
        comboState.enabled = enabled;
        drawRoundedRecessed(canvas, bounds, style, comboState);
        const visual_style::Rect arrowBounds{
            bounds.x + bounds.width - arrowWidth, bounds.y, arrowWidth, bounds.height
        };
        auto arrowPalette = palette;
        arrowPalette.fill = blendColor(style.panelColor, style.fillColor, 0.22f);
        const float arrowInset = std::max(1.0f, std::min(style.borderThickness + 1.0f, bounds.height * 0.25f));
        canvas.setColor(enabled ? arrowPalette.fill : visual_style::blend(arrowPalette.fill, arrowPalette.disabled, 0.55f));
        canvas.fill(arrowBounds.x, arrowBounds.y + arrowInset, std::max(0.0f, arrowBounds.width - arrowInset), std::max(0.0f, arrowBounds.height - arrowInset * 2.0f));
        canvas.setColor(enabled ? style.shadowColor : visual_style::blend(style.shadowColor, style.disabledColor, 0.65f));
        canvas.fill(arrowBounds.x, arrowBounds.y + arrowInset, 1.0f, std::max(0.0f, arrowBounds.height - arrowInset * 2.0f));
        canvas.setColor(enabled ? palette.text : palette.disabled);
        const float arrowCenterX = arrowBounds.x + arrowBounds.width * 0.5f;
        const float arrowCenterY = arrowBounds.y + arrowBounds.height * 0.5f;
        canvas.fill(arrowCenterX - 4.0f, arrowCenterY - 1.0f, 8.0f, 1.0f);
        canvas.fill(arrowCenterX - 3.0f, arrowCenterY, 6.0f, 1.0f);
        canvas.fill(arrowCenterX - 2.0f, arrowCenterY + 1.0f, 4.0f, 1.0f);
        if (drawText) {
            canvas.setColor(visual_style::stateTextColor(palette, enabled));
            const std::string displayText = selectedText.empty() && shouldDrawEditorLabel(showEditorDecorations)
                ? std::string{ "<empty>" }
                : selectedText;
            if (!displayText.empty()) {
                const float padding = std::clamp(style.textPadding, 0.0f, bounds.width * 0.35f);
                canvas.text(displayText, widgetFont, textJustification(style, visage::Font::kTopLeft),
                    bounds.x + padding, alignedTextTop(bounds.y, bounds.height, fontSize, style),
                    std::max(0.0f, bounds.width - arrowWidth - padding * 1.5f), std::max(0.0f, bounds.height - 8.0f));
            }
        }
        break;
    }
    case model::WidgetType::ListBox: {
        const bool enabled = widgetIsEnabled(widget);
        const auto palette = baselinePalette(style, style.fillColor);
        const auto items = model::splitItems(getStringProperty(widget, "items", {}));
        const int selectedIndex = designerCanvas.previewSelectedIndex(widget,
            model::sanitizeSelectedIndex(items, widget.getIntProperty("selectedIndex", items.empty() ? -1 : 0)));
        const float rowHeight = std::max(18.0f, fontSize * 1.5f);
        const float listTop = bounds.y + 4.0f;
        const float visibleHeight = std::max(0.0f, bounds.height - 8.0f);
        const std::size_t visibleCount = std::max<std::size_t>(1, static_cast<std::size_t>(std::floor(visibleHeight / rowHeight)));
        auto listState = visualState;
        listState.enabled = enabled;
        drawRoundedRecessed(canvas, bounds, style, listState);
        float rowTop = listTop;
        for (std::size_t index = 0; index < std::min<std::size_t>(visibleCount, items.size()); ++index) {
            const bool selected = static_cast<int>(index) == selectedIndex;
            canvas.setColor(!enabled ? visual_style::blend(style.fillColor, style.disabledColor, 0.55f)
                : selected ? blendColor(style.accentColor, style.fillColor, 0.32f)
                           : (index % 2 == 0 ? style.fillColor : blendColor(style.panelColor, style.fillColor, 0.18f)));
            canvas.fill(bounds.x + 4.0f, rowTop, std::max(0.0f, bounds.width - 14.0f), rowHeight - 1.0f);
            if (drawText) {
                canvas.setColor(visual_style::stateTextColor(palette, enabled));
                const float padding = std::clamp(style.textPadding, 0.0f, bounds.width * 0.35f);
                canvas.text(items[index], widgetFont, textJustification(style, visage::Font::kTopLeft),
                    bounds.x + padding, rowTop + std::max(2.0f, (rowHeight - fontSize * 1.4f) * 0.5f),
                    std::max(0.0f, bounds.width - padding - 12.0f), std::max(0.0f, rowHeight - 4.0f));
            }
            rowTop += rowHeight;
        }
        if (items.size() > visibleCount) {
            canvas.setColor(style.panelColor);
            canvas.fill(bounds.x + bounds.width - 8.0f, bounds.y + 4.0f, 4.0f, std::max(0.0f, bounds.height - 8.0f));
            canvas.setColor(style.accentColor);
            canvas.fill(bounds.x + bounds.width - 8.0f, bounds.y + 10.0f, 4.0f, std::max(16.0f, bounds.height * 0.22f));
        }
        if (items.empty() && drawText && shouldDrawEditorLabel(showEditorDecorations)) {
            canvas.setColor(style.textColor);
            canvas.text("<empty>", widgetFont, visage::Font::kCenter,
                bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }
    case model::WidgetType::TableGrid: {
        const auto columns = model::splitTableColumns(getStringProperty(widget, "columns", {}));
        const auto rows = model::splitTableRows(getStringProperty(widget, "rows", {}));
        const auto modelSelection = model::clampSelectedCell(
            columns,
            rows,
            widget.getIntProperty("selectedRow", rows.empty() ? -1 : 0),
            widget.getIntProperty("selectedColumn", columns.empty() ? -1 : 0));
        const auto selection = designerCanvas.previewTableGridSelection(widget, modelSelection);
        const bool showHeader = getBoolProperty(widget, "showHeader", true);
        const bool showGridLines = getBoolProperty(widget, "showGridLines", true);
        const float headerHeight = showHeader ? std::max(18.0f, getNumericProperty(widget, "headerHeight", 30.0f)) : 0.0f;
        const float rowHeight = std::max(16.0f, getNumericProperty(widget, "rowHeight", 28.0f));
        const float contentX = bounds.x + 4.0f;
        const float contentY = bounds.y + 4.0f;
        const float contentWidth = std::max(0.0f, bounds.width - 8.0f);
        const float contentHeight = std::max(0.0f, bounds.height - 8.0f);
        const float columnWidth = contentWidth / static_cast<float>(std::max<std::size_t>(1, columns.empty() ? 1 : columns.size()));
        const float visibleRowsHeight = std::max(0.0f, contentHeight - headerHeight);
        const std::size_t visibleRowCount = std::max<std::size_t>(1, static_cast<std::size_t>(std::floor(visibleRowsHeight / rowHeight)));

        drawRoundedBox(canvas, bounds, style);

        if (showHeader) {
            canvas.setColor(blendColor(style.panelColor, style.fillColor, 0.18f));
            canvas.fill(contentX, contentY, contentWidth, std::min(headerHeight, contentHeight));
        }

        for (std::size_t columnIndex = 0; columnIndex < std::max<std::size_t>(1, columns.empty() ? 1 : columns.size()); ++columnIndex) {
            const float columnX = contentX + static_cast<float>(columnIndex) * columnWidth;
            const bool selectedColumn = static_cast<int>(columnIndex) == selection.column;
            if (showHeader && selectedColumn) {
                canvas.setColor(blendColor(style.accentColor, style.fillColor, 0.24f));
                canvas.fill(columnX, contentY, std::max(0.0f, columnWidth - 1.0f), std::min(headerHeight, contentHeight));
            }

            if (showGridLines && columnIndex > 0) {
                canvas.setColor(blendColor(style.borderColor, style.fillColor, 0.35f));
                canvas.fill(columnX, contentY, 1.0f, contentHeight);
            }

            if (showHeader && drawText && columnIndex < columns.size()) {
                const std::string& headerText = columns[columnIndex];
                canvas.setColor(style.textColor);
                canvas.text(headerText, widgetFont, visage::Font::kTopLeft,
                    columnX + 6.0f, contentY + std::max(3.0f, (headerHeight - fontSize * 1.3f) * 0.35f),
                    std::max(0.0f, columnWidth - 12.0f), std::max(0.0f, headerHeight - 6.0f));
            }
        }

        float rowTop = contentY + headerHeight;
        for (std::size_t rowIndex = 0; rowIndex < std::min<std::size_t>(visibleRowCount, rows.size()); ++rowIndex) {
            const bool selectedRow = static_cast<int>(rowIndex) == selection.row;
            for (std::size_t columnIndex = 0; columnIndex < std::max<std::size_t>(1, columns.empty() ? 1 : columns.size()); ++columnIndex) {
                const float columnX = contentX + static_cast<float>(columnIndex) * columnWidth;
                const bool selectedCell = selectedRow && static_cast<int>(columnIndex) == selection.column;
                canvas.setColor(selectedCell ? blendColor(style.accentColor, style.fillColor, 0.30f)
                    : (selectedRow ? blendColor(style.accentColor, style.fillColor, 0.16f)
                                   : (rowIndex % 2 == 0 ? style.fillColor : blendColor(style.panelColor, style.fillColor, 0.14f))));
                canvas.fill(columnX, rowTop, std::max(0.0f, columnWidth - 1.0f), std::max(0.0f, rowHeight - 1.0f));

                if (drawText) {
                    canvas.setColor(style.textColor);
                    canvas.text(model::getCellText(rows, static_cast<int>(rowIndex), static_cast<int>(columnIndex)), widgetFont, visage::Font::kTopLeft,
                        columnX + 6.0f, rowTop + std::max(2.0f, (rowHeight - fontSize * 1.3f) * 0.35f),
                        std::max(0.0f, columnWidth - 12.0f), std::max(0.0f, rowHeight - 4.0f));
                }
            }

            if (showGridLines) {
                canvas.setColor(blendColor(style.borderColor, style.fillColor, 0.35f));
                canvas.fill(contentX, rowTop + rowHeight - 1.0f, contentWidth, 1.0f);
            }

            rowTop += rowHeight;
        }

        if (rows.size() > visibleRowCount) {
            canvas.setColor(style.panelColor);
            canvas.fill(bounds.x + bounds.width - 8.0f, bounds.y + 4.0f, 4.0f, std::max(0.0f, bounds.height - 8.0f));
            canvas.setColor(style.accentColor);
            canvas.fill(bounds.x + bounds.width - 8.0f, bounds.y + 10.0f, 4.0f, std::max(16.0f, bounds.height * 0.22f));
        }

        if (columns.empty() && drawText && shouldDrawEditorLabel(showEditorDecorations)) {
            canvas.setColor(style.textColor);
            canvas.text("<no columns>", widgetFont, visage::Font::kCenter,
                bounds.x, bounds.y, bounds.width, bounds.height);
        }
        else if (rows.empty() && drawText && shouldDrawEditorLabel(showEditorDecorations)) {
            canvas.setColor(style.textColor);
            canvas.text("<no rows>", widgetFont, visage::Font::kCenter,
                bounds.x, bounds.y + headerHeight, bounds.width, std::max(0.0f, bounds.height - headerHeight));
        }
        break;
    }
    case model::WidgetType::TreeView: {
        const std::string nodesText = getStringProperty(widget, "nodes", {});
        const bool showRoot = getBoolProperty(widget, "showRoot", true);
        const bool showLines = getBoolProperty(widget, "showLines", true);
        const std::string expandedNodePaths = getStringProperty(widget, "expandedNodePaths", {});
        const auto visibleNodes = model::flattenVisibleTreeNodes(nodesText, showRoot, expandedNodePaths);
        const std::string selectedNodePath = model::clampSelectedTreeNode(
            nodesText,
            getStringProperty(widget, "selectedNodePath", {}),
            showRoot,
            expandedNodePaths);
        const float rowHeight = std::max(18.0f, fontSize * 1.45f);
        const float indentWidth = 16.0f;
        const float markerSize = 10.0f;
        const float listTop = bounds.y + 4.0f;
        const float visibleHeight = std::max(0.0f, bounds.height - 8.0f);
        const std::size_t visibleCount = std::max<std::size_t>(1, static_cast<std::size_t>(std::floor(visibleHeight / rowHeight)));
        drawRoundedBox(canvas, bounds, style);

        float rowTop = listTop;
        const int lineColor = blendColor(style.borderColor, style.fillColor, 0.35f);
        for (std::size_t index = 0; index < std::min<std::size_t>(visibleCount, visibleNodes.size()); ++index) {
            const auto& node = visibleNodes[index];
            const bool selected = node.path == selectedNodePath;
            const float markerCenterX = bounds.x + 12.0f + static_cast<float>(node.visualDepth) * indentWidth;
            const float markerCenterY = rowTop + rowHeight * 0.5f;
            const float textX = markerCenterX + (node.hasChildren ? 12.0f : 8.0f);

            canvas.setColor(selected ? blendColor(style.accentColor, style.fillColor, 0.28f) : (index % 2 == 0 ? style.fillColor : blendColor(style.panelColor, style.fillColor, 0.14f)));
            canvas.fill(bounds.x + 4.0f, rowTop, std::max(0.0f, bounds.width - 14.0f), rowHeight - 1.0f);

            if (showLines && node.visualDepth > 0) {
                canvas.setColor(lineColor);
                const float indentX = bounds.x + 12.0f + static_cast<float>(node.visualDepth - 1) * indentWidth;
                canvas.fill(indentX, rowTop, 1.0f, rowHeight);
                canvas.fill(indentX, markerCenterY, markerCenterX - indentX, 1.0f);
            }

            if (node.hasChildren) {
                const PanelRect markerBounds{ markerCenterX - markerSize * 0.5f, markerCenterY - markerSize * 0.5f, markerSize, markerSize };
                canvas.setColor(blendColor(style.panelColor, style.fillColor, 0.22f));
                canvas.fill(markerBounds.x, markerBounds.y, markerBounds.width, markerBounds.height);
                drawBorder(canvas, markerBounds, style.borderColor, 1.0f);
                canvas.setColor(style.borderColor);
                canvas.fill(markerBounds.x + 2.0f, markerCenterY, markerBounds.width - 4.0f, 1.0f);
                if (!node.expanded) {
                    canvas.fill(markerCenterX, markerBounds.y + 2.0f, 1.0f, markerBounds.height - 4.0f);
                }
            }

            if (drawText) {
                canvas.setColor(style.textColor);
                canvas.text(node.text, widgetFont, visage::Font::kTopLeft,
                    textX + 6.0f, rowTop + std::max(2.0f, (rowHeight - fontSize * 1.3f) * 0.5f),
                    std::max(0.0f, bounds.x + bounds.width - textX - 18.0f), std::max(0.0f, rowHeight - 4.0f));
            }

            rowTop += rowHeight;
        }

        if (visibleNodes.size() > visibleCount) {
            canvas.setColor(style.panelColor);
            canvas.fill(bounds.x + bounds.width - 8.0f, bounds.y + 4.0f, 4.0f, std::max(0.0f, bounds.height - 8.0f));
            canvas.setColor(style.accentColor);
            canvas.fill(bounds.x + bounds.width - 8.0f, bounds.y + 10.0f, 4.0f, std::max(16.0f, bounds.height * 0.22f));
        }

        if (visibleNodes.empty() && drawText && shouldDrawEditorLabel(showEditorDecorations)) {
            canvas.setColor(style.textColor);
            canvas.text("<empty>", widgetFont, visage::Font::kCenter,
                bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }
    case model::WidgetType::CheckBox: {
        const bool enabled = widgetIsEnabled(widget);
        const bool checked = visualState.checkedOrSelected;
        const float boxSize = 18.0f;
        const float squareX = bounds.x + 6.0f;
        const float squareY = bounds.y + (bounds.height - boxSize) * 0.5f;
        const float textX = squareX + boxSize + 12.0f;
        const auto palette = baselinePalette(style, style.fillColor);
        auto indicatorState = visualState;
        indicatorState.checkedOrSelected = checked;
        visual_style::drawRecessed(canvas, { squareX, squareY, boxSize, boxSize }, palette, indicatorState);
        if (checked) {
            canvas.setColor(enabled ? style.accentColor : visual_style::blend(style.accentColor, style.disabledColor, 0.62f));
            canvas.fill(squareX + 4.0f, squareY + 4.0f, 10.0f, 10.0f);
            canvas.setColor(palette.highlight);
            canvas.fill(squareX + 5.0f, squareY + 5.0f, 8.0f, 1.0f);
        }
        if (drawText) {
            const std::string text = runtimeTextOrEditorLabel(widget, "text", showEditorDecorations);
            if (!text.empty()) {
                canvas.setColor(visual_style::stateTextColor(palette, enabled));
                drawWidgetText(canvas, text, widgetFont,
                    { textX, bounds.y + 4.0f, std::max(0.0f, bounds.x + bounds.width - textX - 8.0f), std::max(0.0f, bounds.height - 8.0f) },
                    style,
                    false,
                    "Left",
                    "Center");
            }
        }
        break;
    }
    case model::WidgetType::RadioButton: {
        const bool enabled = widgetIsEnabled(widget);
        const bool selected = visualState.checkedOrSelected;
        const auto palette = baselinePalette(style, style.fillColor);
        const float boxSize = 18.0f;
        const float outerX = bounds.x + 6.0f;
        const float outerY = bounds.y + (bounds.height - boxSize) * 0.5f;
        const float centerX = outerX + boxSize * 0.5f;
        const float centerY = outerY + boxSize * 0.5f;
        const float textX = outerX + boxSize + 12.0f;
        fillCircleApprox(canvas, centerX, centerY, boxSize * 0.5f,
            enabled ? palette.border : visual_style::blend(palette.border, palette.disabled, 0.65f));
        fillCircleApprox(canvas, centerX, centerY,
            std::max(2.0f, boxSize * 0.5f - std::max(2.0f, style.borderThickness + 1.0f)),
            enabled ? palette.recessedFill : visual_style::blend(palette.recessedFill, palette.disabled, 0.55f));
        fillCircleApprox(canvas, centerX - 1.0f, centerY - 1.0f, std::max(1.0f, boxSize * 0.5f - 4.0f), palette.highlight);
        fillCircleApprox(canvas, centerX, centerY, std::max(1.0f, boxSize * 0.5f - 5.0f),
            enabled ? palette.recessedFill : visual_style::blend(palette.recessedFill, palette.disabled, 0.55f));
        if (selected) {
            fillCircleApprox(canvas, centerX, centerY, 4.0f,
                enabled ? style.accentColor : visual_style::blend(style.accentColor, style.disabledColor, 0.62f));
        }
        if (visualState.focused && enabled) {
            visual_style::drawFocus(canvas, baselineRect(bounds), palette);
        }
        if (drawText) {
            const std::string text = runtimeTextOrEditorLabel(widget, "text", showEditorDecorations);
            if (!text.empty()) {
                canvas.setColor(visual_style::stateTextColor(palette, enabled));
                drawWidgetText(canvas, text, widgetFont,
                    { textX, bounds.y + 4.0f, std::max(0.0f, bounds.x + bounds.width - textX - 8.0f), std::max(0.0f, bounds.height - 8.0f) },
                    style,
                    false,
                    "Left",
                    "Center");
            }
        }
        break;
    }
    case model::WidgetType::Slider: {
        const bool enabled = widgetIsEnabled(widget);
        const float normalized = normalizedSliderValue(widget,
            designerCanvas.previewNumericValue(widget, getNumericProperty(widget, "value", 50.0f)));
        const float trackY = std::floor(bounds.y + bounds.height * 0.5f - 3.0f);
        const float trackLeft = bounds.x + 8.0f;
        const float trackWidth = std::max(0.0f, bounds.width - 16.0f);
        const float handleCenterX = trackLeft + trackWidth * normalized;
        const float handleX = std::clamp(handleCenterX - 6.0f, trackLeft - 6.0f, trackLeft + trackWidth - 6.0f);
        const auto palette = baselinePalette(style, style.fillColor);
        auto trackState = visualState;
        trackState.focused = false;
        visual_style::drawRecessed(canvas, { trackLeft, trackY, trackWidth, 6.0f }, palette, trackState);
        const visual_style::Rect thumb{
            handleX, std::floor(bounds.y + bounds.height * 0.5f - 9.0f), 12.0f, 18.0f
        };
        auto thumbPalette = palette;
        thumbPalette.fill = enabled ? style.accentColor : visual_style::blend(style.accentColor, style.disabledColor, 0.62f);
        visual_style::drawBevel(canvas, thumb, thumbPalette, visualState);
        const std::string editorLabel = editorOnlyLabel(widget, showEditorDecorations);
        if (drawText && !editorLabel.empty()) {
            canvas.setColor(visual_style::stateTextColor(palette, enabled));
            canvas.text(editorLabel, widgetFont, visage::Font::kTopLeft,
                bounds.x, bounds.y - 18.0f, bounds.width, 16.0f);
        }
        break;
    }
    case model::WidgetType::ScrollBar: {
        const bool enabled = widgetIsEnabled(widget);
        const auto palette = baselinePalette(style, style.fillColor);
        const bool vertical = getStringProperty(widget, "orientation", "Horizontal") == "Vertical";
        const float pageSize = std::max(1.0f, getNumericProperty(widget, "pageSize", 10.0f));
        const float minimum = getNumericProperty(widget, "min", 0.0f);
        const float maximum = std::max(minimum + 1.0f, getNumericProperty(widget, "max", 100.0f));
        const float value = std::clamp(designerCanvas.previewNumericValue(widget, getNumericProperty(widget, "value", minimum)), minimum, maximum);
        const float normalized = std::clamp((value - minimum) / (maximum - minimum), 0.0f, 1.0f);
        const float thumbFactor = std::clamp(pageSize / (maximum - minimum + pageSize), 0.18f, 0.55f);
        const float arrowSize = vertical ? std::min(bounds.width, 20.0f) : std::min(bounds.height, 20.0f);
        auto trackState = visualState;
        trackState.focused = false;
        visual_style::drawRecessed(canvas, baselineRect(bounds), palette, trackState);
        canvas.setColor(style.borderColor);
        if (vertical) {
            const float trackTop = bounds.y + arrowSize;
            const float trackHeight = std::max(0.0f, bounds.height - arrowSize * 2.0f);
            const float thumbHeight = std::clamp(trackHeight * thumbFactor, 18.0f, std::max(18.0f, trackHeight));
            const float thumbY = trackTop + std::max(0.0f, trackHeight - thumbHeight) * normalized;
            visual_style::drawBevel(canvas, { bounds.x, bounds.y, bounds.width, arrowSize }, palette, false, false, enabled);
            visual_style::drawBevel(canvas, { bounds.x, bounds.y + bounds.height - arrowSize, bounds.width, arrowSize }, palette, false, false, enabled);
            auto thumbPalette = palette;
            thumbPalette.fill = style.accentColor;
            visual_style::drawBevel(canvas, { bounds.x + 4.0f, thumbY, std::max(0.0f, bounds.width - 8.0f), thumbHeight }, thumbPalette, visualState);
            canvas.setColor(visual_style::stateTextColor(palette, enabled));
            canvas.fill(bounds.x + bounds.width * 0.5f - 3.0f, bounds.y + 6.0f, 6.0f, 3.0f);
            canvas.fill(bounds.x + bounds.width * 0.5f - 3.0f, bounds.y + bounds.height - 9.0f, 6.0f, 3.0f);
        }
        else {
            const float trackLeft = bounds.x + arrowSize;
            const float trackWidth = std::max(0.0f, bounds.width - arrowSize * 2.0f);
            const float thumbWidth = std::clamp(trackWidth * thumbFactor, 18.0f, std::max(18.0f, trackWidth));
            const float thumbX = trackLeft + std::max(0.0f, trackWidth - thumbWidth) * normalized;
            visual_style::drawBevel(canvas, { bounds.x, bounds.y, arrowSize, bounds.height }, palette, false, false, enabled);
            visual_style::drawBevel(canvas, { bounds.x + bounds.width - arrowSize, bounds.y, arrowSize, bounds.height }, palette, false, false, enabled);
            auto thumbPalette = palette;
            thumbPalette.fill = style.accentColor;
            visual_style::drawBevel(canvas, { thumbX, bounds.y + 4.0f, thumbWidth, std::max(0.0f, bounds.height - 8.0f) }, thumbPalette, visualState);
            canvas.setColor(visual_style::stateTextColor(palette, enabled));
            canvas.fill(bounds.x + 6.0f, bounds.y + bounds.height * 0.5f - 3.0f, 3.0f, 6.0f);
            canvas.fill(bounds.x + bounds.width - 9.0f, bounds.y + bounds.height * 0.5f - 3.0f, 3.0f, 6.0f);
        }
        break;
    }
    case model::WidgetType::Image:
    {
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);

        const auto resolvedSource = resources::ImageResourceCache::resolveWidgetImageSource(document, widget);
        const auto scaleMode = resources::ImageResourceCache::parseScaleMode(widget.getStringProperty("scaleMode", "Fit"));
        const int warningBorderColor = 0xfff5c16c;
        const int errorBorderColor = 0xffe17d7d;
        int borderColor = style.borderColor;
        std::string placeholderText = imageWidgetPlaceholderText(resolvedSource);
        const bool simplifyImagePreview = simplifySelectedImages
            && (document.isPrimarySelected(widget.id) || document.isSecondarySelected(widget.id));
        bool drewImage = false;

        if (resolvedSource.missingResource) {
            borderColor = warningBorderColor;
        }
        else if (simplifyImagePreview) {
            const PanelRect imageBounds{
                bounds.x + 4.0f,
                bounds.y + 4.0f,
                std::max(0.0f, bounds.width - 8.0f),
                std::max(0.0f, bounds.height - 8.0f)
            };
            canvas.setColor(blendColor(style.fillColor, style.accentColor, 0.12f));
            canvas.fill(imageBounds.x, imageBounds.y, imageBounds.width, imageBounds.height);
            canvas.setColor(blendColor(style.borderColor, style.accentColor, 0.35f));
            canvas.fill(imageBounds.x, imageBounds.y, imageBounds.width, 1.0f);
            canvas.fill(imageBounds.x, imageBounds.y + imageBounds.height - 1.0f, imageBounds.width, 1.0f);
            canvas.fill(imageBounds.x, imageBounds.y, 1.0f, imageBounds.height);
            canvas.fill(imageBounds.x + imageBounds.width - 1.0f, imageBounds.y, 1.0f, imageBounds.height);

            const float inset = std::min(std::min(imageBounds.width, imageBounds.height) * 0.2f, 12.0f);
            canvas.setColor(style.accentColor);
            canvas.fill(imageBounds.x + inset, imageBounds.y + inset,
                std::max(1.0f, imageBounds.width - inset * 2.0f), 1.0f);
            canvas.fill(imageBounds.x + inset, imageBounds.y + imageBounds.height - inset - 1.0f,
                std::max(1.0f, imageBounds.width - inset * 2.0f), 1.0f);
            if (drawText && !resolvedSource.displayText.empty()) {
                placeholderText = resolvedSource.displayText;
            }
            else {
                placeholderText = "Image";
            }
        }
        else if (resolvedSource.hasImage && imageCache != nullptr) {
            const auto cachedImage = imageCache->getOrLoad(resolvedSource.sourcePath);
            placeholderText = imageWidgetPlaceholderText(resolvedSource, &cachedImage);
            if (cachedImage.info.available && cachedImage.encodedBytes != nullptr && !cachedImage.encodedBytes->empty()) {
                const auto drawRect = resources::ImageResourceCache::computeDrawRect(
                    bounds.x + 4.0f,
                    bounds.y + 4.0f,
                    std::max(0.0f, bounds.width - 8.0f),
                    std::max(0.0f, bounds.height - 8.0f),
                    cachedImage.info.width,
                    cachedImage.info.height,
                    scaleMode);
                canvas.setColor(0xffffffff);
                canvas.image(cachedImage.encodedBytes->data(),
                    static_cast<int>(cachedImage.encodedBytes->size()),
                    drawRect.x,
                    drawRect.y,
                    drawRect.width,
                    drawRect.height);
                drewImage = true;
            }
            else if (placeholderText == "Missing image file") {
                borderColor = warningBorderColor;
            }
            else if (placeholderText == "Image load failed") {
                borderColor = errorBorderColor;
            }
        }

        drawBorder(canvas, bounds, borderColor, style.borderThickness);
        if (drawText && !drewImage && shouldDrawEditorLabel(showEditorDecorations)) {
            canvas.setColor(borderColor == style.borderColor ? style.textColor : borderColor);
            canvas.text(placeholderText, widgetFont, visage::Font::kCenter,
                bounds.x + 6.0f, bounds.y, std::max(0.0f, bounds.width - 12.0f), bounds.height);
        }
        break;
    }
    case model::WidgetType::Spacer:
        canvas.setColor(blendColor(style.fillColor, style.panelColor, 0.45f));
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, blendColor(style.borderColor, style.accentColor, 0.45f), 1.0f);
        if (drawText && shouldDrawEditorLabel(showEditorDecorations)) {
            canvas.setColor(style.textColor);
            const std::string label = model::parseSpacerKind(widget) == model::SpacerKind::Stretch ? "Stretch Spacer" : "Fixed Spacer";
            canvas.text(label, widgetFont, visage::Font::kCenter, bounds.x + 4.0f, bounds.y, std::max(0.0f, bounds.width - 8.0f), bounds.height);
        }
        break;
    }

    // Draw children from back to front so later children appear on top.
    for (const auto& child : widget.children) {
        if (!isChildVisibleInParent(widget, child)) {
            continue;
        }
        drawWidget(canvas, font, drawText, document, designerCanvas, imageCache, simplifySelectedImages, child, formScreenX, formScreenY, widgetLocalX, widgetLocalY,
            scale, selectedWidgetId, showGrid, showMinorGrid, gridSize, majorGridSize,
            showEditorDecorations, appearancePreviewState);
    }

    if (showEditorDecorations && document.isPrimarySelected(widget.id)) {
        drawSelectionOutline(canvas, bounds);
        if (showsDirectResizeAffordance(widget)) {
            drawSelectionHandles(canvas, bounds, hasAssignedCallbacks(widget));
        }
    }
    else if (showEditorDecorations && document.isSecondarySelected(widget.id)) {
        drawSecondarySelectionOutline(canvas, bounds);
    }
}

} // namespace

float DesignerCanvas::measureWidgetTextWidth(const model::ProjectDocument& document,
    const model::WidgetNode& widget,
    const std::string& text,
    const visage::Font& fallbackFont) const
{
    if (text.empty()) {
        return 0.0f;
    }

    const ResolvedWidgetStyle style = resolveWidgetStyle(document, widget);
    visage::Font fontStorage{};
    const visage::Font& widgetFont = resolvedWidgetFont(widget, style, fallbackFont, fontStorage);
    const std::u32string utf32Text = visage::String::convertUtf8ToUtf32<std::u32string>(text);
    return widgetFont.stringWidth(utf32Text);
}

void DesignerCanvas::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

void DesignerCanvas::setMode(Mode mode)
{
    if (mode_ == mode) {
        return;
    }
    clearPreviewInteraction();
    mode_ = mode;
}

bool DesignerCanvas::updatePreviewHover(const model::ProjectDocument& document, float x, float y)
{
    if (mode_ != Mode::Preview) {
        return false;
    }
    const auto hit = hitTestWidgetId(document, x, y);
    const std::string nextId = hit.has_value() && !document.isRootWidgetId(*hit) ? *hit : std::string{};
    if (previewHoveredWidgetId_ == nextId) {
        return false;
    }
    previewHoveredWidgetId_ = nextId;
    return true;
}

bool DesignerCanvas::beginPreviewInteraction(const model::ProjectDocument& document, float x, float y)
{
    if (mode_ != Mode::Preview) {
        return false;
    }
    const auto hit = hitTestWidgetId(document, x, y);
    const model::WidgetNode* widget = hit.has_value() ? document.findWidgetById(*hit) : nullptr;
    const std::string nextId = widget != nullptr
            && !document.isRootWidgetId(widget->id)
            && widgetIsEnabled(*widget)
        ? widget->id
        : std::string{};
    const bool changed = previewPressedWidgetId_ != nextId
        || previewFocusedWidgetId_ != nextId
        || previewHoveredWidgetId_ != nextId;
    previewPressedWidgetId_ = nextId;
    previewFocusedWidgetId_ = nextId;
    previewHoveredWidgetId_ = nextId;
    previewDraggingValueWidgetId_.clear();
    if (widget != nullptr
        && (widget->type == model::WidgetType::Slider || widget->type == model::WidgetType::ScrollBar)) {
        previewDraggingValueWidgetId_ = widget->id;
        (void)updatePreviewInteraction(document, x, y);
    }
    return changed;
}

bool DesignerCanvas::updatePreviewInteraction(const model::ProjectDocument& document, float x, float y)
{
    if (mode_ != Mode::Preview || previewDraggingValueWidgetId_.empty()) {
        return false;
    }

    const model::WidgetNode* widget = document.findWidgetById(previewDraggingValueWidgetId_);
    if (widget == nullptr || !widgetIsEnabled(*widget)) {
        previewDraggingValueWidgetId_.clear();
        return true;
    }

    const auto bounds = widgetScreenBounds(document, widget->id);
    if (!bounds.has_value()) {
        return false;
    }

    const float minimum = getNumericProperty(*widget, "min", 0.0f);
    const float maximum = std::max(minimum, getNumericProperty(*widget, "max", 100.0f));
    const float span = maximum - minimum;
    if (span <= 0.0f) {
        return false;
    }

    float normalized = 0.0f;
    if (widget->type == model::WidgetType::ScrollBar
        && getStringProperty(*widget, "orientation", "Horizontal") == "Vertical") {
        const float arrowSize = std::min(bounds->width, 20.0f);
        const float trackTop = bounds->y + arrowSize;
        const float trackHeight = std::max(1.0f, bounds->height - arrowSize * 2.0f);
        normalized = std::clamp((y - trackTop) / trackHeight, 0.0f, 1.0f);
    }
    else {
        const float trackLeft = widget->type == model::WidgetType::ScrollBar
            ? bounds->x + std::min(bounds->height, 20.0f)
            : bounds->x + 8.0f;
        const float trackWidth = widget->type == model::WidgetType::ScrollBar
            ? std::max(1.0f, bounds->width - std::min(bounds->height, 20.0f) * 2.0f)
            : std::max(1.0f, bounds->width - 16.0f);
        normalized = std::clamp((x - trackLeft) / trackWidth, 0.0f, 1.0f);
    }

    const float rawValue = minimum + span * normalized;
    const float step = std::max(0.0f, getNumericProperty(*widget, "step", 1.0f));
    const float steppedValue = step > 0.0f
        ? minimum + std::round((rawValue - minimum) / step) * step
        : rawValue;
    const float nextValue = std::clamp(steppedValue, minimum, maximum);
    const auto current = previewNumericValues_.find(widget->id);
    if (current != previewNumericValues_.end() && std::abs(current->second - nextValue) < 0.001f) {
        return false;
    }
    previewNumericValues_[widget->id] = nextValue;
    return true;
}

bool DesignerCanvas::endPreviewInteraction(const model::ProjectDocument& document, float x, float y)
{
    if (mode_ != Mode::Preview) {
        return false;
    }

    const std::string pressedId = previewPressedWidgetId_;
    previewPressedWidgetId_.clear();
    previewDraggingValueWidgetId_.clear();
    const auto hit = hitTestWidgetId(document, x, y);
    if (pressedId.empty() || !hit.has_value() || *hit != pressedId) {
        return !pressedId.empty();
    }

    const model::WidgetNode* widget = document.findWidgetById(pressedId);
    if (widget == nullptr || !widgetIsEnabled(*widget)) {
        return true;
    }

    switch (widget->type) {
    case model::WidgetType::Button:
        if (widget->getBoolProperty("toggleMode", false)) {
            const bool current = previewChecked_.contains(widget->id)
                ? previewChecked_.at(widget->id)
                : widget->getBoolProperty("checked", false);
            previewChecked_[widget->id] = !current;
        }
        break;
    case model::WidgetType::CheckBox: {
        const bool current = previewChecked_.contains(widget->id)
            ? previewChecked_.at(widget->id)
            : widget->getBoolProperty("checked", false);
        previewChecked_[widget->id] = !current;
        break;
    }
    case model::WidgetType::RadioButton: {
        const std::string group = widget->getStringProperty("group", "default");
        const auto selectGroup = [&](const auto& self, const model::WidgetNode& node) -> void {
            if (node.type == model::WidgetType::RadioButton
                && node.getStringProperty("group", "default") == group) {
                previewSelected_[node.id] = node.id == widget->id;
            }
            for (const auto& child : node.children) {
                self(self, child);
            }
        };
        selectGroup(selectGroup, document.root);
        break;
    }
    case model::WidgetType::TabControl:
        if (const auto tabIndex = hitTestTabHeader(document, widget->id, x, y)) {
            previewSelectedTab_[widget->id] = *tabIndex;
        }
        break;
    case model::WidgetType::ComboBox:
        return false;
    case model::WidgetType::ListBox: {
        const auto items = model::splitItems(widget->getStringProperty("items", {}));
        if (!items.empty()) {
            const auto bounds = widgetScreenBounds(document, widget->id);
            if (widget->type == model::WidgetType::ListBox && bounds.has_value()) {
                const float fontSize = resolvedFontSize(*widget, resolveWidgetStyle(document, *widget));
                const float rowHeight = std::max(18.0f, fontSize * 1.5f);
                const int row = static_cast<int>(std::floor((y - (bounds->y + 4.0f)) / rowHeight));
                if (row >= 0 && row < static_cast<int>(items.size())) {
                    previewSelectedIndex_[widget->id] = row;
                }
            }
            else {
                const int current = previewSelectedIndex(*widget,
                    model::sanitizeSelectedIndex(items, widget->getIntProperty("selectedIndex", 0)));
                previewSelectedIndex_[widget->id] = (current + 1) % static_cast<int>(items.size());
            }
        }
        break;
    }
    case model::WidgetType::TableGrid: {
        const auto columns = model::splitTableColumns(widget->getStringProperty("columns", {}));
        const auto rows = model::splitTableRows(widget->getStringProperty("rows", {}));
        const auto bounds = widgetScreenBounds(document, widget->id);
        if (!columns.empty() && !rows.empty() && bounds.has_value()) {
            const bool showHeader = getBoolProperty(*widget, "showHeader", true);
            const float headerHeight = showHeader ? std::max(18.0f, getNumericProperty(*widget, "headerHeight", 30.0f)) : 0.0f;
            const float rowHeight = std::max(16.0f, getNumericProperty(*widget, "rowHeight", 28.0f));
            const float contentX = bounds->x + 4.0f;
            const float contentY = bounds->y + 4.0f;
            const float contentWidth = std::max(0.0f, bounds->width - 8.0f);
            const float columnWidth = contentWidth / static_cast<float>(std::max<std::size_t>(1, columns.size()));
            const float rowOffset = y - (contentY + headerHeight);
            if (x >= contentX && x <= contentX + contentWidth && rowOffset >= 0.0f && columnWidth > 0.0f) {
                const int column = std::clamp(static_cast<int>((x - contentX) / columnWidth), 0, static_cast<int>(columns.size()) - 1);
                const int row = static_cast<int>(std::floor(rowOffset / rowHeight));
                if (row >= 0 && row < static_cast<int>(rows.size())) {
                    previewTableGridSelections_[widget->id] = model::clampSelectedCell(columns, rows, row, column);
                }
            }
        }
        break;
    }
    default:
        break;
    }

    return true;
}

bool DesignerCanvas::focusNextPreviewWidget(const model::ProjectDocument& document, bool reverse)
{
    if (mode_ != Mode::Preview) {
        return false;
    }

    std::vector<std::string> focusableIds;
    const auto collectFocusable = [&](const auto& self, const model::WidgetNode& node) -> void {
        if (!document.isRootWidgetId(node.id) && widgetIsEnabled(node)) {
            switch (node.type) {
            case model::WidgetType::Button:
            case model::WidgetType::TextBox:
            case model::WidgetType::CheckBox:
            case model::WidgetType::RadioButton:
            case model::WidgetType::ComboBox:
            case model::WidgetType::ListBox:
            case model::WidgetType::Slider:
            case model::WidgetType::ScrollBar:
            case model::WidgetType::TabControl:
            case model::WidgetType::TableGrid:
                focusableIds.push_back(node.id);
                break;
            default:
                break;
            }
        }
        for (const auto& child : node.children) {
            if (isChildVisibleInParent(node, child)) {
                self(self, child);
            }
        }
    };
    collectFocusable(collectFocusable, document.root);

    if (focusableIds.empty()) {
        previewFocusedWidgetId_.clear();
        return false;
    }

    auto iterator = std::find(focusableIds.begin(), focusableIds.end(), previewFocusedWidgetId_);
    if (iterator == focusableIds.end()) {
        previewFocusedWidgetId_ = reverse ? focusableIds.back() : focusableIds.front();
        return true;
    }

    if (reverse) {
        previewFocusedWidgetId_ = iterator == focusableIds.begin() ? focusableIds.back() : *std::prev(iterator);
    }
    else {
        ++iterator;
        previewFocusedWidgetId_ = iterator == focusableIds.end() ? focusableIds.front() : *iterator;
    }
    return true;
}

bool DesignerCanvas::activateFocusedPreviewWidget(const model::ProjectDocument& document)
{
    if (mode_ != Mode::Preview || previewFocusedWidgetId_.empty()) {
        return false;
    }

    const model::WidgetNode* widget = document.findWidgetById(previewFocusedWidgetId_);
    if (widget == nullptr || !widgetIsEnabled(*widget)) {
        previewFocusedWidgetId_.clear();
        return true;
    }

    switch (widget->type) {
    case model::WidgetType::Button:
        previewPressedWidgetId_.clear();
        return true;
    case model::WidgetType::CheckBox: {
        const bool current = previewChecked_.contains(widget->id)
            ? previewChecked_.at(widget->id)
            : widget->getBoolProperty("checked", false);
        previewChecked_[widget->id] = !current;
        return true;
    }
    case model::WidgetType::RadioButton: {
        const std::string group = widget->getStringProperty("group", "default");
        const auto selectGroup = [&](const auto& self, const model::WidgetNode& node) -> void {
            if (node.type == model::WidgetType::RadioButton
                && node.getStringProperty("group", "default") == group) {
                previewSelected_[node.id] = node.id == widget->id;
            }
            for (const auto& child : node.children) {
                self(self, child);
            }
        };
        selectGroup(selectGroup, document.root);
        return true;
    }
    case model::WidgetType::ComboBox:
        return false;
    case model::WidgetType::ListBox: {
        const auto items = model::splitItems(widget->getStringProperty("items", {}));
        if (items.empty()) {
            return true;
        }
        const int current = previewSelectedIndex(*widget,
            model::sanitizeSelectedIndex(items, widget->getIntProperty("selectedIndex", 0)));
        previewSelectedIndex_[widget->id] = (current + 1) % static_cast<int>(items.size());
        return true;
    }
    case model::WidgetType::TabControl: {
        const std::vector<std::string> labels = tabLabels(*widget);
        if (labels.empty()) {
            return true;
        }
        const int current = previewSelectedTab(*widget, selectedTabIndex(*widget));
        previewSelectedTab_[widget->id] = (current + 1) % static_cast<int>(labels.size());
        return true;
    }
    case model::WidgetType::TableGrid:
        return true;
    default:
        return false;
    }
}

const std::string& DesignerCanvas::previewFocusedWidgetId() const
{
    return previewFocusedWidgetId_;
}

void DesignerCanvas::clearPreviewInteraction()
{
    previewHoveredWidgetId_.clear();
    previewPressedWidgetId_.clear();
    previewFocusedWidgetId_.clear();
    previewChecked_.clear();
    previewSelected_.clear();
    previewSelectedIndex_.clear();
    previewSelectedTab_.clear();
    previewTableGridSelections_.clear();
    previewNumericValues_.clear();
    previewTextValues_.clear();
    previewDraggingValueWidgetId_.clear();
    previewTextOverlayWidgetId_.clear();
}

visual_style::State DesignerCanvas::resolvedVisualState(const model::WidgetNode& widget) const
{
    visual_style::State state;
    state.enabled = widgetIsEnabled(widget);
    state.readOnly = widget.getBoolProperty("readOnly", false);
    state.hovered = mode_ == Mode::Preview && previewHoveredWidgetId_ == widget.id;
    state.pressed = mode_ == Mode::Preview && previewPressedWidgetId_ == widget.id;
    state.focused = mode_ == Mode::Preview && previewFocusedWidgetId_ == widget.id;
    const bool checked = mode_ == Mode::Preview && previewChecked_.contains(widget.id)
        ? previewChecked_.at(widget.id)
        : widget.getBoolProperty("checked", false);
    const bool selected = mode_ == Mode::Preview && previewSelected_.contains(widget.id)
        ? previewSelected_.at(widget.id)
        : widget.getBoolProperty("selected", false);
    const bool itemSelected = mode_ == Mode::Preview
        && ((widget.type == model::WidgetType::ListBox && previewSelectedIndex_.contains(widget.id))
            || (widget.type == model::WidgetType::TabControl && previewSelectedTab_.contains(widget.id)));
    state.checkedOrSelected = checked || selected || itemSelected;
    state.active = state.checkedOrSelected;
    return state;
}

int DesignerCanvas::previewSelectedIndex(const model::WidgetNode& widget, int fallback) const
{
    const auto found = previewSelectedIndex_.find(widget.id);
    return mode_ == Mode::Preview && found != previewSelectedIndex_.end() ? found->second : fallback;
}

int DesignerCanvas::previewSelectedTab(const model::WidgetNode& widget, int fallback) const
{
    const auto found = previewSelectedTab_.find(widget.id);
    return mode_ == Mode::Preview && found != previewSelectedTab_.end() ? found->second : fallback;
}

model::TableGridSelection DesignerCanvas::previewTableGridSelection(const model::WidgetNode& widget,
    model::TableGridSelection fallback) const
{
    const auto found = previewTableGridSelections_.find(widget.id);
    return mode_ == Mode::Preview && found != previewTableGridSelections_.end() ? found->second : fallback;
}

float DesignerCanvas::previewNumericValue(const model::WidgetNode& widget, float fallback) const
{
    const auto found = previewNumericValues_.find(widget.id);
    return mode_ == Mode::Preview && found != previewNumericValues_.end() ? found->second : fallback;
}

std::string DesignerCanvas::previewText(const model::WidgetNode& widget, std::string fallback) const
{
    const auto found = previewTextValues_.find(widget.id);
    return mode_ == Mode::Preview && found != previewTextValues_.end() ? found->second : std::move(fallback);
}

bool DesignerCanvas::isPreviewTextOverlayWidget(const std::string& widgetId) const
{
    return mode_ == Mode::Preview && !widgetId.empty() && previewTextOverlayWidgetId_ == widgetId;
}

void DesignerCanvas::setPreviewSelectedIndex(const std::string& widgetId, int selectedIndex)
{
    if (mode_ != Mode::Preview || widgetId.empty()) {
        return;
    }
    previewSelectedIndex_[widgetId] = selectedIndex;
}

void DesignerCanvas::setPreviewText(const std::string& widgetId, std::string text)
{
    if (mode_ != Mode::Preview || widgetId.empty()) {
        return;
    }
    previewTextValues_[widgetId] = std::move(text);
}

void DesignerCanvas::setPreviewTextOverlayWidgetId(std::string widgetId)
{
    previewTextOverlayWidgetId_ = mode_ == Mode::Preview ? std::move(widgetId) : std::string{};
}

void DesignerCanvas::setShowGrid(bool showGrid)
{
    showGrid_ = showGrid;
}

void DesignerCanvas::setSnapToGrid(bool snapToGrid)
{
    snapToGrid_ = snapToGrid;
}

void DesignerCanvas::setGridSize(int gridSize)
{
    gridSize_ = std::max(1, gridSize);
}

void DesignerCanvas::setMajorGridSize(int majorGridSize)
{
    majorGridSize_ = std::max(1, majorGridSize);
}

bool DesignerCanvas::showGrid() const
{
    return showGrid_;
}

DesignerCanvas::Mode DesignerCanvas::mode() const
{
    return mode_;
}

bool DesignerCanvas::snapToGrid() const
{
    return snapToGrid_;
}

int DesignerCanvas::gridSize() const
{
    return gridSize_;
}

int DesignerCanvas::majorGridSize() const
{
    return majorGridSize_;
}

float DesignerCanvas::zoom() const
{
    return zoom_;
}

int DesignerCanvas::zoomPercent() const
{
    return static_cast<int>(std::lround(zoom_ * 100.0f));
}

bool DesignerCanvas::contains(float x, float y) const
{
    return x >= x_ && y >= y_ && x <= x_ + width_ && y <= y_ + height_;
}

bool DesignerCanvas::containsViewport(float x, float y) const
{
    if (!contains(x, y)) {
        return false;
    }

    const float top = y_ + (mode_ == Mode::Preview ? kPadding : kHeaderHeight + 12.0f);
    const float bottom = y_ + height_ - kPadding;
    return x >= x_ + kPadding && x <= x_ + width_ - kPadding && y >= top && y <= bottom;
}

DesignerCanvas::ViewPoint DesignerCanvas::viewportCenter() const
{
    const float top = y_ + (mode_ == Mode::Preview ? kPadding : kHeaderHeight + 12.0f);
    const float bottom = y_ + height_ - kPadding;
    return { x_ + width_ * 0.5f, top + std::max(0.0f, bottom - top) * 0.5f };
}

DesignerCanvas::FormPoint DesignerCanvas::viewToModelPoint(const model::ProjectDocument& document, float x, float y) const
{
    const PreviewLayout previewLayout = calculatePreviewLayout(
        x_, y_, width_, height_, document, mode_ == Mode::Preview, zoom_, panX_, panY_);
    if (previewLayout.scale <= 0.0f) {
        return {};
    }

    return {
        (x - previewLayout.form.x) / previewLayout.scale + document.root.bounds.x,
        (y - previewLayout.form.y) / previewLayout.scale + document.root.bounds.y
    };
}

DesignerCanvas::ViewPoint DesignerCanvas::modelToViewPoint(const model::ProjectDocument& document, float x, float y) const
{
    const PreviewLayout previewLayout = calculatePreviewLayout(
        x_, y_, width_, height_, document, mode_ == Mode::Preview, zoom_, panX_, panY_);
    return {
        previewLayout.form.x + (x - document.root.bounds.x) * previewLayout.scale,
        previewLayout.form.y + (y - document.root.bounds.y) * previewLayout.scale
    };
}

model::Rect DesignerCanvas::viewToModelRect(const model::ProjectDocument& document, const model::Rect& rect) const
{
    const FormPoint topLeft = viewToModelPoint(document, rect.x, rect.y);
    const FormPoint bottomRight = viewToModelPoint(document, rect.x + rect.width, rect.y + rect.height);
    return { topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y };
}

model::Rect DesignerCanvas::modelToViewRect(const model::ProjectDocument& document, const model::Rect& rect) const
{
    const ViewPoint topLeft = modelToViewPoint(document, rect.x, rect.y);
    const ViewPoint bottomRight = modelToViewPoint(document, rect.x + rect.width, rect.y + rect.height);
    return { topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y };
}

void DesignerCanvas::setZoomAround(const model::ProjectDocument& document, float zoom, float viewX, float viewY)
{
    if (!document.root.bounds.isValid()) {
        return;
    }

    const float nextZoom = std::clamp(zoom, kMinimumZoom, kMaximumZoom);
    if (std::abs(nextZoom - zoom_) < 0.0001f) {
        return;
    }

    const FormPoint anchor = viewToModelPoint(document, viewX, viewY);
    zoom_ = nextZoom;

    const PreviewLayout centeredLayout = calculatePreviewLayout(
        x_, y_, width_, height_, document, mode_ == Mode::Preview, zoom_, 0.0f, 0.0f);
    panX_ = viewX - centeredLayout.form.x - (anchor.x - document.root.bounds.x) * zoom_;
    panY_ = viewY - centeredLayout.form.y - (anchor.y - document.root.bounds.y) * zoom_;
}

void DesignerCanvas::resetView(const model::ProjectDocument&)
{
    zoom_ = 1.0f;
    panX_ = 0.0f;
    panY_ = 0.0f;
}

void DesignerCanvas::fitFormToCanvas(const model::ProjectDocument& document)
{
    const PreviewLayout centeredLayout = calculatePreviewLayout(
        x_, y_, width_, height_, document, mode_ == Mode::Preview, 1.0f, 0.0f, 0.0f);
    if (!centeredLayout.preview.isValid() || !document.root.bounds.isValid()) {
        return;
    }

    const float availableWidth = std::max(0.0f, centeredLayout.preview.width - kPreviewPadding * 2.0f);
    const float availableHeight = std::max(0.0f, centeredLayout.preview.height - kPreviewPadding * 2.0f);
    if (availableWidth <= 0.0f || availableHeight <= 0.0f) {
        return;
    }

    zoom_ = std::clamp(
        std::min(availableWidth / document.root.bounds.width, availableHeight / document.root.bounds.height),
        kMinimumZoom,
        kMaximumZoom);
    panX_ = 0.0f;
    panY_ = 0.0f;
}

void DesignerCanvas::panBy(float deltaX, float deltaY)
{
    panX_ += deltaX;
    panY_ += deltaY;
}

std::optional<DesignerCanvas::FormPoint> DesignerCanvas::toFormPoint(const model::ProjectDocument& document, float x, float y) const
{
    if (!containsViewport(x, y) || !document.root.bounds.isValid()) {
        return std::nullopt;
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(
        x_, y_, width_, height_, document, mode_ == Mode::Preview, zoom_, panX_, panY_);
    if (!previewLayout.form.contains(x, y) || previewLayout.scale <= 0.0f) {
        return std::nullopt;
    }

    return viewToModelPoint(document, x, y);
}

std::optional<std::string> DesignerCanvas::hitTestWidgetId(const model::ProjectDocument& document, float x, float y) const
{
    if (!containsViewport(x, y) || !document.root.bounds.isValid()) {
        return std::nullopt;
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(
        x_, y_, width_, height_, document, mode_ == Mode::Preview, zoom_, panX_, panY_);
    if (!previewLayout.form.contains(x, y) || previewLayout.scale <= 0.0f) {
        return std::nullopt;
    }

    if (auto widgetId = hitTestWidgetScreenId(document.root, previewLayout.form.x, previewLayout.form.y,
            -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale, x, y, smallWidgetHitPadding_)) {
        return widgetId;
    }

    return std::nullopt;
}

std::optional<int> DesignerCanvas::hitTestTabHeader(const model::ProjectDocument& document, const std::string& widgetId, float x, float y) const
{
    if (!containsViewport(x, y) || widgetId.empty() || !document.root.bounds.isValid()) {
        return std::nullopt;
    }

    const model::WidgetNode* widget = document.findWidgetById(widgetId);
    if (widget == nullptr || widget->type != model::WidgetType::TabControl) {
        return std::nullopt;
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(
        x_, y_, width_, height_, document, mode_ == Mode::Preview, zoom_, panX_, panY_);
    if (!previewLayout.form.isValid()) {
        return std::nullopt;
    }

    const auto widgetInfo = findWidgetScreenInfo(document.root, widgetId, previewLayout.form.x, previewLayout.form.y,
        -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale);
    if (!widgetInfo.has_value()) {
        return std::nullopt;
    }

    const float headerHeight = std::min(32.0f, std::max(24.0f, widgetInfo->bounds.height * 0.18f));
    const PanelRect headerBounds{ widgetInfo->bounds.x, widgetInfo->bounds.y, widgetInfo->bounds.width, headerHeight };
    if (!headerBounds.contains(x, y)) {
        return std::nullopt;
    }

    const std::vector<std::string> labels = tabLabels(*widget);
    const float tabWidth = headerBounds.width / static_cast<float>(std::max<std::size_t>(1, labels.size()));
    const int index = std::clamp(static_cast<int>((x - headerBounds.x) / std::max(1.0f, tabWidth)), 0, static_cast<int>(labels.size()) - 1);
    return index;
}

std::optional<DesignerCanvas::SelectionRect> DesignerCanvas::widgetScreenBounds(const model::ProjectDocument& document,
    const std::string& widgetId) const
{
    if (widgetId.empty() || !containsViewport(x_, y_) || !document.root.bounds.isValid()) {
        return std::nullopt;
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(
        x_, y_, width_, height_, document, mode_ == Mode::Preview, zoom_, panX_, panY_);
    if (!previewLayout.form.isValid()) {
        return std::nullopt;
    }

    const auto widgetInfo = findWidgetScreenInfo(document.root, widgetId, previewLayout.form.x, previewLayout.form.y,
        -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale);
    if (!widgetInfo.has_value()) {
        return std::nullopt;
    }

    return SelectionRect{
        widgetInfo->bounds.x,
        widgetInfo->bounds.y,
        widgetInfo->bounds.width,
        widgetInfo->bounds.height
    };
}

std::optional<DesignerCanvas::InteractionHit> DesignerCanvas::hitTestInteraction(const model::ProjectDocument& document,
    float x,
    float y,
    const std::string& selectedWidgetId) const
{
    if (!containsViewport(x, y)) {
        return std::nullopt;
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(
        x_, y_, width_, height_, document, mode_ == Mode::Preview, zoom_, panX_, panY_);
    if (!previewLayout.form.isValid()) {
        return std::nullopt;
    }

    const auto* selectedWidget = document.findWidgetById(selectedWidgetId);
    if (selectedWidget != nullptr && selectedWidget->type == model::WidgetType::TabPage) {
        return std::nullopt;
    }

    if (!selectedWidgetId.empty() && selectedWidgetId != document.root.id) {
        const auto selectedWidgetInfo = findWidgetScreenInfo(document.root, selectedWidgetId, previewLayout.form.x, previewLayout.form.y,
            -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale);
        if (selectedWidget != nullptr && selectedWidgetInfo.has_value() && showsDirectResizeAffordance(*selectedWidget)) {
            const HitRegion handle = hitHandle(selectedWidgetInfo->bounds, x, y);
            if (handle != HitRegion::None) {
                return InteractionHit{ selectedWidgetId, handle };
            }
        }
    }

    const auto hitWidgetId = hitTestWidgetId(document, x, y);
    if (!hitWidgetId.has_value()) {
        return std::nullopt;
    }

    const auto widgetInfo = findWidgetScreenInfo(document.root, *hitWidgetId, previewLayout.form.x, previewLayout.form.y,
        -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale);
    if (!widgetInfo.has_value()) {
        return std::nullopt;
    }

    InteractionHit hit{ *hitWidgetId, HitRegion::Body };
    if (*hitWidgetId == selectedWidgetId && *hitWidgetId != document.root.id) {
        const auto* selectedWidget = document.findWidgetById(*hitWidgetId);
        const HitRegion handle = selectedWidget != nullptr && showsDirectResizeAffordance(*selectedWidget)
            ? hitHandle(widgetInfo->bounds, x, y)
            : HitRegion::None;
        if (handle != HitRegion::None) {
            hit.region = handle;
            return hit;
        }
    }

    if (widgetInfo->bounds.contains(x, y)) {
        return hit;
    }

    return std::nullopt;
}

float DesignerCanvas::snap(float value) const
{
    if (!snapToGrid_ || gridSize_ <= 1) {
        return value;
    }

    return std::round(value / static_cast<float>(gridSize_)) * static_cast<float>(gridSize_);
}

model::Rect DesignerCanvas::moveBounds(const model::Rect& originalBounds, const FormPoint& dragStart, const FormPoint& currentPoint) const
{
    model::Rect updated = originalBounds;
    updated.x = originalBounds.x + (currentPoint.x - dragStart.x);
    updated.y = originalBounds.y + (currentPoint.y - dragStart.y);
    updated.x = snap(updated.x);
    updated.y = snap(updated.y);
    return updated;
}

model::Rect DesignerCanvas::resizeBounds(const model::Rect& originalBounds,
    HitRegion region,
    const FormPoint& dragStart,
    const FormPoint& currentPoint) const
{
    float left = originalBounds.x;
    float top = originalBounds.y;
    float right = originalBounds.x + originalBounds.width;
    float bottom = originalBounds.y + originalBounds.height;

    const float deltaX = currentPoint.x - dragStart.x;
    const float deltaY = currentPoint.y - dragStart.y;

    switch (region) {
    case HitRegion::TopLeftHandle:
        left = snap(left + deltaX);
        top = snap(top + deltaY);
        break;
    case HitRegion::TopRightHandle:
        right = snap(right + deltaX);
        top = snap(top + deltaY);
        break;
    case HitRegion::BottomLeftHandle:
        left = snap(left + deltaX);
        bottom = snap(bottom + deltaY);
        break;
    case HitRegion::BottomRightHandle:
        right = snap(right + deltaX);
        bottom = snap(bottom + deltaY);
        break;
    case HitRegion::None:
    case HitRegion::Body:
        return originalBounds;
    }

    if (right - left < minimumWidgetSize_) {
        if (region == HitRegion::TopLeftHandle || region == HitRegion::BottomLeftHandle) {
            left = right - minimumWidgetSize_;
        }
        else {
            right = left + minimumWidgetSize_;
        }
    }

    if (bottom - top < minimumWidgetSize_) {
        if (region == HitRegion::TopLeftHandle || region == HitRegion::TopRightHandle) {
            top = bottom - minimumWidgetSize_;
        }
        else {
            bottom = top + minimumWidgetSize_;
        }
    }

    return { left, top, std::max(minimumWidgetSize_, right - left), std::max(minimumWidgetSize_, bottom - top) };
}

void DesignerCanvas::draw(visage::Canvas& canvas,
    const visage::Font& font,
    bool drawText,
    const model::ProjectDocument& document,
    resources::ImageResourceCache* imageCache,
    bool simplifySelectedImages,
    const std::optional<SelectionRect>& marqueeRect,
    const std::vector<SmartGuide>& smartGuides,
    const std::optional<model::WidgetAppearanceState>& appearancePreviewState) const
{
    (void)simplifySelectedImages;

    if (width_ <= 0.0f || height_ <= 0.0f) {
        return;
    }

    canvas.setColor(0xff1f242d);
    canvas.fill(x_, y_, width_, height_);

    const bool showEditorDecorations = mode_ == Mode::Design;
    if (showEditorDecorations) {
        canvas.setColor(0xff2a303a);
        canvas.fill(x_, y_, width_, kHeaderHeight);
    }

    canvas.setColor(0xff101318);
    canvas.fill(x_, y_, width_, 1.0f);
    canvas.fill(x_, y_ + height_ - 1.0f, width_, 1.0f);
    canvas.fill(x_, y_, 1.0f, height_);
    canvas.fill(x_ + width_ - 1.0f, y_, 1.0f, height_);

    if (drawText && showEditorDecorations) {
        canvas.setColor(0xfff3f5f8);
        canvas.text("Designer Canvas", font, visage::Font::kTopLeft,
            x_ + kPadding, y_ + 6.0f, width_ - kPadding * 2.0f, kHeaderHeight - 8.0f);
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(
        x_, y_, width_, height_, document, mode_ == Mode::Preview, zoom_, panX_, panY_);
    if (!previewLayout.preview.isValid()) {
        return;
    }

    if (showEditorDecorations) {
        canvas.setColor(0xff303746);
        canvas.fill(previewLayout.preview.x, previewLayout.preview.y, previewLayout.preview.width, previewLayout.preview.height);
        canvas.setColor(0xff475064);
        canvas.fill(previewLayout.preview.x + 1.0f, previewLayout.preview.y + 1.0f,
            previewLayout.preview.width - 2.0f, previewLayout.preview.height - 2.0f);
    }

    if (!previewLayout.form.isValid()) {
        return;
    }

    canvas.saveState();
    canvas.trimClampBounds(previewLayout.preview.x, previewLayout.preview.y, previewLayout.preview.width, previewLayout.preview.height);

    drawWidget(canvas, font, drawText, document, *this, imageCache, simplifySelectedImages, document.root, previewLayout.form.x, previewLayout.form.y,
        -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale, document.selectedWidgetId,
        showEditorDecorations && showGrid_, showMinorGrid_, gridSize_, majorGridSize_,
        showEditorDecorations, appearancePreviewState);

    if (showEditorDecorations && marqueeRect.has_value()) {
        const PanelRect screenRect = selectionRectToScreenRect(previewLayout, *marqueeRect);
        canvas.setColor(0x226fa9ff);
        canvas.fill(screenRect.x, screenRect.y, screenRect.width, screenRect.height);
        canvas.setColor(0xff6fa9ff);
        canvas.fill(screenRect.x, screenRect.y, screenRect.width, 1.0f);
        canvas.fill(screenRect.x, screenRect.y + screenRect.height - 1.0f, screenRect.width, 1.0f);
        canvas.fill(screenRect.x, screenRect.y, 1.0f, screenRect.height);
        canvas.fill(screenRect.x + screenRect.width - 1.0f, screenRect.y, 1.0f, screenRect.height);
    }

    if (showEditorDecorations) {
        for (const auto& guide : smartGuides) {
            canvas.setColor(0xffff6b2c);
            if (guide.orientation == GuideOrientation::Vertical) {
                const float x = previewLayout.form.x + guide.position * previewLayout.scale;
                canvas.fill(x, previewLayout.form.y, 1.0f, previewLayout.form.height);
            }
            else {
                const float y = previewLayout.form.y + guide.position * previewLayout.scale;
                canvas.fill(previewLayout.form.x, y, previewLayout.form.width, 1.0f);
            }
        }
    }

    canvas.restoreState();

    if (drawText && showEditorDecorations) {
        canvas.setColor(0xff243041);
        const std::string selectedLabel = document.hasSelection() && document.selectedWidget() != nullptr
            ? "Selected: " + widgetLabel(*document.selectedWidget())
            : "Selected: None";
        canvas.text(selectedLabel, font, visage::Font::kTopLeft,
            previewLayout.preview.x + 8.0f, previewLayout.preview.y + previewLayout.preview.height - 28.0f,
            previewLayout.preview.width - 16.0f, 22.0f);
    }
}

} // namespace visiform::ui
