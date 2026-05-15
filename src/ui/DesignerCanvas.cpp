#include "ui/DesignerCanvas.h"

#include "ui/DesignerCanvas.h"

#include <algorithm>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kPadding = 16.0f;

} // namespace

void DesignerCanvas::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

void DesignerCanvas::draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const
{
    if (width_ <= 0.0f || height_ <= 0.0f) {
        return;
    }

    canvas.setColor(0xff1f242d);
    canvas.fill(x_, y_, width_, height_);

    canvas.setColor(0xff2a303a);
    canvas.fill(x_, y_, width_, kHeaderHeight);

    canvas.setColor(0xff101318);
    canvas.fill(x_, y_, width_, 1.0f);
    canvas.fill(x_, y_ + height_ - 1.0f, width_, 1.0f);
    canvas.fill(x_, y_, 1.0f, height_);
    canvas.fill(x_ + width_ - 1.0f, y_, 1.0f, height_);

    if (drawText) {
        canvas.setColor(0xfff3f5f8);
        canvas.text("Designer Canvas", font, visage::Font::kTopLeft,
            x_ + kPadding, y_ + 6.0f, width_ - kPadding * 2.0f, kHeaderHeight - 8.0f);
    }

    const float previewX = x_ + kPadding * 2.0f;
    const float previewY = y_ + kHeaderHeight + 20.0f;
    const float previewWidth = std::max(0.0f, width_ - kPadding * 4.0f);
    const float previewHeight = std::max(0.0f, height_ - (kHeaderHeight + 84.0f));

    if (previewWidth <= 0.0f || previewHeight <= 0.0f) {
        return;
    }

    canvas.setColor(0xff303746);
    canvas.fill(previewX, previewY, previewWidth, previewHeight);
    canvas.setColor(0xff475064);
    canvas.fill(previewX + 1.0f, previewY + 1.0f, previewWidth - 2.0f, previewHeight - 2.0f);
    canvas.setColor(0xffeceff5);
    canvas.fill(previewX + 2.0f, previewY + 2.0f, previewWidth - 4.0f, previewHeight - 4.0f);

    const float formX = previewX + 30.0f;
    const float formY = previewY + 34.0f;
    const float formWidth = std::min(360.0f, previewWidth - 60.0f);
    const float formHeight = std::min(240.0f, previewHeight - 60.0f);

    if (formWidth <= 0.0f || formHeight <= 0.0f) {
        return;
    }

    canvas.setColor(0xffd9dee8);
    canvas.fill(formX, formY, formWidth, formHeight);
    canvas.setColor(0xffbcc4d2);
    canvas.fill(formX, formY, formWidth, 28.0f);

    if (drawText) {
        canvas.setColor(0xff243041);
        canvas.text("MainWindow", font, visage::Font::kTopLeft,
            formX + 10.0f, formY + 4.0f, formWidth - 20.0f, 22.0f);
    }

    const float buttonX = formX + 40.0f;
    const float buttonY = formY + 62.0f;
    const float buttonWidth = 160.0f;
    const float buttonHeight = 40.0f;

    canvas.setColor(0xff2d7ff9);
    canvas.fill(buttonX - 3.0f, buttonY - 3.0f, buttonWidth + 6.0f, buttonHeight + 6.0f);
    canvas.setColor(0xffebedf2);
    canvas.fill(buttonX, buttonY, buttonWidth, buttonHeight);
    canvas.setColor(0xffccd2dc);
    canvas.fill(buttonX, buttonY, buttonWidth, 1.0f);
    canvas.fill(buttonX, buttonY + buttonHeight - 1.0f, buttonWidth, 1.0f);
    canvas.fill(buttonX, buttonY, 1.0f, buttonHeight);
    canvas.fill(buttonX + buttonWidth - 1.0f, buttonY, 1.0f, buttonHeight);

    if (drawText) {
        canvas.setColor(0xff1f2530);
        canvas.text("Click Me", font, visage::Font::kCenter,
            buttonX, buttonY, buttonWidth, buttonHeight);

        canvas.setColor(0xff243041);
        canvas.text("Selected: helloButton", font, visage::Font::kTopLeft,
            formX + 16.0f, formY + formHeight - 42.0f, formWidth - 32.0f, 24.0f);
    }
}

} // namespace visiform::ui
