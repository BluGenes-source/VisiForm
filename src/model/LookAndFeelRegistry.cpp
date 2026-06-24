#include "model/LookAndFeelRegistry.h"
#include "model/ProjectDocument.h"
#include "model/WidgetNode.h"

#include <algorithm>
#include <array>
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
    definition.fontFamily = "Default";
    definition.fontSize = fontSize;
    definition.fontWeight = 400;
    definition.italic = false;
    definition.controlPadding = controlPadding;
    definition.textPadding = controlPadding;
    definition.disabledTextTreatment = "Muted";
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

constexpr std::array<std::string_view, 10> kWidgetOverrideKeys = {
    "controlSurfaceColor",
    "textColor",
    "borderColor",
    "accentColor",
    "focusOutlineColor",
    "highlightEdgeColor",
    "shadowEdgeColor",
    "borderThickness",
    "cornerRadius",
    "controlPadding"
};

constexpr std::array<std::string_view, 7> kWidgetTypographyOverrideKeys = {
    "fontFamily",
    "fontSize",
    "fontWeight",
    "italic",
    "horizontalTextAlignment",
    "verticalTextAlignment",
    "textPadding"
};

constexpr std::array<std::string_view, 7> kWidgetStateOverrideKeys = {
    "controlSurfaceColor",
    "textColor",
    "borderColor",
    "accentColor",
    "focusOutlineColor",
    "highlightEdgeColor",
    "shadowEdgeColor"
};

bool isSupportedWidgetType(WidgetType type)
{
    switch (type) {
    case WidgetType::Button:
    case WidgetType::TextBox:
    case WidgetType::CheckBox:
    case WidgetType::RadioButton:
    case WidgetType::ComboBox:
    case WidgetType::ListBox:
    case WidgetType::Slider:
    case WidgetType::ScrollBar:
    case WidgetType::ProgressBar:
    case WidgetType::ColorPicker:
    case WidgetType::Frame:
    case WidgetType::GroupBox:
    case WidgetType::Panel:
    case WidgetType::TabControl:
        return true;
    default:
        return false;
    }
}

bool isTextBearingWidgetType(WidgetType type)
{
    switch (type) {
    case WidgetType::Button:
    case WidgetType::Label:
    case WidgetType::TextBox:
    case WidgetType::CheckBox:
    case WidgetType::RadioButton:
    case WidgetType::ComboBox:
    case WidgetType::ListBox:
    case WidgetType::GroupBox:
    case WidgetType::Frame:
    case WidgetType::TabControl:
    case WidgetType::StatusBar:
    case WidgetType::MenuBar:
        return true;
    default:
        return false;
    }
}

bool isStateSupportedWidgetType(WidgetType type)
{
    switch (type) {
    case WidgetType::Button:
    case WidgetType::TextBox:
    case WidgetType::CheckBox:
    case WidgetType::RadioButton:
    case WidgetType::ComboBox:
    case WidgetType::ListBox:
    case WidgetType::Slider:
    case WidgetType::ScrollBar:
    case WidgetType::ProgressBar:
    case WidgetType::ColorPicker:
    case WidgetType::TabControl:
        return true;
    default:
        return false;
    }
}

void applyStateOverrides(const WidgetStateLookAndFeelOverrides& overrides,
    WidgetAppearanceState state,
    ResolvedLookAndFeelStyle& style)
{
    const auto apply = [](const std::optional<std::string>& value, std::string& target) {
        if (value.has_value() && isValidColor(*value)) {
            target = *value;
        }
    };

    if (overrides.controlSurfaceColor.has_value() && isValidColor(*overrides.controlSurfaceColor)) {
        switch (state) {
        case WidgetAppearanceState::Hover:
            style.hoverStateColor = *overrides.controlSurfaceColor;
            break;
        case WidgetAppearanceState::Pressed:
            style.pressedStateColor = *overrides.controlSurfaceColor;
            break;
        case WidgetAppearanceState::CheckedOrSelected:
            style.checkedStateColor = *overrides.controlSurfaceColor;
            style.selectedStateColor = *overrides.controlSurfaceColor;
            break;
        case WidgetAppearanceState::Disabled:
            style.disabledSurfaceColor = *overrides.controlSurfaceColor;
            break;
        case WidgetAppearanceState::Focused:
        case WidgetAppearanceState::Normal:
            style.controlSurfaceColor = *overrides.controlSurfaceColor;
            break;
        }
    }
    if (state == WidgetAppearanceState::Disabled) {
        apply(overrides.textColor, style.disabledTextColor);
    }
    else {
        apply(overrides.textColor, style.primaryTextColor);
    }
    apply(overrides.borderColor, style.borderColor);
    apply(overrides.accentColor, style.accentColor);
    apply(overrides.focusOutlineColor, style.focusOutlineColor);
    apply(overrides.highlightEdgeColor, style.highlightEdgeColor);
    apply(overrides.shadowEdgeColor, style.shadowEdgeColor);
}

std::string normalizedAlignment(std::string value, bool vertical)
{
    if (value == "Left" || value == "Center" || value == "Right") {
        return vertical && value != "Center" ? "Default" : value;
    }
    if (vertical && (value == "Top" || value == "Bottom")) {
        return value;
    }
    return "Default";
}

std::string normalizedDisabledTextTreatment(std::string value)
{
    return value == "Muted" || value == "Normal" ? value : "Muted";
}

} // namespace

std::string_view toString(WidgetAppearanceState state)
{
    switch (state) {
    case WidgetAppearanceState::Normal: return "normal";
    case WidgetAppearanceState::Hover: return "hover";
    case WidgetAppearanceState::Pressed: return "pressed";
    case WidgetAppearanceState::Focused: return "focused";
    case WidgetAppearanceState::CheckedOrSelected: return "checkedOrSelected";
    case WidgetAppearanceState::Disabled: return "disabled";
    }
    return "normal";
}

std::optional<WidgetAppearanceState> widgetAppearanceStateFromString(std::string_view value)
{
    if (value == "normal") return WidgetAppearanceState::Normal;
    if (value == "hover") return WidgetAppearanceState::Hover;
    if (value == "pressed") return WidgetAppearanceState::Pressed;
    if (value == "focused") return WidgetAppearanceState::Focused;
    if (value == "checkedOrSelected") return WidgetAppearanceState::CheckedOrSelected;
    if (value == "disabled") return WidgetAppearanceState::Disabled;
    return std::nullopt;
}

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
    style.fontFamily = valueOrFallback(definition->fontFamily, fallback.fontFamily);
    style.fontSize = definition->fontSize > 0.0f ? definition->fontSize : fallback.fontSize;
    style.fontWeight = normalizeFontWeight(definition->fontWeight);
    style.italic = definition->italic;
    style.controlPadding = definition->controlPadding >= 0.0f ? definition->controlPadding : fallback.controlPadding;
    style.textPadding = definition->textPadding >= 0.0f ? definition->textPadding : fallback.textPadding;
    style.disabledTextTreatment = normalizedDisabledTextTreatment(
        valueOrFallback(definition->disabledTextTreatment, fallback.disabledTextTreatment));
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
        style.borderThickness = std::clamp(*overrides.borderThickness, 0.0f, 25.0f);
    }
    if (overrides.cornerRadius.has_value()) {
        style.cornerRadius = std::clamp(*overrides.cornerRadius, 0.0f, 50.0f);
    }
    if (overrides.controlPadding.has_value()) {
        style.controlPadding = std::clamp(*overrides.controlPadding, 0.0f, 40.0f);
    }
    if (overrides.fontFamily.has_value() && !overrides.fontFamily->empty()) {
        style.fontFamily = *overrides.fontFamily;
    }
    if (overrides.fontSize.has_value()) {
        style.fontSize = std::clamp(*overrides.fontSize, 8.0f, 72.0f);
    }
    if (overrides.fontWeight.has_value()) {
        style.fontWeight = normalizeFontWeight(*overrides.fontWeight);
    }
    if (overrides.italic.has_value()) {
        style.italic = *overrides.italic;
    }
    if (overrides.textPadding.has_value()) {
        style.textPadding = std::clamp(*overrides.textPadding, 0.0f, 40.0f);
    }
    if (overrides.disabledTextTreatment.has_value()) {
        style.disabledTextTreatment = normalizedDisabledTextTreatment(*overrides.disabledTextTreatment);
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
    // Legacy widget style properties remain readable for existing project files.
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
    const std::string legacyFontFamily = widget.getStringProperty("fontFamily", {});
    if (!legacyFontFamily.empty()) {
        style.fontFamily = legacyFontFamily;
    }
    if (widget.getBoolProperty("fontBold", false)) {
        style.fontWeight = normalizeFontWeight(700);
    }
    if (widget.getBoolProperty("fontItalic", false)) {
        style.italic = true;
    }

    const auto applyColorOverride = [&widget](const char* key, std::string& value) {
        const std::string overrideValue = widget.getStringProperty(key, {});
        if (!overrideValue.empty()) {
            value = overrideValue;
        }
    };
    const auto applyAppearanceOverride = [](const std::optional<std::string>& overrideValue, std::string& value) {
        if (overrideValue.has_value() && !overrideValue->empty()) {
            value = *overrideValue;
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

    if (supportsWidgetOverrides(widget.type)) {
        const auto& overrides = widget.appearanceOverrides;
        applyAppearanceOverride(overrides.controlSurfaceColor, style.controlSurfaceColor);
        applyAppearanceOverride(overrides.textColor, style.primaryTextColor);
        applyAppearanceOverride(overrides.borderColor, style.borderColor);
        applyAppearanceOverride(overrides.accentColor, style.accentColor);
        applyAppearanceOverride(overrides.focusOutlineColor, style.focusOutlineColor);
        applyAppearanceOverride(overrides.highlightEdgeColor, style.highlightEdgeColor);
        applyAppearanceOverride(overrides.shadowEdgeColor, style.shadowEdgeColor);
        if (overrides.borderThickness.has_value()) {
            style.borderThickness = std::clamp(*overrides.borderThickness, 0.0f, 25.0f);
        }
        if (overrides.cornerRadius.has_value()) {
            style.cornerRadius = std::clamp(*overrides.cornerRadius, 0.0f, 50.0f);
        }
        if (overrides.controlPadding.has_value()) {
            style.controlPadding = std::clamp(*overrides.controlPadding, 0.0f, 40.0f);
        }
        if (overrides.fontFamily.has_value() && !overrides.fontFamily->empty()) {
            style.fontFamily = *overrides.fontFamily;
        }
        if (overrides.fontSize.has_value()) {
            style.fontSize = std::clamp(*overrides.fontSize, 8.0f, 72.0f);
        }
        if (overrides.fontWeight.has_value()) {
            style.fontWeight = normalizeFontWeight(*overrides.fontWeight);
        }
        if (overrides.italic.has_value()) {
            style.italic = *overrides.italic;
        }
        if (overrides.horizontalTextAlignment.has_value()) {
            style.horizontalTextAlignment = normalizedAlignment(*overrides.horizontalTextAlignment, false);
        }
        if (overrides.verticalTextAlignment.has_value()) {
            style.verticalTextAlignment = normalizedAlignment(*overrides.verticalTextAlignment, true);
        }
        if (overrides.textPadding.has_value()) {
            style.textPadding = std::clamp(*overrides.textPadding, 0.0f, 40.0f);
        }
    }

    return style;
}

ResolvedLookAndFeelStyle LookAndFeelRegistry::resolve(
    const ProjectDocument& document,
    const WidgetNode& widget,
    WidgetAppearanceState state,
    bool focusedOverlay) const
{
    ResolvedLookAndFeelStyle style = resolve(document, widget);
    if (!supportsWidgetState(widget.type, state)) {
        state = WidgetAppearanceState::Normal;
    }

    if (state != WidgetAppearanceState::Normal) {
        const auto found = widget.stateAppearanceOverrides.find(state);
        if (found != widget.stateAppearanceOverrides.end()) {
            applyStateOverrides(found->second, state, style);
        }
    }
    if (focusedOverlay && state != WidgetAppearanceState::Disabled
        && supportsWidgetState(widget.type, WidgetAppearanceState::Focused)) {
        const auto found = widget.stateAppearanceOverrides.find(WidgetAppearanceState::Focused);
        if (found != widget.stateAppearanceOverrides.end()) {
            applyStateOverrides(found->second, state, style);
        }
    }
    return style;
}

bool LookAndFeelRegistry::supportsWidgetOverrides(WidgetType type)
{
    return isSupportedWidgetType(type) || isTextBearingWidgetType(type);
}

bool LookAndFeelRegistry::supportsWidgetOverride(WidgetType type, std::string_view key)
{
    if (!isSupportedWidgetType(type) && !isTextBearingWidgetType(type)) {
        return false;
    }

    if (std::find(kWidgetTypographyOverrideKeys.begin(), kWidgetTypographyOverrideKeys.end(), key)
        != kWidgetTypographyOverrideKeys.end()) {
        if (!isTextBearingWidgetType(type)) {
            return false;
        }
        if (key == "verticalTextAlignment") {
            return type == WidgetType::Button
                || type == WidgetType::Label
                || type == WidgetType::TextBox
                || type == WidgetType::ComboBox
                || type == WidgetType::ListBox
                || type == WidgetType::StatusBar;
        }
        return true;
    }

    if (!isSupportedWidgetType(type)) {
        return false;
    }

    if (key == "textColor") {
        return type != WidgetType::Panel
            && type != WidgetType::ScrollBar;
    }
    if (key == "focusOutlineColor") {
        return type != WidgetType::Frame
            && type != WidgetType::GroupBox
            && type != WidgetType::Panel
            && type != WidgetType::ProgressBar;
    }
    if (key == "accentColor") {
        return type != WidgetType::Frame
            && type != WidgetType::GroupBox
            && type != WidgetType::Panel;
    }
    if (key == "controlPadding") {
        return type != WidgetType::Slider
            && type != WidgetType::ScrollBar
            && type != WidgetType::ProgressBar
            && type != WidgetType::Panel;
    }

    return std::find(kWidgetOverrideKeys.begin(), kWidgetOverrideKeys.end(), key) != kWidgetOverrideKeys.end();
}

std::vector<std::string_view> LookAndFeelRegistry::supportedWidgetOverrideKeys(WidgetType type)
{
    std::vector<std::string_view> keys;
    if (isSupportedWidgetType(type)) {
        for (const auto key : kWidgetOverrideKeys) {
            if (supportsWidgetOverride(type, key)) {
                keys.push_back(key);
            }
        }
    }
    for (const auto key : kWidgetTypographyOverrideKeys) {
        if (supportsWidgetOverride(type, key)) {
            keys.push_back(key);
        }
    }
    return keys;
}

bool LookAndFeelRegistry::supportsWidgetStateOverrides(WidgetType type)
{
    return isStateSupportedWidgetType(type);
}

bool LookAndFeelRegistry::supportsWidgetState(WidgetType type, WidgetAppearanceState state)
{
    if (!isStateSupportedWidgetType(type)) {
        return false;
    }
    if (state == WidgetAppearanceState::Normal) {
        return true;
    }
    if (type == WidgetType::ProgressBar) {
        return state == WidgetAppearanceState::Disabled;
    }
    if (state == WidgetAppearanceState::CheckedOrSelected) {
        return type == WidgetType::CheckBox
            || type == WidgetType::RadioButton
            || type == WidgetType::ListBox
            || type == WidgetType::TabControl;
    }
    if (state == WidgetAppearanceState::Pressed) {
        return type != WidgetType::TextBox
            && type != WidgetType::ListBox;
    }
    return true;
}

bool LookAndFeelRegistry::supportsWidgetStateOverride(
    WidgetType type,
    WidgetAppearanceState state,
    std::string_view key)
{
    if (state == WidgetAppearanceState::Normal) {
        return supportsWidgetOverride(type, key);
    }
    if (!supportsWidgetState(type, state)
        || std::find(kWidgetStateOverrideKeys.begin(), kWidgetStateOverrideKeys.end(), key)
            == kWidgetStateOverrideKeys.end()) {
        return false;
    }
    return supportsWidgetOverride(type, key);
}

std::vector<WidgetAppearanceState> LookAndFeelRegistry::supportedWidgetStates(WidgetType type)
{
    constexpr std::array<WidgetAppearanceState, 6> states = {
        WidgetAppearanceState::Normal,
        WidgetAppearanceState::Hover,
        WidgetAppearanceState::Pressed,
        WidgetAppearanceState::Focused,
        WidgetAppearanceState::CheckedOrSelected,
        WidgetAppearanceState::Disabled
    };
    std::vector<WidgetAppearanceState> supported;
    for (const auto state : states) {
        if (supportsWidgetState(type, state)) {
            supported.push_back(state);
        }
    }
    return supported;
}

std::vector<std::string_view> LookAndFeelRegistry::supportedWidgetStateOverrideKeys(
    WidgetType type,
    WidgetAppearanceState state)
{
    std::vector<std::string_view> keys;
    for (const auto key : kWidgetStateOverrideKeys) {
        if (supportsWidgetStateOverride(type, state, key)) {
            keys.push_back(key);
        }
    }
    return keys;
}

} // namespace visiform::model
