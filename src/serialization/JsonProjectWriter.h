#pragma once

#include "model/ProjectDocument.h"

#include <string>

namespace visiform::serialization {

// Placeholder JSON writer for `.vfb.json` project files.
class JsonProjectWriter {
public:
    [[nodiscard]] std::string writeToString(const model::ProjectDocument& document) const;
};

} // namespace visiform::serialization
