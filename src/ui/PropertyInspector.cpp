#include "ui/PropertyInspector.h"

#include "ui/PropertyInspector.h"

#include "model/WidgetRegistry.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kRowHeight = 30.0f;
constexpr float kSuggestionRowHeight = 24.0f;
constexpr float kSuggestionSpacing = 2.0f;
constexpr float kPadding = 12.0f;
constexpr float kLabelColumnWidth = 104.0f;
constexpr float kScrollBarWidth = 18.0f;
constexpr float kScrollBarGap = 6.0f;
constexpr float kMinimumThumbSize = 18.0f;
constexpr float kMouseWheelSensitivity = 40.0f;

struct RowLayout {
    PropertyInspector::PropertyRow row;
    float top = 0.0f;

    [[nodiscard]] float bottom() const
    {
        return top + kRowHeight;
    }
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

bool isColorPropertyKey(const std::string& key);

PropertyInspector::PropertyEditKind editKindForProperty(const std::string& key, const model::PropertyValue& value)
{
    if (isColorPropertyKey(key) && (value.isString() || value.isEmpty())) {
        return PropertyInspector::PropertyEditKind::Color;
    }

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

const model::WidgetEventDefinition* findEventDefinition(model::WidgetType type, const std::string& key)
{
    if (const auto* definition = model::WidgetRegistry::instance().find(type)) {
        for (const auto& event : definition->events) {
            if (event.key == key) {
                return &event;
            }
        }
    }

    return nullptr;
}

bool isStylePropertyKey(const std::string& key)
{
    return key == "lookAndFeelId"
        || key == "fillColor"
        || key == "textColor"
        || key == "borderColor"
        || key == "accentColor"
        || key == "borderThickness"
        || key == "cornerRadius"
        || key == "fontFamily"
        || key == "fontSize"
        || key == "fontBold"
        || key == "fontItalic";
}

bool isColorPropertyKey(const std::string& key)
{
    return key == "backgroundColor"
        || key == "fillColor"
        || key == "textColor"
        || key == "borderColor"
        || key == "accentColor"
        || key == "panelColor"
        || key == "controlFillColor"
        || key == "controlTextColor"
        || key == "controlBorderColor"
        || key == "disabledColor";
}

bool isValidHexColor(const std::string& value)
{
    if (value.empty() || value.front() != '#') {
        return false;
    }

    if (value.size() != 7 && value.size() != 9) {
        return false;
    }

    return std::all_of(value.begin() + 1, value.end(), [](unsigned char character) {
        return std::isxdigit(character) != 0;
    });
}

int parseColorOrDefault(const std::string& value, int defaultColor)
{
    if (!isValidHexColor(value)) {
        return defaultColor;
    }

    try {
        const std::string digits = value.substr(1);
        const std::uint32_t parsed = static_cast<std::uint32_t>(std::stoul(digits, nullptr, 16));
        if (digits.size() == 6) {
            return static_cast<int>(0xff000000u | parsed);
        }

        return static_cast<int>(parsed);
    }
    catch (...) {
        return defaultColor;
    }
}

void collectMatchingHandlers(const model::WidgetNode& widget,
    const std::string& signatureKind,
    std::set<std::string>& handlerNames)
{
    if (const auto* definition = model::WidgetRegistry::instance().find(widget.type)) {
        for (const auto& event : definition->events) {
            if (event.handlerSignatureKind != signatureKind) {
                continue;
            }

            const std::string handlerName = widget.getStringProperty(event.key, {});
            if (!handlerName.empty()) {
                handlerNames.insert(handlerName);
            }
        }
    }

    for (const auto& child : widget.children) {
        collectMatchingHandlers(child, signatureKind, handlerNames);
    }
}

PropertyInspector::PropertyEditKind editKindForDefinition(const model::WidgetPropertyDefinition& property)
{
    if (!property.editable) {
        return PropertyInspector::PropertyEditKind::ReadOnly;
    }

    if (!property.choices.empty()) {
        return PropertyInspector::PropertyEditKind::Choice;
    }

    switch (property.editKind) {
    case model::PropertyEditKind::Text:
    case model::PropertyEditKind::FilePath:
        return PropertyInspector::PropertyEditKind::Text;
    case model::PropertyEditKind::Color:
        return PropertyInspector::PropertyEditKind::Color;
    case model::PropertyEditKind::Integer:
        return PropertyInspector::PropertyEditKind::Integer;
    case model::PropertyEditKind::Float:
        return PropertyInspector::PropertyEditKind::Float;
    case model::PropertyEditKind::Bool:
        return PropertyInspector::PropertyEditKind::Bool;
    case model::PropertyEditKind::ReadOnly:
        return PropertyInspector::PropertyEditKind::ReadOnly;
    }

    return PropertyInspector::PropertyEditKind::ReadOnly;
}

std::vector<RowLayout> buildRowLayouts(float top, const std::vector<PropertyInspector::PropertyRow>& rows)
{
    std::vector<RowLayout> layouts;
    layouts.reserve(rows.size());
    float rowTop = top;
    for (const auto& row : rows) {
        layouts.push_back({ row, rowTop });
        rowTop += kRowHeight;
    }

    return layouts;
}

bool containsPoint(const PropertyInspector::ValueCellBounds& bounds, float x, float y)
{
    return x >= bounds.x && y >= bounds.y && x <= bounds.x + bounds.width && y <= bounds.y + bounds.height;
}

} // namespace

void PropertyInspector::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
    clampScrollOffset();
}

bool PropertyInspector::contains(float x, float y) const
{
    return x >= x_ && y >= y_ && x <= x_ + width_ && y <= y_ + height_;
}

void PropertyInspector::updateScrollMetrics(const std::vector<PropertyRow>& rows)
{
    visibleHeight_ = std::max(0.0f, height_ - kHeaderHeight - 16.0f);
    contentHeight_ = static_cast<float>(rows.size()) * kRowHeight;
    needsVerticalScrollBar_ = contentHeight_ > visibleHeight_ + 0.5f;
    clampScrollOffset();
}

void PropertyInspector::clampScrollOffset()
{
    const float maxScroll = std::max(0.0f, contentHeight_ - visibleHeight_);
    scrollOffsetY_ = std::clamp(scrollOffsetY_, 0.0f, maxScroll);
    if (!needsVerticalScrollBar_ || maxScroll <= 0.0f) {
        scrollOffsetY_ = 0.0f;
    }
}

float PropertyInspector::rowYWithScroll(float originalY) const
{
    return originalY - scrollOffsetY_;
}

PropertyInspector::ValueCellBounds PropertyInspector::contentBounds() const
{
    return {
        x_ + 8.0f,
        y_ + kHeaderHeight + 8.0f,
        std::max(0.0f, width_ - 16.0f - (needsVerticalScrollBar_ ? (kScrollBarWidth + kScrollBarGap) : 0.0f)),
        visibleHeight_
    };
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::scrollBarBounds() const
{
    if (!needsVerticalScrollBar_ || visibleHeight_ <= 0.0f) {
        return std::nullopt;
    }

    return ValueCellBounds{
        x_ + width_ - 8.0f - kScrollBarWidth,
        y_ + kHeaderHeight + 8.0f,
        kScrollBarWidth,
        visibleHeight_
    };
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::scrollBarThumbBounds() const
{
    const auto scrollBar = scrollBarBounds();
    if (!scrollBar.has_value()) {
        return std::nullopt;
    }

    const float arrowSize = std::min(scrollBar->width, 20.0f);
    const float trackTop = scrollBar->y + arrowSize;
    const float trackHeight = std::max(0.0f, scrollBar->height - arrowSize * 2.0f);
    if (trackHeight <= 0.0f || contentHeight_ <= 0.0f) {
        return std::nullopt;
    }

    const float maxScroll = std::max(0.0f, contentHeight_ - visibleHeight_);
    const float normalized = maxScroll <= 0.0f ? 0.0f : scrollOffsetY_ / maxScroll;
    const float thumbHeight = std::clamp(trackHeight * (visibleHeight_ / contentHeight_), kMinimumThumbSize, trackHeight);
    const float thumbY = trackTop + (trackHeight - thumbHeight) * normalized;
    return ValueCellBounds{ scrollBar->x + 4.0f, thumbY, std::max(0.0f, scrollBar->width - 8.0f), thumbHeight };
}

bool PropertyInspector::isWithinVisibleContent(float x, float y) const
{
    const ValueCellBounds bounds = contentBounds();
    return containsPoint(bounds, x, y);
}

float PropertyInspector::valueCellWidth() const
{
    const ValueCellBounds bounds = contentBounds();
    return std::max(0.0f, bounds.width - (kLabelColumnWidth - (bounds.x - x_)) - 12.0f);
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::colorSwatchBoundsForRow(const PropertyRow& row, float rowTop) const
{
    if (row.editKind != PropertyEditKind::Color || row.isSection) {
        return std::nullopt;
    }

    const float swatchSize = std::max(0.0f, kRowHeight - 12.0f);
    const float swatchX = x_ + kLabelColumnWidth + valueCellWidth() - swatchSize - 8.0f;
    return ValueCellBounds{ swatchX, rowTop + 6.0f, swatchSize, swatchSize };
}

std::vector<PropertyInspector::PropertyRow> PropertyInspector::buildRows(const model::ProjectDocument& document) const
{
    std::vector<PropertyRow> rows;
    const model::WidgetNode* selectedWidget = document.selectedWidget();
    if (selectedWidget == nullptr) {
        return rows;
    }

    rows.push_back({ "id", "id", selectedWidget->id, PropertyEditKind::ReadOnly });
    rows.push_back({ "type", "type", selectedWidget->typeName(), PropertyEditKind::ReadOnly });
    rows.push_back({ "name", "name", selectedWidget->name, PropertyEditKind::Text });
    if (selectedWidget->type == model::WidgetType::FormWindow) {
        rows.push_back({ "projectName", "projectName", document.projectName, PropertyEditKind::Text });
        rows.push_back({ "executableName", "executableName", document.executableName, PropertyEditKind::Text });
        rows.push_back({ "generatedBaseClassName", "generatedBaseClassName", "MainWindow", PropertyEditKind::ReadOnly });
        rows.push_back({ "userSubclassName", "userSubclassName", document.userSubclassName, PropertyEditKind::Text });
        rows.push_back({ "windowTitle", "windowTitle", document.windowTitle, PropertyEditKind::Text });
        rows.push_back({ "lookAndFeelId", "lookAndFeelId", document.lookAndFeelId, PropertyEditKind::Text });
    }
    rows.push_back({ "x", "x", formatFloat(selectedWidget->bounds.x), PropertyEditKind::Float });
    rows.push_back({ "y", "y", formatFloat(selectedWidget->bounds.y), PropertyEditKind::Float });
    rows.push_back({ "width", "width", formatFloat(selectedWidget->bounds.width), PropertyEditKind::Float });
    rows.push_back({ "height", "height", formatFloat(selectedWidget->bounds.height), PropertyEditKind::Float });

    std::set<std::string> drawnKeys;
    for (const auto& row : rows) {
        drawnKeys.insert(row.key);
    }
    if (selectedWidget->type == model::WidgetType::FormWindow) {
        drawnKeys.insert("title");
    }

    const auto addEventProperty = [&](const std::string& key, const std::string& label, const std::string& fallback = {}) {
        if (drawnKeys.contains(key)) {
            return;
        }

        const std::string displayValue = fallback.empty()
            ? displayTextOrFallback(selectedWidget, key, {})
            : displayTextOrFallback(selectedWidget, key, fallback);
        rows.push_back({ key, label, displayValue, PropertyEditKind::Text });
        drawnKeys.insert(key);
    };

    if (const auto* definition = model::WidgetRegistry::instance().find(selectedWidget->type)) {
        bool styleSectionInserted = false;
        for (const auto& property : definition->properties) {
            if (isStylePropertyKey(property.key) && !styleSectionInserted) {
                rows.push_back({ "__section_style", "Style", {}, PropertyEditKind::ReadOnly, true });
                styleSectionInserted = true;
            }
            const auto* propertyValue = selectedWidget->getProperty(property.key);
            const std::string displayValue = propertyValue != nullptr ? propertyValueText(*propertyValue) : property.defaultValue.toDisplayString();
            rows.push_back({ property.key, property.label, displayValue, editKindForDefinition(property), false, property.choices });
            drawnKeys.insert(property.key);
        }

        const std::size_t propertyCountBeforeEvents = rows.size();
        for (const auto& event : definition->events) {
            addEventProperty(event.key, event.label);
        }

        if (rows.size() > propertyCountBeforeEvents) {
            rows.insert(rows.begin() + static_cast<std::ptrdiff_t>(propertyCountBeforeEvents),
                PropertyRow{ "__section_events", "Events", {}, PropertyEditKind::ReadOnly, true });
        }
    }

    for (const auto& [key, value] : selectedWidget->properties) {
        if (drawnKeys.contains(key)) {
            continue;
        }

        rows.push_back({ key, key, propertyValueText(value), editKindForProperty(key, value) });
    }

    return rows;
}

void PropertyInspector::rebuildSuggestions(const model::ProjectDocument& document)
{
    suggestions_.clear();
    activeCallbackPropertyKey_.clear();

    if (!isEditing()) {
        return;
    }

    const auto rows = buildRows(document);
    updateScrollMetrics(rows);
    const auto layouts = buildRowLayouts(contentBounds().y, rows);
    const auto activeLayout = std::find_if(layouts.begin(), layouts.end(), [this](const RowLayout& layout) {
        return layout.row.key == activeCallbackPropertyKey_;
    });
    if (activeLayout == layouts.end()) {
        return;
    }

    std::vector<std::string> suggestionValues;
    if (!activeLayout->row.choices.empty()) {
        suggestionValues = activeLayout->row.choices;
    }
    else {
        const model::WidgetNode* selectedWidget = document.selectedWidget();
        if (selectedWidget == nullptr) {
            return;
        }

        const auto* eventDefinition = findEventDefinition(selectedWidget->type, activeKey_);
        if (eventDefinition == nullptr) {
            return;
        }

        activeCallbackPropertyKey_ = activeKey_;
        std::set<std::string> handlerNames;
        collectMatchingHandlers(document.root, eventDefinition->handlerSignatureKind, handlerNames);
        suggestionValues.assign(handlerNames.begin(), handlerNames.end());
    }

    if (suggestionValues.empty()) {
        return;
    }

    const ValueCellBounds bounds = contentBounds();
    const float visibleRowTop = rowYWithScroll(activeLayout->top);
    if (visibleRowTop + kRowHeight < bounds.y || visibleRowTop > bounds.y + bounds.height) {
        return;
    }

    const float valueLeft = x_ + kLabelColumnWidth;
    const float valueWidth = valueCellWidth();
    const auto totalHeightForCount = [](std::size_t count) {
        if (count == 0) {
            return 0.0f;
        }

        return static_cast<float>(count) * kSuggestionRowHeight
            + static_cast<float>(count - 1) * kSuggestionSpacing;
    };
    const auto fitCountForSpace = [&](float availableSpace) {
        std::size_t count = 0;
        while (count < suggestionValues.size() && totalHeightForCount(count + 1) <= availableSpace) {
            ++count;
        }
        return count;
    };

    const float spaceBelow = std::max(0.0f, bounds.y + bounds.height - (visibleRowTop + kRowHeight));
    const float spaceAbove = std::max(0.0f, visibleRowTop - bounds.y);
    std::size_t visibleCountBelow = fitCountForSpace(spaceBelow);
    std::size_t visibleCountAbove = fitCountForSpace(spaceAbove);
    const bool drawUpward = visibleCountAbove > visibleCountBelow;
    std::size_t visibleCount = drawUpward ? visibleCountAbove : visibleCountBelow;
    if (visibleCount == 0) {
        return;
    }

    const float totalHeight = totalHeightForCount(visibleCount);
    float itemTop = drawUpward
        ? std::max(bounds.y, visibleRowTop - kSuggestionSpacing - totalHeight)
        : visibleRowTop + kRowHeight;
    for (std::size_t index = 0; index < visibleCount; ++index) {
        suggestions_.push_back(CallbackSuggestionItem{
            suggestionValues[index],
            valueLeft,
            itemTop,
            valueWidth,
            kSuggestionRowHeight
        });
        itemTop += kSuggestionRowHeight + kSuggestionSpacing;
    }
}

std::optional<PropertyInspector::PropertyRow> PropertyInspector::hitTestRow(const model::ProjectDocument& document, float x, float y)
{
    const auto rows = buildRows(document);
    updateScrollMetrics(rows);
    if (!isWithinVisibleContent(x, y)) {
        return std::nullopt;
    }

    const auto layouts = buildRowLayouts(contentBounds().y, rows);
    for (const auto& layout : layouts) {
        const float rowTop = rowYWithScroll(layout.top);
        if (rowTop + kRowHeight < contentBounds().y || rowTop > contentBounds().y + contentBounds().height) {
            continue;
        }

        if (y >= rowTop && y <= rowTop + kRowHeight) {
            return layout.row;
        }
    }

    return std::nullopt;
}

std::optional<std::string> PropertyInspector::hitTestColorSwatch(const model::ProjectDocument& document, float x, float y)
{
    const auto rows = buildRows(document);
    updateScrollMetrics(rows);
    if (!isWithinVisibleContent(x, y)) {
        return std::nullopt;
    }

    const auto layouts = buildRowLayouts(contentBounds().y, rows);
    const auto bounds = contentBounds();
    for (const auto& layout : layouts) {
        const float rowTop = rowYWithScroll(layout.top);
        if (rowTop + kRowHeight < bounds.y || rowTop > bounds.y + bounds.height) {
            continue;
        }

        const auto swatch = colorSwatchBoundsForRow(layout.row, rowTop);
        if (swatch.has_value() && containsPoint(*swatch, x, y)) {
            return layout.row.key;
        }
    }

    return std::nullopt;
}

std::optional<std::string> PropertyInspector::hitTestSuggestion(const model::ProjectDocument& document, float x, float y)
{
    if (!contains(x, y)) {
        return std::nullopt;
    }

    rebuildSuggestions(document);

    for (const auto& item : suggestions_) {
        if (x >= item.x && x <= item.x + item.width && y >= item.y && y <= item.y + item.height) {
            return item.value;
        }
    }

    return std::nullopt;
}

bool PropertyInspector::mouseDown(const model::ProjectDocument& document, float x, float y)
{
    const auto rows = buildRows(document);
    updateScrollMetrics(rows);
    const auto scrollBar = scrollBarBounds();
    if (!scrollBar.has_value() || !containsPoint(*scrollBar, x, y)) {
        return false;
    }

    const float arrowSize = std::min(scrollBar->width, 20.0f);
    const auto thumb = scrollBarThumbBounds();
    if (thumb.has_value() && containsPoint(*thumb, x, y)) {
        draggingScrollBarThumb_ = true;
        scrollBarDragOffsetY_ = y - thumb->y;
        return true;
    }

    if (y < scrollBar->y + arrowSize) {
        scrollOffsetY_ -= kRowHeight;
    }
    else if (y > scrollBar->y + scrollBar->height - arrowSize) {
        scrollOffsetY_ += kRowHeight;
    }
    else if (thumb.has_value() && y < thumb->y) {
        scrollOffsetY_ -= std::max(kRowHeight, visibleHeight_ * 0.85f);
    }
    else {
        scrollOffsetY_ += std::max(kRowHeight, visibleHeight_ * 0.85f);
    }

    clampScrollOffset();
    rebuildSuggestions(document);
    return true;
}

bool PropertyInspector::mouseDrag(const model::ProjectDocument& document, float x, float y)
{
    (void)x;
    if (!draggingScrollBarThumb_) {
        return false;
    }

    const auto scrollBar = scrollBarBounds();
    const auto thumb = scrollBarThumbBounds();
    if (!scrollBar.has_value() || !thumb.has_value()) {
        draggingScrollBarThumb_ = false;
        return false;
    }

    const float arrowSize = std::min(scrollBar->width, 20.0f);
    const float trackTop = scrollBar->y + arrowSize;
    const float trackHeight = std::max(0.0f, scrollBar->height - arrowSize * 2.0f);
    const float maxThumbTop = trackTop + std::max(0.0f, trackHeight - thumb->height);
    const float thumbTop = std::clamp(y - scrollBarDragOffsetY_, trackTop, maxThumbTop);
    const float maxScroll = std::max(0.0f, contentHeight_ - visibleHeight_);
    if (trackHeight > thumb->height && maxScroll > 0.0f) {
        scrollOffsetY_ = maxScroll * ((thumbTop - trackTop) / (trackHeight - thumb->height));
    }
    else {
        scrollOffsetY_ = 0.0f;
    }

    clampScrollOffset();
    rebuildSuggestions(document);
    return true;
}

bool PropertyInspector::mouseUp()
{
    const bool wasDragging = draggingScrollBarThumb_;
    draggingScrollBarThumb_ = false;
    scrollBarDragOffsetY_ = 0.0f;
    return wasDragging;
}

bool PropertyInspector::mouseWheel(const model::ProjectDocument& document, float deltaY, float x, float y)
{
    if (!contains(x, y)) {
        return false;
    }

    const auto rows = buildRows(document);
    updateScrollMetrics(rows);
    if (!needsVerticalScrollBar_) {
        return false;
    }

    scrollOffsetY_ += -deltaY * kMouseWheelSensitivity;
    clampScrollOffset();
    rebuildSuggestions(document);
    return true;
}

bool PropertyInspector::beginEditing(const model::ProjectDocument& document, const std::string& key)
{
    const auto rows = buildRows(document);
    const model::WidgetNode* selectedWidget = document.selectedWidget();
    for (const auto& row : rows) {
        if (row.key == key && row.editKind != PropertyEditKind::ReadOnly && row.editKind != PropertyEditKind::Bool) {
            activeKey_ = key;
            activeEditKind_ = row.editKind;
            editBuffer_ = row.displayValue;
            activeCallbackPropertyKey_.clear();
            if (row.choices.empty() && selectedWidget != nullptr && findEventDefinition(selectedWidget->type, key) != nullptr) {
                activeCallbackPropertyKey_ = key;
            }
            rebuildSuggestions(document);
            return true;
        }
    }

    return false;
}

std::optional<PropertyInspector::PropertyRow> PropertyInspector::activeRow(const model::ProjectDocument& document) const
{
    if (!isEditing()) {
        return std::nullopt;
    }

    const auto rows = buildRows(document);
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
    activeCallbackPropertyKey_.clear();
    suggestions_.clear();
    activeEditKind_ = PropertyEditKind::ReadOnly;
}

void PropertyInspector::cancelEditing()
{
    clearEditing();
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::activeEditorBounds(const model::ProjectDocument& document)
{
    const auto active = activeRow(document);
    if (!active.has_value()) {
        return std::nullopt;
    }

    if (active->editKind == PropertyEditKind::Choice) {
        return std::nullopt;
    }

    const auto rows = buildRows(document);
    updateScrollMetrics(rows);
    const auto bounds = contentBounds();
    const auto layouts = buildRowLayouts(bounds.y, rows);
    for (const auto& layout : layouts) {
        if (layout.row.key == active->key) {
            const float rowTop = rowYWithScroll(layout.top);
            if (rowTop + kRowHeight < bounds.y || rowTop > bounds.y + bounds.height) {
                return std::nullopt;
            }

            const float valueLeft = x_ + kLabelColumnWidth;
            float editorWidth = valueCellWidth() - 4.0f;
            if (active->editKind == PropertyEditKind::Color) {
                editorWidth -= (kRowHeight - 4.0f);
            }
            return ValueCellBounds{ valueLeft + 2.0f, rowTop + 2.0f, std::max(24.0f, editorWidth), kRowHeight - 6.0f };
        }
    }

    return std::nullopt;
}

std::optional<PropertyInspector::PendingEdit> PropertyInspector::buildPendingEdit(const std::string& valueText) const
{
    if (!isEditing()) {
        return std::nullopt;
    }

    if (activeEditKind_ == PropertyEditKind::Choice) {
        return std::nullopt;
    }

    return PendingEdit{ activeKey_, valueText, activeEditKind_ };
}

bool PropertyInspector::isEditing() const
{
    return !activeKey_.empty();
}

void PropertyInspector::draw(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::ProjectDocument& document, std::size_t selectionCount)
{
    const model::WidgetNode* selectedWidget = document.selectedWidget();
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

    const auto rows = buildRows(document);
    updateScrollMetrics(rows);
    const ValueCellBounds bounds = contentBounds();
    const auto layouts = buildRowLayouts(bounds.y, rows);
    rebuildSuggestions(document);

    canvas.saveState();
    canvas.setClampBounds(bounds.x, bounds.y, bounds.width, bounds.height);
    for (std::size_t index = 0; index < layouts.size(); ++index) {
        const auto& row = layouts[index].row;
        const float rowTop = rowYWithScroll(layouts[index].top);
        if (rowTop + kRowHeight < bounds.y || rowTop > bounds.y + bounds.height) {
            continue;
        }

        canvas.setColor(index % 2 == 0 ? 0xff2b313d : 0xff262c37);
        canvas.fill(bounds.x, rowTop, bounds.width, kRowHeight - 2.0f);

        const float labelLeft = x_ + 18.0f;
        const float valueLeft = x_ + kLabelColumnWidth;
        const float valueWidth = valueCellWidth();
        const bool isReadOnly = row.editKind == PropertyEditKind::ReadOnly;
        const bool isActive = isEditing() && row.key == activeKey_;
        const auto swatchBounds = colorSwatchBoundsForRow(row, rowTop);
        const float valueTextWidth = swatchBounds.has_value()
            ? std::max(0.0f, valueWidth - swatchBounds->width - 16.0f)
            : valueWidth;

        if (row.isSection) {
            canvas.setColor(0xff253246);
            canvas.fill(bounds.x, rowTop, bounds.width, kRowHeight - 2.0f);
            if (drawText) {
                canvas.setColor(0xffa9c7f6);
                canvas.text(row.label, font, visage::Font::kTopLeft,
                    labelLeft, rowTop + 5.0f, bounds.width - 24.0f, kRowHeight - 8.0f);
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
            if (!isActive || row.editKind == PropertyEditKind::Choice) {
                canvas.text(row.displayValue, font, visage::Font::kTopLeft,
                    valueLeft + 8.0f, rowTop + 5.0f, valueTextWidth - 12.0f, kRowHeight - 8.0f);
            }

            if (isActive && row.editKind != PropertyEditKind::Choice) {
                canvas.setColor(0xff92b9ff);
                canvas.text("Enter=Apply  Esc=Cancel", font, visage::Font::kTopLeft,
                    valueLeft + 8.0f, rowTop + kRowHeight - 2.0f, valueTextWidth - 12.0f, 16.0f);
            }

            if (swatchBounds.has_value()) {
                const int swatchColor = row.displayValue.empty()
                    ? 0xff2b313d
                    : parseColorOrDefault(row.displayValue, 0xff2b313d);
                canvas.setColor(swatchColor);
                canvas.fill(swatchBounds->x, swatchBounds->y, swatchBounds->width, swatchBounds->height);
                canvas.setColor(0xff11141a);
                canvas.fill(swatchBounds->x, swatchBounds->y, swatchBounds->width, 1.0f);
                canvas.fill(swatchBounds->x, swatchBounds->y + swatchBounds->height - 1.0f, swatchBounds->width, 1.0f);
                canvas.fill(swatchBounds->x, swatchBounds->y, 1.0f, swatchBounds->height);
                canvas.fill(swatchBounds->x + swatchBounds->width - 1.0f, swatchBounds->y, 1.0f, swatchBounds->height);
            }
        }
    }
    if (drawText && !suggestions_.empty()) {
        for (const auto& suggestion : suggestions_) {
            canvas.setColor(0xff314055);
            canvas.fill(suggestion.x, suggestion.y, suggestion.width, suggestion.height);
            canvas.setColor(0xff92b9ff);
            canvas.fill(suggestion.x, suggestion.y + suggestion.height - 1.0f, suggestion.width, 1.0f);
            canvas.setColor(0xffeef2f8);
            canvas.text(suggestion.value, font, visage::Font::kTopLeft,
                suggestion.x + 8.0f, suggestion.y + 4.0f, suggestion.width - 12.0f, suggestion.height - 6.0f);
        }
    }
    canvas.restoreState();

    const auto scrollBar = scrollBarBounds();
    if (scrollBar.has_value()) {
        const float arrowSize = std::min(scrollBar->width, 20.0f);
        const float trackTop = scrollBar->y + arrowSize;
        const float trackHeight = std::max(0.0f, scrollBar->height - arrowSize * 2.0f);
        const auto thumb = scrollBarThumbBounds();

        canvas.setColor(0xff39414f);
        canvas.fill(scrollBar->x, scrollBar->y, scrollBar->width, scrollBar->height);
        canvas.setColor(0xff11141a);
        canvas.fill(scrollBar->x, scrollBar->y, scrollBar->width, 1.0f);
        canvas.fill(scrollBar->x, scrollBar->y + scrollBar->height - 1.0f, scrollBar->width, 1.0f);
        canvas.fill(scrollBar->x, scrollBar->y, 1.0f, scrollBar->height);
        canvas.fill(scrollBar->x + scrollBar->width - 1.0f, scrollBar->y, 1.0f, scrollBar->height);

        canvas.setColor(0xff2b313d);
        canvas.fill(scrollBar->x, scrollBar->y, scrollBar->width, arrowSize);
        canvas.fill(scrollBar->x, scrollBar->y + scrollBar->height - arrowSize, scrollBar->width, arrowSize);
        canvas.setColor(0xff1d222b);
        canvas.fill(scrollBar->x + 2.0f, trackTop, scrollBar->width - 4.0f, trackHeight);

        canvas.setColor(0xff92b9ff);
        canvas.fill(scrollBar->x + scrollBar->width * 0.5f - 3.0f, scrollBar->y + 6.0f, 6.0f, 3.0f);
        canvas.fill(scrollBar->x + scrollBar->width * 0.5f - 3.0f, scrollBar->y + scrollBar->height - 9.0f, 6.0f, 3.0f);

        if (thumb.has_value()) {
            canvas.setColor(draggingScrollBarThumb_ ? 0xff92b9ff : 0xff4b79bc);
            canvas.fill(thumb->x, thumb->y, thumb->width, thumb->height);
            canvas.setColor(0xff1d2a3c);
            canvas.fill(thumb->x, thumb->y, thumb->width, 1.0f);
            canvas.fill(thumb->x, thumb->y + thumb->height - 1.0f, thumb->width, 1.0f);
            canvas.fill(thumb->x, thumb->y, 1.0f, thumb->height);
            canvas.fill(thumb->x + thumb->width - 1.0f, thumb->y, 1.0f, thumb->height);
        }
    }
}

} // namespace visiform::ui
