#include "ui/Splitter.h"
#include "ui/VisualStyleBaseline.h"

#include <visage/graphics.h>

namespace visiform::ui {

void Splitter::draw(visage::Canvas& canvas) const
{
    const Bounds divider = dividerBounds();
    if (divider.width <= 0.0f || divider.height <= 0.0f) {
        return;
    }

    const auto palette = visual_style::makePalette(
        0xff242b36, 0xff566174, 0xffeef2f8, 0xff72a7ff, 0xff6c7788);
    visual_style::drawBevel(canvas,
        { divider.x, divider.y, divider.width, divider.height },
        palette, dragging_, hovered_ || dragging_);

    canvas.setColor(dragging_ ? palette.accent : (hovered_ ? palette.highlight : palette.border));
    if (orientation_ == Orientation::Vertical) {
        const float lineX = std::floor(divider.x + divider.width * 0.5f);
        canvas.fill(lineX, divider.y, 1.0f, divider.height);
        const float gripTop = divider.y + std::max(0.0f, (divider.height - 24.0f) * 0.5f);
        for (int index = -1; index <= 1; ++index) {
            canvas.fill(lineX + static_cast<float>(index * 2), gripTop, 1.0f, std::min(24.0f, divider.height));
        }
    }
    else {
        const float lineY = std::floor(divider.y + divider.height * 0.5f);
        canvas.fill(divider.x, lineY, divider.width, 1.0f);
        const float gripLeft = divider.x + std::max(0.0f, (divider.width - 24.0f) * 0.5f);
        for (int index = -1; index <= 1; ++index) {
            canvas.fill(gripLeft, lineY + static_cast<float>(index * 2), std::min(24.0f, divider.width), 1.0f);
        }
    }
}

} // namespace visiform::ui
