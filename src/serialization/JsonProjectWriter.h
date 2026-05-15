#pragma once

#pragma once

#include "model/ProjectDocument.h"

#include <filesystem>
#include <string>

namespace visiform::serialization {

class JsonProjectWriter {
public:
    [[nodiscard]] std::string writeToString(const model::ProjectDocument& document) const;
    [[nodiscard]] bool writeToFile(const model::ProjectDocument& document, const std::filesystem::path& path, std::string& errorMessage) const;
};

} // namespace visiform::serialization
