#pragma once

#pragma once

#include "model/WidgetNode.h"

#include <cstdint>
#include <map>
#include <string>

namespace visiform::model {
class ProjectDocument;
}

namespace visiform::utils {

class IdGenerator {
public:
    [[nodiscard]] std::string next(model::WidgetType widgetType, const model::ProjectDocument& document);
    [[nodiscard]] static std::string prefixForType(model::WidgetType widgetType);

private:
    std::map<model::WidgetType, std::uint64_t> nextValues_{};
};

} // namespace visiform::utils
