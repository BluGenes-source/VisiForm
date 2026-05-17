#pragma once

#include "model/WidgetDefinition.h"

#include <string>
#include <vector>

namespace visiform::model {

class WidgetRegistry {
public:
    [[nodiscard]] static const WidgetRegistry& instance();

    [[nodiscard]] const WidgetDefinition* find(WidgetType type) const;
    [[nodiscard]] const WidgetDefinition* findByTypeName(const std::string& typeName) const;
    [[nodiscard]] const std::vector<WidgetDefinition>& definitions() const;
    [[nodiscard]] WidgetNode createDefaultWidget(WidgetType type, const std::string& id) const;

private:
    WidgetRegistry();

    std::vector<WidgetDefinition> definitions_{};
};

} // namespace visiform::model
