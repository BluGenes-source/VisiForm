#pragma once

#pragma once

#include "model/ProjectDocument.h"

#include <string>

namespace visiform::generator {

class CMakeEmitter {
public:
    [[nodiscard]] std::string emitCMakeLists(const model::ProjectDocument& document) const;
};

} // namespace visiform::generator
