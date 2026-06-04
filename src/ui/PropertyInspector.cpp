#include "ui/PropertyInspector.h"

#include "ui/PropertyInspector.h"

#include "model/LookAndFeelRegistry.h"
#include "model/WidgetItemUtils.h"
#include "model/WidgetRegistry.h"
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
#include <vector>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kRowHeight = 32.0f;
constexpr float kSuggestionRowHeight = 24.0f;
constexpr float kSuggestionSpacing = 2.0f;
constexpr float kPadding = 12.0f;
constexpr float kMinLabelColumnWidth = 128.0f;
constexpr float kPreferredLabelColumnWidth = 152.0f;
constexpr float kMaxLabelColumnWidth = 168.0f;
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
    for (const auto& definition : model::LookAndFeelRegistry::instance().definitions()) {
        choices.push_back(makeChoice(definition.id, definition.id, "Applies this registered look and feel preset."));
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
        choices.push_back(makeChoice(handlerName, handlerName, "Compatible callback with signature kind " + eventDefinition.handlerSignatureKind + "."));
    }
    return choices;
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
    case model::PropertyEditKind::Slider:
        return PropertyInspector::PropertyEditKind::Slider;
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
    const float labelWidth = labelColumnWidth();
    return std::max(0.0f, bounds.width - (labelWidth - (bounds.x - x_)) - 12.0f);
}

float PropertyInspector::labelColumnWidth() const
{
    const ValueCellBounds bounds = contentBounds();
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
    rows.push_back({ "x", "X", "Widget left position relative to the current parent container.", formatFloat(selectedWidget->bounds.x), PropertyEditKind::Float });
    rows.push_back({ "y", "Y", "Widget top position relative to the current parent container.", formatFloat(selectedWidget->bounds.y), PropertyEditKind::Float });
    rows.push_back({ "width", "Width", "Widget width in form coordinates.", formatFloat(selectedWidget->bounds.width), PropertyEditKind::Float });
    rows.push_back({ "height", "Height", "Widget height in form coordinates.", formatFloat(selectedWidget->bounds.height), PropertyEditKind::Float });

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

    std::set<std::string> drawnKeys;
    for (const auto& row : rows) {
        drawnKeys.insert(row.key);
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
                    "Click the value area or Edit... to open the item list editor.",
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
            if (property.key == "anchor" && selectedWidget->dockMode() != model::DockMode::None) {
                rowEditKind = PropertyEditKind::ReadOnly;
                if (!rowHint.empty()) {
                    rowHint += " ";
                }
                rowHint += "Ignored while Dock is not None.";
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
        for (const auto& event : definition->events) {
            const std::string displayValue = displayTextOrFallback(selectedWidget, event.key, {});
            rows.push_back({
                event.key,
                event.label,
                event.hint,
                displayValue,
                PropertyEditKind::Text,
                false,
                callbackChoices(document, event)
            });
            drawnKeys.insert(event.key);
        }

        if (rows.size() > propertyCountBeforeEvents) {
            rows.insert(rows.begin() + static_cast<std::ptrdiff_t>(propertyCountBeforeEvents),
                PropertyRow{ "__section_events", "Events", {}, {}, PropertyEditKind::ReadOnly, true });
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

std::optional<PropertyInspector::PropertyRow> PropertyInspector::hitTestRow(const model::ProjectDocument& document, const utils::AppSettings& settings, float x, float y)
{
    const auto rows = buildRows(document, settings);
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

std::optional<std::string> PropertyInspector::hitTestColorSwatch(const model::ProjectDocument& document, const utils::AppSettings& settings, float x, float y)
{
    const auto rows = buildRows(document, settings);
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

bool PropertyInspector::mouseDown(const model::ProjectDocument& document, const utils::AppSettings& settings, float x, float y)
{
    const auto rows = buildRows(document, settings);
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
        if (rowTop + kRowHeight < contentBounds().y || rowTop > contentBounds().y + contentBounds().height) {
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

        const auto rows = buildRows(document, settings);
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
    if (!contains(x, y)) {
        return false;
    }

    const auto rows = buildRows(document, settings);
    updateScrollMetrics(rows);
    if (!needsVerticalScrollBar_) {
        return false;
    }

    setScrollOffsetY(scrollOffsetY_ + (-deltaY * kMouseWheelSensitivity));
    return true;
}

bool PropertyInspector::beginEditing(const model::ProjectDocument& document, const utils::AppSettings& settings, const std::string& key)
{
    const auto rows = buildRows(document, settings);
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

    const auto rows = buildRows(document, settings);
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

std::optional<PropertyInspector::ValueCellBounds> PropertyInspector::activeEditorBounds(const model::ProjectDocument& document, const utils::AppSettings& settings)
{
    const auto active = activeRow(document, settings);
    if (!active.has_value() || active->editKind == PropertyEditKind::Slider) {
        return std::nullopt;
    }

    const auto rows = buildRows(document, settings);
    updateScrollMetrics(rows);
    const auto bounds = contentBounds();
    const auto layouts = buildRowLayouts(bounds.y, rows);
    for (const auto& layout : layouts) {
        if (layout.row.key == active->key) {
            const float rowTop = rowYWithScroll(layout.top);
            if (rowTop + kRowHeight < bounds.y || rowTop > bounds.y + bounds.height) {
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

    if (selectedWidget == nullptr) {
        if (drawText) {
            canvas.setColor(0xffdde2ea);
            canvas.text("No selection", font, visage::Font::kTopLeft,
                x_ + 18.0f, y_ + kHeaderHeight + 12.0f, width_ - 30.0f, kRowHeight - 8.0f);
        }
        return;
    }

    const auto rows = buildRows(document, settings);
    updateScrollMetrics(rows);
    const ValueCellBounds bounds = contentBounds();
    const auto layouts = buildRowLayouts(bounds.y, rows);

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
            if (isSlider) {
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
