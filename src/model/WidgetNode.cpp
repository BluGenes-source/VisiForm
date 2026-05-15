#include "model/WidgetNode.h"

#include "model/WidgetNode.h"

#include <utility>

namespace visiform::model {
namespace {

const std::string& widgetTypeName(WidgetType type)
{
    static const std::string formWindow = "FormWindow";
    static const std::string frame = "Frame";
    static const std::string label = "Label";
    static const std::string button = "Button";
    static const std::string textBox = "TextBox";
    static const std::string checkBox = "CheckBox";
    static const std::string slider = "Slider";
    static const std::string image = "Image";
    static const std::string spacer = "Spacer";

    switch (type) {
    case WidgetType::FormWindow:
        return formWindow;
    case WidgetType::Frame:
        return frame;
    case WidgetType::Label:
        return label;
    case WidgetType::Button:
        return button;
    case WidgetType::TextBox:
        return textBox;
    case WidgetType::CheckBox:
        return checkBox;
    case WidgetType::Slider:
        return slider;
    case WidgetType::Image:
        return image;
    case WidgetType::Spacer:
        return spacer;
    }

    return frame;
}

} // namespace

std::string toString(WidgetType type)
{
    return widgetTypeName(type);
}

std::optional<WidgetType> widgetTypeFromString(const std::string& value)
{
    if (value == "FormWindow") {
        return WidgetType::FormWindow;
    }
    if (value == "Frame") {
        return WidgetType::Frame;
    }
    if (value == "Label") {
        return WidgetType::Label;
    }
    if (value == "Button") {
        return WidgetType::Button;
    }
    if (value == "TextBox") {
        return WidgetType::TextBox;
    }
    if (value == "CheckBox") {
        return WidgetType::CheckBox;
    }
    if (value == "Slider") {
        return WidgetType::Slider;
    }
    if (value == "Image") {
        return WidgetType::Image;
    }
    if (value == "Spacer") {
        return WidgetType::Spacer;
    }

    return std::nullopt;
}

bool Rect::contains(float px, float py) const
{
    return px >= x && py >= y && px <= x + width && py <= y + height;
}

bool Rect::isValid() const
{
    return width > 0.0f && height > 0.0f;
}

WidgetNode::WidgetNode(std::string idValue, std::string nameValue, WidgetType widgetType, Rect widgetBounds)
    : id(std::move(idValue))
    , name(std::move(nameValue))
    , type(widgetType)
    , bounds(widgetBounds)
{
}

const std::string& WidgetNode::typeName() const
{
    return widgetTypeName(type);
}

PropertyValue* WidgetNode::getProperty(const std::string& key)
{
    const auto iterator = properties.find(key);
    if (iterator == properties.end()) {
        return nullptr;
    }

    return &iterator->second;
}

const PropertyValue* WidgetNode::getProperty(const std::string& key) const
{
    const auto iterator = properties.find(key);
    if (iterator == properties.end()) {
        return nullptr;
    }

    return &iterator->second;
}

void WidgetNode::setProperty(const std::string& key, PropertyValue value)
{
    properties.insert_or_assign(key, std::move(value));
}

std::string WidgetNode::getStringProperty(const std::string& key, const std::string& defaultValue) const
{
    if (const auto* property = getProperty(key)) {
        return property->asString(defaultValue);
    }

    return defaultValue;
}

float WidgetNode::getFloatProperty(const std::string& key, float defaultValue) const
{
    if (const auto* property = getProperty(key)) {
        return property->asFloat(defaultValue);
    }

    return defaultValue;
}

int WidgetNode::getIntProperty(const std::string& key, int defaultValue) const
{
    if (const auto* property = getProperty(key)) {
        return property->asInt(defaultValue);
    }

    return defaultValue;
}

bool WidgetNode::getBoolProperty(const std::string& key, bool defaultValue) const
{
    if (const auto* property = getProperty(key)) {
        return property->asBool(defaultValue);
    }

    return defaultValue;
}

WidgetNode* WidgetNode::findById(const std::string& searchId)
{
    return const_cast<WidgetNode*>(std::as_const(*this).findById(searchId));
}

const WidgetNode* WidgetNode::findById(const std::string& searchId) const
{
    if (id == searchId) {
        return this;
    }

    for (const auto& child : children) {
        if (const auto* match = child.findById(searchId)) {
            return match;
        }
    }

    return nullptr;
}

WidgetNode* WidgetNode::hitTest(float x, float y)
{
    return const_cast<WidgetNode*>(std::as_const(*this).hitTest(x, y));
}

const WidgetNode* WidgetNode::hitTest(float x, float y) const
{
    for (auto iterator = children.rbegin(); iterator != children.rend(); ++iterator) {
        if (const auto* match = iterator->hitTest(x, y)) {
            return match;
        }
    }

    if (bounds.contains(x, y)) {
        return this;
    }

    return nullptr;
}

} // namespace visiform::model
