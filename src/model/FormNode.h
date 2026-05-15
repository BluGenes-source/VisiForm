#pragma once

#include "model/WidgetNode.h"

#include <string>
#include <vector>

namespace visiform::model {

// Placeholder form node that owns a flat list of widgets.
class FormNode {
public:
    explicit FormNode(std::string name = {});

    void addWidget(WidgetNode widget);

    [[nodiscard]] const std::string& name() const;
    [[nodiscard]] const std::vector<WidgetNode>& widgets() const;

private:
    std::string name_{};
    std::vector<WidgetNode> widgets_{};
};

} // namespace visiform::model
