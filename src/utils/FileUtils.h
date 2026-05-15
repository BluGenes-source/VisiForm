#pragma once

#include <filesystem>
#include <string>

namespace visiform::utils {

// Placeholder file helpers shared across the project.
class FileUtils {
public:
    [[nodiscard]] static std::string normalizeSeparators(std::string path);
    [[nodiscard]] static bool hasProjectExtension(const std::filesystem::path& path);
};

} // namespace visiform::utils
