#include "ui/Splitter.h"
#include "model/LookAndFeelDefinition.h"
#include "ui/VisualStyleBaseline.h"

#include <visage/graphics.h>

namespace visiform::ui {
namespace {

int parseColorOrDefault(const std::string& value, int fallback)
{
    if (value.empty() || value.front() != '#') {
        return fallback;
    }
    try {
        const std::string digits = value.substr(1);
        const unsigned long parsed = std::stoul(digits, nullptr, 16);
        if (digits.size() == 6) {
            return static_cast<int>(0xff000000u | parsed);
        }
        if (digits.size() == 8) {
            return static_cast<int>(parsed);
        }
    }
    catch (...) {
    }
    return fallback;
}

} // namespace

void Splitter::draw(visage::Canvas& canvas, const model::ResolvedLookAndFeelStyle& style) const
{
    const Bounds divider = dividerBounds();
    if (divider.width <= 0.0f || divider.height <= 0.0f) {
        return;
    }

    visual_style::Palette palette = visual_style::makePalette(
        parseColorOrDefault(style.raisedSurfaceColor, 0xff242b36),
        parseColorOrDefault(style.borderColor, 0xff566174),
        parseColorOrDefault(style.primaryTextColor, 0xffeef2f8),
        parseColorOrDefault(style.accentColor, 0xff72a7ff),
        parseColorOrDefault(style.disabledTextColor, 0xff6c7788));
    palette.focus = parseColorOrDefault(style.focusOutlineColor, palette.accent);
    palette.highlight = parseColorOrDefault(style.highlightEdgeColor, palette.highlight);
    palette.shadow = parseColorOrDefault(style.shadowEdgeColor, palette.shadow);
    palette.hoverFill = parseColorOrDefault(style.hoverStateColor, palette.hoverFill);
    palette.pressedFill = parseColorOrDefault(style.pressedStateColor, palette.pressedFill);
    palette.borderThickness = style.borderThickness;
    palette.highlightThickness = style.splitterHighlightThickness;
    palette.shadowThickness = style.splitterShadowThickness;
    const Bounds feedback = hovered_ || dragging_ ? hitBounds() : divider;
    visual_style::State state;
    state.hovered = hovered_;
    state.pressed = dragging_;
    visual_style::drawBevel(canvas,
        { feedback.x, feedback.y, feedback.width, feedback.height },
        palette, state);

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
