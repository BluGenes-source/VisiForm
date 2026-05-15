#pragma once

#pragma once

#include "model/WidgetNode.h"

#include <string>

namespace visiform::model {

class FormNode {
public:
    FormNode() = default;
    explicit FormNode(std::string className, WidgetNode rootWidget = {});

    std::string className{};
    WidgetNode rootWidget{};

    [[nodiscard]] const std::string& name() const;
    [[nodiscard]] WidgetNode* findWidgetById(const std::string& id);
    [[nodiscard]] const WidgetNode* findWidgetById(const std::string& id) const;
};

} // namespace visiform::model
