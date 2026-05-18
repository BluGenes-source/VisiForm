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

    float borderThickness = 1.0f;
    float cornerRadius = 0.0f;
    float fontSize = 16.0f;
};

} // namespace visiform::model
