#pragma once

#include <array>
#include <optional>
#include <map>
#include <string>
#include <string_view>

namespace visiform::model {

struct FontWeightChoice {
    int value = 400;
    std::string_view label = "Regular";
};

[[nodiscard]] inline constexpr std::array<FontWeightChoice, 2> supportedFontWeightChoices()
{
    return { {
        { 400, "Regular" },
        { 700, "Bold" }
    } };
}

[[nodiscard]] inline constexpr int normalizeFontWeight(int value)
{
    for (const auto choice : supportedFontWeightChoices()) {
        if (choice.value == value) {
            return value;
        }
    }
    return 400;
}

[[nodiscard]] inline constexpr std::string_view fontWeightLabel(int value)
{
    const int normalized = normalizeFontWeight(value);
    for (const auto choice : supportedFontWeightChoices()) {
        if (choice.value == normalized) {
            return choice.label;
        }
    }
    return "Regular";
}

struct LookAndFeelOverrides {
    std::optional<std::string> applicationSurfaceColor{};
    std::optional<std::string> controlSurfaceColor{};
    std::optional<std::string> recessedSurfaceColor{};
    std::optional<std::string> primaryTextColor{};
    std::optional<std::string> disabledTextColor{};
    std::optional<std::string> borderColor{};
    std::optional<std::string> focusOutlineColor{};
    std::optional<std::string> accentColor{};
    std::optional<std::string> highlightEdgeColor{};
    std::optional<std::string> shadowEdgeColor{};
    std::optional<float> borderThickness{};
    std::optional<float> cornerRadius{};
    std::optional<float> controlPadding{};
    std::optional<float> splitterHighlightThickness{};
    std::optional<float> splitterShadowThickness{};
    std::optional<std::string> fontFamily{};
    std::optional<float> fontSize{};
    std::optional<int> fontWeight{};
    std::optional<bool> italic{};
    std::optional<float> textPadding{};
    std::optional<std::string> disabledTextTreatment{};

    [[nodiscard]] bool empty() const
    {
        return !applicationSurfaceColor.has_value()
            && !controlSurfaceColor.has_value()
            && !recessedSurfaceColor.has_value()
            && !primaryTextColor.has_value()
            && !disabledTextColor.has_value()
            && !borderColor.has_value()
            && !focusOutlineColor.has_value()
            && !accentColor.has_value()
            && !highlightEdgeColor.has_value()
            && !shadowEdgeColor.has_value()
            && !borderThickness.has_value()
            && !cornerRadius.has_value()
            && !controlPadding.has_value()
            && !splitterHighlightThickness.has_value()
            && !splitterShadowThickness.has_value()
            && !fontFamily.has_value()
            && !fontSize.has_value()
            && !fontWeight.has_value()
            && !italic.has_value()
            && !textPadding.has_value()
            && !disabledTextTreatment.has_value();
    }

    bool operator==(const LookAndFeelOverrides&) const = default;
};

struct WidgetLookAndFeelOverrides {
    std::optional<std::string> controlSurfaceColor{};
    std::optional<std::string> textColor{};
    std::optional<std::string> borderColor{};
    std::optional<std::string> accentColor{};
    std::optional<std::string> focusOutlineColor{};
    std::optional<std::string> highlightEdgeColor{};
    std::optional<std::string> shadowEdgeColor{};
    std::optional<float> borderThickness{};
    std::optional<float> cornerRadius{};
    std::optional<float> controlPadding{};
    std::optional<std::string> fontFamily{};
    std::optional<float> fontSize{};
    std::optional<int> fontWeight{};
    std::optional<bool> italic{};
    std::optional<std::string> horizontalTextAlignment{};
    std::optional<std::string> verticalTextAlignment{};
    std::optional<float> textPadding{};

    [[nodiscard]] bool empty() const
    {
        return !controlSurfaceColor.has_value()
            && !textColor.has_value()
            && !borderColor.has_value()
            && !accentColor.has_value()
            && !focusOutlineColor.has_value()
            && !highlightEdgeColor.has_value()
            && !shadowEdgeColor.has_value()
            && !borderThickness.has_value()
            && !cornerRadius.has_value()
            && !controlPadding.has_value()
            && !fontFamily.has_value()
            && !fontSize.has_value()
            && !fontWeight.has_value()
            && !italic.has_value()
            && !horizontalTextAlignment.has_value()
            && !verticalTextAlignment.has_value()
            && !textPadding.has_value();
    }

    bool operator==(const WidgetLookAndFeelOverrides&) const = default;
};

enum class WidgetAppearanceState {
    Normal,
    Hover,
    Pressed,
    Focused,
    CheckedOrSelected,
    Disabled
};

[[nodiscard]] std::string_view toString(WidgetAppearanceState state);
[[nodiscard]] std::optional<WidgetAppearanceState> widgetAppearanceStateFromString(std::string_view value);

struct WidgetStateLookAndFeelOverrides {
    std::optional<std::string> controlSurfaceColor{};
    std::optional<std::string> textColor{};
    std::optional<std::string> borderColor{};
    std::optional<std::string> accentColor{};
    std::optional<std::string> focusOutlineColor{};
    std::optional<std::string> highlightEdgeColor{};
    std::optional<std::string> shadowEdgeColor{};

    [[nodiscard]] bool empty() const
    {
        return !controlSurfaceColor.has_value()
            && !textColor.has_value()
            && !borderColor.has_value()
            && !accentColor.has_value()
            && !focusOutlineColor.has_value()
            && !highlightEdgeColor.has_value()
            && !shadowEdgeColor.has_value();
    }

    bool operator==(const WidgetStateLookAndFeelOverrides&) const = default;
};

using WidgetStateLookAndFeelOverrideMap =
    std::map<WidgetAppearanceState, WidgetStateLookAndFeelOverrides>;

struct LookAndFeelDefinition {
    std::string id;
    std::string displayName;

    std::string panelColor;
    std::string controlFillColor;
    std::string controlTextColor;
    std::string controlBorderColor;
    std::string accentColor;
    std::string disabledColor;
    std::string recessedSurfaceColor;
    std::string raisedSurfaceColor;
    std::string secondaryTextColor;
    std::string disabledTextColor;
    std::string focusOutlineColor;
    std::string selectedStateColor;
    std::string hoverStateColor;
    std::string pressedStateColor;
    std::string checkedStateColor;
    std::string highlightEdgeColor;
    std::string shadowEdgeColor;

    float borderThickness = 1.0f;
    float cornerRadius = 0.0f;
    std::string fontFamily = "Default";
    float fontSize = 16.0f;
    int fontWeight = 400;
    bool italic = false;
    float controlPadding = 8.0f;
    float textPadding = 8.0f;
    std::string disabledTextTreatment = "Muted";
    float splitterHighlightThickness = 1.0f;
    float splitterShadowThickness = 1.0f;

    bool operator==(const LookAndFeelDefinition&) const = default;
};

struct ResolvedLookAndFeelStyle {
    std::string id;
    std::string applicationSurfaceColor;
    std::string controlSurfaceColor;
    std::string recessedSurfaceColor;
    std::string raisedSurfaceColor;
    std::string primaryTextColor;
    std::string secondaryTextColor;
    std::string disabledTextColor;
    std::string disabledSurfaceColor;
    std::string borderColor;
    std::string focusOutlineColor;
    std::string accentColor;
    std::string selectedStateColor;
    std::string hoverStateColor;
    std::string pressedStateColor;
    std::string checkedStateColor;
    std::string highlightEdgeColor;
    std::string shadowEdgeColor;
    float borderThickness = 1.0f;
    float cornerRadius = 0.0f;
    std::string fontFamily = "Default";
    float fontSize = 16.0f;
    int fontWeight = 400;
    bool italic = false;
    float controlPadding = 8.0f;
    float textPadding = 8.0f;
    std::string disabledTextTreatment = "Muted";
    std::string horizontalTextAlignment = "Default";
    std::string verticalTextAlignment = "Default";
    float splitterHighlightThickness = 1.0f;
    float splitterShadowThickness = 1.0f;
};

} // namespace visiform::model
