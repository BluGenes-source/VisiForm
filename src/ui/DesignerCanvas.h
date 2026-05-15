#pragma once

#pragma once

#include <visage/graphics.h>

namespace visiform::ui {

class DesignerCanvas {
public:
    void setBounds(float x, float y, float width, float height);
    void draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const;

private:
    float x_{};
    float y_{};
    float width_{};
    float height_{};
};

} // namespace visiform::ui
