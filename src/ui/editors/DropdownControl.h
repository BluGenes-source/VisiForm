#pragma once
#pragma once

#include <optional>
#include <string>
#include <vector>

#include <visage/app.h>
#include <visage/graphics.h>

namespace visiform::ui::editors {

class DropdownControl {
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

    struct Item {
        std::string value;
        std::string label;
        std::string hint;
    };

    struct Selection {
        std::string key;
        std::string value;
        std::string label;
        std::string hint;
    };

    void open(std::string key,
        const Bounds& anchorBounds,
        const Bounds& viewportBounds,
        std::vector<Item> items,
        const std::string& selectedValue);
    void close();

    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] bool isOpenFor(const std::string& key) const;
    [[nodiscard]] const std::string& activeKey() const;

    [[nodiscard]] bool contains(float x, float y) const;
    [[nodiscard]] bool mouseDown(float x, float y);
    [[nodiscard]] bool mouseWheel(float deltaY, float x, float y);
    [[nodiscard]] bool keyPress(const visage::KeyEvent& event);

    [[nodiscard]] std::optional<Selection> consumeSelection();
    [[nodiscard]] std::optional<std::string> hintAt(float x, float y) const;

    void draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const;

private:
    [[nodiscard]] Bounds popupBounds() const;
    [[nodiscard]] std::size_t visibleItemCount() const;
    [[nodiscard]] std::size_t clampedSelectedIndex() const;
    [[nodiscard]] std::optional<std::size_t> hitTestIndex(float x, float y) const;
    void clampScrollOffset();

    std::string activeKey_{};
    Bounds anchorBounds_{};
    Bounds viewportBounds_{};
    std::vector<Item> items_{};
    std::size_t selectedIndex_ = 0;
    std::size_t scrollOffset_ = 0;
    std::optional<Selection> pendingSelection_{};
    bool open_ = false;
};

} // namespace visiform::ui::editors
