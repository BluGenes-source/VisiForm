#pragma once

#pragma once

#include "model/WidgetNode.h"

#include <visage/graphics.h>

#include <optional>
#include <string>

namespace visiform::ui {

class WidgetPalette {
public:
    void setBounds(float x, float y, float width, float height);
    [[nodiscard]] bool contains(float x, float y) const;
    [[nodiscard]] std::optional<model::WidgetType> hitTestWidgetType(float x, float y) const;
    [[nodiscard]] std::optional<std::string> hitTestHint(float x, float y) const;
    void draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const;

private:
    float x_{};
    float y_{};
    float width_{};
    float height_{};
};

} // namespace visiform::ui
