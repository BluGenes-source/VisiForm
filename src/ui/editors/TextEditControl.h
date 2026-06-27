#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

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

    struct State {
        std::string text{};
        std::size_t cursorIndex = 0;
        std::size_t selectionStart = 0;
        std::size_t selectionEnd = 0;
        float scrollX = 0.0f;
        float scrollY = 0.0f;
        float preferredCursorX = -1.0f;
    };

    void setBounds(float x, float y, float width, float height);
    [[nodiscard]] const Bounds& bounds() const;
    [[nodiscard]] bool contains(float x, float y) const;

    void begin(std::string text, bool selectAll = true, bool multiline = false, bool wordWrap = false, bool readOnly = false);
    void clear();
    [[nodiscard]] bool isActive() const;
    [[nodiscard]] bool isFocused() const;
    [[nodiscard]] bool isMultiline() const;

    void setText(std::string text);
    [[nodiscard]] const std::string& text() const;
    [[nodiscard]] State state() const;
    void restoreState(const State& state);
    void selectAll();

    [[nodiscard]] bool mouseDown(float x, float y);
    [[nodiscard]] bool mouseDrag(float x, float y);
    [[nodiscard]] bool mouseUp();
    [[nodiscard]] bool mouseWheel(float deltaY, float x, float y);
    [[nodiscard]] bool keyPress(const visage::KeyEvent& event);
    [[nodiscard]] bool textInput(const std::string& text);
    void showCaret();
    void toggleCaretVisibility();
    [[nodiscard]] bool shouldBlinkCaret() const;
    [[nodiscard]] bool isCaretVisible() const;

    [[nodiscard]] std::optional<PendingAction> consumePendingAction();

    void setMetricsFont(const visage::Font& font, float dpiScale);
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
    void noteEditingInteraction();
    [[nodiscard]] std::size_t lineStartForIndex(std::size_t index) const;
    [[nodiscard]] std::size_t lineEndForIndex(std::size_t index) const;
    [[nodiscard]] std::size_t hitTestCharacterIndex(float x, float y) const;
    [[nodiscard]] std::size_t currentLineIndex() const;
    [[nodiscard]] std::size_t currentColumnIndex() const;
    [[nodiscard]] std::size_t lineCount() const;
    [[nodiscard]] std::size_t indexForLineColumn(std::size_t lineIndex, std::size_t columnIndex) const;
    [[nodiscard]] std::size_t indexForLineOffset(std::size_t lineIndex, float offset) const;
    [[nodiscard]] std::vector<std::size_t> lineBoundaries(std::size_t lineStart, std::size_t lineEnd) const;
    [[nodiscard]] std::vector<std::pair<std::size_t, std::size_t>> visualLineRanges() const;
    [[nodiscard]] float measuredTextWidth(std::size_t start, std::size_t end) const;
    [[nodiscard]] float cursorOffsetForIndex(std::size_t index) const;
    [[nodiscard]] float lineAdvance() const;

    Bounds bounds_{};
    mutable std::optional<visage::Font> metricsFont_{};
    std::string originalText_{};
    std::string text_{};
    std::size_t cursorIndex_ = 0;
    std::size_t selectionStart_ = 0;
    std::size_t selectionEnd_ = 0;
    bool focused_ = false;
    bool editing_ = false;
    bool multiline_ = false;
    bool wordWrap_ = false;
    bool readOnly_ = false;
    bool draggingSelection_ = false;
    bool caretVisible_ = false;
    float scrollX_ = 0.0f;
    float scrollY_ = 0.0f;
    float preferredCursorX_ = -1.0f;
    PendingAction pendingAction_ = PendingAction::None;
};

} // namespace visiform::ui::editors
