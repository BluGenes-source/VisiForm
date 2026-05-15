#pragma once

#include "model/ProjectDocument.h"

#include <string>

namespace visiform::serialization {

// Placeholder JSON loader for `.vfb.json` project files.
class JsonProjectReader {
public:
    [[nodiscard]] model::ProjectDocument readFromString(const std::string& content) const;
};

} // namespace visiform::serialization
