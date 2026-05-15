#pragma once

#include "generator/CMakeEmitter.h"
#include "generator/VisageCppEmitter.h"
#include "model/ProjectDocument.h"

#include <filesystem>
#include <string>

namespace visiform::generator {

class CodeGenerator {
public:
    [[nodiscard]] bool generateProject(
        const model::ProjectDocument& document,
        const std::filesystem::path& outputDirectory,
        std::string& errorMessage) const;

private:
    VisageCppEmitter visageCppEmitter_{};
    CMakeEmitter cmakeEmitter_{};
};

} // namespace visiform::generator
