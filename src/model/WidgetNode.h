#pragma once

#include "model/PropertyValue.h"

#include <map>
#include <string>

namespace visiform::model {

// Placeholder widget node stored in a form tree.
class WidgetNode {
public:
    WidgetNode(std::string id = {}, std::string typeName = {});

    [[nodiscard]] const std::string& id() const;
    [[nodiscard]] const std::string& typeName() const;
    [[nodiscard]] const std::map<std::string, PropertyValue>& properties() const;

    void setProperty(std::string name, PropertyValue value);

private:
    std::string id_{};
    std::string typeName_{};
    std::map<std::string, PropertyValue> properties_{};
};

} // namespace visiform::model
