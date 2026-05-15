#pragma once

#include "model/ProjectDocument.h"

#include <string>

namespace visiform::generator {

// Placeholder emitter for future Visage C++ output.
class VisageCppEmitter {
public:
    [[nodiscard]] std::string emit(const model::ProjectDocument& document) const;
};

} // namespace visiform::generator
