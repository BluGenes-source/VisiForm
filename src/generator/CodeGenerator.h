#pragma once

#include "generator/CMakeEmitter.h"
#include "generator/VisageCppEmitter.h"
#include "model/ProjectDocument.h"

#include <filesystem>
#include <string>
#include <functional>

namespace visiform::generator {

class CodeGenerator {
public:
    using ProgressCallback = std::function<void(int, const std::string&)>;
    [[nodiscard]] bool generateProject(
        const model::ProjectDocument& document,
        const std::filesystem::path& outputDirectory,
        std::string& errorMessage,
        ProgressCallback progressCallback = {}) const;

private:
    VisageCppEmitter visageCppEmitter_{};
    CMakeEmitter cmakeEmitter_{};
};

} // namespace visiform::generator
