#include "utils/FileUtils.h"

#include "utils/FileUtils.h"

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

bool FileUtils::hasProjectExtension(const std::filesystem::path& path)
{
    return path.extension() == std::filesystem::path{L".json"}
        && path.stem().extension() == std::filesystem::path{L".vfb"};
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
