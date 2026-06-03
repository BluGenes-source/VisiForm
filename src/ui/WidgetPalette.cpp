#include "ui/WidgetPalette.h"

#include "ui/WidgetPalette.h"

#include "model/WidgetRegistry.h"

#include <algorithm>
#include <cmath>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kRowHeight = 32.0f;
constexpr float kPadding = 12.0f;
constexpr float kScrollBarWidth = 12.0f;
constexpr float kMouseWheelSensitivity = 40.0f;

std::vector<const model::WidgetDefinition*> paletteEntries()
{
    return model::WidgetRegistry::instance().paletteDefinitions();
}

bool containsPoint(const model::Rect& bounds, float x, float y)
{
    return x >= bounds.x && y >= bounds.y && x <= bounds.x + bounds.width && y <= bounds.y + bounds.height;
}

} // namespace

void WidgetPalette::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
    clampScrollOffset();
}

bool WidgetPalette::contains(float x, float y) const
{
    return x >= x_ && y >= y_ && x <= x_ + width_ && y <= y_ + height_;
}

bool WidgetPalette::mouseDown(float x, float y)
{
    if (!contains(x, y)) {
        return false;
    }

    updateScrollMetrics();
    const auto scrollBar = scrollBarBounds();
    if (!scrollBar.has_value() || !containsPoint(*scrollBar, x, y)) {
        return false;
    }

    const auto thumb = scrollBarThumbBounds();
    if (thumb.has_value() && containsPoint(*thumb, x, y)) {
        draggingScrollBarThumb_ = true;
        scrollBarDragOffsetY_ = y - thumb->y;
        return true;
    }

    if (thumb.has_value() && y < thumb->y) {
        setScrollOffsetY(scrollOffsetY_ - std::max(kRowHeight, visibleHeight_ * 0.85f));
    }
    else {
        setScrollOffsetY(scrollOffsetY_ + std::max(kRowHeight, visibleHeight_ * 0.85f));
    }

    return true;
}

bool WidgetPalette::mouseDrag(float x, float y)
{
    if (!draggingScrollBarThumb_) {
        return false;
    }

    updateScrollMetrics();
    const auto scrollBar = scrollBarBounds();
    const auto thumb = scrollBarThumbBounds();
    if (!scrollBar.has_value() || !thumb.has_value()) {
        draggingScrollBarThumb_ = false;
        return false;
    }

    const float trackTop = scrollBar->y;
    const float trackHeight = scrollBar->height;
    const float maxThumbTop = trackTop + std::max(0.0f, trackHeight - thumb->height);
    const float thumbTop = std::clamp(y - scrollBarDragOffsetY_, trackTop, maxThumbTop);
    const float maxScroll = std::max(0.0f, contentHeight_ - visibleHeight_);
    if (trackHeight > thumb->height && maxScroll > 0.0f) {
        setScrollOffsetY(maxScroll * ((thumbTop - trackTop) / (trackHeight - thumb->height)));
    }
    else {
        setScrollOffsetY(0.0f);
    }

    return true;
}

bool WidgetPalette::mouseUp()
{
    const bool wasDragging = draggingScrollBarThumb_;
    draggingScrollBarThumb_ = false;
    scrollBarDragOffsetY_ = 0.0f;
    return wasDragging;
}

bool WidgetPalette::mouseWheel(float deltaY, float x, float y)
{
    if (!contains(x, y)) {
        return false;
    }

    updateScrollMetrics();
    if (!needsVerticalScrollBar_) {
        return false;
    }

    return setScrollOffsetY(scrollOffsetY_ + (-deltaY * kMouseWheelSensitivity));
}

std::optional<model::WidgetType> WidgetPalette::hitTestWidgetType(float x, float y) const
{
    if (!contains(x, y)) {
        return std::nullopt;
    }

    const auto entries = paletteEntries();
    updateScrollMetrics();
    if (!isWithinVisibleContent(x, y)) {
        return std::nullopt;
    }

    const model::Rect bounds = contentBounds();
    float rowTop = bounds.y - scrollOffsetY_;
    for (const auto* entry : entries) {
        if (rowTop + kRowHeight < bounds.y) {
            rowTop += kRowHeight;
            continue;
        }

        if (rowTop > bounds.y + bounds.height) {
            break;
        }

        if (y >= rowTop && y <= rowTop + kRowHeight && x >= bounds.x && x <= bounds.x + bounds.width) {
            return entry->type;
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

    const auto entries = paletteEntries();
    updateScrollMetrics();
    if (!isWithinVisibleContent(x, y)) {
        return std::nullopt;
    }

    const model::Rect bounds = contentBounds();
    float rowTop = bounds.y - scrollOffsetY_;
    for (const auto* entry : entries) {
        if (rowTop + kRowHeight < bounds.y) {
            rowTop += kRowHeight;
            continue;
        }

        if (rowTop > bounds.y + bounds.height) {
            break;
        }

        if (y >= rowTop && y <= rowTop + kRowHeight && x >= bounds.x && x <= bounds.x + bounds.width) {
            return entry->defaultHint;
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

    const auto entries = paletteEntries();
    updateScrollMetrics();
    const model::Rect bounds = contentBounds();
    float rowTop = bounds.y - scrollOffsetY_;
    for (std::size_t index = 0; index < entries.size(); ++index) {
        if (rowTop + kRowHeight < bounds.y) {
            rowTop += kRowHeight;
            continue;
        }

        if (rowTop > bounds.y + bounds.height) {
            break;
        }

        const float visibleTop = std::max(rowTop, bounds.y);
        const float visibleHeight = std::min(rowTop + kRowHeight - 2.0f, bounds.y + bounds.height) - visibleTop;
        if (visibleHeight <= 0.0f) {
            rowTop += kRowHeight;
            continue;
        }

        canvas.setColor(index % 2 == 0 ? 0xff2a303c : 0xff262c37);
        canvas.fill(bounds.x, visibleTop, bounds.width, visibleHeight);

        canvas.setColor(0xff3a4252);
        canvas.fill(bounds.x, visibleTop, 4.0f, visibleHeight);

        if (drawText && rowTop >= bounds.y && rowTop + kRowHeight <= bounds.y + bounds.height) {
            canvas.setColor(0xffdde2ea);
            canvas.text(entries[index]->displayName, font, visage::Font::kTopLeft,
                bounds.x + 12.0f, rowTop + 6.0f, std::max(0.0f, bounds.width - 20.0f), kRowHeight - 8.0f);
        }

        rowTop += kRowHeight;
    }

    if (const auto scrollBar = scrollBarBounds(); scrollBar.has_value()) {
        canvas.setColor(0xff1c212a);
        canvas.fill(scrollBar->x, scrollBar->y, scrollBar->width, scrollBar->height);
        if (const auto thumb = scrollBarThumbBounds(); thumb.has_value()) {
            canvas.setColor(0xff556070);
            canvas.fill(thumb->x, thumb->y, thumb->width, thumb->height);
        }
    }
}

void WidgetPalette::updateScrollMetrics() const
{
    const auto entries = paletteEntries();
    visibleHeight_ = std::max(0.0f, height_ - kHeaderHeight - 16.0f);
    contentHeight_ = static_cast<float>(entries.size()) * kRowHeight;
    needsVerticalScrollBar_ = contentHeight_ > visibleHeight_ + 0.5f;
    if (!needsVerticalScrollBar_) {
        scrollOffsetY_ = 0.0f;
    }
    else {
        const float maxScroll = std::max(0.0f, contentHeight_ - visibleHeight_);
        scrollOffsetY_ = std::clamp(scrollOffsetY_, 0.0f, maxScroll);
    }
}

void WidgetPalette::clampScrollOffset()
{
    updateScrollMetrics();
}

bool WidgetPalette::setScrollOffsetY(float newScrollOffsetY)
{
    updateScrollMetrics();
    const float maxScroll = std::max(0.0f, contentHeight_ - visibleHeight_);
    const float clamped = std::clamp(newScrollOffsetY, 0.0f, maxScroll);
    if (std::abs(clamped - scrollOffsetY_) < 0.01f) {
        return false;
    }

    scrollOffsetY_ = clamped;
    return true;
}

model::Rect WidgetPalette::contentBounds() const
{
    const float innerX = x_ + 8.0f;
    const float innerY = y_ + kHeaderHeight + 8.0f;
    const float innerWidth = std::max(0.0f, width_ - 16.0f);
    const float reservedWidth = needsVerticalScrollBar_ ? (kScrollBarWidth + 6.0f) : 0.0f;
    return {
        innerX,
        innerY,
        std::max(0.0f, innerWidth - reservedWidth),
        std::max(0.0f, height_ - kHeaderHeight - 16.0f)
    };
}

std::optional<model::Rect> WidgetPalette::scrollBarBounds() const
{
    if (!needsVerticalScrollBar_) {
        return std::nullopt;
    }

    const float innerX = x_ + 8.0f;
    const float innerY = y_ + kHeaderHeight + 8.0f;
    const float innerWidth = std::max(0.0f, width_ - 16.0f);
    return model::Rect{
        innerX + std::max(0.0f, innerWidth - kScrollBarWidth),
        innerY,
        kScrollBarWidth,
        std::max(0.0f, height_ - kHeaderHeight - 16.0f)
    };
}

std::optional<model::Rect> WidgetPalette::scrollBarThumbBounds() const
{
    if (!needsVerticalScrollBar_) {
        return std::nullopt;
    }

    const auto scrollBar = scrollBarBounds();
    if (!scrollBar.has_value()) {
        return std::nullopt;
    }

    const float maxScroll = std::max(0.0f, contentHeight_ - visibleHeight_);
    const float thumbHeight = maxScroll <= 0.0f
        ? scrollBar->height
        : std::max(24.0f, scrollBar->height * (visibleHeight_ / contentHeight_));
    const float thumbTop = maxScroll <= 0.0f
        ? scrollBar->y
        : scrollBar->y + (scrollBar->height - thumbHeight) * (scrollOffsetY_ / maxScroll);
    return model::Rect{ scrollBar->x, thumbTop, scrollBar->width, thumbHeight };
}

bool WidgetPalette::isWithinVisibleContent(float x, float y) const
{
    return containsPoint(contentBounds(), x, y);
}

} // namespace visiform::ui
