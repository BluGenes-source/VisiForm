#include "ui/PropertyInspector.h"

#include "model/BoxSizerLayout.h"
#include "model/LookAndFeelRegistry.h"
#include "model/WidgetItemUtils.h"
#include "model/WidgetRegistry.h"
#include "utils/CppIdentifier.h"
#include "utils/FileUtils.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kTabStripHeight = 34.0f;
constexpr float kRowHeight = 32.0f;
constexpr float kEventErrorHeight = 30.0f;
constexpr float kSuggestionRowHeight = 24.0f;
constexpr float kSuggestionSpacing = 2.0f;
constexpr float kPadding = 12.0f;
constexpr float kMinLabelColumnWidth = 128.0f;
constexpr float kPreferredLabelColumnWidth = 152.0f;
constexpr float kMaxLabelColumnWidth = 168.0f;
constexpr float kEventLabelColumnWidth = 132.0f;
constexpr float kMinimumValueCellWidth = 112.0f;
constexpr float kScrollBarWidth = 18.0f;
constexpr float kScrollBarGap = 6.0f;
constexpr float kMinimumThumbSize = 18.0f;
constexpr float kMouseWheelSensitivity = 40.0f;
constexpr float kSliderTrackHeight = 4.0f;
constexpr float kSliderThumbWidth = 10.0f;
constexpr float kSliderThumbHeight = 16.0f;
constexpr float kSliderValueWidth = 56.0f;
constexpr float kActionButtonWidth = 72.0f;
constexpr float kTabGap = 8.0f;
constexpr float kEventActionGap = 6.0f;
constexpr float kEventCreateWidth = 54.0f;
constexpr float kEventExistingWidth = 66.0f;
constexpr float kEventClearWidth = 48.0f;
constexpr float kEventValueInset = 6.0f;
constexpr float kEventMinimumValueCellWidth = kEventCreateWidth + kEventExistingWidth + kEventClearWidth + kEventActionGap * 2.0f + kEventValueInset;
constexpr float kEventSelectorTextPadding = 10.0f;
constexpr float kEventSelectorArrowWidth = 20.0f;
constexpr float kEventSelectorArrowGap = 6.0f;
constexpr std::string_view kEventSuggestionPrefix = "__event_suggestion:";

struct RowLayout {
    PropertyInspector::PropertyRow row;
    float top = 0.0f;
    float height = kRowHeight;

    [[nodiscard]] float bottom() const
    {
        return top + height;
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

std::optional<float> tryParseFloatText(const std::string& text)
{
    if (text.empty() || text == "<unset>") {
        return std::nullopt;
    }

    std::istringstream stream(text);
    float value = 0.0f;
    char trailing = '\0';
    if (!(stream >> value)) {
        return std::nullopt;
    }
    if (stream >> trailing) {
        return std::nullopt;
    }
    return value;
}

float clampAndStepSliderValue(float value, float minimumValue, float maximumValue, float stepValue)
{
    const float safeMaximum = std::max(minimumValue, maximumValue);
    const float safeStep = stepValue <= 0.0f ? 1.0f : stepValue;
    const float clamped = std::clamp(value, minimumValue, safeMaximum);
    const float stepped = minimumValue + std::round((clamped - minimumValue) / safeStep) * safeStep;
    return std::clamp(stepped, minimumValue, safeMaximum);
}

std::string formatSliderValue(float value, float stepValue)
{
    if (stepValue >= 1.0f) {
        return std::to_string(static_cast<int>(std::round(value)));
    }

    return formatFloat(value);
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

std::string fileNameFromPathText(const std::string& pathText)
{
    if (pathText.empty()) {
        return {};
    }

    return std::filesystem::path{ pathText }.filename().string();
}

std::string fallbackHintForPropertyKey(const std::string& key)
{
    if (key == "resourceId") {
        return "Selects a managed image resource from the project Resource Manager.";
    }
    if (key == "imagePath") {
        return "Optional direct image file path used when no managed resource is selected.";
    }
    if (key == "scaleMode") {
        return "Controls how the image is fitted inside the widget bounds.";
    }
    if (key == "lookAndFeelId") {
        return "Optional widget look and feel override. Empty means inherit from the project.";
    }
    if (key == "fillColor") {
        return "Overrides this widget's fill color.";
    }
    if (key == "textColor") {
        return "Overrides this widget's text color.";
    }
    if (key == "borderColor") {
        return "Overrides this widget's border color.";
    }
    if (key == "accentColor") {
        return "Overrides this widget's accent color.";
    }
    if (key == "borderThickness") {
        return "Overrides this widget's border thickness.";
    }
    if (key == "cornerRadius") {
        return "Overrides this widget's corner radius.";
    }
    if (key == "fontSize") {
        return "Overrides the widget text size.";
    }
    if (key == "fontFamily") {
        return "Overrides the font family used for preview text when supported.";
    }
    if (key == "fontBold") {
        return "Uses bold text when the preview font supports it.";
    }
    if (key == "fontItalic") {
        return "Uses italic text when the preview font supports it.";
    }

    return {};
}

std::string resolvedPropertyHint(const std::string& definitionHint, const std::string& key)
{
    return definitionHint.empty() ? fallbackHintForPropertyKey(key) : definitionHint;
}

PropertyInspector::PropertyChoice makeChoice(std::string value, std::string label = {}, std::string hint = {})
{
    if (label.empty()) {
        label = value;
    }

    return PropertyInspector::PropertyChoice{ std::move(value), std::move(label), std::move(hint) };
}

void collectMatchingHandlers(const model::WidgetNode& widget,
    const std::string& signatureKind,
    std::set<std::string>& handlerNames);
std::optional<std::string> incompatibleHandlerSignatureKind(const model::WidgetNode& widget,
    const std::string& handlerName,
    const std::string& signatureKind);

std::string choiceLabelForValue(const std::vector<PropertyInspector::PropertyChoice>& choices, const std::string& value)
{
    const auto iterator = std::find_if(choices.begin(), choices.end(), [&value](const PropertyInspector::PropertyChoice& choice) {
        return choice.value == value;
    });
    if (iterator != choices.end()) {
        return iterator->label;
    }

    return value;
}

std::string imageResourceChoiceLabel(const model::ProjectResource& resource)
{
    std::string name = resource.displayName;
    if (name.empty()) {
        name = fileNameFromPathText(resource.sourcePath);
    }
    if (name.empty()) {
        name = resource.id;
    }

    return name + " (" + resource.id + ")";
}

std::string widgetHierarchyLabel(const model::WidgetNode& widget)
{
    const std::string displayName = widget.name.empty() ? widget.id : widget.name;
    return displayName + " [" + widget.typeName() + "] (" + widget.id + ")";
}

std::vector<PropertyInspector::PropertyChoice> lookAndFeelChoices()
{
    std::vector<PropertyInspector::PropertyChoice> choices;
    choices.push_back(makeChoice({}, "<inherit>", "Uses the project look and feel setting."));
    const auto& registry = model::LookAndFeelRegistry::instance();
    for (const auto& definition : registry.definitions()) {
        choices.push_back(makeChoice(
            definition.id,
            definition.displayName + (registry.isBuiltIn(definition.id) ? " (Built-in)" : " (Custom)"),
            "Applies this registered look and feel preset."));
    }
    return choices;
}

std::vector<PropertyInspector::PropertyChoice> callbackChoices(const model::ProjectDocument& document,
    const model::WidgetEventDefinition& eventDefinition)
{
    std::set<std::string> handlerNames;
    collectMatchingHandlers(document.root, eventDefinition.handlerSignatureKind, handlerNames);

    std::vector<PropertyInspector::PropertyChoice> choices;
    choices.reserve(handlerNames.size());
    for (const auto& handlerName : handlerNames) {
        if (!utils::isValidCppIdentifier(handlerName)) {
            continue;
        }
        choices.push_back(makeChoice(handlerName, handlerName, "Compatible callback with signature kind " + eventDefinition.handlerSignatureKind + "."));
    }
    return choices;
}

std::string joinedChoiceValues(const std::vector<PropertyInspector::PropertyChoice>& choices)
{
    std::ostringstream stream;
    for (std::size_t index = 0; index < choices.size(); ++index) {
        if (index > 0) {
            stream << ", ";
        }
        stream << choices[index].value;
    }

    return stream.str();
}

bool isEventSupportRow(const PropertyInspector::PropertyRow& row)
{
    return row.key == "__section_events"
        || row.key == "__section_compatible_suggestions"
        || row.key.starts_with(kEventSuggestionPrefix);
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

bool isLegacyWidgetLookAndFeelPropertyKey(const std::string& key)
{
    return key == "lookAndFeelId"
        || key == "fillColor"
        || key == "textColor"
        || key == "borderColor"
        || key == "accentColor"
        || key == "borderThickness"
        || key == "cornerRadius";
}

std::string appearanceLabel(std::string_view key)
{
    if (key == "controlSurfaceColor") return "Control Surface";
    if (key == "textColor") return "Text";
    if (key == "borderColor") return "Border";
    if (key == "accentColor") return "Accent";
    if (key == "focusOutlineColor") return "Focus Outline";
    if (key == "highlightEdgeColor") return "Highlight Edge";
    if (key == "shadowEdgeColor") return "Shadow Edge";
    if (key == "borderThickness") return "Border Thickness";
    if (key == "cornerRadius") return "Corner Radius";
    if (key == "controlPadding") return "Control Padding";
    return std::string{ key };
}

std::optional<std::string> appearanceColorOverride(
    const model::WidgetLookAndFeelOverrides& overrides,
    std::string_view key)
{
    if (key == "controlSurfaceColor") return overrides.controlSurfaceColor;
    if (key == "textColor") return overrides.textColor;
    if (key == "borderColor") return overrides.borderColor;
    if (key == "accentColor") return overrides.accentColor;
    if (key == "focusOutlineColor") return overrides.focusOutlineColor;
    if (key == "highlightEdgeColor") return overrides.highlightEdgeColor;
    if (key == "shadowEdgeColor") return overrides.shadowEdgeColor;
    return std::nullopt;
}

std::optional<float> appearanceMetricOverride(
    const model::WidgetLookAndFeelOverrides& overrides,
    std::string_view key)
{
    if (key == "borderThickness") return overrides.borderThickness;
    if (key == "cornerRadius") return overrides.cornerRadius;
    if (key == "controlPadding") return overrides.controlPadding;
    return std::nullopt;
}

std::optional<std::string> stateAppearanceColorOverride(
    const model::WidgetStateLookAndFeelOverrides& overrides,
    std::string_view key)
{
    if (key == "controlSurfaceColor") return overrides.controlSurfaceColor;
    if (key == "textColor") return overrides.textColor;
    if (key == "borderColor") return overrides.borderColor;
    if (key == "accentColor") return overrides.accentColor;
    if (key == "focusOutlineColor") return overrides.focusOutlineColor;
    if (key == "highlightEdgeColor") return overrides.highlightEdgeColor;
    if (key == "shadowEdgeColor") return overrides.shadowEdgeColor;
    return std::nullopt;
}

std::string appearanceStateLabel(model::WidgetAppearanceState state)
{
    switch (state) {
    case model::WidgetAppearanceState::Normal: return "Normal";
    case model::WidgetAppearanceState::Hover: return "Hover";
    case model::WidgetAppearanceState::Pressed: return "Pressed";
    case model::WidgetAppearanceState::Focused: return "Focused";
    case model::WidgetAppearanceState::CheckedOrSelected: return "Checked / Selected";
    case model::WidgetAppearanceState::Disabled: return "Disabled";
    }
    return "Normal";
}

struct AppearanceEditorMetadata {
    PropertyInspector::PropertyEditKind editKind = PropertyInspector::PropertyEditKind::Float;
    float minimumValue = 0.0f;
    float maximumValue = 0.0f;
    float stepValue = 1.0f;
};

AppearanceEditorMetadata appearanceEditorMetadata(model::WidgetType widgetType, std::string_view key)
{
    if (const auto* definition = model::WidgetRegistry::instance().find(widgetType)) {
        const auto property = std::find_if(
            definition->properties.begin(),
            definition->properties.end(),
            [key](const model::WidgetPropertyDefinition& candidate) {
                return candidate.key == key;
            });
        if (property != definition->properties.end()
            && property->editKind == model::PropertyEditKind::Slider) {
            return {
                PropertyInspector::PropertyEditKind::Slider,
                property->minimumValue,
                property->maximumValue,
                property->stepValue
            };
        }
    }
    if (key == "borderThickness" || key == "cornerRadius") {
        return { PropertyInspector::PropertyEditKind::Slider, 0.0f, 25.0f, 1.0f };
    }
    return {};
}

std::string inheritedAppearanceValue(const model::ResolvedLookAndFeelStyle& style, std::string_view key)
{
    if (key == "controlSurfaceColor") return style.controlSurfaceColor;
    if (key == "textColor") return style.primaryTextColor;
    if (key == "borderColor") return style.borderColor;
    if (key == "accentColor") return style.accentColor;
    if (key == "focusOutlineColor") return style.focusOutlineColor;
    if (key == "highlightEdgeColor") return style.highlightEdgeColor;
    if (key == "shadowEdgeColor") return style.shadowEdgeColor;
    if (key == "borderThickness") return formatFloat(style.borderThickness);
    if (key == "cornerRadius") return formatFloat(style.cornerRadius);
    if (key == "controlPadding") return formatFloat(style.controlPadding);
    return {};
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

    if (signatureKind == "void_event" && model::supportsItemActions(widget.type)) {
        for (const auto& actionName : model::getWidgetItemActions(widget)) {
            if (!actionName.empty()) {
                handlerNames.insert(actionName);
            }
        }
    }

    for (const auto& child : widget.children) {
        collectMatchingHandlers(child, signatureKind, handlerNames);
    }
}

std::optional<std::string> incompatibleHandlerSignatureKind(const model::WidgetNode& widget,
    const std::string& handlerName,
    const std::string& signatureKind)
{
    if (handlerName.empty()) {
        return std::nullopt;
    }

    if (const auto* definition = model::WidgetRegistry::instance().find(widget.type)) {
        for (const auto& event : definition->events) {
            if (widget.getStringProperty(event.key, {}) == handlerName
                && event.handlerSignatureKind != signatureKind) {
                return event.handlerSignatureKind;
            }
        }
    }

    if (model::supportsItemActions(widget.type)) {
        for (const auto& actionName : model::getWidgetItemActions(widget)) {
            if (actionName == handlerName && signatureKind != "void_event") {
                return std::string{ "void_event" };
            }
        }
    }

    for (const auto& child : widget.children) {
        if (auto incompatibleSignature = incompatibleHandlerSignatureKind(child, handlerName, signatureKind)) {
            return incompatibleSignature;
        }
    }

    return std::nullopt;
}

std::string eventRowErrorText(const model::ProjectDocument& document,
    const std::string& handlerName,
    const model::WidgetEventDefinition& eventDefinition)
{
    if (handlerName.empty()) {
        return {};
    }

    if (!utils::isValidCppIdentifier(handlerName)) {
        return handlerName + " is not a valid C++ identifier. Use letters, digits, or underscores, and start with a letter or underscore.";
    }

    if (auto incompatibleSignature = incompatibleHandlerSignatureKind(document.root, handlerName, eventDefinition.handlerSignatureKind)) {
        return handlerName + " already exists with " + *incompatibleSignature + ". This event needs " + eventDefinition.handlerSignatureKind + ", so reuse would conflict during validation/export.";
    }

    return {};
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
    case model::PropertyEditKind::Slider:
        return PropertyInspector::PropertyEditKind::Slider;
    case model::PropertyEditKind::Bool:
        return PropertyInspector::PropertyEditKind::Bool;
    case model::PropertyEditKind::ReadOnly:
        return PropertyInspector::PropertyEditKind::ReadOnly;
    }

    return PropertyInspector::PropertyEditKind::ReadOnly;
}

float rowHeightForRow(const PropertyInspector::PropertyRow& row)
{
    return kRowHeight + (row.errorText.empty() ? 0.0f : kEventErrorHeight);
}

std::vector<RowLayout> buildRowLayouts(float top, const std::vector<PropertyInspector::PropertyRow>& rows)
{
    std::vector<RowLayout> layouts;
    layouts.reserve(rows.size());
    float rowTop = top;
    for (const auto& row : rows) {
        const float height = rowHeightForRow(row);
        layouts.push_back({ row, rowTop, height });
        rowTop += height;
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
    visibleHeight_ = std::max(0.0f, height_ - kHeaderHeight - kTabStripHeight - 18.0f);
    contentHeight_ = 0.0f;
    for (const auto& row : rows) {
        contentHeight_ += rowHeightForRow(row);
    }
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

bool PropertyInspector::setScrollOffsetY(float newScrollOffsetY)
{
    const float previousOffsetY = scrollOffsetY_;
    scrollOffsetY_ = newScrollOffsetY;
    clampScrollOffset();
    const bool changed = std::abs(scrollOffsetY_ - previousOffsetY) > 0.5f;
    if (changed) {
        pendingScrollInteraction_ = true;
    }
    return changed;
}

float PropertyInspector::rowYWithScroll(float originalY) const
{
    return originalY - scrollOffsetY_;
}

PropertyInspector::ValueCellBounds PropertyInspector::contentBounds() const
{
    return {
        x_ + 8.0f,
        y_ + kHeaderHeight + kTabStripHeight + 10.0f,
        std::max(0.0f, width_ - 16.0f - (needsVerticalScrollBar_ ? (kScrollBarWidth + kScrollBarGap) : 0.0f)),
        visibleHeight_
    };
}

PropertyInspector::ValueCellBounds PropertyInspector::tabStripBounds() const
{
    return {
        x_ + 8.0f,
        y_ + kHeaderHeight + 4.0f,
        std::max(0.0f, width_ - 16.0f),
        kTabStripHeight - 6.0f
    };
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::scrollBarBounds() const
{
    if (!needsVerticalScrollBar_ || visibleHeight_ <= 0.0f) {
        return std::nullopt;
    }

    return ValueCellBounds{
        x_ + width_ - 8.0f - kScrollBarWidth,
        y_ + kHeaderHeight + kTabStripHeight + 10.0f,
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

std::optional<PropertyInspector::InspectorTab> PropertyInspector::hitTestTab(float x, float y) const
{
    const ValueCellBounds strip = tabStripBounds();
    if (!containsPoint(strip, x, y)) {
        return std::nullopt;
    }

    const float tabWidth = std::max(48.0f, (strip.width - kTabGap) * 0.5f);
    const ValueCellBounds propertiesTab{ strip.x, strip.y, tabWidth, strip.height };
    const ValueCellBounds eventsTab{ strip.x + tabWidth + kTabGap, strip.y, tabWidth, strip.height };
    if (containsPoint(propertiesTab, x, y)) {
        return InspectorTab::Properties;
    }
    if (containsPoint(eventsTab, x, y)) {
        return InspectorTab::Events;
    }

    return std::nullopt;
}

float PropertyInspector::valueCellWidth() const
{
    const ValueCellBounds bounds = contentBounds();
    const float labelWidth = labelColumnWidth();
    return std::max(0.0f, bounds.width - (labelWidth - (bounds.x - x_)) - 12.0f);
}

float PropertyInspector::labelColumnWidth() const
{
    const ValueCellBounds bounds = contentBounds();
    if (activeTab_ == InspectorTab::Events) {
        const float maxAllowedWidth = std::max(72.0f,
            bounds.width - kEventMinimumValueCellWidth - 12.0f - (bounds.x - x_));
        return std::min(kEventLabelColumnWidth, maxAllowedWidth);
    }

    const float maxAllowedWidth = std::max(kMinLabelColumnWidth,
        bounds.width - kMinimumValueCellWidth - 12.0f - (bounds.x - x_));
    return std::clamp(kPreferredLabelColumnWidth, kMinLabelColumnWidth, std::min(kMaxLabelColumnWidth, maxAllowedWidth));
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::colorSwatchBoundsForRow(const PropertyRow& row, float rowTop) const
{
    if (row.editKind != PropertyEditKind::Color || row.isSection) {
        return std::nullopt;
    }

    const float swatchSize = std::max(0.0f, kRowHeight - 12.0f);
    const float swatchX = x_ + labelColumnWidth() + valueCellWidth() - swatchSize - 8.0f;
    return ValueCellBounds{ swatchX, rowTop + 6.0f, swatchSize, swatchSize };
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::eventSelectorBoundsForRow(const PropertyRow& row, float rowTop) const
{
    if (!row.isEvent || row.isSection) {
        return std::nullopt;
    }

    const auto createBounds = eventActionBoundsForRow(row, rowTop, EventAction::Create);
    if (!createBounds.has_value()) {
        return std::nullopt;
    }

    const float valueLeft = x_ + labelColumnWidth();
    const float selectorLeft = valueLeft + kEventValueInset;
    const float selectorRight = createBounds->x - kEventActionGap;
    const float selectorWidth = std::max(0.0f, selectorRight - selectorLeft);
    if (selectorWidth < 36.0f) {
        return std::nullopt;
    }

    return ValueCellBounds{
        selectorLeft,
        rowTop + 4.0f,
        selectorWidth,
        kRowHeight - 10.0f
    };
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::eventSelectorTextBoundsForRow(const PropertyRow& row, float rowTop) const
{
    const auto selectorBounds = eventSelectorBoundsForRow(row, rowTop);
    if (!selectorBounds.has_value()) {
        return std::nullopt;
    }

    const bool showArrow = !row.choices.empty();
    const float textRightInset = showArrow
        ? kEventSelectorTextPadding + kEventSelectorArrowGap + kEventSelectorArrowWidth
        : kEventSelectorTextPadding;
    const float textLeft = selectorBounds->x + kEventSelectorTextPadding;
    const float textWidth = std::max(0.0f, selectorBounds->width - kEventSelectorTextPadding - textRightInset);
    return ValueCellBounds{
        textLeft,
        selectorBounds->y,
        textWidth,
        selectorBounds->height
    };
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::eventSelectorArrowBoundsForRow(const PropertyRow& row, float rowTop) const
{
    if (row.choices.empty()) {
        return std::nullopt;
    }

    const auto selectorBounds = eventSelectorBoundsForRow(row, rowTop);
    if (!selectorBounds.has_value()) {
        return std::nullopt;
    }

    return ValueCellBounds{
        selectorBounds->x + selectorBounds->width - kEventSelectorArrowWidth - kEventSelectorTextPadding,
        selectorBounds->y,
        kEventSelectorArrowWidth,
        selectorBounds->height
    };
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::eventActionBoundsForRow(const PropertyRow& row, float rowTop, EventAction action) const
{
    if (!row.isEvent || row.isSection) {
        return std::nullopt;
    }

    const float valueLeft = x_ + labelColumnWidth();
    const float valueWidth = valueCellWidth();
    const float clearLeft = valueLeft + valueWidth - kEventClearWidth - kEventValueInset;
    const float existingLeft = clearLeft - kEventActionGap - kEventExistingWidth;
    const float createLeft = existingLeft - kEventActionGap - kEventCreateWidth;
    const float top = rowTop + 5.0f;
    switch (action) {
    case EventAction::Create:
        return ValueCellBounds{ createLeft, top, kEventCreateWidth, kRowHeight - 12.0f };
    case EventAction::Existing:
        return ValueCellBounds{ existingLeft, top, kEventExistingWidth, kRowHeight - 12.0f };
    case EventAction::Clear:
        return ValueCellBounds{ clearLeft, top, kEventClearWidth, kRowHeight - 12.0f };
    }

    return std::nullopt;
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::sliderTrackBoundsForRow(const PropertyRow& row, float rowTop) const
{
    if (row.editKind != PropertyEditKind::Slider || row.isSection) {
        return std::nullopt;
    }

    const float valueLeft = x_ + labelColumnWidth();
    const float trackLeft = valueLeft + 10.0f;
    const float trackWidth = std::max(36.0f, valueCellWidth() - kSliderValueWidth - 20.0f);
    return ValueCellBounds{
        trackLeft,
        rowTop + (kRowHeight - kSliderTrackHeight) * 0.5f,
        trackWidth,
        kSliderTrackHeight
    };
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::sliderThumbBoundsForRow(const PropertyRow& row, float rowTop, float value) const
{
    const auto track = sliderTrackBoundsForRow(row, rowTop);
    if (!track.has_value()) {
        return std::nullopt;
    }

    const float safeMaximum = std::max(row.minimumValue, row.maximumValue);
    const float range = std::max(0.001f, safeMaximum - row.minimumValue);
    const float normalized = std::clamp((value - row.minimumValue) / range, 0.0f, 1.0f);
    const float thumbCenterX = track->x + normalized * track->width;
    return ValueCellBounds{
        thumbCenterX - kSliderThumbWidth * 0.5f,
        rowTop + (kRowHeight - kSliderThumbHeight) * 0.5f,
        kSliderThumbWidth,
        kSliderThumbHeight
    };
}

std::vector<PropertyInspector::PropertyRow> PropertyInspector::buildRows(const model::ProjectDocument& document, const utils::AppSettings& settings) const
{
    std::vector<PropertyRow> rows;
    const model::WidgetNode* selectedWidget = document.selectedWidget();
    if (selectedWidget == nullptr) {
        return rows;
    }

    rows.push_back({ "id", "Id", "Stable widget id used by the editor and generated code.", selectedWidget->id, PropertyEditKind::ReadOnly });
    rows.push_back({ "type", "Type", "Registered widget type used for preview and export.", selectedWidget->typeName(), PropertyEditKind::ReadOnly });
    rows.push_back({ "name", "Name", "Readable widget name used by the editor and generated helpers.", selectedWidget->name, PropertyEditKind::Text });
    const model::WidgetNode* selectedParent = document.findParentOf(selectedWidget->id);
    const bool parentControlsSizerLayout = selectedParent != nullptr && selectedParent->type == model::WidgetType::Sizer;
    if (selectedWidget->type == model::WidgetType::FormWindow) {
        rows.push_back({ "projectName", "Project Name", "Project display name used by the editor and export.", document.projectName, PropertyEditKind::Text });
        rows.push_back({ "executableName", "Executable Name", "Executable target name used for generated builds.", document.executableName, PropertyEditKind::Text });
        rows.push_back({ "generatedBaseClassName", "Generated Base Class", "Generated base class name is fixed to MainWindow.", "MainWindow", PropertyEditKind::ReadOnly });
        rows.push_back({ "userSubclassName", "User Subclass Name", "Editable generated user subclass name.", document.userSubclassName, PropertyEditKind::Text });
        rows.push_back({ "windowTitle", "Window Title", "Generated runtime window title.", document.windowTitle, PropertyEditKind::Text });
        rows.push_back({ "lookAndFeelId", "Look and Feel", resolvedPropertyHint({}, "lookAndFeelId"), document.lookAndFeelId, PropertyEditKind::Choice, false, lookAndFeelChoices() });
        rows.push_back({ "__section_export_dependencies", "Export / Dependencies", {}, {}, PropertyEditKind::ReadOnly, true });
        rows.push_back({ "localVisageSourceDirectory", "Local Visage Source", "Optional local Visage source checkout used during export.", utils::FileUtils::normalizeSeparators(settings.localVisageSourceDirectory.string()), PropertyEditKind::Text });
        rows.push_back({ "visageGitRepository", "Visage Git Repository", "Fallback Git repository used when no local Visage source is configured.", settings.visageGitRepository, PropertyEditKind::Text });
        rows.push_back({ "visageGitTag", "Visage Git Tag", "Fallback Git tag or branch used when no local Visage source is configured.", settings.visageGitTag, PropertyEditKind::Text });
    }
    const PropertyEditKind boundsEditKind = parentControlsSizerLayout ? PropertyEditKind::ReadOnly : PropertyEditKind::Float;
    const std::string boundsHint = parentControlsSizerLayout
        ? "Position and size are controlled by the parent Sizer; mouse resize edits the sizer item's preferred size."
        : "Widget geometry relative to the current parent container.";
    rows.push_back({ "x", "X", boundsHint, formatFloat(selectedWidget->bounds.x), boundsEditKind });
    rows.push_back({ "y", "Y", boundsHint, formatFloat(selectedWidget->bounds.y), boundsEditKind });
    rows.push_back({ "width", "Width", boundsHint, formatFloat(selectedWidget->bounds.width), boundsEditKind });
    rows.push_back({ "height", "Height", boundsHint, formatFloat(selectedWidget->bounds.height), boundsEditKind });

    if (selectedWidget->type == model::WidgetType::Sizer) {
        const model::BoxSizerLayout layout = model::boxSizerLayoutFor(*selectedWidget);
        rows.push_back({ "__section_sizer", "Sizer", {}, {}, PropertyEditKind::ReadOnly, true });
        rows.push_back({ std::string(model::sizer_properties::kOrientation), "Orientation", "Sizer main-axis direction.", model::toString(layout.orientation), PropertyEditKind::Choice, false,
            { makeChoice("Vertical"), makeChoice("Horizontal") } });
        rows.push_back({ std::string(model::sizer_properties::kPaddingLeft), "Padding Left", "Left inset around child widgets.", std::to_string(layout.paddingLeft), PropertyEditKind::Integer });
        rows.push_back({ std::string(model::sizer_properties::kPaddingTop), "Padding Top", "Top inset around child widgets.", std::to_string(layout.paddingTop), PropertyEditKind::Integer });
        rows.push_back({ std::string(model::sizer_properties::kPaddingRight), "Padding Right", "Right inset around child widgets.", std::to_string(layout.paddingRight), PropertyEditKind::Integer });
        rows.push_back({ std::string(model::sizer_properties::kPaddingBottom), "Padding Bottom", "Bottom inset around child widgets.", std::to_string(layout.paddingBottom), PropertyEditKind::Integer });
        rows.push_back({ std::string(model::sizer_properties::kGap), "Gap", "Spacing between participating child widgets.", std::to_string(layout.gap), PropertyEditKind::Integer });
    }

    if (parentControlsSizerLayout) {
        const model::SizerItemLayout layout = model::sizerItemLayoutFor(*selectedWidget);
        rows.push_back({ "__section_sizer_item", "Sizer Item", {}, {}, PropertyEditKind::ReadOnly, true });
        rows.push_back({ std::string(model::sizer_properties::kItemProportion), "Proportion", "Relative main-axis growth weight inside the parent Sizer.", std::to_string(layout.proportion), PropertyEditKind::Integer });
        rows.push_back({ std::string(model::sizer_properties::kItemExpand), "Expand", "Fill the cross axis inside the parent Sizer slot.", layout.expand ? "true" : "false", PropertyEditKind::Bool });
        rows.push_back({ std::string(model::sizer_properties::kItemAlignment), "Alignment", "Cross-axis alignment when Expand is false.", model::toString(layout.alignment), PropertyEditKind::Choice, false,
            { makeChoice("Start"), makeChoice("Center"), makeChoice("End") } });
        rows.push_back({ std::string(model::sizer_properties::kItemBorder), "Border", "Margin amount applied to enabled border sides.", std::to_string(layout.border), PropertyEditKind::Integer });
        rows.push_back({ std::string(model::sizer_properties::kItemBorderSides), "Border Sides", "Enabled border sides: None, All, or Left|Top|Right|Bottom.", model::toString(layout.borderSides), PropertyEditKind::Choice, false,
            { makeChoice("None"), makeChoice("Left"), makeChoice("Top"), makeChoice("Right"), makeChoice("Bottom"), makeChoice("All") } });
        rows.push_back({ std::string(model::sizer_properties::kItemPreferredWidth), "Preferred Width", "Preferred width request used by designer resizing, or -1 for automatic.", std::to_string(layout.preferredWidth), PropertyEditKind::Integer });
        rows.push_back({ std::string(model::sizer_properties::kItemPreferredHeight), "Preferred Height", "Preferred height request used by designer resizing, or -1 for automatic.", std::to_string(layout.preferredHeight), PropertyEditKind::Integer });
        rows.push_back({ std::string(model::sizer_properties::kItemMinimumWidth), "Minimum Width", "Minimum width override, or -1 for automatic.", std::to_string(layout.minimumWidth), PropertyEditKind::Integer });
        rows.push_back({ std::string(model::sizer_properties::kItemMinimumHeight), "Minimum Height", "Minimum height override, or -1 for automatic.", std::to_string(layout.minimumHeight), PropertyEditKind::Integer });
        rows.push_back({ std::string(model::sizer_properties::kItemShown), "Shown", "Whether this child participates in the parent Sizer layout.", layout.shown ? "true" : "false", PropertyEditKind::Bool });
    }

    if (selectedWidget->type == model::WidgetType::GroupBox) {
        rows.push_back({ "__section_children", "Children", {}, {}, PropertyEditKind::ReadOnly, true });
        rows.push_back({ "__groupbox_child_count", "Child Count", "Number of widgets currently parented to this GroupBox.", std::to_string(selectedWidget->children.size()), PropertyEditKind::ReadOnly });

        for (const auto& child : selectedWidget->children) {
            rows.push_back({
                "__groupbox_child_info_" + child.id,
                child.name.empty() ? child.id : child.name,
                "Current GroupBox child widget.",
                widgetHierarchyLabel(child),
                PropertyEditKind::ReadOnly
            });
        }

        std::vector<PropertyChoice> childChoices;
        childChoices.reserve(selectedWidget->children.size());
        for (const auto& child : selectedWidget->children) {
            childChoices.push_back(makeChoice(child.id, widgetHierarchyLabel(child), "Select this child widget."));
        }

        if (childChoices.empty()) {
            rows.push_back({ "__groupbox_select_child", "Select Child", "Selects a child widget from this GroupBox.", "No children", PropertyEditKind::ReadOnly });
            rows.push_back({ "__groupbox_remove_child", "Remove Child", "Moves a child widget from this GroupBox back to the root form.", "No children", PropertyEditKind::ReadOnly });
        }
        else {
            rows.push_back({ "__groupbox_select_child", "Select Child", "Selects a child widget from this GroupBox.", "Choose child", PropertyEditKind::Choice, false, childChoices });
            rows.push_back({ "__groupbox_remove_child", "Remove Child", "Moves a child widget from this GroupBox back to the root form.", "Choose child", PropertyEditKind::Choice, false, childChoices });
        }

        std::vector<PropertyChoice> addChoices;
        addChoices.reserve(document.root.children.size());
        for (const auto& rootChild : document.root.children) {
            if (rootChild.id == selectedWidget->id) {
                continue;
            }

            addChoices.push_back(makeChoice(rootChild.id, widgetHierarchyLabel(rootChild), "Add this root-level widget to the selected GroupBox."));
        }

        if (addChoices.empty()) {
            rows.push_back({ "__groupbox_add_existing_child", "Add Existing Child", "Adds a root-level widget to this GroupBox.", "No root widgets", PropertyEditKind::ReadOnly });
        }
        else {
            rows.push_back({ "__groupbox_add_existing_child", "Add Existing Child", "Adds a root-level widget to this GroupBox.", "Choose root widget", PropertyEditKind::Choice, false, addChoices });
        }
    }

    if (selectedWidget->type == model::WidgetType::TabControl) {
        rows.push_back({ "__section_tabs", "Tabs", {}, {}, PropertyEditKind::ReadOnly, true });
        rows.push_back({ "__tabcontrol_tab_count", "Tab Count", "Number of tab pages owned by this TabControl.", std::to_string(selectedWidget->tabPageCount()), PropertyEditKind::ReadOnly });

        std::vector<PropertyChoice> tabChoices;
        int pageIndex = 0;
        for (const auto& child : selectedWidget->children) {
            if (child.type != model::WidgetType::TabPage) {
                continue;
            }

            const std::string title = child.tabTitle();
            tabChoices.push_back(makeChoice(child.id, title, "Activate this tab page in the designer preview."));
            rows.push_back({
                "__tabcontrol_tab_info_" + child.id,
                child.name.empty() ? child.id : child.name,
                "Current tab page owned by this TabControl.",
                title + " [TabPage] (#" + std::to_string(pageIndex + 1) + ")",
                PropertyEditKind::ReadOnly
            });
            ++pageIndex;
        }

        const model::WidgetNode* selectedPage = selectedWidget->tabPageAt(selectedWidget->selectedTabIndex());
        const std::string selectedTabTitle = selectedPage != nullptr ? selectedPage->tabTitle() : std::string{};
        if (tabChoices.empty()) {
            rows.push_back({ "__tabcontrol_tabs", "Selected Tab", "Selects the active tab page.", "No tab pages", PropertyEditKind::ReadOnly });
            rows.push_back({ "__tabcontrol_selected_tab_title", "Selected Tab Title", "Renames the active tab page title.", "No tab pages", PropertyEditKind::ReadOnly });
        }
        else {
            rows.push_back({ "__tabcontrol_tabs", "Selected Tab", "Selects the active tab page.", selectedTabTitle, PropertyEditKind::Choice, false, tabChoices });
            rows.push_back({ "__tabcontrol_selected_tab_title", "Selected Tab Title", "Renames the active tab page title.", selectedTabTitle, PropertyEditKind::Text });
        }

        rows.push_back({
            "__tabcontrol_add_tab",
            "Add Tab",
            "Creates a new empty tab page.",
            "Add Tab",
            PropertyEditKind::Choice,
            false,
            { makeChoice("add", "Add Tab", "Create a new tab page on this TabControl.") }
        });

        rows.push_back({
            "__tabcontrol_remove_selected_tab",
            "Remove Selected Tab",
            "Removes the active tab page when the current rules allow removal.",
            selectedTabTitle.empty() ? std::string{ "Remove Selected Tab" } : selectedTabTitle,
            PropertyEditKind::Choice,
            false,
            { makeChoice("remove", "Remove Selected Tab", "Remove the currently active tab page.") }
        });
    }

    if (selectedWidget->type == model::WidgetType::TabPage) {
        rows.push_back({ "__section_tabpage", "Tab Page", {}, {}, PropertyEditKind::ReadOnly, true });

        const auto* parentTabControl = document.findParentOf(selectedWidget->id);
        int pageIndex = -1;
        if (parentTabControl != nullptr && parentTabControl->type == model::WidgetType::TabControl) {
            int currentIndex = 0;
            for (const auto& child : parentTabControl->children) {
                if (child.type != model::WidgetType::TabPage) {
                    continue;
                }
                if (child.id == selectedWidget->id) {
                    pageIndex = currentIndex;
                    break;
                }
                ++currentIndex;
            }
        }

        rows.push_back({ "__tabpage_parent", "Parent TabControl", "Owning tab control for this tab page.",
            parentTabControl != nullptr ? widgetHierarchyLabel(*parentTabControl) : std::string{ "<none>" }, PropertyEditKind::ReadOnly });
        rows.push_back({ "__tabpage_index", "Page Index", "Zero-based tab page index inside the parent TabControl.",
            pageIndex >= 0 ? std::to_string(pageIndex) : std::string{ "<invalid>" }, PropertyEditKind::ReadOnly });
        rows.push_back({ "__tabpage_child_count", "Child Count", "Number of widgets currently parented to this tab page.",
            std::to_string(selectedWidget->children.size()), PropertyEditKind::ReadOnly });
        rows.push_back({ "__tabpage_note", "Ownership", "TabPage is owned by its parent TabControl.", "TabPage is owned by TabControl.", PropertyEditKind::ReadOnly });
    }

    if (selectedWidget->type == model::WidgetType::TreeView) {
        const std::string nodesText = selectedWidget->getStringProperty("nodes", {});
        const auto parseResult = model::parseTreeNodes(nodesText);
        const auto expandedPaths = model::splitTreeNodePaths(selectedWidget->getStringProperty("expandedNodePaths", {}));
        rows.push_back({ "__section_treeview", "Tree View", {}, {}, PropertyEditKind::ReadOnly, true });
        rows.push_back({
            "nodes",
            "Nodes",
            "Click the value area or Edit... to edit indented tree-node text.",
            std::to_string(parseResult.nodes.size()) + (parseResult.nodes.size() == 1 ? " node" : " nodes"),
            PropertyEditKind::Text,
            false,
            {},
            0.0f,
            0.0f,
            1.0f,
            "Edit..."
        });
        rows.push_back({
            "expandedNodePaths",
            "Expanded Nodes",
            "Comma-separated list of expanded node paths.",
            selectedWidget->getStringProperty("expandedNodePaths", {}),
            PropertyEditKind::Text
        });
    }

    if (selectedWidget->type == model::WidgetType::TableGrid) {
        const auto columns = model::splitTableColumns(selectedWidget->getStringProperty("columns", {}));
        const auto rowsData = model::splitTableRows(selectedWidget->getStringProperty("rows", {}));
        const auto selection = model::clampSelectedCell(
            columns,
            rowsData,
            selectedWidget->getIntProperty("selectedRow", rowsData.empty() ? -1 : 0),
            selectedWidget->getIntProperty("selectedColumn", columns.empty() ? -1 : 0));
        rows.push_back({ "__section_tablegrid", "Table / Grid", {}, {}, PropertyEditKind::ReadOnly, true });
        rows.push_back({
            "columns",
            "Columns",
            "Click the value area or Edit... to open the visual table editor.",
            std::to_string(columns.size()) + (columns.size() == 1 ? " column" : " columns"),
            PropertyEditKind::Text,
            false,
            {},
            0.0f,
            0.0f,
            1.0f,
            "Edit..."
        });
        std::string rowsSummary = std::to_string(rowsData.size()) + (rowsData.size() == 1 ? " row" : " rows");
        if (selection.row >= 0 && selection.column >= 0) {
            rowsSummary += ", selected R" + std::to_string(selection.row + 1) + " C" + std::to_string(selection.column + 1);
        }
        rows.push_back({
            "rows",
            "Rows",
            "Click the value area or Edit... to open the visual table editor.",
            rowsSummary,
            PropertyEditKind::Text,
            false,
            {},
            0.0f,
            0.0f,
            1.0f,
            "Edit..."
        });
    }

    if (model::LookAndFeelRegistry::supportsWidgetOverrides(selectedWidget->type)) {
        rows.push_back({ "__section_appearance", "Appearance", {}, {}, PropertyEditKind::ReadOnly, true });
        if (document.hasMultiSelection()) {
            rows.push_back({
                "__appearance_multi_selection",
                "Widget Overrides",
                "Per-widget Appearance editing is available for one selected widget at a time.",
                "Select one widget to edit",
                PropertyEditKind::ReadOnly
            });
        }
        else {
            const auto supportedStates =
                model::LookAndFeelRegistry::supportedWidgetStates(selectedWidget->type);
            const auto selectedState = std::find(
                supportedStates.begin(), supportedStates.end(), appearanceState_) != supportedStates.end()
                ? appearanceState_
                : model::WidgetAppearanceState::Normal;
            if (!supportedStates.empty()) {
                std::vector<PropertyChoice> stateChoices;
                for (const auto state : supportedStates) {
                    stateChoices.push_back(makeChoice(
                        std::string{ model::toString(state) },
                        appearanceStateLabel(state),
                        state == model::WidgetAppearanceState::Normal
                            ? "Edit normal widget Appearance overrides."
                            : "Edit sparse overrides for this runtime state."));
                }
                rows.push_back({
                    "__appearance_state",
                    "Appearance State",
                    "Choose which compatible runtime state to edit.",
                    std::string{ model::toString(selectedState) },
                    PropertyEditKind::Choice,
                    false,
                    std::move(stateChoices)
                });
                rows.push_back({
                    "__appearance_preview_state",
                    "Preview State",
                    "Temporarily render the selected widget using this Appearance state in Design Mode.",
                    appearancePreviewEnabledFor(selectedWidget->id) ? "true" : "false",
                    PropertyEditKind::Bool
                });
            }

            const auto inheritedStyle = selectedState == model::WidgetAppearanceState::Normal
                ? model::LookAndFeelRegistry::instance().resolveProjectStyle(
                    document.lookAndFeelId, document.lookAndFeelOverrides)
                : model::LookAndFeelRegistry::instance().resolve(document, *selectedWidget);
            std::vector<PropertyChoice> resetChoices;
            const auto keys = selectedState == model::WidgetAppearanceState::Normal
                ? model::LookAndFeelRegistry::supportedWidgetOverrideKeys(selectedWidget->type)
                : model::LookAndFeelRegistry::supportedWidgetStateOverrideKeys(selectedWidget->type, selectedState);
            const auto stateOverrides = selectedWidget->stateAppearanceOverrides.find(selectedState);
            for (const auto key : keys) {
                const auto colorOverride = selectedState == model::WidgetAppearanceState::Normal
                    ? appearanceColorOverride(selectedWidget->appearanceOverrides, key)
                    : stateOverrides != selectedWidget->stateAppearanceOverrides.end()
                        ? stateAppearanceColorOverride(stateOverrides->second, key)
                        : std::nullopt;
                const auto metricOverride = selectedState == model::WidgetAppearanceState::Normal
                    ? appearanceMetricOverride(selectedWidget->appearanceOverrides, key)
                    : std::nullopt;
                const bool overridden = colorOverride.has_value() || metricOverride.has_value();
                const std::string value = colorOverride.has_value()
                    ? *colorOverride
                    : (metricOverride.has_value() ? formatFloat(*metricOverride) : inheritedAppearanceValue(inheritedStyle, key));
                const auto editorMetadata = appearanceEditorMetadata(selectedWidget->type, key);
                rows.push_back({
                    "__appearance_" + std::string{ key },
                    appearanceLabel(key) + (overridden ? " (Override)" : " (Inherited)"),
                    overridden
                        ? "Explicit " + appearanceStateLabel(selectedState) + " override. Use Reset Property to inherit again."
                        : "Inherited from the resolved normal widget Appearance. Editing creates an explicit override.",
                    value,
                    key.ends_with("Color") ? PropertyEditKind::Color : editorMetadata.editKind,
                    false,
                    {},
                    editorMetadata.minimumValue,
                    editorMetadata.maximumValue,
                    editorMetadata.stepValue
                });
                if (overridden) {
                    resetChoices.push_back(makeChoice(std::string{ key }, appearanceLabel(key),
                        "Remove this explicit override and inherit the normal widget value."));
                }
            }

            if (!resetChoices.empty()) {
                rows.push_back({
                    "__appearance_reset_property",
                    "Reset Property",
                    "Removes one explicit widget Appearance override.",
                    "Choose override",
                    PropertyEditKind::Choice,
                    false,
                    std::move(resetChoices)
                });
            }
            if (selectedState != model::WidgetAppearanceState::Normal
                && stateOverrides != selectedWidget->stateAppearanceOverrides.end()
                && !stateOverrides->second.empty()) {
                rows.push_back({
                    "__appearance_reset_state",
                    "Reset State",
                    "Removes every explicit override for the selected Appearance state.",
                    "Reset state",
                    PropertyEditKind::Choice,
                    false,
                    { makeChoice("reset", "Reset " + appearanceStateLabel(selectedState),
                        "Restore normal widget Appearance inheritance for this state.") }
                });
            }
            if (!selectedWidget->appearanceOverrides.empty()
                || !selectedWidget->stateAppearanceOverrides.empty()) {
                rows.push_back({
                    "__appearance_reset_all",
                    "Reset All Appearance",
                    "Removes normal and state Appearance overrides from this widget.",
                    "Reset all",
                    PropertyEditKind::Choice,
                    false,
                    { makeChoice("reset", "Reset All", "Restore inheritance for every widget Appearance property and state.") }
                });
            }
        }
    }

    std::set<std::string> drawnKeys;
    for (const auto& row : rows) {
        drawnKeys.insert(row.key);
    }
    if (selectedWidget->type == model::WidgetType::Sizer) {
        drawnKeys.insert(std::string{ model::sizer_properties::kLegacyPadding });
    }
    if (selectedWidget->type == model::WidgetType::FormWindow) {
        drawnKeys.insert("title");
    }
    else if (selectedWidget->type == model::WidgetType::Image) {
        drawnKeys.insert("source");
    }
    else if (selectedWidget->type == model::WidgetType::TabControl) {
        drawnKeys.insert("selectedTabIndex");
    }

    const auto addEventProperty = [&](const std::string& key, const std::string& label, const std::string& hint, const std::string& fallback = {}) {
        if (drawnKeys.contains(key)) {
            return;
        }

        const std::string displayValue = fallback.empty()
            ? displayTextOrFallback(selectedWidget, key, {})
            : displayTextOrFallback(selectedWidget, key, fallback);
        rows.push_back({ key, label, hint, displayValue, PropertyEditKind::Text });
        drawnKeys.insert(key);
    };

    if (const auto* definition = model::WidgetRegistry::instance().find(selectedWidget->type)) {
        bool styleSectionInserted = false;
        for (const auto& property : definition->properties) {
            if (isLegacyWidgetLookAndFeelPropertyKey(property.key)) {
                drawnKeys.insert(property.key);
                continue;
            }
            if (isStylePropertyKey(property.key) && !styleSectionInserted) {
                rows.push_back({ "__section_style", "Style", {}, {}, PropertyEditKind::ReadOnly, true });
                styleSectionInserted = true;
            }

            std::vector<PropertyChoice> choices;
            choices.reserve(property.choices.size() + 1);
            for (const auto& choice : property.choices) {
                choices.push_back(makeChoice(choice));
            }
            if (property.key == "lookAndFeelId") {
                choices = lookAndFeelChoices();
            }
            if (selectedWidget->type == model::WidgetType::Image && property.key == "resourceId") {
                choices.clear();
                choices.push_back(makeChoice({}, "<none>", "Clears the managed image resource binding."));
                for (const auto& resource : document.resources) {
                    if (resource.type == model::ProjectResourceType::Image) {
                        choices.push_back(makeChoice(resource.id,
                            imageResourceChoiceLabel(resource),
                            "Source: " + utils::FileUtils::normalizeSeparators(resource.sourcePath)));
                    }
                }
            }

            if (model::supportsItemList(selectedWidget->type) && property.key == "items") {
                const auto items = model::getWidgetItems(*selectedWidget);
                const std::string selectedIndexKey = std::string(model::selectedItemIndexPropertyKey(selectedWidget->type));
                const int selectedIndex = model::clampSelectedIndex(items,
                    selectedWidget->getIntProperty(selectedIndexKey, items.empty() ? -1 : 0));
                const std::string selectedItemText = model::getSelectedItemText(items, selectedIndex);
                std::string summary = std::to_string(items.size()) + (items.size() == 1 ? " item" : " items");
                if (selectedIndex >= 0) {
                    summary += ", selected #" + std::to_string(selectedIndex);
                    if (!selectedItemText.empty()) {
                        summary += " (" + selectedItemText + ")";
                    }
                }
                else {
                    summary += ", no selection";
                }

                rows.push_back({
                    property.key,
                    property.label,
                    model::supportsItemActions(selectedWidget->type)
                        ? "Click the value area or Edit... to open the item/action editor."
                        : "Click the value area or Edit... to open the item list editor.",
                    summary,
                    PropertyEditKind::Text,
                    false,
                    {},
                    0.0f,
                    0.0f,
                    1.0f,
                    "Edit..."
                });
                drawnKeys.insert(property.key);
                continue;
            }

            if (model::supportsItemActions(selectedWidget->type) && property.key == "itemActions") {
                const auto bindings = model::getWidgetItemActionBindings(*selectedWidget);
                const std::size_t boundCount = static_cast<std::size_t>(std::count_if(bindings.begin(), bindings.end(), [](const model::WidgetItemActionBinding& binding) {
                    return !binding.action.empty();
                }));
                const std::string selectedAction = model::getSelectedItemAction(*selectedWidget);
                std::string summary;
                if (bindings.empty()) {
                    summary = "No items";
                }
                else {
                    summary = std::to_string(boundCount) + " bound of " + std::to_string(bindings.size());
                }

                rows.push_back({
                    property.key,
                    property.label,
                    "Click the value area or Edit... to open the item/action editor.",
                    summary,
                    PropertyEditKind::Text,
                    false,
                    {},
                    0.0f,
                    0.0f,
                    1.0f,
                    "Edit..."
                });
                rows.push_back({
                    "__selected_item_action",
                    "Selected Action",
                    "Action bound to the currently selected menu or tool item.",
                    selectedAction.empty() ? std::string{ "<none>" } : selectedAction,
                    PropertyEditKind::ReadOnly
                });
                drawnKeys.insert(property.key);
                drawnKeys.insert("__selected_item_action");
                continue;
            }

            if (model::supportsTableGrid(selectedWidget->type) && (property.key == "columns" || property.key == "rows")) {
                drawnKeys.insert(property.key);
                continue;
            }

            if (model::supportsTreeNodes(selectedWidget->type) && property.key == "nodes") {
                drawnKeys.insert(property.key);
                continue;
            }

            if (model::supportsTreeNodes(selectedWidget->type) && property.key == "selectedNodePath") {
                choices.clear();
                choices.push_back(makeChoice({}, "<none>", "Clears the selected node path."));
                const auto parsedNodes = model::parseTreeNodes(selectedWidget->getStringProperty("nodes", {}));
                for (const auto& node : parsedNodes.nodes) {
                    const std::string label = std::string(static_cast<std::size_t>(std::max(0, node.depth)) * 2, ' ') + node.path;
                    choices.push_back(makeChoice(node.path, label, node.text));
                }
            }

            if (model::supportsTreeNodes(selectedWidget->type) && property.key == "expandedNodePaths") {
                drawnKeys.insert(property.key);
                continue;
            }

            const auto* propertyValue = selectedWidget->getProperty(property.key);
            std::string displayValue;
            if (property.editKind == model::PropertyEditKind::Slider) {
                float sliderValue = property.minimumValue;
                if (propertyValue != nullptr) {
                    if (propertyValue->isInt()) {
                        sliderValue = static_cast<float>(propertyValue->asInt(static_cast<int>(property.minimumValue)));
                    }
                    else if (propertyValue->isFloat()) {
                        sliderValue = propertyValue->asFloat(property.minimumValue);
                    }
                    else if (propertyValue->isString()) {
                        sliderValue = tryParseFloatText(propertyValue->asString()).value_or(property.minimumValue);
                    }
                }
                else if (property.defaultValue.isInt()) {
                    sliderValue = static_cast<float>(property.defaultValue.asInt(static_cast<int>(property.minimumValue)));
                }
                else if (property.defaultValue.isFloat()) {
                    sliderValue = property.defaultValue.asFloat(property.minimumValue);
                }

                sliderValue = clampAndStepSliderValue(sliderValue, property.minimumValue, property.maximumValue, property.stepValue);
                displayValue = formatSliderValue(sliderValue, property.stepValue);
            }
            else {
                displayValue = propertyValue != nullptr ? propertyValueText(*propertyValue) : property.defaultValue.toDisplayString();
            }
            if (property.key == "dock") {
                displayValue = model::toString(model::dockModeFromString(displayValue).value_or(model::DockMode::None));
            }
            if (property.key == "anchor") {
                displayValue = model::toString(model::anchorModeFromString(displayValue).value_or(model::AnchorMode::TopLeft));
            }
            if (selectedWidget->type == model::WidgetType::Image && property.key == "imagePath" && displayValue.empty()) {
                displayValue = selectedWidget->getStringProperty("source", {});
            }
            if (!choices.empty()) {
                displayValue = choiceLabelForValue(choices, displayValue);
            }
            PropertyEditKind rowEditKind = editKindForDefinition(property);
            std::string rowHint = resolvedPropertyHint(property.hint, property.key);
            if (parentControlsSizerLayout && property.key == "dock") {
                rowEditKind = PropertyEditKind::ReadOnly;
                if (!rowHint.empty()) {
                    rowHint += " ";
                }
                rowHint += "Position and size are controlled by the parent Sizer; mouse resize edits the sizer item's preferred size.";
            }
            if (property.key == "anchor" && selectedWidget->dockMode() != model::DockMode::None) {
                rowEditKind = PropertyEditKind::ReadOnly;
                if (!rowHint.empty()) {
                    rowHint += " ";
                }
                rowHint += "Ignored while Dock is not None.";
            }
            if (parentControlsSizerLayout && property.key == "anchor") {
                rowEditKind = PropertyEditKind::ReadOnly;
                if (!rowHint.empty()) {
                    rowHint += " ";
                }
                rowHint += "Position and size are controlled by the parent Sizer; mouse resize edits the sizer item's preferred size.";
            }
            rows.push_back({
                property.key,
                property.label,
                rowHint,
                displayValue,
                rowEditKind,
                false,
                std::move(choices),
                property.minimumValue,
                property.maximumValue,
                property.stepValue });
            drawnKeys.insert(property.key);
        }

        const std::size_t propertyCountBeforeEvents = rows.size();
        std::map<std::string, std::vector<PropertyChoice>> compatibleSuggestionsBySignature;
        for (const auto& event : definition->events) {
            const std::string displayValue = displayTextOrFallback(selectedWidget, event.key, {});
            const auto choices = callbackChoices(document, event);
            auto& signatureChoices = compatibleSuggestionsBySignature[event.handlerSignatureKind];
            for (const auto& choice : choices) {
                const auto existing = std::find_if(signatureChoices.begin(), signatureChoices.end(), [&choice](const PropertyChoice& existingChoice) {
                    return existingChoice.value == choice.value;
                });
                if (existing == signatureChoices.end()) {
                    signatureChoices.push_back(choice);
                }
            }
            rows.push_back({
                event.key,
                event.label,
                event.hint,
                displayValue,
                PropertyEditKind::ReadOnly,
                false,
                choices,
                0.0f,
                0.0f,
                1.0f,
                {},
                true,
                event.handlerSignatureKind,
                eventRowErrorText(document, displayValue, event)
            });
            drawnKeys.insert(event.key);
        }

        if (rows.size() > propertyCountBeforeEvents) {
            rows.insert(rows.begin() + static_cast<std::ptrdiff_t>(propertyCountBeforeEvents),
                PropertyRow{ "__section_events", selectedWidget->typeName() + " Events", {}, {}, PropertyEditKind::ReadOnly, true });
            rows.push_back(PropertyRow{ "__section_compatible_suggestions", "Compatible Suggestions", {}, {}, PropertyEditKind::ReadOnly, true });
            for (const auto& [signatureKind, choices] : compatibleSuggestionsBySignature) {
                rows.push_back({
                    std::string{ kEventSuggestionPrefix } + signatureKind,
                    signatureKind,
                    "Handlers already used with this signature kind.",
                    joinedChoiceValues(choices),
                    PropertyEditKind::ReadOnly
                });
            }
        }
    }

    for (const auto& [key, value] : selectedWidget->properties) {
        if (drawnKeys.contains(key)) {
            continue;
        }

        rows.push_back({ key, key, fallbackHintForPropertyKey(key), propertyValueText(value), editKindForProperty(key, value) });
    }

    return rows;
}

bool PropertyInspector::isEventRow(const model::ProjectDocument& document, const PropertyRow& row) const
{
    if (row.isSection || row.key.empty()) {
        return false;
    }

    if (row.isEvent) {
        return true;
    }

    const model::WidgetNode* selectedWidget = document.selectedWidget();
    if (selectedWidget == nullptr) {
        return false;
    }

    return findEventDefinition(selectedWidget->type, row.key) != nullptr;
}

std::vector<PropertyInspector::PropertyRow> PropertyInspector::rowsForActiveTab(const model::ProjectDocument& document, const utils::AppSettings& settings) const
{
    const auto allRows = buildRows(document, settings);
    std::vector<PropertyRow> filteredRows;
    filteredRows.reserve(allRows.size());

    for (const auto& row : allRows) {
        const bool eventRow = isEventRow(document, row);
        if (activeTab_ == InspectorTab::Events) {
            if (eventRow || isEventSupportRow(row)) {
                filteredRows.push_back(row);
            }
            continue;
        }

        if (isEventSupportRow(row) || eventRow) {
            continue;
        }

        filteredRows.push_back(row);
    }

    return filteredRows;
}

std::optional<PropertyInspector::PropertyRow> PropertyInspector::hitTestRow(const model::ProjectDocument& document, const utils::AppSettings& settings, float x, float y)
{
    const auto rows = rowsForActiveTab(document, settings);
    updateScrollMetrics(rows);
    if (!isWithinVisibleContent(x, y)) {
        return std::nullopt;
    }

    const auto layouts = buildRowLayouts(contentBounds().y, rows);
    for (const auto& layout : layouts) {
        const float rowTop = rowYWithScroll(layout.top);
        if (rowTop + layout.height < contentBounds().y || rowTop > contentBounds().y + contentBounds().height) {
            continue;
        }

        if (y >= rowTop && y <= rowTop + kRowHeight) {
            return layout.row;
        }
    }

    return std::nullopt;
}

std::optional<PropertyInspector::PendingEventAction> PropertyInspector::hitTestEventAction(const model::ProjectDocument& document, const utils::AppSettings& settings, float x, float y)
{
    const auto rows = rowsForActiveTab(document, settings);
    updateScrollMetrics(rows);
    if (!isWithinVisibleContent(x, y)) {
        return std::nullopt;
    }

    const auto layouts = buildRowLayouts(contentBounds().y, rows);
    const auto bounds = contentBounds();
    for (const auto& layout : layouts) {
        const float rowTop = rowYWithScroll(layout.top);
        if (rowTop + layout.height < bounds.y || rowTop > bounds.y + bounds.height) {
            continue;
        }

        if (!layout.row.isEvent) {
            continue;
        }

        const auto selectorBounds = eventSelectorBoundsForRow(layout.row, rowTop);
        if (selectorBounds.has_value() && containsPoint(*selectorBounds, x, y)) {
            return PendingEventAction{ layout.row.key, EventAction::Existing, *selectorBounds, layout.row.choices, layout.row.displayValue, layout.row.handlerSignatureKind };
        }

        const auto createBounds = eventActionBoundsForRow(layout.row, rowTop, EventAction::Create);
        if (createBounds.has_value() && containsPoint(*createBounds, x, y)) {
            return PendingEventAction{ layout.row.key, EventAction::Create, *createBounds, layout.row.choices, layout.row.displayValue, layout.row.handlerSignatureKind };
        }

        const auto existingBounds = eventActionBoundsForRow(layout.row, rowTop, EventAction::Existing);
        if (existingBounds.has_value() && containsPoint(*existingBounds, x, y)) {
            const auto anchorBounds = selectorBounds.value_or(*existingBounds);
            return PendingEventAction{ layout.row.key, EventAction::Existing, anchorBounds, layout.row.choices, layout.row.displayValue, layout.row.handlerSignatureKind };
        }

        const auto clearBounds = eventActionBoundsForRow(layout.row, rowTop, EventAction::Clear);
        if (clearBounds.has_value() && containsPoint(*clearBounds, x, y)) {
            return PendingEventAction{ layout.row.key, EventAction::Clear, *clearBounds, layout.row.choices, layout.row.displayValue, layout.row.handlerSignatureKind };
        }
    }

    return std::nullopt;
}

std::optional<std::string> PropertyInspector::hitTestColorSwatch(const model::ProjectDocument& document, const utils::AppSettings& settings, float x, float y)
{
    const auto rows = rowsForActiveTab(document, settings);
    updateScrollMetrics(rows);
    if (!isWithinVisibleContent(x, y)) {
        return std::nullopt;
    }

    const auto layouts = buildRowLayouts(contentBounds().y, rows);
    const auto bounds = contentBounds();
    for (const auto& layout : layouts) {
        const float rowTop = rowYWithScroll(layout.top);
        if (rowTop + layout.height < bounds.y || rowTop > bounds.y + bounds.height) {
            continue;
        }

        const auto swatch = colorSwatchBoundsForRow(layout.row, rowTop);
        if (swatch.has_value() && containsPoint(*swatch, x, y)) {
            return layout.row.key;
        }
    }

    return std::nullopt;
}

bool PropertyInspector::mouseDown(const model::ProjectDocument& document, const utils::AppSettings& settings, float x, float y)
{
    if (const auto tab = hitTestTab(x, y)) {
        if (activeTab_ != *tab) {
            activeTab_ = *tab;
            if (activeTab_ != InspectorTab::Properties) {
                clearAppearancePreview();
            }
            clearEditing();
            pendingInteractionEdit_.reset();
            draggingScrollBarThumb_ = false;
            draggingSlider_ = false;
            draggingSliderKey_.clear();
            clampScrollOffset();
        }
        return true;
    }

    const auto rows = rowsForActiveTab(document, settings);
    updateScrollMetrics(rows);
    const auto scrollBar = scrollBarBounds();
    if (scrollBar.has_value() && containsPoint(*scrollBar, x, y)) {
        const float arrowSize = std::min(scrollBar->width, 20.0f);
        const auto thumb = scrollBarThumbBounds();
        if (thumb.has_value() && containsPoint(*thumb, x, y)) {
            draggingScrollBarThumb_ = true;
            scrollBarDragOffsetY_ = y - thumb->y;
            pendingScrollInteraction_ = true;
            return true;
        }

        if (y < scrollBar->y + arrowSize) {
            setScrollOffsetY(scrollOffsetY_ - kRowHeight);
        }
        else if (y > scrollBar->y + scrollBar->height - arrowSize) {
            setScrollOffsetY(scrollOffsetY_ + kRowHeight);
        }
        else if (thumb.has_value() && y < thumb->y) {
            setScrollOffsetY(scrollOffsetY_ - std::max(kRowHeight, visibleHeight_ * 0.85f));
        }
        else {
            setScrollOffsetY(scrollOffsetY_ + std::max(kRowHeight, visibleHeight_ * 0.85f));
        }
        return true;
    }

    if (!isWithinVisibleContent(x, y)) {
        return false;
    }

    pendingInteractionEdit_.reset();
    const auto layouts = buildRowLayouts(contentBounds().y, rows);
    for (const auto& layout : layouts) {
        const float rowTop = rowYWithScroll(layout.top);
        if (rowTop + layout.height < contentBounds().y || rowTop > contentBounds().y + contentBounds().height) {
            continue;
        }
        if (layout.row.editKind != PropertyEditKind::Slider) {
            continue;
        }

        const auto track = sliderTrackBoundsForRow(layout.row, rowTop);
        if (!track.has_value()) {
            continue;
        }

        const ValueCellBounds sliderHitBounds{
            track->x,
            rowTop + 4.0f,
            track->width + kSliderValueWidth,
            kRowHeight - 8.0f
        };
        if (!containsPoint(sliderHitBounds, x, y)) {
            continue;
        }

        draggingSlider_ = true;
        draggingSliderKey_ = layout.row.key;
        pendingInteractionEdit_ = sliderEditAtPoint(rows, x, y);
        return true;
    }

    return false;
}

bool PropertyInspector::mouseDrag(const model::ProjectDocument& document, const utils::AppSettings& settings, float x, float y)
{
    if (!draggingScrollBarThumb_) {
        if (!draggingSlider_) {
            return false;
        }

        const auto rows = rowsForActiveTab(document, settings);
        updateScrollMetrics(rows);
        pendingInteractionEdit_ = sliderEditAtPoint(rows, x, y);
        return pendingInteractionEdit_.has_value();
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
        setScrollOffsetY(maxScroll * ((thumbTop - trackTop) / (trackHeight - thumb->height)));
    }
    else {
        setScrollOffsetY(0.0f);
    }
    return true;
}

bool PropertyInspector::mouseUp()
{
    const bool wasDragging = draggingScrollBarThumb_ || draggingSlider_;
    draggingScrollBarThumb_ = false;
    scrollBarDragOffsetY_ = 0.0f;
    draggingSlider_ = false;
    draggingSliderKey_.clear();
    return wasDragging;
}

bool PropertyInspector::mouseWheel(const model::ProjectDocument& document, const utils::AppSettings& settings, float deltaY, float x, float y)
{
    if (!isWithinVisibleContent(x, y)) {
        return false;
    }

    const auto rows = rowsForActiveTab(document, settings);
    updateScrollMetrics(rows);
    if (!needsVerticalScrollBar_) {
        return false;
    }

    setScrollOffsetY(scrollOffsetY_ + (-deltaY * kMouseWheelSensitivity));
    return true;
}

bool PropertyInspector::beginEditing(const model::ProjectDocument& document, const utils::AppSettings& settings, const std::string& key)
{
    const auto rows = rowsForActiveTab(document, settings);
    for (const auto& row : rows) {
        if (row.key == key && row.editKind != PropertyEditKind::ReadOnly && row.editKind != PropertyEditKind::Bool && row.editKind != PropertyEditKind::Slider) {
            activeKey_ = key;
            activeEditKind_ = row.editKind;
            editBuffer_ = row.displayValue;
            return true;
        }
    }

    return false;
}

std::optional<PropertyInspector::PropertyRow> PropertyInspector::activeRow(const model::ProjectDocument& document, const utils::AppSettings& settings) const
{
    if (!isEditing()) {
        return std::nullopt;
    }

    const auto rows = rowsForActiveTab(document, settings);
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
    clearActiveEventControl();
}

void PropertyInspector::cancelEditing()
{
    clearEditing();
}

void PropertyInspector::setActiveEventControl(const std::string& key, EventAction action)
{
    activeEventKey_ = key;
    activeEventAction_ = action;
}

void PropertyInspector::clearActiveEventControl()
{
    activeEventKey_.clear();
    activeEventAction_.reset();
}

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::activeEditorBounds(const model::ProjectDocument& document, const utils::AppSettings& settings)
{
    const auto active = activeRow(document, settings);
    if (!active.has_value() || active->editKind == PropertyEditKind::Slider) {
        return std::nullopt;
    }

    const auto rows = rowsForActiveTab(document, settings);
    updateScrollMetrics(rows);
    const auto bounds = contentBounds();
    const auto layouts = buildRowLayouts(bounds.y, rows);
    for (const auto& layout : layouts) {
        if (layout.row.key == active->key) {
            const float rowTop = rowYWithScroll(layout.top);
            if (rowTop + layout.height < bounds.y || rowTop > bounds.y + bounds.height) {
                return std::nullopt;
            }

            const float valueLeft = x_ + labelColumnWidth();
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

    if (activeEditKind_ == PropertyEditKind::Choice || activeEditKind_ == PropertyEditKind::Slider) {
        return std::nullopt;
    }

    return PendingEdit{ activeKey_, valueText, activeEditKind_ };
}

std::optional<PropertyInspector::PendingEdit> PropertyInspector::consumeInteractionEdit()
{
    if (!pendingInteractionEdit_.has_value()) {
        return std::nullopt;
    }

    auto edit = pendingInteractionEdit_;
    pendingInteractionEdit_.reset();
    return edit;
}

bool PropertyInspector::consumeScrollInteraction()
{
    const bool hadScrollInteraction = pendingScrollInteraction_;
    pendingScrollInteraction_ = false;
    return hadScrollInteraction;
}

std::optional<PropertyInspector::PendingEdit> PropertyInspector::sliderEditAtPoint(const std::vector<PropertyRow>& rows, float x, float y)
{
    const auto layouts = buildRowLayouts(contentBounds().y, rows);
    for (const auto& layout : layouts) {
        if (layout.row.key != draggingSliderKey_) {
            continue;
        }

        const float rowTop = rowYWithScroll(layout.top);
        const auto track = sliderTrackBoundsForRow(layout.row, rowTop);
        if (!track.has_value()) {
            return std::nullopt;
        }

        const float relative = std::clamp(x - track->x, 0.0f, track->width);
        const float safeMaximum = std::max(layout.row.minimumValue, layout.row.maximumValue);
        const float ratio = track->width <= 0.0f ? 0.0f : (relative / track->width);
        const float rawValue = layout.row.minimumValue + ratio * (safeMaximum - layout.row.minimumValue);
        const float value = clampAndStepSliderValue(rawValue, layout.row.minimumValue, layout.row.maximumValue, layout.row.stepValue);
        return PendingEdit{ layout.row.key, formatSliderValue(value, layout.row.stepValue), layout.row.editKind };
    }

    return std::nullopt;
}

bool PropertyInspector::isEditing() const
{
    return !activeKey_.empty();
}

bool PropertyInspector::isDraggingSlider() const
{
    return draggingSlider_;
}

const std::string& PropertyInspector::draggingSliderKey() const
{
    return draggingSliderKey_;
}

void PropertyInspector::setAppearanceState(model::WidgetAppearanceState state)
{
    appearanceState_ = state;
    clearEditing();
}

model::WidgetAppearanceState PropertyInspector::appearanceState() const
{
    return appearanceState_;
}

void PropertyInspector::setAppearancePreviewEnabled(bool enabled, const std::string& widgetId)
{
    appearancePreviewEnabled_ = enabled && !widgetId.empty();
    appearancePreviewWidgetId_ = appearancePreviewEnabled_ ? widgetId : std::string{};
    clearEditing();
}

void PropertyInspector::clearAppearancePreview()
{
    appearancePreviewEnabled_ = false;
    appearancePreviewWidgetId_.clear();
}

void PropertyInspector::synchronizeAppearancePreviewSelection(const std::string& widgetId)
{
    if (appearancePreviewEnabled_ && appearancePreviewWidgetId_ != widgetId) {
        clearAppearancePreview();
    }
}

bool PropertyInspector::appearancePreviewEnabledFor(const std::string& widgetId) const
{
    return appearancePreviewEnabled_
        && !widgetId.empty()
        && appearancePreviewWidgetId_ == widgetId;
}

void PropertyInspector::draw(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::ProjectDocument& document, const utils::AppSettings& settings, std::size_t selectionCount)
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

    const ValueCellBounds strip = tabStripBounds();
    const float tabWidth = std::max(48.0f, (strip.width - kTabGap) * 0.5f);
    const ValueCellBounds propertiesTab{ strip.x, strip.y, tabWidth, strip.height };
    const ValueCellBounds eventsTab{ strip.x + tabWidth + kTabGap, strip.y, tabWidth, strip.height };
    const auto drawTab = [&](const ValueCellBounds& bounds, InspectorTab tab, const char* label) {
        const bool selected = activeTab_ == tab;
        canvas.setColor(selected ? 0xff4b79bc : 0xff303744);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        canvas.setColor(selected ? 0xff92b9ff : 0xff1a2029);
        canvas.fill(bounds.x, bounds.y + bounds.height - 2.0f, bounds.width, 2.0f);
        if (drawText) {
            canvas.setColor(selected ? 0xffeef5ff : 0xffc7cfda);
            canvas.text(label, font, visage::Font::kCenter,
                bounds.x, bounds.y, bounds.width, bounds.height);
        }
    };

    canvas.setColor(0xff252b36);
    canvas.fill(strip.x, strip.y, strip.width, strip.height);
    drawTab(propertiesTab, InspectorTab::Properties, "Properties");
    drawTab(eventsTab, InspectorTab::Events, "Events");

    const auto rows = rowsForActiveTab(document, settings);
    updateScrollMetrics(rows);
    const ValueCellBounds bounds = contentBounds();

    if (selectedWidget == nullptr) {
        if (drawText) {
            canvas.setColor(0xffdde2ea);
            if (activeTab_ == InspectorTab::Events) {
                canvas.text("No widget selected.",
                    font, visage::Font::kTopLeft,
                    bounds.x + 10.0f, bounds.y + 10.0f, bounds.width - 20.0f, kRowHeight - 8.0f);
                canvas.setColor(0xffc7cfda);
                canvas.text("Select one widget to view supported events and assign handlers.",
                    font, visage::Font::kTopLeft,
                    bounds.x + 10.0f, bounds.y + 32.0f, bounds.width - 20.0f, kRowHeight - 8.0f);
            }
            else {
                canvas.text("No selection",
                    font, visage::Font::kTopLeft,
                    bounds.x + 10.0f, bounds.y + 10.0f, bounds.width - 20.0f, kRowHeight - 8.0f);
            }
        }
        return;
    }

    if (rows.empty()) {
        if (drawText) {
            canvas.setColor(0xffdde2ea);
            if (activeTab_ == InspectorTab::Events) {
                canvas.text("No supported events.",
                    font, visage::Font::kTopLeft,
                    bounds.x + 10.0f, bounds.y + 10.0f, bounds.width - 20.0f, kRowHeight - 8.0f);
                canvas.setColor(0xffc7cfda);
                canvas.text("Use the Properties tab for this widget's editable attributes.",
                    font, visage::Font::kTopLeft,
                    bounds.x + 10.0f, bounds.y + 32.0f, bounds.width - 20.0f, kRowHeight - 8.0f);
            }
            else {
                canvas.text("No properties available",
                    font, visage::Font::kTopLeft,
                    bounds.x + 10.0f, bounds.y + 10.0f, bounds.width - 20.0f, kRowHeight - 8.0f);
            }
        }
        return;
    }

    const auto layouts = buildRowLayouts(bounds.y, rows);

    canvas.saveState();
    canvas.setClampBounds(bounds.x, bounds.y, bounds.width, bounds.height);
    for (std::size_t index = 0; index < layouts.size(); ++index) {
        const auto& row = layouts[index].row;
        const float rowTop = rowYWithScroll(layouts[index].top);
        if (rowTop + layouts[index].height < bounds.y || rowTop > bounds.y + bounds.height) {
            continue;
        }

        canvas.setColor(index % 2 == 0 ? 0xff2b313d : 0xff262c37);
        canvas.fill(bounds.x, rowTop, bounds.width, kRowHeight - 2.0f);

        const float labelWidth = labelColumnWidth();
        const float labelLeft = x_ + 18.0f;
        const float valueLeft = x_ + labelWidth;
        const float valueWidth = valueCellWidth();
        const bool isReadOnly = row.editKind == PropertyEditKind::ReadOnly;
        const bool isActive = isEditing() && row.key == activeKey_;
        const bool isChoice = row.editKind == PropertyEditKind::Choice;
        const bool isSlider = row.editKind == PropertyEditKind::Slider;
        const auto swatchBounds = colorSwatchBoundsForRow(row, rowTop);
        const float actionWidth = row.actionText.empty() ? 0.0f : kActionButtonWidth;
        const float valueTextWidth = swatchBounds.has_value()
            ? std::max(0.0f, valueWidth - swatchBounds->width - 16.0f)
            : valueWidth;
        const float summaryTextWidth = std::max(0.0f, valueTextWidth - (actionWidth > 0.0f ? actionWidth + 10.0f : 0.0f));

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
                labelLeft, rowTop + 5.0f, std::max(40.0f, labelWidth - 32.0f), kRowHeight - 8.0f);

            canvas.setColor(isReadOnly ? 0xffb3bcc9 : 0xffeef2f8);
            if (row.isEvent) {
                const auto eventControlIsActive = [&](EventAction action) {
                    return activeEventAction_.has_value()
                        && activeEventKey_ == row.key
                        && *activeEventAction_ == action;
                };
                const auto selectorBounds = eventSelectorBoundsForRow(row, rowTop);
                const auto selectorTextBounds = eventSelectorTextBoundsForRow(row, rowTop);
                const auto selectorArrowBounds = eventSelectorArrowBoundsForRow(row, rowTop);
                const auto createBounds = eventActionBoundsForRow(row, rowTop, EventAction::Create);
                const auto existingBounds = eventActionBoundsForRow(row, rowTop, EventAction::Existing);
                const auto clearBounds = eventActionBoundsForRow(row, rowTop, EventAction::Clear);
                const std::string assignmentText = row.displayValue.empty() ? "<unset>" : row.displayValue;
                if (selectorBounds.has_value()) {
                    const bool selectorActive = eventControlIsActive(EventAction::Existing);
                    canvas.setColor(selectorActive ? 0xff2f476d : (row.displayValue.empty() ? 0xff39414f : 0xff3f4a5c));
                    canvas.fill(selectorBounds->x, selectorBounds->y, selectorBounds->width, selectorBounds->height);
                    canvas.setColor(selectorActive ? 0xff92b9ff : 0xff1a2029);
                    canvas.fill(selectorBounds->x, selectorBounds->y, selectorBounds->width, 1.0f);
                    canvas.fill(selectorBounds->x, selectorBounds->y + selectorBounds->height - 1.0f, selectorBounds->width, 1.0f);
                    if (selectorArrowBounds.has_value()) {
                        canvas.setColor(selectorActive ? 0xff4e617d : 0xff465366);
                        canvas.fill(selectorArrowBounds->x - kEventSelectorArrowGap,
                            selectorArrowBounds->y + 3.0f,
                            1.0f,
                            std::max(0.0f, selectorArrowBounds->height - 6.0f));
                        canvas.setColor(0xffaeb8c6);
                        canvas.text("v", font, visage::Font::kCenter,
                            selectorArrowBounds->x, selectorArrowBounds->y, selectorArrowBounds->width, selectorArrowBounds->height);
                    }
                }

                const float assignmentLeft = selectorTextBounds.has_value() ? selectorTextBounds->x : (selectorBounds.has_value() ? selectorBounds->x + kEventSelectorTextPadding : valueLeft + 8.0f);
                const float assignmentRight = selectorTextBounds.has_value()
                    ? selectorTextBounds->x + selectorTextBounds->width
                    : (createBounds.has_value() ? createBounds->x - 8.0f : valueLeft + valueWidth - 8.0f);
                canvas.setColor(row.displayValue.empty() ? 0xffaeb8c6 : 0xffeef2f8);
                canvas.text(assignmentText, font, visage::Font::kTopLeft,
                    assignmentLeft, rowTop + 5.0f, std::max(24.0f, assignmentRight - assignmentLeft), kRowHeight - 8.0f);

                const auto drawEventButton = [&](const std::optional<ValueCellBounds>& buttonBounds, const char* label, EventAction action, bool enabled, bool danger = false) {
                    if (!buttonBounds.has_value()) {
                        return;
                    }

                    const bool buttonActive = eventControlIsActive(action);
                    canvas.setColor(enabled ? (buttonActive ? 0xff4b79bc : (danger ? 0xff6c3038 : 0xff355382)) : 0xff303541);
                    canvas.fill(buttonBounds->x, buttonBounds->y, buttonBounds->width, buttonBounds->height);
                    if (buttonActive) {
                        canvas.setColor(0xff92b9ff);
                        canvas.fill(buttonBounds->x, buttonBounds->y + buttonBounds->height - 2.0f, buttonBounds->width, 2.0f);
                    }
                    canvas.setColor(enabled ? (danger ? 0xffffd6dc : 0xffd8e8ff) : 0xff8d98a8);
                    canvas.text(label, font, visage::Font::kCenter,
                        buttonBounds->x, buttonBounds->y, buttonBounds->width, buttonBounds->height);
                };

                drawEventButton(createBounds, "Create", EventAction::Create, true);
                drawEventButton(existingBounds, "Existing", EventAction::Existing, !row.choices.empty());
                drawEventButton(clearBounds, "Clear", EventAction::Clear, true, true);

                if (!row.errorText.empty()) {
                    const float errorTop = rowTop + kRowHeight;
                    canvas.setColor(0xff352229);
                    canvas.fill(valueLeft, errorTop, valueWidth, kEventErrorHeight - 4.0f);
                    canvas.setColor(0xffff8a8a);
                    canvas.fill(valueLeft, errorTop, 2.0f, kEventErrorHeight - 4.0f);
                    canvas.text(row.errorText, font, visage::Font::kTopLeft,
                        valueLeft + 8.0f, errorTop + 4.0f, valueWidth - 14.0f, kEventErrorHeight - 8.0f);
                }
            }
            else if (isSlider) {
                const auto track = sliderTrackBoundsForRow(row, rowTop);
                const float sliderValue = tryParseFloatText(row.displayValue).value_or(row.minimumValue);
                const auto thumb = sliderThumbBoundsForRow(row, rowTop, sliderValue);
                if (track.has_value()) {
                    canvas.setColor(0xff1d222b);
                    canvas.fill(track->x, track->y, track->width, track->height);

                    if (thumb.has_value()) {
                        const float fillWidth = std::max(0.0f, thumb->x + thumb->width * 0.5f - track->x);
                        canvas.setColor(0xff4b79bc);
                        canvas.fill(track->x, track->y, fillWidth, track->height);
                        canvas.setColor((draggingSlider_ && draggingSliderKey_ == row.key) ? 0xff92b9ff : 0xffd7e6ff);
                        canvas.fill(thumb->x, thumb->y, thumb->width, thumb->height);
                    }
                }

                const std::string valueLabel = formatSliderValue(sliderValue, row.stepValue)
                    + " / " + formatSliderValue(std::max(row.minimumValue, row.maximumValue), row.stepValue);
                canvas.text(valueLabel, font, visage::Font::kTopLeft,
                    valueLeft + valueWidth - kSliderValueWidth, rowTop + 5.0f, kSliderValueWidth - 6.0f, kRowHeight - 8.0f);
            }
            else if (!isActive || row.editKind == PropertyEditKind::Choice) {
                std::string valueText = row.displayValue;
                if (isChoice) {
                    if (valueText.empty()) {
                        valueText = "Select";
                    }
                    valueText += "  >";
                }
                canvas.text(valueText, font, visage::Font::kTopLeft,
                    valueLeft + 8.0f, rowTop + 5.0f, summaryTextWidth - 12.0f, kRowHeight - 8.0f);

                if (!row.actionText.empty()) {
                    const float actionLeft = valueLeft + valueWidth - actionWidth - 8.0f;
                    canvas.setColor(0xff355382);
                    canvas.fill(actionLeft, rowTop + 5.0f, actionWidth, kRowHeight - 12.0f);
                    canvas.setColor(0xffd8e8ff);
                    canvas.text(row.actionText, font, visage::Font::kCenter,
                        actionLeft, rowTop + 5.0f, actionWidth, kRowHeight - 12.0f);
                    canvas.setColor(isReadOnly ? 0xffb3bcc9 : 0xffeef2f8);
                }
            }

            if (isActive && row.editKind != PropertyEditKind::Choice && row.editKind != PropertyEditKind::Slider) {
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
