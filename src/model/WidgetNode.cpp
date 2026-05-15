#include "model/WidgetNode.h"

namespace visiform::model {

WidgetNode::WidgetNode(std::string id, std::string typeName)
    : id_(std::move(id))
    , typeName_(std::move(typeName))
{
}

const std::string& WidgetNode::id() const
{
    return id_;
}

const std::string& WidgetNode::typeName() const
{
    return typeName_;
}

const std::map<std::string, PropertyValue>& WidgetNode::properties() const
{
    return properties_;
}

void WidgetNode::setProperty(std::string name, PropertyValue value)
{
    properties_.insert_or_assign(std::move(name), std::move(value));
}

} // namespace visiform::model
