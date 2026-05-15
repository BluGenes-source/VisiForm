#pragma once

#include <filesystem>
#include <string>

namespace visiform::utils {

class FileUtils {
public:
    [[nodiscard]] static std::string normalizeSeparators(std::string path);
    [[nodiscard]] static bool hasProjectExtension(const std::filesystem::path& path);
    [[nodiscard]] static bool readTextFile(const std::filesystem::path& path, std::string& output, std::string& errorMessage);
    [[nodiscard]] static bool writeTextFile(const std::filesystem::path& path, const std::string& text, std::string& errorMessage);
    [[nodiscard]] static bool ensureDirectoryExists(const std::filesystem::path& path, std::string& errorMessage);
};

} // namespace visiform::utils
