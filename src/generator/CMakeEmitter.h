#pragma once

#include "model/ProjectDocument.h"
#include "utils/AppSettings.h"

#include <string>

namespace visiform::generator {

class CMakeEmitter {
public:
    [[nodiscard]] std::string emitCMakeLists(const model::ProjectDocument& document, const utils::AppSettings& settings) const;
    [[nodiscard]] std::string emitCMakePresets(const utils::AppSettings& settings) const;
    [[nodiscard]] std::string emitReadme(const model::ProjectDocument& document, const utils::AppSettings& settings) const;
    [[nodiscard]] std::string emitGitIgnore() const;
    [[nodiscard]] std::string emitConfigureScript(bool release) const;
    [[nodiscard]] std::string emitBuildScript(bool release) const;
};

} // namespace visiform::generator
