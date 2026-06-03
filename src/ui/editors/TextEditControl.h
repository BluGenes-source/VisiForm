#pragma once
#pragma once

#include <optional>
#include <string>

#include <visage/app.h>
#include <visage/graphics.h>

namespace visiform::ui::editors {

class TextEditControl {
public:
    struct Bounds {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;

        [[nodiscard]] bool contains(float px, float py) const
        {
            return px >= x && py >= y && px <= x + width && py <= y + height;
        }
    };

    enum class PendingAction {
        None,
        Commit,
        Cancel
    };

    void setBounds(float x, float y, float width, float height);
    [[nodiscard]] const Bounds& bounds() const;
    [[nodiscard]] bool contains(float x, float y) const;

    void begin(std::string text, bool selectAll = true, bool multiline = false);
    void clear();
    [[nodiscard]] bool isActive() const;
    [[nodiscard]] bool isFocused() const;
    [[nodiscard]] bool isMultiline() const;

    void setText(std::string text);
    [[nodiscard]] const std::string& text() const;
    void selectAll();

    [[nodiscard]] bool mouseDown(float x, float y);
    [[nodiscard]] bool keyPress(const visage::KeyEvent& event);
    [[nodiscard]] bool textInput(const std::string& text);

    [[nodiscard]] std::optional<PendingAction> consumePendingAction();

    void draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const;

private:
    [[nodiscard]] bool hasSelection() const;
    [[nodiscard]] std::size_t selectionMin() const;
    [[nodiscard]] std::size_t selectionMax() const;
    void clearSelection();
    void deleteSelection();
    void insertText(const std::string& text);
    void moveCursor(std::size_t index, bool extendSelection = false);
    void ensureCursorVisible();
    [[nodiscard]] std::size_t lineStartForIndex(std::size_t index) const;
    [[nodiscard]] std::size_t lineEndForIndex(std::size_t index) const;
    [[nodiscard]] std::size_t hitTestCharacterIndex(float x, float y) const;
    [[nodiscard]] std::size_t currentLineIndex() const;
    [[nodiscard]] std::size_t currentColumnIndex() const;
    [[nodiscard]] std::size_t lineCount() const;
    [[nodiscard]] std::size_t indexForLineColumn(std::size_t lineIndex, std::size_t columnIndex) const;
    [[nodiscard]] float characterAdvance() const;
    [[nodiscard]] float lineAdvance() const;

    Bounds bounds_{};
    std::string originalText_{};
    std::string text_{};
    std::size_t cursorIndex_ = 0;
    std::size_t selectionStart_ = 0;
    std::size_t selectionEnd_ = 0;
    bool focused_ = false;
    bool editing_ = false;
    bool multiline_ = false;
    float scrollX_ = 0.0f;
    float scrollY_ = 0.0f;
    PendingAction pendingAction_ = PendingAction::None;
};

} // namespace visiform::ui::editors
