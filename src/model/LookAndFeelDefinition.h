#pragma once

#include <optional>
#include <string>

namespace visiform::model {

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
            && !splitterShadowThickness.has_value();
    }

    bool operator==(const LookAndFeelOverrides&) const = default;
};

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
    float fontSize = 16.0f;
    float controlPadding = 8.0f;
    float splitterHighlightThickness = 1.0f;
    float splitterShadowThickness = 1.0f;
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
    float fontSize = 16.0f;
    float controlPadding = 8.0f;
    float splitterHighlightThickness = 1.0f;
    float splitterShadowThickness = 1.0f;
};

} // namespace visiform::model
