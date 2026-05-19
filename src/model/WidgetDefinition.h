#pragma once

#include "model/PropertyValue.h"
#include "model/WidgetNode.h"

#include <string>
#include <vector>

namespace visiform::model {

enum class PropertyEditKind {
    Text,
    Integer,
    Float,
    Bool,
    Color,
    FilePath,
    ReadOnly
};

struct WidgetPropertyDefinition {
    std::string key;
    std::string label;
    PropertyValue defaultValue{};
    PropertyEditKind editKind = PropertyEditKind::Text;
    bool editable = true;
    std::string hint{};
    std::vector<std::string> choices{};
};

struct WidgetEventDefinition {
    std::string key;
    std::string label;
    std::string handlerSignatureKind;
    std::string hint{};
};

struct WidgetSizeDefinition {
    float defaultWidth = 20.0f;
    float defaultHeight = 20.0f;
    float minWidth = 20.0f;
    float minHeight = 20.0f;
};

struct WidgetDefinition {
    WidgetType type = WidgetType::Frame;
    std::string typeName{};
    std::string displayName{};
    std::string paletteGroup{};
    std::string defaultNamePrefix{};
    std::string defaultHint{};
    WidgetSizeDefinition size{};
    std::vector<WidgetPropertyDefinition> properties{};
    std::vector<WidgetEventDefinition> events{};
};

} // namespace visiform::model
