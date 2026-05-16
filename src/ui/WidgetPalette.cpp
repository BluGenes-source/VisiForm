#include "ui/WidgetPalette.h"

#include "ui/WidgetPalette.h"

#include <algorithm>
#include <array>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kRowHeight = 32.0f;
constexpr float kPadding = 12.0f;

struct PaletteEntry {
    const char* label;
    const char* hint;
    model::WidgetType type;
};

constexpr std::array<PaletteEntry, 8> kPaletteEntries = {{
    { "Label", "Add static text to the form", model::WidgetType::Label },
    { "Button", "Add a clickable button", model::WidgetType::Button },
    { "TextBox", "Add a text input field", model::WidgetType::TextBox },
    { "CheckBox", "Add a checkable option", model::WidgetType::CheckBox },
    { "Slider", "Add a numeric slider control", model::WidgetType::Slider },
    { "Frame", "Add a visual group frame", model::WidgetType::Frame },
    { "Image", "Add an image placeholder", model::WidgetType::Image },
    { "Spacer", "Add spacing or a layout placeholder", model::WidgetType::Spacer }
}};

} // namespace

void WidgetPalette::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

bool WidgetPalette::contains(float x, float y) const
{
    return x >= x_ && y >= y_ && x <= x_ + width_ && y <= y_ + height_;
}

std::optional<model::WidgetType> WidgetPalette::hitTestWidgetType(float x, float y) const
{
    if (!contains(x, y)) {
        return std::nullopt;
    }

    float rowTop = y_ + kHeaderHeight + 8.0f;
    for (const auto& entry : kPaletteEntries) {
        if (rowTop + kRowHeight > y_ + height_ - 8.0f) {
            break;
        }

        if (y >= rowTop && y <= rowTop + kRowHeight && x >= x_ + 8.0f && x <= x_ + width_ - 8.0f) {
            return entry.type;
        }

        rowTop += kRowHeight;
    }

    return std::nullopt;
}

std::optional<std::string> WidgetPalette::hitTestHint(float x, float y) const
{
    if (!contains(x, y)) {
        return std::nullopt;
    }

    float rowTop = y_ + kHeaderHeight + 8.0f;
    for (const auto& entry : kPaletteEntries) {
        if (rowTop + kRowHeight > y_ + height_ - 8.0f) {
            break;
        }

        if (y >= rowTop && y <= rowTop + kRowHeight && x >= x_ + 8.0f && x <= x_ + width_ - 8.0f) {
            return std::string{ entry.hint };
        }

        rowTop += kRowHeight;
    }

    return std::nullopt;
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

    float rowTop = y_ + kHeaderHeight + 8.0f;
    for (std::size_t index = 0; index < kPaletteEntries.size(); ++index) {
        if (rowTop + kRowHeight > y_ + height_ - 8.0f) {
            break;
        }

        canvas.setColor(index % 2 == 0 ? 0xff2a303c : 0xff262c37);
        canvas.fill(x_ + 8.0f, rowTop, width_ - 16.0f, kRowHeight - 2.0f);

        canvas.setColor(0xff3a4252);
        canvas.fill(x_ + 8.0f, rowTop, 4.0f, kRowHeight - 2.0f);

        if (drawText) {
            canvas.setColor(0xffdde2ea);
            canvas.text(kPaletteEntries[index].label, font, visage::Font::kTopLeft,
                x_ + 20.0f, rowTop + 6.0f, width_ - 32.0f, kRowHeight - 8.0f);
        }

        rowTop += kRowHeight;
    }
}

} // namespace visiform::ui
