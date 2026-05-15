#include "ui/WidgetPalette.h"

#include "ui/WidgetPalette.h"

#include <algorithm>
#include <array>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kRowHeight = 32.0f;
constexpr float kPadding = 12.0f;

} // namespace

void WidgetPalette::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

void WidgetPalette::draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const
{
    if (width_ <= 0.0f || height_ <= 0.0f) {
        return;
    }

    canvas.setColor(0xff232833);
    canvas.fill(x_, y_, width_, height_);

    canvas.setColor(0xff2c3240);
    canvas.fill(x_, y_, width_, kHeaderHeight);

    canvas.setColor(0xff11141a);
    canvas.fill(x_, y_, width_, 1.0f);
    canvas.fill(x_, y_ + height_ - 1.0f, width_, 1.0f);
    canvas.fill(x_, y_, 1.0f, height_);
    canvas.fill(x_ + width_ - 1.0f, y_, 1.0f, height_);

    if (drawText) {
        canvas.setColor(0xfff3f5f8);
        canvas.text("Widget Palette", font, visage::Font::kTopLeft,
            x_ + kPadding, y_ + 6.0f, width_ - kPadding * 2.0f, kHeaderHeight - 8.0f);
    }

    static constexpr std::array<const char*, 8> kWidgetNames = {
        "Label",
        "Button",
        "TextBox",
        "CheckBox",
        "Slider",
        "Frame",
        "Image",
        "Spacer"
    };

    float rowTop = y_ + kHeaderHeight + 8.0f;
    for (std::size_t index = 0; index < kWidgetNames.size(); ++index) {
        if (rowTop + kRowHeight > y_ + height_ - 8.0f) {
            break;
        }

        canvas.setColor(index % 2 == 0 ? 0xff2a303c : 0xff262c37);
        canvas.fill(x_ + 8.0f, rowTop, width_ - 16.0f, kRowHeight - 2.0f);

        canvas.setColor(0xff3a4252);
        canvas.fill(x_ + 8.0f, rowTop, 4.0f, kRowHeight - 2.0f);

        if (drawText) {
            canvas.setColor(0xffdde2ea);
            canvas.text(kWidgetNames[index], font, visage::Font::kTopLeft,
                x_ + 20.0f, rowTop + 6.0f, width_ - 32.0f, kRowHeight - 8.0f);
        }

        rowTop += kRowHeight;
    }
}

} // namespace visiform::ui
