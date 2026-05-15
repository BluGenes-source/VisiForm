#include "model/FormNode.h"

#include "model/FormNode.h"

#include <utility>

namespace visiform::model {

FormNode::FormNode(std::string classNameValue, WidgetNode rootWidgetValue)
    : className(std::move(classNameValue))
    , rootWidget(std::move(rootWidgetValue))
{
}

const std::string& FormNode::name() const
{
    return className;
}

WidgetNode* FormNode::findWidgetById(const std::string& id)
{
    return rootWidget.findById(id);
}

const WidgetNode* FormNode::findWidgetById(const std::string& id) const
{
    return rootWidget.findById(id);
}

} // namespace visiform::model
