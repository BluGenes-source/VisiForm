#include "ui/ProjectTree.h"

#include "ui/ProjectTree.h"

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kPadding = 12.0f;

} // namespace

void ProjectTree::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

void ProjectTree::draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const
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

    if (!drawText) {
        return;
    }

    canvas.setColor(0xfff3f5f8);
    canvas.text("Project Tree", font, visage::Font::kTopLeft,
        x_ + kPadding, y_ + 6.0f, width_ - kPadding * 2.0f, kHeaderHeight - 8.0f);

    canvas.setColor(0xffdde2ea);
    canvas.text("MainWindow", font, visage::Font::kTopLeft,
        x_ + 16.0f, y_ + kHeaderHeight + 10.0f, width_ - 24.0f, 24.0f);
    canvas.text("- helloButton", font, visage::Font::kTopLeft,
        x_ + 28.0f, y_ + kHeaderHeight + 38.0f, width_ - 36.0f, 24.0f);
}

} // namespace visiform::ui
