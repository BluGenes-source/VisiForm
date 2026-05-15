#pragma once

#include "generator/CMakeEmitter.h"
#include "generator/VisageCppEmitter.h"
#include "model/ProjectDocument.h"

#include <string>

namespace visiform::generator {

// Placeholder coordinator for future project code generation.
class CodeGenerator {
public:
    [[nodiscard]] std::string generateSummary(const model::ProjectDocument& document) const;

private:
    VisageCppEmitter visageCppEmitter_{};
    CMakeEmitter cmakeEmitter_{};
};

} // namespace visiform::generator
