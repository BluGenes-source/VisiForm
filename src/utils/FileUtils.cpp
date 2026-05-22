#include "utils/FileUtils.h"

#include "utils/FileUtils.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace visiform::utils {

std::string FileUtils::normalizeSeparators(std::string path)
{
    for (auto& character : path) {
        if (character == '\\') {
            character = '/';
        }
    }

    return path;
}

std::string FileUtils::sanitizeFileName(std::string value)
{
    for (auto& character : value) {
        const unsigned char ch = static_cast<unsigned char>(character);
        if (std::isalnum(ch) != 0 || character == '_' || character == '-' || character == '.') {
            continue;
        }

        character = '_';
    }

    while (!value.empty() && (value.front() == '.' || value.front() == ' ')) {
        value.erase(value.begin());
    }
    while (!value.empty() && (value.back() == '.' || value.back() == ' ')) {
        value.pop_back();
    }

    return value.empty() ? std::string{"resource"} : value;
}

std::string FileUtils::sanitizeRelativeAssetPath(const std::string& path)
{
    const std::filesystem::path normalized = std::filesystem::path{ normalizeSeparators(path) }.lexically_normal();
    std::filesystem::path result;
    for (const auto& part : normalized) {
        const std::string partText = part.generic_string();
        if (partText.empty() || partText == ".") {
            continue;
        }
        if (partText == "..") {
            continue;
        }

        result /= sanitizeFileName(partText);
    }

    return normalizeSeparators(result.generic_string());
}

std::string FileUtils::fileStem(const std::filesystem::path& path)
{
    const std::string stem = path.stem().string();
    return stem.empty() ? std::string{"resource"} : stem;
}

bool FileUtils::hasProjectExtension(const std::filesystem::path& path)
{
    return path.extension() == std::filesystem::path{L".json"}
        && path.stem().extension() == std::filesystem::path{L".vfb"};
}

bool FileUtils::isRelativePathWithinDirectory(const std::filesystem::path& path, const std::string& topLevelDirectory)
{
    if (path.empty() || path.is_absolute()) {
        return false;
    }

    const std::filesystem::path normalized = std::filesystem::path{ normalizeSeparators(path.generic_string()) }.lexically_normal();
    const auto iterator = normalized.begin();
    if (iterator == normalized.end()) {
        return false;
    }

    const std::string firstPart = iterator->generic_string();
    if (firstPart != topLevelDirectory) {
        return false;
    }

    for (const auto& part : normalized) {
        const std::string partText = part.generic_string();
        if (partText == "..") {
            return false;
        }
    }

    return true;
}

bool FileUtils::readTextFile(const std::filesystem::path& path, std::string& output, std::string& errorMessage)
{
    output.clear();
    errorMessage.clear();

    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        errorMessage = "Unable to open file for reading: " + normalizeSeparators(path.string());
        return false;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    if (!input.good() && !input.eof()) {
        errorMessage = "Failed while reading file: " + normalizeSeparators(path.string());
        return false;
    }

    output = buffer.str();
    return true;
}

bool FileUtils::copyFile(const std::filesystem::path& source, const std::filesystem::path& destination, std::string& errorMessage)
{
    errorMessage.clear();

    if (source.empty()) {
        errorMessage = "Source file path is empty.";
        return false;
    }
    if (destination.empty()) {
        errorMessage = "Destination file path is empty.";
        return false;
    }

    const auto parentPath = destination.parent_path();
    if (!parentPath.empty() && !ensureDirectoryExists(parentPath, errorMessage)) {
        return false;
    }

    std::error_code errorCode;
    std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing, errorCode);
    if (errorCode) {
        errorMessage = "Failed to copy file from " + normalizeSeparators(source.string())
            + " to " + normalizeSeparators(destination.string());
        return false;
    }

    return true;
}

bool FileUtils::writeTextFile(const std::filesystem::path& path, const std::string& text, std::string& errorMessage)
{
    errorMessage.clear();

    const auto parentPath = path.parent_path();
    if (!parentPath.empty() && !ensureDirectoryExists(parentPath, errorMessage)) {
        return false;
    }

    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        errorMessage = "Unable to open file for writing: " + normalizeSeparators(path.string());
        return false;
    }

    output << text;
    if (!output.good()) {
        errorMessage = "Failed while writing file: " + normalizeSeparators(path.string());
        return false;
    }

    return true;
}

bool FileUtils::ensureDirectoryExists(const std::filesystem::path& path, std::string& errorMessage)
{
    errorMessage.clear();

    if (path.empty()) {
        return true;
    }

    std::error_code errorCode;
    if (std::filesystem::exists(path, errorCode)) {
        if (errorCode) {
            errorMessage = "Failed to inspect directory: " + normalizeSeparators(path.string());
            return false;
        }

        if (!std::filesystem::is_directory(path, errorCode)) {
            errorMessage = "Path is not a directory: " + normalizeSeparators(path.string());
            return false;
        }

        if (errorCode) {
            errorMessage = "Failed to inspect directory type: " + normalizeSeparators(path.string());
            return false;
        }

        return true;
    }

    std::filesystem::create_directories(path, errorCode);
    if (errorCode) {
        errorMessage = "Failed to create directory: " + normalizeSeparators(path.string());
        return false;
    }

    return true;
}

} // namespace visiform::utils
