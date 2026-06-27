#include "ui/editors/TextEditControl.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string_view>
#include <vector>

#include <visage_utils/string_utils.h>
#include <visage_windowing/windowing.h>

namespace visiform::ui::editors {
namespace {

constexpr float kHorizontalPadding = 8.0f;
constexpr float kVerticalPadding = 4.0f;
constexpr float kEstimatedTextHeight = 18.0f;
constexpr float kApproximateCharacterWidth = 8.0f;
constexpr float kApproximateLineHeight = 20.0f;
constexpr float kDefaultDpiScale = 1.0f;

float normalizedDpiScale(float dpiScale)
{
    return std::isfinite(dpiScale) && dpiScale > 0.0f ? dpiScale : kDefaultDpiScale;
}

bool isPrintableInput(const std::string& text)
{
    return std::any_of(text.begin(), text.end(), [](unsigned char character) {
        return character >= 32 && character != 127;
    });
}

bool isUtf8ContinuationByte(unsigned char character)
{
    return (character & 0xc0) == 0x80;
}

std::string normalizePastedText(std::string text, bool multiline)
{
    std::string result;
    result.reserve(text.size());
    for (std::size_t index = 0; index < text.size(); ++index) {
        const char character = text[index];
        if (character == '\r') {
            if (index + 1 < text.size() && text[index + 1] == '\n') {
                ++index;
            }
            result.push_back(multiline ? '\n' : ' ');
            continue;
        }
        if (character == '\n') {
            result.push_back(multiline ? '\n' : ' ');
            continue;
        }
        result.push_back(character);
    }
    return result;
}

std::size_t previousTextBoundary(std::string_view text, std::size_t index)
{
    std::size_t boundary = std::min(index, text.size());
    if (boundary == 0) {
        return 0;
    }

    --boundary;
    while (boundary > 0 && isUtf8ContinuationByte(static_cast<unsigned char>(text[boundary]))) {
        --boundary;
    }
    return boundary;
}

std::size_t nextTextBoundary(std::string_view text, std::size_t index)
{
    std::size_t boundary = std::min(index, text.size());
    if (boundary >= text.size()) {
        return text.size();
    }

    ++boundary;
    while (boundary < text.size() && isUtf8ContinuationByte(static_cast<unsigned char>(text[boundary]))) {
        ++boundary;
    }
    return boundary;
}

} // namespace

void TextEditControl::setBounds(float x, float y, float width, float height)
{
    bounds_ = { x, y, width, height };
    ensureCursorVisible();
}

const TextEditControl::Bounds& TextEditControl::bounds() const
{
    return bounds_;
}

bool TextEditControl::contains(float x, float y) const
{
    return editing_ && bounds_.contains(x, y);
}

void TextEditControl::begin(std::string text, bool selectAllText, bool multiline, bool wordWrap, bool readOnly)
{
    originalText_ = text;
    text_ = std::move(text);
    editing_ = true;
    focused_ = true;
    multiline_ = multiline;
    wordWrap_ = multiline && wordWrap;
    readOnly_ = readOnly;
    draggingSelection_ = false;
    caretVisible_ = true;
    pendingAction_ = PendingAction::None;
    cursorIndex_ = text_.size();
    selectionStart_ = cursorIndex_;
    selectionEnd_ = cursorIndex_;
    scrollX_ = 0.0f;
    scrollY_ = 0.0f;
    preferredCursorX_ = -1.0f;
    if (selectAllText) {
        selectAll();
    }
    else {
        ensureCursorVisible();
    }
}

void TextEditControl::clear()
{
    editing_ = false;
    focused_ = false;
    pendingAction_ = PendingAction::None;
    originalText_.clear();
    text_.clear();
    cursorIndex_ = 0;
    selectionStart_ = 0;
    selectionEnd_ = 0;
    scrollX_ = 0.0f;
    scrollY_ = 0.0f;
    multiline_ = false;
    wordWrap_ = false;
    readOnly_ = false;
    draggingSelection_ = false;
    caretVisible_ = false;
    preferredCursorX_ = -1.0f;
}

bool TextEditControl::isActive() const
{
    return editing_;
}

bool TextEditControl::isFocused() const
{
    return editing_ && focused_;
}

bool TextEditControl::isMultiline() const
{
    return editing_ && multiline_;
}

void TextEditControl::setText(std::string text)
{
    text_ = std::move(text);
    cursorIndex_ = std::min(cursorIndex_, text_.size());
    selectionStart_ = std::min(selectionStart_, text_.size());
    selectionEnd_ = std::min(selectionEnd_, text_.size());
    ensureCursorVisible();
}

const std::string& TextEditControl::text() const
{
    return text_;
}

TextEditControl::State TextEditControl::state() const
{
    return {
        text_,
        cursorIndex_,
        selectionStart_,
        selectionEnd_,
        scrollX_,
        scrollY_,
        preferredCursorX_
    };
}

void TextEditControl::restoreState(const State& state)
{
    text_ = state.text;
    cursorIndex_ = std::min(state.cursorIndex, text_.size());
    selectionStart_ = std::min(state.selectionStart, text_.size());
    selectionEnd_ = std::min(state.selectionEnd, text_.size());
    scrollX_ = std::max(0.0f, state.scrollX);
    scrollY_ = std::max(0.0f, state.scrollY);
    preferredCursorX_ = state.preferredCursorX;
    ensureCursorVisible();
}

void TextEditControl::selectAll()
{
    if (!editing_) {
        return;
    }

    selectionStart_ = 0;
    selectionEnd_ = text_.size();
    cursorIndex_ = text_.size();
    ensureCursorVisible();
}

bool TextEditControl::mouseDown(float x, float y)
{
    if (!editing_) {
        return false;
    }

    focused_ = bounds_.contains(x, y);
    if (!focused_) {
        caretVisible_ = false;
        return false;
    }

    const std::size_t hitIndex = hitTestCharacterIndex(x, y);
    cursorIndex_ = hitIndex;
    selectionStart_ = hitIndex;
    selectionEnd_ = hitIndex;
    ensureCursorVisible();
    draggingSelection_ = true;
    noteEditingInteraction();
    return true;
}

bool TextEditControl::mouseDrag(float x, float y)
{
    if (!editing_ || !focused_ || !draggingSelection_) {
        return false;
    }

    const std::size_t hitIndex = hitTestCharacterIndex(x, y);
    moveCursor(hitIndex, true);
    return true;
}

bool TextEditControl::mouseUp()
{
    const bool wasDragging = draggingSelection_;
    draggingSelection_ = false;
    return wasDragging;
}

bool TextEditControl::mouseWheel(float deltaY, float x, float y)
{
    if (!editing_ || !focused_ || !multiline_ || !bounds_.contains(x, y)) {
        return false;
    }

    const float innerHeight = std::max(0.0f, bounds_.height - kVerticalPadding * 2.0f);
    const float maxVerticalScroll = std::max(0.0f, static_cast<float>(lineCount()) * lineAdvance() - innerHeight);
    if (maxVerticalScroll <= 0.0f) {
        return false;
    }

    const float direction = deltaY > 0.0f ? -1.0f : 1.0f;
    scrollY_ = std::clamp(scrollY_ + direction * lineAdvance() * 3.0f, 0.0f, maxVerticalScroll);
    noteEditingInteraction();
    return true;
}

bool TextEditControl::keyPress(const visage::KeyEvent& event)
{
    if (!editing_ || !focused_) {
        return false;
    }

    using KeyCode = visage::KeyCode;
    const bool extending = event.isShiftDown();
    const bool control = event.isCtrlDown() || event.isCmdDown() || event.isMetaDown();
    switch (event.keyCode()) {
    case KeyCode::Backspace:
        if (readOnly_) {
            return true;
        }
        if (hasSelection()) {
            deleteSelection();
        }
        else if (cursorIndex_ > 0) {
            const std::size_t eraseStart = previousTextBoundary(text_, cursorIndex_);
            text_.erase(eraseStart, cursorIndex_ - eraseStart);
            cursorIndex_ = eraseStart;
            selectionStart_ = cursorIndex_;
            selectionEnd_ = cursorIndex_;
        }
        ensureCursorVisible();
        noteEditingInteraction();
        return true;
    case KeyCode::Delete:
        if (readOnly_) {
            return true;
        }
        if (hasSelection()) {
            deleteSelection();
        }
        else if (cursorIndex_ < text_.size()) {
            text_.erase(cursorIndex_, nextTextBoundary(text_, cursorIndex_) - cursorIndex_);
        }
        ensureCursorVisible();
        noteEditingInteraction();
        return true;
    case KeyCode::A:
        if (control) {
            selectAll();
            noteEditingInteraction();
            return true;
        }
        return false;
    case KeyCode::C:
        if (control) {
            if (hasSelection()) {
                visage::setClipboardText(text_.substr(selectionMin(), selectionMax() - selectionMin()));
            }
            noteEditingInteraction();
            return true;
        }
        return false;
    case KeyCode::X:
        if (control) {
            if (!readOnly_ && hasSelection()) {
                visage::setClipboardText(text_.substr(selectionMin(), selectionMax() - selectionMin()));
                deleteSelection();
                ensureCursorVisible();
            }
            noteEditingInteraction();
            return true;
        }
        return false;
    case KeyCode::V:
        if (control) {
            if (!readOnly_) {
                const std::string clipboardText = normalizePastedText(visage::readClipboardText(), multiline_);
                if (!clipboardText.empty()) {
                    insertText(clipboardText);
                }
            }
            noteEditingInteraction();
            return true;
        }
        return false;
    case KeyCode::Left:
        preferredCursorX_ = -1.0f;
        if (hasSelection() && !extending) {
            moveCursor(selectionMin());
        }
        else if (cursorIndex_ > 0) {
            moveCursor(previousTextBoundary(text_, cursorIndex_), extending);
        }
        return true;
    case KeyCode::Right:
        preferredCursorX_ = -1.0f;
        if (hasSelection() && !extending) {
            moveCursor(selectionMax());
        }
        else if (cursorIndex_ < text_.size()) {
            moveCursor(nextTextBoundary(text_, cursorIndex_), extending);
        }
        return true;
    case KeyCode::Home:
        preferredCursorX_ = -1.0f;
        moveCursor(control ? 0 : (multiline_ ? lineStartForIndex(cursorIndex_) : 0), extending);
        return true;
    case KeyCode::End:
        preferredCursorX_ = -1.0f;
        moveCursor(control ? text_.size() : (multiline_ ? lineEndForIndex(cursorIndex_) : text_.size()), extending);
        return true;
    case KeyCode::Return:
        if (readOnly_) {
            return true;
        }
        if (multiline_) {
            insertText("\n");
        }
        else {
            pendingAction_ = PendingAction::Commit;
        }
        return true;
    case KeyCode::Up:
        if (multiline_) {
            const std::size_t lineIndex = currentLineIndex();
            if (preferredCursorX_ < 0.0f) {
                preferredCursorX_ = cursorOffsetForIndex(cursorIndex_);
            }
            if (lineIndex > 0) {
                moveCursor(indexForLineOffset(lineIndex - 1, preferredCursorX_), extending);
            }
            return true;
        }
        return false;
    case KeyCode::Down:
        if (multiline_) {
            const std::size_t lineIndex = currentLineIndex();
            if (preferredCursorX_ < 0.0f) {
                preferredCursorX_ = cursorOffsetForIndex(cursorIndex_);
            }
            if (lineIndex + 1 < lineCount()) {
                moveCursor(indexForLineOffset(lineIndex + 1, preferredCursorX_), extending);
            }
            return true;
        }
        return true;
    case KeyCode::Escape:
        text_ = originalText_;
        cursorIndex_ = text_.size();
        selectionStart_ = cursorIndex_;
        selectionEnd_ = cursorIndex_;
        ensureCursorVisible();
        noteEditingInteraction();
        pendingAction_ = PendingAction::Cancel;
        return true;
    default:
        return false;
    }
}

bool TextEditControl::textInput(const std::string& text)
{
    if (!editing_ || !focused_ || !isPrintableInput(text)) {
        return false;
    }
    if (readOnly_) {
        return true;
    }

    insertText(text);
    return true;
}

void TextEditControl::showCaret()
{
    if (editing_ && focused_) {
        caretVisible_ = true;
    }
}

void TextEditControl::toggleCaretVisibility()
{
    if (editing_ && focused_) {
        caretVisible_ = !caretVisible_;
    }
}

bool TextEditControl::shouldBlinkCaret() const
{
    return editing_ && focused_;
}

bool TextEditControl::isCaretVisible() const
{
    return editing_ && focused_ && caretVisible_;
}

std::optional<TextEditControl::PendingAction> TextEditControl::consumePendingAction()
{
    if (pendingAction_ == PendingAction::None) {
        return std::nullopt;
    }

    const PendingAction action = pendingAction_;
    pendingAction_ = PendingAction::None;
    return action;
}

void TextEditControl::setMetricsFont(const visage::Font& font, float dpiScale)
{
    if (font.packedFont() == nullptr) {
        metricsFont_.reset();
        return;
    }

    metricsFont_ = font.withDpiScale(normalizedDpiScale(dpiScale));
    if (editing_) {
        ensureCursorVisible();
    }
}

void TextEditControl::draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const
{
    if (!editing_ || !drawText || bounds_.width <= 0.0f || bounds_.height <= 0.0f) {
        return;
    }

    metricsFont_ = font.withDpiScale(normalizedDpiScale(canvas.dpiScale()));
    const float textLeft = bounds_.x + kHorizontalPadding;
    const float textWidth = std::max(0.0f, bounds_.width - kHorizontalPadding * 2.0f);
    const float textHeight = std::min(kEstimatedTextHeight, std::max(0.0f, bounds_.height - kVerticalPadding * 2.0f));
    const float textTop = multiline_
        ? bounds_.y + kVerticalPadding
        : bounds_.y + std::max(kVerticalPadding, (bounds_.height - textHeight) * 0.5f);

    canvas.saveState();
    canvas.setClampBounds(bounds_.x + 1.0f, bounds_.y + 1.0f, std::max(0.0f, bounds_.width - 2.0f), std::max(0.0f, bounds_.height - 2.0f));
    if (!multiline_ && hasSelection()) {
        const float selectionLeft = textLeft - scrollX_ + cursorOffsetForIndex(selectionMin());
        const float selectionWidth = std::max(0.0f, cursorOffsetForIndex(selectionMax()) - cursorOffsetForIndex(selectionMin()));
        canvas.setColor(0x66355382);
        canvas.fill(selectionLeft, bounds_.y + kVerticalPadding, selectionWidth, std::max(0.0f, bounds_.height - kVerticalPadding * 2.0f));
    }

    canvas.setColor(0xffeef2f8);
    if (multiline_) {
        const auto ranges = visualLineRanges();
        float lineTop = textTop - scrollY_;
        for (const auto& [lineStart, safeLineEnd] : ranges) {
            const std::string lineText = text_.substr(lineStart, safeLineEnd - lineStart);
            if (hasSelection()) {
                const std::size_t selectedStart = std::max(selectionMin(), lineStart);
                const std::size_t selectedEnd = std::min(selectionMax(), safeLineEnd);
                if (selectedStart < selectedEnd
                    || (selectionMax() > safeLineEnd && selectionMin() <= safeLineEnd && safeLineEnd < text_.size() && text_[safeLineEnd] == '\n')) {
                    const float left = textLeft - scrollX_ + measuredTextWidth(lineStart, selectedStart);
                    const float right = selectedStart < selectedEnd
                        ? textLeft - scrollX_ + measuredTextWidth(lineStart, selectedEnd)
                        : left + 4.0f;
                    canvas.setColor(0x66355382);
                    canvas.fill(left, lineTop, std::max(1.0f, right - left), std::max(textHeight, lineAdvance() - 2.0f));
                    canvas.setColor(0xffeef2f8);
                }
            }
            canvas.text(lineText, font, visage::Font::kTopLeft, textLeft - scrollX_, lineTop, std::max(textWidth + scrollX_, textWidth), textHeight);
            lineTop += lineAdvance();
        }

        if (isCaretVisible()) {
            const std::size_t caretLineIndex = currentLineIndex();
            const float cursorX = textLeft - scrollX_ + cursorOffsetForIndex(cursorIndex_);
            const float cursorY = textTop - scrollY_ + static_cast<float>(caretLineIndex) * lineAdvance();
            canvas.setColor(0xff92b9ff);
            canvas.fill(cursorX, cursorY, 1.0f, std::max(textHeight, lineAdvance() - 2.0f));
        }
    }
    else {
        const float cursorX = textLeft - scrollX_ + cursorOffsetForIndex(cursorIndex_);
        canvas.text(text_, font, visage::Font::kTopLeft, textLeft - scrollX_, textTop, std::max(textWidth + scrollX_, textWidth), textHeight);
        if (isCaretVisible()) {
            canvas.setColor(0xff92b9ff);
            canvas.fill(cursorX, bounds_.y + kVerticalPadding, 1.0f, std::max(0.0f, bounds_.height - kVerticalPadding * 2.0f));
        }
    }
    canvas.restoreState();
}

bool TextEditControl::hasSelection() const
{
    return selectionStart_ != selectionEnd_;
}

std::size_t TextEditControl::selectionMin() const
{
    return std::min(selectionStart_, selectionEnd_);
}

std::size_t TextEditControl::selectionMax() const
{
    return std::max(selectionStart_, selectionEnd_);
}

void TextEditControl::clearSelection()
{
    selectionStart_ = cursorIndex_;
    selectionEnd_ = cursorIndex_;
}

void TextEditControl::deleteSelection()
{
    if (!hasSelection()) {
        return;
    }

    const std::size_t start = selectionMin();
    const std::size_t length = selectionMax() - start;
    text_.erase(start, length);
    cursorIndex_ = start;
    clearSelection();
    noteEditingInteraction();
}

void TextEditControl::insertText(const std::string& text)
{
    if (hasSelection()) {
        deleteSelection();
    }

    text_.insert(cursorIndex_, text);
    cursorIndex_ += text.size();
    clearSelection();
    ensureCursorVisible();
    preferredCursorX_ = -1.0f;
    noteEditingInteraction();
}

void TextEditControl::moveCursor(std::size_t index, bool extendSelection)
{
    cursorIndex_ = std::min(index, text_.size());
    while (cursorIndex_ > 0 && cursorIndex_ < text_.size()
        && isUtf8ContinuationByte(static_cast<unsigned char>(text_[cursorIndex_]))) {
        --cursorIndex_;
    }
    if (extendSelection) {
        selectionEnd_ = cursorIndex_;
    }
    else {
        clearSelection();
    }
    ensureCursorVisible();
    if (!extendSelection) {
        preferredCursorX_ = -1.0f;
    }
    noteEditingInteraction();
}

void TextEditControl::ensureCursorVisible()
{
    const float innerWidth = std::max(0.0f, bounds_.width - kHorizontalPadding * 2.0f);
    const float cursorX = cursorOffsetForIndex(cursorIndex_);
    if (wordWrap_) {
        scrollX_ = 0.0f;
    }
    else if (cursorX < scrollX_) {
        scrollX_ = cursorX;
    }
    else if (cursorX > scrollX_ + innerWidth) {
        scrollX_ = std::max(0.0f, cursorX - innerWidth);
    }

    const float maxScroll = wordWrap_ ? 0.0f : std::max(0.0f, cursorOffsetForIndex(lineEndForIndex(cursorIndex_)) - innerWidth);
    scrollX_ = std::clamp(scrollX_, 0.0f, maxScroll);

    if (multiline_) {
        const float innerHeight = std::max(0.0f, bounds_.height - kVerticalPadding * 2.0f);
        const float cursorY = static_cast<float>(currentLineIndex()) * lineAdvance();
        if (cursorY < scrollY_) {
            scrollY_ = cursorY;
        }
        else if (cursorY > scrollY_ + innerHeight - lineAdvance()) {
            scrollY_ = std::max(0.0f, cursorY - innerHeight + lineAdvance());
        }

        const float maxVerticalScroll = std::max(0.0f, static_cast<float>(lineCount()) * lineAdvance() - innerHeight);
        scrollY_ = std::clamp(scrollY_, 0.0f, maxVerticalScroll);
    }
    else {
        scrollY_ = 0.0f;
    }
}

void TextEditControl::noteEditingInteraction()
{
    showCaret();
}

std::size_t TextEditControl::lineStartForIndex(std::size_t index) const
{
    if (wordWrap_) {
        const std::size_t safeIndex = std::min(index, text_.size());
        for (const auto& [start, end] : visualLineRanges()) {
            if (safeIndex >= start && safeIndex <= end) {
                return start;
            }
        }
    }

    const std::size_t safeIndex = std::min(index, text_.size());
    const std::size_t searchStart = safeIndex == 0 ? 0 : safeIndex - 1;
    const std::size_t newline = text_.rfind('\n', searchStart);
    if (newline == std::string::npos) {
        return 0;
    }

    return newline + 1;
}

std::size_t TextEditControl::lineEndForIndex(std::size_t index) const
{
    if (wordWrap_) {
        const std::size_t safeIndex = std::min(index, text_.size());
        for (const auto& [start, end] : visualLineRanges()) {
            if (safeIndex >= start && safeIndex <= end) {
                return end;
            }
        }
    }

    const std::size_t safeIndex = std::min(index, text_.size());
    const std::size_t newline = text_.find('\n', safeIndex);
    return newline == std::string::npos ? text_.size() : newline;
}

std::size_t TextEditControl::hitTestCharacterIndex(float x, float y) const
{
    const float relativeX = std::max(0.0f, x - bounds_.x - kHorizontalPadding + scrollX_);
    if (!multiline_) {
        const auto boundaries = lineBoundaries(0, text_.size());
        for (std::size_t i = 0; i + 1 < boundaries.size(); ++i) {
            const float left = measuredTextWidth(0, boundaries[i]);
            const float right = measuredTextWidth(0, boundaries[i + 1]);
            if (relativeX < left + (right - left) * 0.5f) {
                return boundaries[i];
            }
        }
        return text_.size();
    }

    const float relativeY = std::max(0.0f, y - bounds_.y - kVerticalPadding + scrollY_);
    const std::size_t lineIndex = std::min<std::size_t>(
        static_cast<std::size_t>(std::floor(relativeY / lineAdvance())),
        lineCount() == 0 ? 0 : lineCount() - 1);
    const auto ranges = visualLineRanges();
    const std::size_t lineStart = lineIndex < ranges.size() ? ranges[lineIndex].first : 0;
    const std::size_t lineEnd = lineIndex < ranges.size() ? ranges[lineIndex].second : text_.size();
    const auto boundaries = lineBoundaries(lineStart, lineEnd);
    for (std::size_t i = 0; i + 1 < boundaries.size(); ++i) {
        const float left = measuredTextWidth(lineStart, boundaries[i]);
        const float right = measuredTextWidth(lineStart, boundaries[i + 1]);
        if (relativeX < left + (right - left) * 0.5f) {
            return boundaries[i];
        }
    }
    return lineEnd;
}

std::size_t TextEditControl::currentLineIndex() const
{
    if (wordWrap_) {
        const std::size_t safeIndex = std::min(cursorIndex_, text_.size());
        const auto ranges = visualLineRanges();
        for (std::size_t index = 0; index < ranges.size(); ++index) {
            if (safeIndex >= ranges[index].first && safeIndex <= ranges[index].second) {
                return index;
            }
        }
        return ranges.empty() ? 0 : ranges.size() - 1;
    }

    return static_cast<std::size_t>(std::count(text_.begin(), text_.begin() + static_cast<std::ptrdiff_t>(std::min(cursorIndex_, text_.size())), '\n'));
}

std::size_t TextEditControl::currentColumnIndex() const
{
    return std::min(cursorIndex_, text_.size()) - lineStartForIndex(cursorIndex_);
}

std::size_t TextEditControl::lineCount() const
{
    if (wordWrap_) {
        return std::max<std::size_t>(1, visualLineRanges().size());
    }

    return static_cast<std::size_t>(std::count(text_.begin(), text_.end(), '\n')) + 1;
}

std::size_t TextEditControl::indexForLineColumn(std::size_t lineIndex, std::size_t columnIndex) const
{
    if (wordWrap_) {
        const auto ranges = visualLineRanges();
        if (lineIndex >= ranges.size()) {
            return text_.size();
        }
        return std::min(ranges[lineIndex].first + columnIndex, ranges[lineIndex].second);
    }

    std::size_t currentLine = 0;
    std::size_t currentIndex = 0;
    while (currentLine < lineIndex && currentIndex < text_.size()) {
        const std::size_t newline = text_.find('\n', currentIndex);
        if (newline == std::string::npos) {
            currentIndex = text_.size();
            break;
        }

        currentIndex = newline + 1;
        ++currentLine;
    }

    if (currentLine < lineIndex) {
        return text_.size();
    }

    const std::size_t lineEnd = lineEndForIndex(currentIndex);
    return std::min(currentIndex + columnIndex, lineEnd);
}

std::size_t TextEditControl::indexForLineOffset(std::size_t lineIndex, float offset) const
{
    const std::size_t lineStart = indexForLineColumn(lineIndex, 0);
    const std::size_t lineEnd = lineEndForIndex(lineStart);
    const auto boundaries = lineBoundaries(lineStart, lineEnd);
    for (std::size_t i = 0; i + 1 < boundaries.size(); ++i) {
        const float left = measuredTextWidth(lineStart, boundaries[i]);
        const float right = measuredTextWidth(lineStart, boundaries[i + 1]);
        if (offset < left + (right - left) * 0.5f) {
            return boundaries[i];
        }
    }
    return lineEnd;
}

std::vector<std::size_t> TextEditControl::lineBoundaries(std::size_t lineStart, std::size_t lineEnd) const
{
    const std::size_t safeStart = std::min(lineStart, text_.size());
    const std::size_t safeEnd = std::min(std::max(lineEnd, safeStart), text_.size());
    std::vector<std::size_t> boundaries;
    boundaries.push_back(safeStart);
    std::size_t index = safeStart;
    while (index < safeEnd) {
        index = nextTextBoundary(text_, index);
        boundaries.push_back(std::min(index, safeEnd));
    }
    if (boundaries.back() != safeEnd) {
        boundaries.push_back(safeEnd);
    }
    return boundaries;
}

std::vector<std::pair<std::size_t, std::size_t>> TextEditControl::visualLineRanges() const
{
    std::vector<std::pair<std::size_t, std::size_t>> ranges;
    if (!wordWrap_) {
        std::size_t lineStart = 0;
        while (lineStart <= text_.size()) {
            const std::size_t lineEnd = text_.find('\n', lineStart);
            const std::size_t safeLineEnd = lineEnd == std::string::npos ? text_.size() : lineEnd;
            ranges.emplace_back(lineStart, safeLineEnd);
            if (lineEnd == std::string::npos) {
                break;
            }
            lineStart = lineEnd + 1;
        }
        return ranges;
    }

    const float availableWidth = std::max(1.0f, bounds_.width - kHorizontalPadding * 2.0f);
    std::size_t logicalStart = 0;
    while (logicalStart <= text_.size()) {
        const std::size_t logicalEndMarker = text_.find('\n', logicalStart);
        const std::size_t logicalEnd = logicalEndMarker == std::string::npos ? text_.size() : logicalEndMarker;
        std::size_t visualStart = logicalStart;
        while (visualStart < logicalEnd) {
            std::size_t visualEnd = nextTextBoundary(text_, visualStart);
            std::size_t bestEnd = visualEnd;
            std::size_t lastSpaceEnd = std::string::npos;
            while (visualEnd <= logicalEnd) {
                if (visualEnd > visualStart && std::isspace(static_cast<unsigned char>(text_[visualEnd - 1])) != 0) {
                    lastSpaceEnd = visualEnd;
                }
                if (measuredTextWidth(visualStart, visualEnd) > availableWidth) {
                    if (lastSpaceEnd != std::string::npos && lastSpaceEnd > visualStart) {
                        bestEnd = lastSpaceEnd;
                    }
                    break;
                }
                bestEnd = visualEnd;
                if (visualEnd == logicalEnd) {
                    break;
                }
                visualEnd = nextTextBoundary(text_, visualEnd);
            }
            ranges.emplace_back(visualStart, std::max(bestEnd, nextTextBoundary(text_, visualStart)));
            visualStart = ranges.back().second;
            while (visualStart < logicalEnd && std::isspace(static_cast<unsigned char>(text_[visualStart])) != 0) {
                ++visualStart;
            }
        }
        if (logicalStart == logicalEnd) {
            ranges.emplace_back(logicalEnd, logicalEnd);
        }
        if (logicalEndMarker == std::string::npos) {
            break;
        }
        logicalStart = logicalEndMarker + 1;
    }
    if (ranges.empty()) {
        ranges.emplace_back(0, 0);
    }
    return ranges;
}

float TextEditControl::measuredTextWidth(std::size_t start, std::size_t end) const
{
    const std::size_t safeStart = std::min(start, text_.size());
    const std::size_t safeEnd = std::min(std::max(end, safeStart), text_.size());
    if (safeStart == safeEnd) {
        return 0.0f;
    }

    if (!metricsFont_.has_value()) {
        return static_cast<float>(safeEnd - safeStart) * kApproximateCharacterWidth;
    }

    const std::u32string text = visage::String::convertUtf8ToUtf32<std::u32string>(text_.substr(safeStart, safeEnd - safeStart));
    return metricsFont_->stringWidth(text);
}

float TextEditControl::cursorOffsetForIndex(std::size_t index) const
{
    const std::size_t lineStart = multiline_ ? lineStartForIndex(index) : 0;
    return measuredTextWidth(lineStart, std::min(index, text_.size()));
}

float TextEditControl::lineAdvance() const
{
    if (metricsFont_.has_value()) {
        return std::max(1.0f, metricsFont_->lineHeight());
    }
    return kApproximateLineHeight;
}

} // namespace visiform::ui::editors
