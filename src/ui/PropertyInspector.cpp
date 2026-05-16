#include "ui/PropertyInspector.h"

#include "ui/PropertyInspector.h"

#include <iomanip>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kRowHeight = 30.0f;
constexpr float kPadding = 12.0f;
constexpr float kLabelColumnWidth = 104.0f;

struct RowLayout {
    PropertyInspector::PropertyRow row;
    float top = 0.0f;
};

std::string formatFloat(float value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    std::string text = stream.str();
    while (!text.empty() && text.back() == '0') {
        text.pop_back();
    }
    if (!text.empty() && text.back() == '.') {
        text.pop_back();
    }
    return text.empty() ? "0" : text;
}

std::string propertyValueText(const model::PropertyValue& value)
{
    return value.toDisplayString();
}

std::string displayTextOrFallback(const model::WidgetNode* widget, const std::string& key, const std::string& fallback)
{
    if (widget == nullptr) {
        return fallback;
    }

    const std::string value = widget->getStringProperty(key, {});
    return value.empty() ? fallback : value;
}

PropertyInspector::PropertyEditKind editKindForProperty(const model::PropertyValue& value)
{
    if (value.isBool()) {
        return PropertyInspector::PropertyEditKind::Bool;
    }
    if (value.isInt()) {
        return PropertyInspector::PropertyEditKind::Integer;
    }
    if (value.isFloat()) {
        return PropertyInspector::PropertyEditKind::Float;
    }
    if (value.isString() || value.isEmpty()) {
        return PropertyInspector::PropertyEditKind::Text;
    }

    return PropertyInspector::PropertyEditKind::ReadOnly;
}

std::vector<RowLayout> buildRowLayouts(float top, float height, const std::vector<PropertyInspector::PropertyRow>& rows)
{
    std::vector<RowLayout> layouts;
    float rowTop = top;
    for (const auto& row : rows) {
        if (rowTop + kRowHeight > height - 8.0f) {
            break;
        }

        layouts.push_back({ row, rowTop });
        rowTop += kRowHeight;
    }

    return layouts;
}

} // namespace

void PropertyInspector::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

bool PropertyInspector::contains(float x, float y) const
{
    return x >= x_ && y >= y_ && x <= x_ + width_ && y <= y_ + height_;
}

std::vector<PropertyInspector::PropertyRow> PropertyInspector::buildRows(const model::WidgetNode* selectedWidget) const
{
    std::vector<PropertyRow> rows;
    if (selectedWidget == nullptr) {
        return rows;
    }

    rows.push_back({ "id", "id", selectedWidget->id, PropertyEditKind::ReadOnly });
    rows.push_back({ "type", "type", selectedWidget->typeName(), PropertyEditKind::ReadOnly });
    rows.push_back({ "name", "name", selectedWidget->name, PropertyEditKind::Text });
    rows.push_back({ "x", "x", formatFloat(selectedWidget->bounds.x), PropertyEditKind::Float });
    rows.push_back({ "y", "y", formatFloat(selectedWidget->bounds.y), PropertyEditKind::Float });
    rows.push_back({ "width", "width", formatFloat(selectedWidget->bounds.width), PropertyEditKind::Float });
    rows.push_back({ "height", "height", formatFloat(selectedWidget->bounds.height), PropertyEditKind::Float });

    std::set<std::string> drawnKeys;
    for (const auto& row : rows) {
        drawnKeys.insert(row.key);
    }

    const auto addWidgetProperty = [&](const std::string& key) {
        if (const auto* property = selectedWidget->getProperty(key)) {
            rows.push_back({ key, key, propertyValueText(*property), editKindForProperty(*property) });
            drawnKeys.insert(key);
        }
    };
    const auto addEventProperty = [&](const std::string& key, const std::string& fallback = {}) {
        if (drawnKeys.contains(key)) {
            return;
        }

        const std::string displayValue = fallback.empty()
            ? displayTextOrFallback(selectedWidget, key, {})
            : displayTextOrFallback(selectedWidget, key, fallback);
        rows.push_back({ key, key, displayValue, PropertyEditKind::Text });
        drawnKeys.insert(key);
    };

    switch (selectedWidget->type) {
    case model::WidgetType::Label:
    case model::WidgetType::Button:
    case model::WidgetType::TextBox:
        addWidgetProperty("text");
        break;
    case model::WidgetType::CheckBox:
        addWidgetProperty("text");
        addWidgetProperty("checked");
        break;
    case model::WidgetType::Slider:
        addWidgetProperty("min");
        addWidgetProperty("max");
        addWidgetProperty("value");
        break;
    case model::WidgetType::Frame:
        addWidgetProperty("title");
        addWidgetProperty("backgroundColor");
        break;
    case model::WidgetType::Image:
        addWidgetProperty("source");
        break;
    case model::WidgetType::Spacer:
        break;
    case model::WidgetType::FormWindow:
        addWidgetProperty("title");
        addWidgetProperty("backgroundColor");
        break;
    }

    const std::size_t propertyCountBeforeEvents = rows.size();
    switch (selectedWidget->type) {
    case model::WidgetType::Button:
        addEventProperty("onClick");
        break;
    case model::WidgetType::CheckBox:
        addEventProperty("onToggle");
        break;
    case model::WidgetType::Slider:
        addEventProperty("onChanged");
        break;
    case model::WidgetType::TextBox:
        addEventProperty("onTextChanged");
        break;
    case model::WidgetType::FormWindow:
        addEventProperty("onLoad");
        addEventProperty("onClose");
        break;
    case model::WidgetType::Label:
    case model::WidgetType::Frame:
    case model::WidgetType::Image:
    case model::WidgetType::Spacer:
        break;
    }

    if (rows.size() > propertyCountBeforeEvents) {
        rows.insert(rows.begin() + static_cast<std::ptrdiff_t>(propertyCountBeforeEvents),
            PropertyRow{ "__section_events", "Events", {}, PropertyEditKind::ReadOnly, true });
    }

    for (const auto& [key, value] : selectedWidget->properties) {
        if (drawnKeys.contains(key)) {
            continue;
        }

        rows.push_back({ key, key, propertyValueText(value), editKindForProperty(value) });
    }

    return rows;
}

std::optional<PropertyInspector::PropertyRow> PropertyInspector::hitTestRow(const model::WidgetNode* selectedWidget, float x, float y) const
{
    if (!contains(x, y)) {
        return std::nullopt;
    }

    const auto layouts = buildRowLayouts(y_ + kHeaderHeight + 8.0f, y_ + height_, buildRows(selectedWidget));
    for (const auto& layout : layouts) {
        if (y >= layout.top && y <= layout.top + kRowHeight) {
            return layout.row;
        }
    }

    return std::nullopt;
}

bool PropertyInspector::beginEditing(const model::WidgetNode* selectedWidget, const std::string& key)
{
    const auto rows = buildRows(selectedWidget);
    for (const auto& row : rows) {
        if (row.key == key && row.editKind != PropertyEditKind::ReadOnly && row.editKind != PropertyEditKind::Bool) {
            activeKey_ = key;
            activeEditKind_ = row.editKind;
            editBuffer_ = row.displayValue;
            return true;
        }
    }

    return false;
}

std::optional<PropertyInspector::PropertyRow> PropertyInspector::activeRow(const model::WidgetNode* selectedWidget) const
{
    if (!isEditing()) {
        return std::nullopt;
    }

    const auto rows = buildRows(selectedWidget);
    for (const auto& row : rows) {
        if (row.key == activeKey_) {
            return row;
        }
    }

    return std::nullopt;
}

void PropertyInspector::clearEditing()
{
    activeKey_.clear();
    activeEditKind_ = PropertyEditKind::ReadOnly;
}

void PropertyInspector::cancelEditing()
{
    clearEditing();
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::activeEditorBounds(const model::WidgetNode* selectedWidget) const
{
    const auto active = activeRow(selectedWidget);
    if (!active.has_value()) {
        return std::nullopt;
    }

    const auto layouts = buildRowLayouts(y_ + kHeaderHeight + 8.0f, y_ + height_, buildRows(selectedWidget));
    for (const auto& layout : layouts) {
        if (layout.row.key == active->key) {
            const float valueLeft = x_ + kLabelColumnWidth;
            return ValueCellBounds{ valueLeft + 2.0f, layout.top + 2.0f, width_ - kLabelColumnWidth - 24.0f, kRowHeight - 6.0f };
        }
    }

    return std::nullopt;
}

std::optional<PropertyInspector::PendingEdit> PropertyInspector::buildPendingEdit(const std::string& valueText) const
{
    if (!isEditing()) {
        return std::nullopt;
    }

    return PendingEdit{ activeKey_, valueText, activeEditKind_ };
}

bool PropertyInspector::isEditing() const
{
    return !activeKey_.empty();
}

void PropertyInspector::draw(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::WidgetNode* selectedWidget, std::size_t selectionCount) const
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
        const std::string title = selectionCount > 1
            ? "Property Inspector - Multi-select: " + std::to_string(selectionCount)
            : "Property Inspector";
        canvas.text(title, font, visage::Font::kTopLeft,
            x_ + kPadding, y_ + 6.0f, width_ - kPadding * 2.0f, kHeaderHeight - 8.0f);
    }

    if (selectedWidget == nullptr) {
        if (drawText) {
            canvas.setColor(0xffdde2ea);
            canvas.text("No selection", font, visage::Font::kTopLeft,
                x_ + 18.0f, y_ + kHeaderHeight + 12.0f, width_ - 30.0f, kRowHeight - 8.0f);
        }
        return;
    }

    const auto layouts = buildRowLayouts(y_ + kHeaderHeight + 8.0f, y_ + height_, buildRows(selectedWidget));
    for (std::size_t index = 0; index < layouts.size(); ++index) {
        const auto& row = layouts[index].row;
        const float rowTop = layouts[index].top;

        canvas.setColor(index % 2 == 0 ? 0xff2b313d : 0xff262c37);
        canvas.fill(x_ + 8.0f, rowTop, width_ - 16.0f, kRowHeight - 2.0f);

        const float labelLeft = x_ + 18.0f;
        const float valueLeft = x_ + kLabelColumnWidth;
        const float valueWidth = width_ - kLabelColumnWidth - 20.0f;
        const bool isReadOnly = row.editKind == PropertyEditKind::ReadOnly;
        const bool isActive = isEditing() && row.key == activeKey_;

        if (row.isSection) {
            canvas.setColor(0xff253246);
            canvas.fill(x_ + 8.0f, rowTop, width_ - 16.0f, kRowHeight - 2.0f);
            if (drawText) {
                canvas.setColor(0xffa9c7f6);
                canvas.text(row.label, font, visage::Font::kTopLeft,
                    labelLeft, rowTop + 5.0f, width_ - 32.0f, kRowHeight - 8.0f);
            }
            continue;
        }

        canvas.setColor(isActive ? 0xff2f476d : (isReadOnly ? 0xff303541 : 0xff39414f));
        canvas.fill(valueLeft, rowTop + 2.0f, valueWidth, kRowHeight - 6.0f);
        if (isActive) {
            canvas.setColor(0xff92b9ff);
            canvas.fill(valueLeft, rowTop + kRowHeight - 4.0f, valueWidth, 2.0f);
        }

        if (drawText) {
            canvas.setColor(0xffc7cfda);
            canvas.text(row.label, font, visage::Font::kTopLeft,
                labelLeft, rowTop + 5.0f, kLabelColumnWidth - 24.0f, kRowHeight - 8.0f);

            canvas.setColor(isReadOnly ? 0xffb3bcc9 : 0xffeef2f8);
            if (!isActive) {
                canvas.text(row.displayValue, font, visage::Font::kTopLeft,
                    valueLeft + 8.0f, rowTop + 5.0f, valueWidth - 12.0f, kRowHeight - 8.0f);
            }

            if (isActive) {
                canvas.setColor(0xff92b9ff);
                canvas.text("Enter=Apply  Esc=Cancel", font, visage::Font::kTopLeft,
                    valueLeft + 8.0f, rowTop + kRowHeight - 2.0f, valueWidth - 12.0f, 16.0f);
            }
        }
    }
}

} // namespace visiform::ui
