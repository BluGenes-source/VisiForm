#pragma once

#pragma once

#include "model/PropertyValue.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace visiform::model {

enum class WidgetType {
    FormWindow,
    Frame,
    Label,
    Button,
    TextBox,
    CheckBox,
    Slider,
    Image,
    Spacer
};

[[nodiscard]] std::string toString(WidgetType type);
[[nodiscard]] std::optional<WidgetType> widgetTypeFromString(const std::string& value);

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 100.0f;
    float height = 40.0f;

    [[nodiscard]] bool contains(float px, float py) const;
    [[nodiscard]] bool isValid() const;
};

class WidgetNode {
public:
    WidgetNode() = default;
    WidgetNode(std::string id, std::string name, WidgetType type, Rect bounds = {});

    std::string id{};
    std::string name{};
    WidgetType type = WidgetType::Frame;
    Rect bounds{};
    std::map<std::string, PropertyValue> properties{};
    std::vector<WidgetNode> children{};

    [[nodiscard]] const std::string& typeName() const;

    [[nodiscard]] PropertyValue* getProperty(const std::string& key);
    [[nodiscard]] const PropertyValue* getProperty(const std::string& key) const;
    void setProperty(const std::string& key, PropertyValue value);

    [[nodiscard]] std::string getStringProperty(const std::string& key, const std::string& defaultValue) const;
    [[nodiscard]] float getFloatProperty(const std::string& key, float defaultValue) const;
    [[nodiscard]] int getIntProperty(const std::string& key, int defaultValue) const;
    [[nodiscard]] bool getBoolProperty(const std::string& key, bool defaultValue) const;

    [[nodiscard]] WidgetNode* findById(const std::string& searchId);
    [[nodiscard]] const WidgetNode* findById(const std::string& searchId) const;

    [[nodiscard]] WidgetNode* hitTest(float x, float y);
    [[nodiscard]] const WidgetNode* hitTest(float x, float y) const;

private:
    [[nodiscard]] const WidgetNode* hitTestImpl(float x, float y) const;
};

} // namespace visiform::model
