#pragma once

#include "model/WidgetNode.h"

#include <visage/graphics.h>

#include <optional>
#include <string>
#include <vector>

namespace visiform::model {
struct WidgetDefinition;
}

namespace visiform::ui {

class WidgetPalette {
public:
    void setBounds(float x, float y, float width, float height);
    [[nodiscard]] bool contains(float x, float y) const;
    [[nodiscard]] bool mouseDown(float x, float y);
    [[nodiscard]] bool mouseDrag(float x, float y);
    [[nodiscard]] bool mouseUp();
    [[nodiscard]] bool mouseWheel(float deltaY, float x, float y);
    [[nodiscard]] bool mouseMove(float x, float y);
    [[nodiscard]] std::optional<model::WidgetType> hitTestWidgetType(float x, float y) const;
    [[nodiscard]] std::optional<std::string> hitTestHint(float x, float y) const;
    void draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const;

private:
    struct Category {
        std::string name;
        std::vector<const model::WidgetDefinition*> entries;
    };

    [[nodiscard]] std::vector<Category> categories() const;
    [[nodiscard]] const Category* selectedCategory(const std::vector<Category>& categories) const;
    [[nodiscard]] model::Rect tabStripBounds() const;
    [[nodiscard]] model::Rect itemStripBounds() const;
    [[nodiscard]] float tabWidth(const std::string& label) const;
    [[nodiscard]] float itemWidth(const model::WidgetDefinition& definition) const;
    [[nodiscard]] float totalTabWidth(const std::vector<Category>& categories) const;
    [[nodiscard]] float totalItemWidth(const Category& category) const;
    [[nodiscard]] float maximumTabScroll(const std::vector<Category>& categories) const;
    [[nodiscard]] float maximumItemScroll(const Category& category) const;
    void clampScrollOffsets();

    float x_{};
    float y_{};
    float width_{};
    float height_{};
    std::string selectedCategory_ = "Common";
    float tabScrollOffset_ = 0.0f;
    float itemScrollOffset_ = 0.0f;
    std::optional<std::string> hoveredCategory_{};
    std::optional<model::WidgetType> hoveredWidgetType_{};
};

} // namespace visiform::ui
