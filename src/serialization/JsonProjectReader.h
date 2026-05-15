#pragma once

#pragma once

#include "model/ProjectDocument.h"

#include <filesystem>
#include <optional>
#include <string>

namespace visiform::serialization {

class JsonProjectReader {
public:
    [[nodiscard]] std::optional<model::ProjectDocument> readFromString(const std::string& jsonText, std::string& errorMessage) const;
    [[nodiscard]] std::optional<model::ProjectDocument> readFromFile(const std::filesystem::path& path, std::string& errorMessage) const;
};

} // namespace visiform::serialization
