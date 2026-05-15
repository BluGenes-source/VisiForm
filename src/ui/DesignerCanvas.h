#pragma once

#pragma once

#include "model/ProjectDocument.h"

#include <visage/graphics.h>

#include <optional>

namespace visiform::ui {

class DesignerCanvas {
public:
    void setBounds(float x, float y, float width, float height);
    [[nodiscard]] bool contains(float x, float y) const;
    [[nodiscard]] std::optional<std::string> hitTestWidgetId(const model::ProjectDocument& document, float x, float y) const;
    void draw(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::ProjectDocument& document) const;

private:
    float x_{};
    float y_{};
    float width_{};
    float height_{};
};

} // namespace visiform::ui
