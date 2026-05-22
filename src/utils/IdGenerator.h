#pragma once

#pragma once

#include "model/ProjectResource.h"
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
    [[nodiscard]] std::string next(model::ProjectResourceType resourceType, const model::ProjectDocument& document);
    [[nodiscard]] static std::string prefixForType(model::WidgetType widgetType);
    [[nodiscard]] static std::string prefixForType(model::ProjectResourceType resourceType);

private:
    std::map<model::WidgetType, std::uint64_t> nextValues_{};
    std::map<model::ProjectResourceType, std::uint64_t> nextResourceValues_{};
};

} // namespace visiform::utils
