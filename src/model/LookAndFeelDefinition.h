#pragma once

#include <string>

namespace visiform::model {

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
