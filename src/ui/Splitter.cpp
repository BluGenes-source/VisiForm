#include "ui/Splitter.h"

#include <visage/graphics.h>

namespace visiform::ui {

void Splitter::draw(visage::Canvas& canvas) const
{
    const Bounds divider = dividerBounds();
    if (divider.width <= 0.0f || divider.height <= 0.0f) {
        return;
    }

    canvas.setColor(0xff1a2029);
    canvas.fill(divider.x, divider.y, divider.width, divider.height);

    canvas.setColor(dragging_ ? 0xff92b9ff : 0xff3a4252);
    if (orientation_ == Orientation::Vertical) {
        const float lineX = divider.x + divider.width * 0.5f - 0.5f;
        canvas.fill(lineX, divider.y, 1.0f, divider.height);
    }
    else {
        const float lineY = divider.y + divider.height * 0.5f - 0.5f;
        canvas.fill(divider.x, lineY, divider.width, 1.0f);
    }
}

} // namespace visiform::ui
