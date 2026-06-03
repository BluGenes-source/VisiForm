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
    Slider,
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
    float minimumValue = 0.0f;
    float maximumValue = 0.0f;
    float stepValue = 1.0f;
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
    bool paletteVisible = true;
    int paletteOrder = 0;
    std::string defaultNamePrefix{};
    std::string defaultHint{};
    WidgetSizeDefinition size{};
    bool canContainChildren = false;
    bool allowsDrop = false;
    bool clipsChildren = false;
    bool drawsChildrenInside = false;
    LayoutMode defaultChildLayoutMode = LayoutMode::Absolute;
    std::vector<WidgetPropertyDefinition> properties{};
    std::vector<WidgetEventDefinition> events{};
};

} // namespace visiform::model
