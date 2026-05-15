#pragma once

#pragma once

#include "model/WidgetNode.h"

#include <visage/graphics.h>

namespace visiform::ui {

class PropertyInspector {
public:
    void setBounds(float x, float y, float width, float height);
    void draw(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::WidgetNode* selectedWidget) const;

private:
    float x_{};
    float y_{};
    float width_{};
    float height_{};
};

} // namespace visiform::ui
