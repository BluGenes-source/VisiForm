#pragma once

#include "model/ProjectDocument.h"

#include <string>

namespace visiform::generator {

class VisageCppEmitter {
public:
    struct EmittedSources {
        std::string mainCpp;
        std::string mainWindowHeader;
        std::string mainWindowCpp;
    };

    [[nodiscard]] EmittedSources emitProjectSources(const model::ProjectDocument& document) const;
};

} // namespace visiform::generator
