#include "model/FormNode.h"

namespace visiform::model {

FormNode::FormNode(std::string name)
    : name_(std::move(name))
{
}

void FormNode::addWidget(WidgetNode widget)
{
    widgets_.push_back(std::move(widget));
}

const std::string& FormNode::name() const
{
    return name_;
}

const std::vector<WidgetNode>& FormNode::widgets() const
{
    return widgets_;
}

} // namespace visiform::model
