#include "model/LookAndFeelRegistry.h"
#include "model/ProjectDocument.h"
#include "model/WidgetNode.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace visiform::model {
namespace {

LookAndFeelDefinition makeDefinition(
    std::string id,
    std::string displayName,
    std::string panel,
    std::string control,
    std::string text,
    std::string secondaryText,
    std::string border,
    std::string accent,
    std::string disabled,
    std::string recessed,
    std::string raised,
    std::string selected,
    std::string hover,
    std::string pressed,
    std::string checked,
    std::string highlight,
    std::string shadow,
    float borderThickness,
    float cornerRadius,
    float fontSize,
    float controlPadding)
{
    LookAndFeelDefinition definition;
    definition.id = std::move(id);
    definition.displayName = std::move(displayName);
    definition.panelColor = std::move(panel);
    definition.controlFillColor = std::move(control);
    definition.controlTextColor = std::move(text);
    definition.secondaryTextColor = std::move(secondaryText);
    definition.controlBorderColor = std::move(border);
    definition.accentColor = std::move(accent);
    definition.disabledColor = disabled;
    definition.disabledTextColor = std::move(disabled);
    definition.recessedSurfaceColor = std::move(recessed);
    definition.raisedSurfaceColor = std::move(raised);
    definition.focusOutlineColor = definition.accentColor;
    definition.selectedStateColor = std::move(selected);
    definition.hoverStateColor = std::move(hover);
    definition.pressedStateColor = std::move(pressed);
    definition.checkedStateColor = std::move(checked);
    definition.highlightEdgeColor = std::move(highlight);
    definition.shadowEdgeColor = std::move(shadow);
    definition.borderThickness = borderThickness;
    definition.cornerRadius = cornerRadius;
    definition.fontSize = fontSize;
    definition.controlPadding = controlPadding;
    return definition;
}

const std::string& valueOrFallback(const std::string& value, const std::string& fallback)
{
    return value.empty() ? fallback : value;
}

bool isValidColor(const std::string& value)
{
    return (value.size() == 7 || value.size() == 9)
        && value.front() == '#'
        && std::all_of(value.begin() + 1, value.end(), [](unsigned char character) {
               return std::isxdigit(character) != 0;
           });
}

void applyColorOverride(const std::optional<std::string>& overrideValue, std::string& target)
{
    if (overrideValue.has_value() && isValidColor(*overrideValue)) {
        target = *overrideValue;
    }
}

} // namespace

LookAndFeelRegistry::LookAndFeelRegistry()
    : definitions_{
        makeDefinition("VisiFormDark", "VisiForm Dark",
            "#1F242D", "#2B313D", "#EEF2F8", "#AEB8C8", "#97A3B7", "#2D7FF9", "#6C7788",
            "#202630", "#303744", "#355382", "#354052", "#232A35", "#355382", "#C8D2E2", "#11151C",
            1.0f, 4.0f, 16.0f, 8.0f),
        makeDefinition("VisiFormLight", "VisiForm Light",
            "#F2F4F8", "#FFFFFF", "#1B2533", "#596779", "#B8C2D0", "#2D7FF9", "#8A95A5",
            "#E8ECF2", "#FFFFFF", "#D7E6FF", "#EDF4FF", "#DCE4EF", "#D7E6FF", "#FFFFFF", "#8794A6",
            1.0f, 4.0f, 16.0f, 8.0f),
        makeDefinition("ImGuiDark", "ImGui Dark",
            "#1E1E1E", "#2B2B2B", "#E6E6E6", "#A4A4A4", "#4A4F57", "#3D84F7", "#747982",
            "#242424", "#303030", "#35527A", "#344153", "#202020", "#35527A", "#737982", "#101010",
            1.0f, 6.0f, 15.0f, 7.0f),
        makeDefinition("FlatClassic", "Flat Classic",
            "#E7EAEE", "#F7F7F7", "#20262F", "#59616C", "#9CA8B5", "#4C86D9", "#8F99A5",
            "#DFE3E8", "#F7F7F7", "#D9E5F5", "#EEF3F9", "#D5DAE1", "#D9E5F5", "#FFFFFF", "#788493",
            1.0f, 0.0f, 16.0f, 8.0f) }
{
    builtInCount_ = definitions_.size();
}

LookAndFeelRegistry& LookAndFeelRegistry::instance()
{
    static LookAndFeelRegistry registry;
    return registry;
}

const LookAndFeelDefinition* LookAndFeelRegistry::findById(const std::string& id) const
{
    const auto iterator = std::find_if(definitions_.begin(), definitions_.end(),
        [&id](const LookAndFeelDefinition& definition) { return definition.id == id; });
    return iterator == definitions_.end() ? nullptr : &*iterator;
}

const LookAndFeelDefinition& LookAndFeelRegistry::defaultDefinition() const
{
    return definitions_.front();
}

const std::vector<LookAndFeelDefinition>& LookAndFeelRegistry::definitions() const
{
    return definitions_;
}

bool LookAndFeelRegistry::isBuiltIn(const std::string& id) const
{
    return std::any_of(definitions_.begin(), definitions_.begin() + static_cast<std::ptrdiff_t>(builtInCount_),
        [&id](const LookAndFeelDefinition& definition) { return definition.id == id; });
}

void LookAndFeelRegistry::setCustomDefinitions(std::vector<LookAndFeelDefinition> definitions)
{
    definitions_.resize(builtInCount_);
    for (auto& definition : definitions) {
        if (definition.id.empty() || isBuiltIn(definition.id) || findById(definition.id) != nullptr) {
            continue;
        }
        definitions_.push_back(std::move(definition));
    }
}

ResolvedLookAndFeelStyle LookAndFeelRegistry::resolveProjectStyle(
    const std::string& lookAndFeelId,
    const LookAndFeelOverrides& overrides) const
{
    const LookAndFeelDefinition* definition = findById(lookAndFeelId);
    const LookAndFeelDefinition& fallback = defaultDefinition();
    if (definition == nullptr) {
        definition = &fallback;
    }

    ResolvedLookAndFeelStyle style;
    style.id = definition->id;
    style.applicationSurfaceColor = valueOrFallback(definition->panelColor, fallback.panelColor);
    style.controlSurfaceColor = valueOrFallback(definition->controlFillColor, fallback.controlFillColor);
    style.recessedSurfaceColor = valueOrFallback(definition->recessedSurfaceColor, fallback.recessedSurfaceColor);
    style.raisedSurfaceColor = valueOrFallback(definition->raisedSurfaceColor, fallback.raisedSurfaceColor);
    style.primaryTextColor = valueOrFallback(definition->controlTextColor, fallback.controlTextColor);
    style.secondaryTextColor = valueOrFallback(definition->secondaryTextColor, fallback.secondaryTextColor);
    style.disabledTextColor = valueOrFallback(definition->disabledTextColor, fallback.disabledTextColor);
    style.disabledSurfaceColor = valueOrFallback(definition->disabledColor, fallback.disabledColor);
    style.borderColor = valueOrFallback(definition->controlBorderColor, fallback.controlBorderColor);
    style.focusOutlineColor = valueOrFallback(definition->focusOutlineColor, fallback.focusOutlineColor);
    style.accentColor = valueOrFallback(definition->accentColor, fallback.accentColor);
    style.selectedStateColor = valueOrFallback(definition->selectedStateColor, fallback.selectedStateColor);
    style.hoverStateColor = valueOrFallback(definition->hoverStateColor, fallback.hoverStateColor);
    style.pressedStateColor = valueOrFallback(definition->pressedStateColor, fallback.pressedStateColor);
    style.checkedStateColor = valueOrFallback(definition->checkedStateColor, fallback.checkedStateColor);
    style.highlightEdgeColor = valueOrFallback(definition->highlightEdgeColor, fallback.highlightEdgeColor);
    style.shadowEdgeColor = valueOrFallback(definition->shadowEdgeColor, fallback.shadowEdgeColor);
    style.borderThickness = definition->borderThickness > 0.0f ? definition->borderThickness : fallback.borderThickness;
    style.cornerRadius = definition->cornerRadius >= 0.0f ? definition->cornerRadius : fallback.cornerRadius;
    style.fontSize = definition->fontSize > 0.0f ? definition->fontSize : fallback.fontSize;
    style.controlPadding = definition->controlPadding >= 0.0f ? definition->controlPadding : fallback.controlPadding;
    style.splitterHighlightThickness = definition->splitterHighlightThickness > 0.0f
        ? definition->splitterHighlightThickness
        : fallback.splitterHighlightThickness;
    style.splitterShadowThickness = definition->splitterShadowThickness > 0.0f
        ? definition->splitterShadowThickness
        : fallback.splitterShadowThickness;

    applyColorOverride(overrides.applicationSurfaceColor, style.applicationSurfaceColor);
    applyColorOverride(overrides.controlSurfaceColor, style.controlSurfaceColor);
    applyColorOverride(overrides.recessedSurfaceColor, style.recessedSurfaceColor);
    applyColorOverride(overrides.primaryTextColor, style.primaryTextColor);
    applyColorOverride(overrides.disabledTextColor, style.disabledTextColor);
    applyColorOverride(overrides.borderColor, style.borderColor);
    applyColorOverride(overrides.focusOutlineColor, style.focusOutlineColor);
    applyColorOverride(overrides.accentColor, style.accentColor);
    applyColorOverride(overrides.highlightEdgeColor, style.highlightEdgeColor);
    applyColorOverride(overrides.shadowEdgeColor, style.shadowEdgeColor);
    if (overrides.borderThickness.has_value()) {
        style.borderThickness = std::clamp(*overrides.borderThickness, 0.0f, 20.0f);
    }
    if (overrides.cornerRadius.has_value()) {
        style.cornerRadius = std::clamp(*overrides.cornerRadius, 0.0f, 50.0f);
    }
    if (overrides.controlPadding.has_value()) {
        style.controlPadding = std::clamp(*overrides.controlPadding, 0.0f, 40.0f);
    }
    if (overrides.splitterHighlightThickness.has_value()) {
        style.splitterHighlightThickness = std::clamp(*overrides.splitterHighlightThickness, 0.0f, 8.0f);
    }
    if (overrides.splitterShadowThickness.has_value()) {
        style.splitterShadowThickness = std::clamp(*overrides.splitterShadowThickness, 0.0f, 8.0f);
    }

    return style;
}

ResolvedLookAndFeelStyle LookAndFeelRegistry::resolve(
    const ProjectDocument& document,
    const WidgetNode& widget) const
{
    const std::string widgetLookAndFeelId = widget.getStringProperty("lookAndFeelId", {});
    ResolvedLookAndFeelStyle style = widgetLookAndFeelId.empty()
        ? resolveProjectStyle(document.lookAndFeelId, document.lookAndFeelOverrides)
        : resolveProjectStyle(widgetLookAndFeelId, {});

    if (!document.lookAndFeelOverrides.borderThickness.has_value()) {
        style.borderThickness = std::clamp(widget.getFloatProperty("borderThickness", style.borderThickness), 0.0f, 20.0f);
    }
    if (!document.lookAndFeelOverrides.cornerRadius.has_value()) {
        style.cornerRadius = std::clamp(widget.getFloatProperty("cornerRadius", style.cornerRadius), 0.0f, 50.0f);
    }
    style.fontSize = std::clamp(widget.getFloatProperty("fontSize", style.fontSize), 8.0f, 72.0f);

    const auto applyColorOverride = [&widget](const char* key, std::string& value) {
        const std::string overrideValue = widget.getStringProperty(key, {});
        if (!overrideValue.empty()) {
            value = overrideValue;
        }
    };
    applyColorOverride("fillColor", style.controlSurfaceColor);
    applyColorOverride("textColor", style.primaryTextColor);
    applyColorOverride("borderColor", style.borderColor);
    applyColorOverride("accentColor", style.accentColor);

    if (widget.type == WidgetType::FormWindow
        || widget.type == WidgetType::Frame
        || widget.type == WidgetType::GroupBox
        || widget.type == WidgetType::Panel
        || widget.type == WidgetType::Sizer) {
        const std::string backgroundOverride = widget.getStringProperty("backgroundColor", {});
        if (!backgroundOverride.empty()) {
            style.controlSurfaceColor = backgroundOverride;
        }
        else if (widget.type == WidgetType::FormWindow) {
            style.controlSurfaceColor = style.applicationSurfaceColor;
        }
    }

    return style;
}

} // namespace visiform::model
