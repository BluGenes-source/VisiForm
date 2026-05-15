#include "ui/DesignerCanvas.h"

#include "ui/DesignerCanvas.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <sstream>
#include <string>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kPadding = 16.0f;
constexpr float kPreviewPadding = 18.0f;
constexpr float kTitleBarHeight = 28.0f;

struct PanelRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    [[nodiscard]] bool contains(float px, float py) const
    {
        return px >= x && py >= y && px <= x + width && py <= y + height;
    }

    [[nodiscard]] bool isValid() const
    {
        return width > 0.0f && height > 0.0f;
    }
};

struct PreviewLayout {
    PanelRect preview{};
    PanelRect form{};
    float scale = 1.0f;
};

struct WidgetScreenInfo {
    const model::WidgetNode* widget = nullptr;
    PanelRect bounds{};
};

void drawBorder(visage::Canvas& canvas, const PanelRect& bounds, int color, float thickness = 1.0f)
{
    if (!bounds.isValid() || thickness <= 0.0f) {
        return;
    }

    canvas.setColor(color);
    canvas.fill(bounds.x, bounds.y, bounds.width, thickness);
    canvas.fill(bounds.x, bounds.y + bounds.height - thickness, bounds.width, thickness);
    canvas.fill(bounds.x, bounds.y, thickness, bounds.height);
    canvas.fill(bounds.x + bounds.width - thickness, bounds.y, thickness, bounds.height);
}

std::string widgetLabel(const model::WidgetNode& widget)
{
    if (!widget.name.empty()) {
        return widget.name;
    }

    if (!widget.id.empty()) {
        return widget.id;
    }

    return widget.typeName();
}

float getNumericProperty(const model::WidgetNode& widget, const std::string& key, float defaultValue)
{
    const auto* property = widget.getProperty(key);
    if (property == nullptr) {
        return defaultValue;
    }

    if (property->isFloat()) {
        return property->asFloat(defaultValue);
    }
    if (property->isInt()) {
        return static_cast<float>(property->asInt(static_cast<int>(defaultValue)));
    }
    if (property->isString()) {
        try {
            const std::string text = property->asString({});
            std::size_t parsedCharacters = 0;
            const float parsedValue = std::stof(text, &parsedCharacters);
            if (parsedCharacters == text.size()) {
                return parsedValue;
            }
        }
        catch (...) {
        }
    }

    return defaultValue;
}

std::string getStringProperty(const model::WidgetNode& widget, const std::string& key, const std::string& defaultValue)
{
    return widget.getStringProperty(key, defaultValue);
}

std::string getDisplayTextOrFallback(const model::WidgetNode& widget, const std::string& key, const std::string& fallback)
{
    const std::string value = getStringProperty(widget, key, {});
    return value.empty() ? fallback : value;
}

bool getBoolProperty(const model::WidgetNode& widget, const std::string& key, bool defaultValue)
{
    return widget.getBoolProperty(key, defaultValue);
}

float normalizedSliderValue(const model::WidgetNode& widget)
{
    const float minimum = getNumericProperty(widget, "min", 0.0f);
    const float maximum = getNumericProperty(widget, "max", 100.0f);
    const float value = getNumericProperty(widget, "value", 50.0f);
    if (maximum <= minimum) {
        return 0.5f;
    }

    const float clampedValue = std::clamp(value, minimum, maximum);
    return std::clamp((clampedValue - minimum) / (maximum - minimum), 0.0f, 1.0f);
}

int parseColorOrDefault(const std::string& value, int defaultColor)
{
    if (value.empty() || value.front() != '#') {
        return defaultColor;
    }

    try {
        const std::string digits = value.substr(1);
        const std::uint32_t parsed = static_cast<std::uint32_t>(std::stoul(digits, nullptr, 16));
        if (digits.size() == 6) {
            return static_cast<int>(0xff000000u | parsed);
        }
        if (digits.size() == 8) {
            return static_cast<int>(parsed);
        }
    }
    catch (...) {
    }

    return defaultColor;
}

PreviewLayout calculatePreviewLayout(float x, float y, float width, float height, const model::ProjectDocument& document)
{
    PreviewLayout layout;
    layout.preview = {
        x + kPadding,
        y + kHeaderHeight + 12.0f,
        std::max(0.0f, width - kPadding * 2.0f),
        std::max(0.0f, height - (kHeaderHeight + 24.0f))
    };

    if (!layout.preview.isValid() || !document.root.bounds.isValid()) {
        return layout;
    }

    const float availableWidth = std::max(0.0f, layout.preview.width - kPreviewPadding * 2.0f);
    const float availableHeight = std::max(0.0f, layout.preview.height - kPreviewPadding * 2.0f);
    if (availableWidth <= 0.0f || availableHeight <= 0.0f) {
        return layout;
    }

    layout.scale = std::min(availableWidth / document.root.bounds.width, availableHeight / document.root.bounds.height);
    if (layout.scale <= 0.0f) {
        return layout;
    }

    const float formWidth = document.root.bounds.width * layout.scale;
    const float formHeight = document.root.bounds.height * layout.scale;
    layout.form = {
        layout.preview.x + (layout.preview.width - formWidth) * 0.5f,
        layout.preview.y + (layout.preview.height - formHeight) * 0.5f,
        formWidth,
        formHeight
    };
    return layout;
}

void drawSelectionOutline(visage::Canvas& canvas, const PanelRect& bounds)
{
    drawBorder(canvas, { bounds.x - 2.0f, bounds.y - 2.0f, bounds.width + 4.0f, bounds.height + 4.0f }, 0xff2d7ff9, 1.5f);
}

PanelRect handleRect(const PanelRect& bounds, DesignerCanvas::HitRegion region, float handleSize)
{
    const float halfHandle = handleSize * 0.5f;
    switch (region) {
    case DesignerCanvas::HitRegion::TopLeftHandle:
        return { bounds.x - halfHandle, bounds.y - halfHandle, handleSize, handleSize };
    case DesignerCanvas::HitRegion::TopRightHandle:
        return { bounds.x + bounds.width - halfHandle, bounds.y - halfHandle, handleSize, handleSize };
    case DesignerCanvas::HitRegion::BottomLeftHandle:
        return { bounds.x - halfHandle, bounds.y + bounds.height - halfHandle, handleSize, handleSize };
    case DesignerCanvas::HitRegion::BottomRightHandle:
        return { bounds.x + bounds.width - halfHandle, bounds.y + bounds.height - halfHandle, handleSize, handleSize };
    case DesignerCanvas::HitRegion::None:
    case DesignerCanvas::HitRegion::Body:
        return {};
    }

    return {};
}

DesignerCanvas::HitRegion hitHandle(const PanelRect& bounds, float x, float y, float handleSize)
{
    constexpr std::array<DesignerCanvas::HitRegion, 4> kHandles = {
        DesignerCanvas::HitRegion::TopLeftHandle,
        DesignerCanvas::HitRegion::TopRightHandle,
        DesignerCanvas::HitRegion::BottomLeftHandle,
        DesignerCanvas::HitRegion::BottomRightHandle
    };

    for (DesignerCanvas::HitRegion handle : kHandles) {
        if (handleRect(bounds, handle, handleSize).contains(x, y)) {
            return handle;
        }
    }

    return DesignerCanvas::HitRegion::None;
}

void drawSelectionHandles(visage::Canvas& canvas, const PanelRect& bounds, float handleSize)
{
    constexpr std::array<DesignerCanvas::HitRegion, 4> kHandles = {
        DesignerCanvas::HitRegion::TopLeftHandle,
        DesignerCanvas::HitRegion::TopRightHandle,
        DesignerCanvas::HitRegion::BottomLeftHandle,
        DesignerCanvas::HitRegion::BottomRightHandle
    };

    for (DesignerCanvas::HitRegion handle : kHandles) {
        const PanelRect handleBounds = handleRect(bounds, handle, handleSize);
        canvas.setColor(0xffffffff);
        canvas.fill(handleBounds.x, handleBounds.y, handleBounds.width, handleBounds.height);
        drawBorder(canvas, handleBounds, 0xff2d7ff9);
    }
}

void drawGrid(visage::Canvas& canvas, const PanelRect& bounds, float scale, bool showMinorGrid, int gridSize, int majorGridSize)
{
    const float scaledGridSize = gridSize * scale;
    if (scaledGridSize < 4.0f) {
        return;
    }

    const float scaledMajorGridSize = majorGridSize * scale;

    const float contentTop = bounds.y + std::min(kTitleBarHeight, bounds.height);
    const float contentHeight = bounds.height - std::min(kTitleBarHeight, bounds.height);
    if (contentHeight <= 0.0f) {
        return;
    }

    if (showMinorGrid) {
        canvas.setColor(0xff252a33);
        for (float x = bounds.x + scaledGridSize; x < bounds.x + bounds.width; x += scaledGridSize) {
            canvas.fill(x, contentTop, 1.0f, contentHeight);
        }
        for (float y = contentTop + scaledGridSize; y < bounds.y + bounds.height; y += scaledGridSize) {
            canvas.fill(bounds.x, y, bounds.width, 1.0f);
        }
    }

    if (scaledMajorGridSize >= scaledGridSize + 1.0f) {
        canvas.setColor(0xff303744);
        for (float x = bounds.x + scaledMajorGridSize; x < bounds.x + bounds.width; x += scaledMajorGridSize) {
            canvas.fill(x, contentTop, 1.0f, contentHeight);
        }
        for (float y = contentTop + scaledMajorGridSize; y < bounds.y + bounds.height; y += scaledMajorGridSize) {
            canvas.fill(bounds.x, y, bounds.width, 1.0f);
        }
    }
}

std::optional<WidgetScreenInfo> findWidgetScreenInfo(const model::WidgetNode& widget,
    const std::string& widgetId,
    float formScreenX,
    float formScreenY,
    float parentLocalX,
    float parentLocalY,
    float scale)
{
    const float widgetLocalX = parentLocalX + widget.bounds.x;
    const float widgetLocalY = parentLocalY + widget.bounds.y;
    const PanelRect bounds{
        formScreenX + widgetLocalX * scale,
        formScreenY + widgetLocalY * scale,
        std::max(1.0f, widget.bounds.width * scale),
        std::max(1.0f, widget.bounds.height * scale)
    };

    if (widget.id == widgetId) {
        return WidgetScreenInfo{ &widget, bounds };
    }

    for (const auto& child : widget.children) {
        if (auto match = findWidgetScreenInfo(child, widgetId, formScreenX, formScreenY, widgetLocalX, widgetLocalY, scale)) {
            return match;
        }
    }

    return std::nullopt;
}

void drawWidget(visage::Canvas& canvas,
    const visage::Font& font,
    bool drawText,
    const model::WidgetNode& widget,
    float formScreenX,
    float formScreenY,
    float parentLocalX,
    float parentLocalY,
    float scale,
    const std::string& selectedWidgetId,
    float handleSize,
    bool showGrid,
    bool showMinorGrid,
    int gridSize,
    int majorGridSize)
{
    const float widgetLocalX = parentLocalX + widget.bounds.x;
    const float widgetLocalY = parentLocalY + widget.bounds.y;
    const PanelRect bounds{
        formScreenX + widgetLocalX * scale,
        formScreenY + widgetLocalY * scale,
        std::max(1.0f, widget.bounds.width * scale),
        std::max(1.0f, widget.bounds.height * scale)
    };

    switch (widget.type) {
    case model::WidgetType::FormWindow: {
        canvas.setColor(parseColorOrDefault(getStringProperty(widget, "backgroundColor", "#f2f4f8"), 0xfff2f4f8));
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        if (showGrid) {
            drawGrid(canvas, bounds, scale, showMinorGrid, gridSize, majorGridSize);
        }
        canvas.setColor(0xffc4ccd8);
        canvas.fill(bounds.x, bounds.y, bounds.width, std::min(kTitleBarHeight, bounds.height));
        drawBorder(canvas, bounds, 0xff6c7788);

        if (drawText) {
            canvas.setColor(0xff17212f);
            canvas.text(getStringProperty(widget, "title", widgetLabel(widget)), font, visage::Font::kTopLeft,
                bounds.x + 10.0f, bounds.y + 4.0f, std::max(0.0f, bounds.width - 20.0f), 22.0f);
        }
        break;
    }
    case model::WidgetType::Frame:
        canvas.setColor(parseColorOrDefault(getStringProperty(widget, "backgroundColor", "#e3e8f0"), 0xffe3e8f0));
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, 0xff97a3b7);
        if (drawText) {
            canvas.setColor(0xff182333);
            canvas.text(getStringProperty(widget, "title", widgetLabel(widget)), font, visage::Font::kTopLeft,
                bounds.x + 8.0f, bounds.y + 6.0f, std::max(0.0f, bounds.width - 16.0f), 20.0f);
        }
        break;
    case model::WidgetType::Label:
        if (drawText) {
            canvas.setColor(0xffeef3fa);
            canvas.text(getDisplayTextOrFallback(widget, "text", "Label"), font, visage::Font::kTopLeft,
                bounds.x + 4.0f, bounds.y + 3.0f, std::max(0.0f, bounds.width - 4.0f), bounds.height - 4.0f);
        }
        break;
    case model::WidgetType::Button:
        canvas.setColor(0xffeef1f6);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, 0xffc2c9d5);
        if (drawText) {
            canvas.setColor(0xff101b29);
            canvas.text(getStringProperty(widget, "text", widgetLabel(widget)), font, visage::Font::kCenter,
                bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    case model::WidgetType::TextBox:
        canvas.setColor(0xffffffff);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, 0xffb6c0cf);
        if (drawText) {
            canvas.setColor(0xff2d3a4d);
            canvas.text(getStringProperty(widget, "text", ""), font, visage::Font::kTopLeft,
                bounds.x + 8.0f, bounds.y + 6.0f, std::max(0.0f, bounds.width - 12.0f), bounds.height - 8.0f);
        }
        break;
    case model::WidgetType::CheckBox: {
        const float boxSize = std::min(bounds.height, 18.0f);
        canvas.setColor(0xffffffff);
        canvas.fill(bounds.x, bounds.y + (bounds.height - boxSize) * 0.5f, boxSize, boxSize);
        drawBorder(canvas, { bounds.x, bounds.y + (bounds.height - boxSize) * 0.5f, boxSize, boxSize }, 0xff8390a4);
        if (getBoolProperty(widget, "checked", false)) {
            canvas.setColor(0xff2d7ff9);
            canvas.fill(bounds.x + 4.0f, bounds.y + (bounds.height - 10.0f) * 0.5f, 10.0f, 10.0f);
        }
        if (drawText) {
            canvas.setColor(0xffeef3fa);
            canvas.text(getDisplayTextOrFallback(widget, "text", "CheckBox"), font, visage::Font::kTopLeft,
                bounds.x + boxSize + 8.0f, bounds.y + 4.0f,
                std::max(0.0f, bounds.width - boxSize - 12.0f), bounds.height - 6.0f);
        }
        break;
    }
    case model::WidgetType::Slider: {
        const float normalized = normalizedSliderValue(widget);
        const float trackY = bounds.y + bounds.height * 0.5f - 2.0f;
        const float trackLeft = bounds.x + 8.0f;
        const float trackWidth = std::max(0.0f, bounds.width - 16.0f);
        const float handleCenterX = trackLeft + trackWidth * normalized;
        const float handleX = std::clamp(handleCenterX - 6.0f, trackLeft - 6.0f, trackLeft + trackWidth - 6.0f);
        canvas.setColor(0xff8f9bae);
        canvas.fill(trackLeft, trackY, trackWidth, 4.0f);
        canvas.setColor(0xff2d7ff9);
        canvas.fill(handleX, bounds.y + bounds.height * 0.5f - 8.0f, 12.0f, 16.0f);
        drawBorder(canvas, { handleX, bounds.y + bounds.height * 0.5f - 8.0f, 12.0f, 16.0f }, 0xff11448f);
        if (drawText) {
            canvas.setColor(0xff1e2b3c);
            canvas.text(getStringProperty(widget, "text", widgetLabel(widget)), font, visage::Font::kTopLeft,
                bounds.x, bounds.y - 18.0f, bounds.width, 16.0f);
        }
        break;
    }
    case model::WidgetType::Image:
        canvas.setColor(0xffd8dfeb);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, 0xff98a3b3);
        if (drawText) {
            canvas.setColor(0xff314055);
            canvas.text(getStringProperty(widget, "source", "Image"), font, visage::Font::kCenter,
                bounds.x + 6.0f, bounds.y, std::max(0.0f, bounds.width - 12.0f), bounds.height);
        }
        break;
    case model::WidgetType::Spacer:
        canvas.setColor(0xffeef2f7);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, 0xffb1bbc9);
        if (drawText) {
            canvas.setColor(0xff596679);
            canvas.text("Spacer", font, visage::Font::kCenter, bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }

    if (widget.id == selectedWidgetId) {
        drawSelectionOutline(canvas, bounds);
        if (widget.type != model::WidgetType::FormWindow) {
            drawSelectionHandles(canvas, bounds, handleSize);
        }
    }

    for (const auto& child : widget.children) {
        drawWidget(canvas, font, drawText, child, formScreenX, formScreenY, widgetLocalX, widgetLocalY,
            scale, selectedWidgetId, handleSize, showGrid, showMinorGrid, gridSize, majorGridSize);
    }
}

} // namespace

void DesignerCanvas::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

bool DesignerCanvas::contains(float x, float y) const
{
    return x >= x_ && y >= y_ && x <= x_ + width_ && y <= y_ + height_;
}

std::optional<DesignerCanvas::FormPoint> DesignerCanvas::toFormPoint(const model::ProjectDocument& document, float x, float y) const
{
    if (!contains(x, y) || !document.root.bounds.isValid()) {
        return std::nullopt;
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(x_, y_, width_, height_, document);
    if (!previewLayout.form.contains(x, y) || previewLayout.scale <= 0.0f) {
        return std::nullopt;
    }

    return FormPoint{
        (x - previewLayout.form.x) / previewLayout.scale + document.root.bounds.x,
        (y - previewLayout.form.y) / previewLayout.scale + document.root.bounds.y
    };
}

std::optional<std::string> DesignerCanvas::hitTestWidgetId(const model::ProjectDocument& document, float x, float y) const
{
    const auto point = toFormPoint(document, x, y);
    if (!point.has_value()) {
        return std::nullopt;
    }

    if (const auto* hitWidget = document.root.hitTest(point->x, point->y)) {
        return hitWidget->id;
    }

    if (document.root.bounds.contains(point->x, point->y)) {
        return document.root.id;
    }

    return std::nullopt;
}

std::optional<DesignerCanvas::InteractionHit> DesignerCanvas::hitTestInteraction(const model::ProjectDocument& document,
    float x,
    float y,
    const std::string& selectedWidgetId) const
{
    const auto hitWidgetId = hitTestWidgetId(document, x, y);
    if (!hitWidgetId.has_value()) {
        return std::nullopt;
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(x_, y_, width_, height_, document);
    if (!previewLayout.form.isValid()) {
        return std::nullopt;
    }

    const auto widgetInfo = findWidgetScreenInfo(document.root, *hitWidgetId, previewLayout.form.x, previewLayout.form.y,
        -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale);
    if (!widgetInfo.has_value()) {
        return std::nullopt;
    }

    InteractionHit hit{ *hitWidgetId, HitRegion::Body };
    if (*hitWidgetId == selectedWidgetId && *hitWidgetId != document.root.id) {
        const HitRegion handle = hitHandle(widgetInfo->bounds, x, y, handleSize_);
        if (handle != HitRegion::None) {
            hit.region = handle;
            return hit;
        }
    }

    if (widgetInfo->bounds.contains(x, y)) {
        return hit;
    }

    return std::nullopt;
}

float DesignerCanvas::snap(float value) const
{
    if (!snapToGrid_ || gridSize_ <= 1) {
        return value;
    }

    return std::round(value / static_cast<float>(gridSize_)) * static_cast<float>(gridSize_);
}

model::Rect DesignerCanvas::moveBounds(const model::Rect& originalBounds, const FormPoint& dragStart, const FormPoint& currentPoint) const
{
    model::Rect updated = originalBounds;
    updated.x = originalBounds.x + (currentPoint.x - dragStart.x);
    updated.y = originalBounds.y + (currentPoint.y - dragStart.y);
    updated.x = snap(updated.x);
    updated.y = snap(updated.y);
    return updated;
}

model::Rect DesignerCanvas::resizeBounds(const model::Rect& originalBounds,
    HitRegion region,
    const FormPoint& dragStart,
    const FormPoint& currentPoint) const
{
    float left = originalBounds.x;
    float top = originalBounds.y;
    float right = originalBounds.x + originalBounds.width;
    float bottom = originalBounds.y + originalBounds.height;

    const float deltaX = currentPoint.x - dragStart.x;
    const float deltaY = currentPoint.y - dragStart.y;

    switch (region) {
    case HitRegion::TopLeftHandle:
        left = snap(left + deltaX);
        top = snap(top + deltaY);
        break;
    case HitRegion::TopRightHandle:
        right = snap(right + deltaX);
        top = snap(top + deltaY);
        break;
    case HitRegion::BottomLeftHandle:
        left = snap(left + deltaX);
        bottom = snap(bottom + deltaY);
        break;
    case HitRegion::BottomRightHandle:
        right = snap(right + deltaX);
        bottom = snap(bottom + deltaY);
        break;
    case HitRegion::None:
    case HitRegion::Body:
        return originalBounds;
    }

    if (right - left < minimumWidgetSize_) {
        if (region == HitRegion::TopLeftHandle || region == HitRegion::BottomLeftHandle) {
            left = right - minimumWidgetSize_;
        }
        else {
            right = left + minimumWidgetSize_;
        }
    }

    if (bottom - top < minimumWidgetSize_) {
        if (region == HitRegion::TopLeftHandle || region == HitRegion::TopRightHandle) {
            top = bottom - minimumWidgetSize_;
        }
        else {
            bottom = top + minimumWidgetSize_;
        }
    }

    return { left, top, std::max(minimumWidgetSize_, right - left), std::max(minimumWidgetSize_, bottom - top) };
}

void DesignerCanvas::draw(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::ProjectDocument& document) const
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

    const PreviewLayout previewLayout = calculatePreviewLayout(x_, y_, width_, height_, document);
    if (!previewLayout.preview.isValid()) {
        return;
    }

    canvas.setColor(0xff303746);
    canvas.fill(previewLayout.preview.x, previewLayout.preview.y, previewLayout.preview.width, previewLayout.preview.height);
    canvas.setColor(0xff475064);
    canvas.fill(previewLayout.preview.x + 1.0f, previewLayout.preview.y + 1.0f,
        previewLayout.preview.width - 2.0f, previewLayout.preview.height - 2.0f);

    if (!previewLayout.form.isValid()) {
        return;
    }

    drawWidget(canvas, font, drawText, document.root, previewLayout.form.x, previewLayout.form.y,
        -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale, document.selectedWidgetId,
        handleSize_, showGrid_, showMinorGrid_, gridSize_, majorGridSize_);

    if (drawText) {
        canvas.setColor(0xff243041);
        const std::string selectedLabel = document.hasSelection() && document.selectedWidget() != nullptr
            ? "Selected: " + widgetLabel(*document.selectedWidget())
            : "Selected: None";
        canvas.text(selectedLabel, font, visage::Font::kTopLeft,
            previewLayout.preview.x + 8.0f, previewLayout.preview.y + previewLayout.preview.height - 28.0f,
            previewLayout.preview.width - 16.0f, 22.0f);
    }
}

} // namespace visiform::ui
