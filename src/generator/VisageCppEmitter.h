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

    [[nodiscard]] bool emitProjectSources(
        const model::ProjectDocument& document,
        const std::string& existingMainWindowCpp,
        EmittedSources& output,
        std::string& errorMessage) const;
};

} // namespace visiform::generator
