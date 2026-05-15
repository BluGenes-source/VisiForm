#include "ui/DesignerCanvas.h"

#include "ui/DesignerCanvas.h"

#include <algorithm>
#include <cstdint>
#include <optional>
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
    drawBorder(canvas, { bounds.x - 3.0f, bounds.y - 3.0f, bounds.width + 6.0f, bounds.height + 6.0f }, 0xff2d7ff9, 2.0f);
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
    const std::string& selectedWidgetId)
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
        canvas.setColor(parseColorOrDefault(widget.getStringProperty("backgroundColor", "#eceff5"), 0xffeceff5));
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        canvas.setColor(0xffc0c8d5);
        canvas.fill(bounds.x, bounds.y, bounds.width, std::min(kTitleBarHeight, bounds.height));
        drawBorder(canvas, bounds, 0xff6c7788);

        if (drawText) {
            canvas.setColor(0xff243041);
            canvas.text(widget.getStringProperty("title", widgetLabel(widget)), font, visage::Font::kTopLeft,
                bounds.x + 10.0f, bounds.y + 4.0f, std::max(0.0f, bounds.width - 20.0f), 22.0f);
        }
        break;
    }
    case model::WidgetType::Frame:
        canvas.setColor(parseColorOrDefault(widget.getStringProperty("backgroundColor", "#d9dee8"), 0xffd9dee8));
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, 0xff97a3b7);
        if (drawText) {
            canvas.setColor(0xff243041);
            canvas.text(widgetLabel(widget), font, visage::Font::kTopLeft,
                bounds.x + 8.0f, bounds.y + 6.0f, std::max(0.0f, bounds.width - 16.0f), 20.0f);
        }
        break;
    case model::WidgetType::Label:
        if (drawText) {
            canvas.setColor(0xff1f2530);
            canvas.text(widget.getStringProperty("text", widgetLabel(widget)), font, visage::Font::kTopLeft,
                bounds.x, bounds.y + 2.0f, bounds.width, bounds.height);
        }
        break;
    case model::WidgetType::Button:
        canvas.setColor(0xffebedf2);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, 0xffccd2dc);
        if (drawText) {
            canvas.setColor(0xff1f2530);
            canvas.text(widget.getStringProperty("text", widgetLabel(widget)), font, visage::Font::kCenter,
                bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    case model::WidgetType::TextBox:
        canvas.setColor(0xffffffff);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, 0xffb9c2d0);
        if (drawText) {
            canvas.setColor(0xff586275);
            canvas.text(widget.getStringProperty("text", widgetLabel(widget)), font, visage::Font::kTopLeft,
                bounds.x + 8.0f, bounds.y + 6.0f, std::max(0.0f, bounds.width - 12.0f), bounds.height - 8.0f);
        }
        break;
    case model::WidgetType::CheckBox: {
        const float boxSize = std::min(bounds.height, 18.0f);
        canvas.setColor(0xffffffff);
        canvas.fill(bounds.x, bounds.y + (bounds.height - boxSize) * 0.5f, boxSize, boxSize);
        drawBorder(canvas, { bounds.x, bounds.y + (bounds.height - boxSize) * 0.5f, boxSize, boxSize }, 0xff8390a4);
        if (drawText) {
            canvas.setColor(0xff1f2530);
            canvas.text(widget.getStringProperty("text", widgetLabel(widget)), font, visage::Font::kTopLeft,
                bounds.x + boxSize + 8.0f, bounds.y + 4.0f, std::max(0.0f, bounds.width - boxSize - 8.0f), bounds.height - 6.0f);
        }
        break;
    }
    case model::WidgetType::Slider: {
        const float trackY = bounds.y + bounds.height * 0.5f - 2.0f;
        canvas.setColor(0xff98a3b3);
        canvas.fill(bounds.x + 8.0f, trackY, std::max(0.0f, bounds.width - 16.0f), 4.0f);
        canvas.setColor(0xff2d7ff9);
        canvas.fill(bounds.x + bounds.width * 0.55f - 6.0f, bounds.y + bounds.height * 0.5f - 8.0f, 12.0f, 16.0f);
        break;
    }
    case model::WidgetType::Image:
        canvas.setColor(0xffd3dae6);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, 0xff98a3b3);
        if (drawText) {
            canvas.setColor(0xff4e596c);
            canvas.text("Image", font, visage::Font::kCenter, bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    case model::WidgetType::Spacer:
        drawBorder(canvas, bounds, 0xff98a3b3);
        if (drawText) {
            canvas.setColor(0xff4e596c);
            canvas.text("Spacer", font, visage::Font::kCenter, bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }

    if (widget.id == selectedWidgetId) {
        drawSelectionOutline(canvas, bounds);
    }

    for (const auto& child : widget.children) {
        drawWidget(canvas, font, drawText, child, formScreenX, formScreenY, widgetLocalX, widgetLocalY, scale, selectedWidgetId);
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

std::optional<std::string> DesignerCanvas::hitTestWidgetId(const model::ProjectDocument& document, float x, float y) const
{
    if (!contains(x, y) || !document.root.bounds.isValid()) {
        return std::nullopt;
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(x_, y_, width_, height_, document);
    if (!previewLayout.form.contains(x, y) || previewLayout.scale <= 0.0f) {
        return std::nullopt;
    }

    const float localX = (x - previewLayout.form.x) / previewLayout.scale + document.root.bounds.x;
    const float localY = (y - previewLayout.form.y) / previewLayout.scale + document.root.bounds.y;
    if (const auto* hitWidget = document.root.hitTest(localX, localY)) {
        return hitWidget->id;
    }

    if (document.root.bounds.contains(localX, localY)) {
        return document.root.id;
    }

    return std::nullopt;
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
        -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale, document.selectedWidgetId);

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
