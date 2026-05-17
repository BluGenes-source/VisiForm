#pragma once

#include "model/ProjectDocument.h"

#include <string>

namespace visiform::generator {

class VisageCppEmitter {
public:
    struct EmittedSources {
        std::string mainCpp;
        std::string generatedBaseHeaderFilename;
        std::string generatedBaseHeader;
        std::string generatedBaseCppFilename;
        std::string generatedBaseCpp;
        std::string userSubclassHeaderFilename;
        std::string userSubclassHeader;
        std::string userSubclassCppFilename;
        std::string userSubclassCpp;
    };

    [[nodiscard]] bool emitProjectSources(
        const model::ProjectDocument& document,
        const std::string& existingMainWindowCpp,
        EmittedSources& output,
        std::string& errorMessage) const;
};

} // namespace visiform::generator
