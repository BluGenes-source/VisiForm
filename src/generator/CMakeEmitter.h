#pragma once

#include "model/ProjectDocument.h"

#include <string>

namespace visiform::generator {

// Placeholder emitter for generated CMake project files.
class CMakeEmitter {
public:
    [[nodiscard]] std::string emit(const model::ProjectDocument& document) const;
};

} // namespace visiform::generator
