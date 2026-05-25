#include "ui/editors/TextEditControl.h"

#include <algorithm>
#include <cmath>

namespace visiform::ui::editors {
namespace {

constexpr float kHorizontalPadding = 8.0f;
constexpr float kVerticalPadding = 4.0f;
constexpr float kEstimatedTextHeight = 18.0f;
constexpr float kApproximateCharacterWidth = 8.0f;

bool isPrintableInput(const std::string& text)
{
    return std::any_of(text.begin(), text.end(), [](unsigned char character) {
        return character >= 32 && character != 127;
    });
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

void TextEditControl::begin(std::string text, bool selectAllText)
{
    originalText_ = text;
    text_ = std::move(text);
    editing_ = true;
    focused_ = true;
    pendingAction_ = PendingAction::None;
    cursorIndex_ = text_.size();
    selectionStart_ = cursorIndex_;
    selectionEnd_ = cursorIndex_;
    scrollX_ = 0.0f;
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
}

bool TextEditControl::isActive() const
{
    return editing_;
}

bool TextEditControl::isFocused() const
{
    return editing_ && focused_;
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
        return false;
    }

    const std::size_t hitIndex = hitTestCharacterIndex(x);
    cursorIndex_ = hitIndex;
    selectionStart_ = hitIndex;
    selectionEnd_ = hitIndex;
    ensureCursorVisible();
    return true;
}

bool TextEditControl::keyPress(const visage::KeyEvent& event)
{
    if (!editing_ || !focused_) {
        return false;
    }

    using KeyCode = visage::KeyCode;
    switch (event.keyCode()) {
    case KeyCode::Backspace:
        if (hasSelection()) {
            deleteSelection();
        }
        else if (cursorIndex_ > 0) {
            text_.erase(cursorIndex_ - 1, 1);
            --cursorIndex_;
            selectionStart_ = cursorIndex_;
            selectionEnd_ = cursorIndex_;
        }
        ensureCursorVisible();
        return true;
    case KeyCode::Delete:
        if (hasSelection()) {
            deleteSelection();
        }
        else if (cursorIndex_ < text_.size()) {
            text_.erase(cursorIndex_, 1);
        }
        ensureCursorVisible();
        return true;
    case KeyCode::Left:
        if (hasSelection()) {
            moveCursor(selectionMin());
        }
        else if (cursorIndex_ > 0) {
            moveCursor(cursorIndex_ - 1);
        }
        return true;
    case KeyCode::Right:
        if (hasSelection()) {
            moveCursor(selectionMax());
        }
        else if (cursorIndex_ < text_.size()) {
            moveCursor(cursorIndex_ + 1);
        }
        return true;
    case KeyCode::Home:
        moveCursor(0);
        return true;
    case KeyCode::End:
        moveCursor(text_.size());
        return true;
    case KeyCode::Return:
        pendingAction_ = PendingAction::Commit;
        return true;
    case KeyCode::Escape:
        text_ = originalText_;
        cursorIndex_ = text_.size();
        selectionStart_ = cursorIndex_;
        selectionEnd_ = cursorIndex_;
        ensureCursorVisible();
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

    insertText(text);
    return true;
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

void TextEditControl::draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const
{
    if (!editing_ || !drawText || bounds_.width <= 0.0f || bounds_.height <= 0.0f) {
        return;
    }

    const float textLeft = bounds_.x + kHorizontalPadding;
    const float textWidth = std::max(0.0f, bounds_.width - kHorizontalPadding * 2.0f);
    const float textHeight = std::min(kEstimatedTextHeight, std::max(0.0f, bounds_.height - kVerticalPadding * 2.0f));
    const float textTop = bounds_.y + std::max(kVerticalPadding, (bounds_.height - textHeight) * 0.5f);
    const float selectionLeft = textLeft - scrollX_ + static_cast<float>(selectionMin()) * characterAdvance();
    const float selectionWidth = static_cast<float>(selectionMax() - selectionMin()) * characterAdvance();
    const float cursorX = textLeft - scrollX_ + static_cast<float>(cursorIndex_) * characterAdvance();

    canvas.saveState();
    canvas.setClampBounds(bounds_.x + 1.0f, bounds_.y + 1.0f, std::max(0.0f, bounds_.width - 2.0f), std::max(0.0f, bounds_.height - 2.0f));
    if (hasSelection() && selectionWidth > 0.0f) {
        canvas.setColor(0x66355382);
        canvas.fill(selectionLeft, bounds_.y + kVerticalPadding, selectionWidth, std::max(0.0f, bounds_.height - kVerticalPadding * 2.0f));
    }

    canvas.setColor(0xffeef2f8);
    canvas.text(text_, font, visage::Font::kTopLeft, textLeft - scrollX_, textTop, std::max(textWidth + scrollX_, textWidth), textHeight);

    if (focused_) {
        canvas.setColor(0xff92b9ff);
        canvas.fill(cursorX, bounds_.y + kVerticalPadding, 1.0f, std::max(0.0f, bounds_.height - kVerticalPadding * 2.0f));
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
}

void TextEditControl::moveCursor(std::size_t index, bool extendSelection)
{
    cursorIndex_ = std::min(index, text_.size());
    if (extendSelection) {
        selectionEnd_ = cursorIndex_;
    }
    else {
        clearSelection();
    }
    ensureCursorVisible();
}

void TextEditControl::ensureCursorVisible()
{
    const float innerWidth = std::max(0.0f, bounds_.width - kHorizontalPadding * 2.0f);
    const float cursorX = static_cast<float>(cursorIndex_) * characterAdvance();
    if (cursorX < scrollX_) {
        scrollX_ = cursorX;
    }
    else if (cursorX > scrollX_ + innerWidth - characterAdvance()) {
        scrollX_ = std::max(0.0f, cursorX - innerWidth + characterAdvance());
    }

    const float maxScroll = std::max(0.0f, static_cast<float>(text_.size()) * characterAdvance() - innerWidth);
    scrollX_ = std::clamp(scrollX_, 0.0f, maxScroll);
}

std::size_t TextEditControl::hitTestCharacterIndex(float x) const
{
    const float relativeX = std::max(0.0f, x - bounds_.x - kHorizontalPadding + scrollX_);
    const std::size_t index = static_cast<std::size_t>(std::floor((relativeX / characterAdvance()) + 0.5f));
    return std::min(index, text_.size());
}

float TextEditControl::characterAdvance() const
{
    return kApproximateCharacterWidth;
}

} // namespace visiform::ui::editors
