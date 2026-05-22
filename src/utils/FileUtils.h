#pragma once

#include <filesystem>
#include <string>

namespace visiform::utils {

class FileUtils {
public:
    [[nodiscard]] static std::string normalizeSeparators(std::string path);
    [[nodiscard]] static std::string sanitizeFileName(std::string value);
    [[nodiscard]] static std::string sanitizeRelativeAssetPath(const std::string& path);
    [[nodiscard]] static std::string fileStem(const std::filesystem::path& path);
    [[nodiscard]] static bool hasProjectExtension(const std::filesystem::path& path);
    [[nodiscard]] static bool isRelativePathWithinDirectory(const std::filesystem::path& path, const std::string& topLevelDirectory);
    [[nodiscard]] static bool readTextFile(const std::filesystem::path& path, std::string& output, std::string& errorMessage);
    [[nodiscard]] static bool writeTextFile(const std::filesystem::path& path, const std::string& text, std::string& errorMessage);
    [[nodiscard]] static bool copyFile(const std::filesystem::path& source, const std::filesystem::path& destination, std::string& errorMessage);
    [[nodiscard]] static bool ensureDirectoryExists(const std::filesystem::path& path, std::string& errorMessage);
};

} // namespace visiform::utils
