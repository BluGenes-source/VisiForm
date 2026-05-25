#include "utils/RecentFiles.h"

#include "utils/FileUtils.h"

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <nlohmann/json.hpp>

namespace visiform::utils {
namespace {

constexpr std::size_t kMaxRecentFiles = 10;

std::filesystem::path normalizePath(const std::filesystem::path& path)
{
    return path.lexically_normal();
}

std::optional<std::filesystem::path> environmentVariablePath(const char* variableName)
{
    const char* value = std::getenv(variableName);
    if (value == nullptr || *value == '\0') {
        return std::nullopt;
    }

    return normalizePath(std::filesystem::path{ value });
}

} // namespace

const std::vector<std::filesystem::path>& RecentFiles::paths() const
{
    return paths_;
}

bool RecentFiles::load(std::string& errorMessage)
{
    errorMessage.clear();
    paths_.clear();

    const std::filesystem::path path = storagePath();
    if (path.empty()) {
        return true;
    }
    if (!std::filesystem::exists(path)) {
        return true;
    }

    std::string text;
    if (!FileUtils::readTextFile(path, text, errorMessage)) {
        return false;
    }

    try {
        const auto json = nlohmann::json::parse(text);
        if (!json.is_array()) {
            errorMessage = "Recent files config is not a JSON array";
            return false;
        }

        for (const auto& entry : json) {
            if (!entry.is_string()) {
                continue;
            }

            paths_.push_back(normalizePath(entry.get<std::string>()));
            if (paths_.size() >= kMaxRecentFiles) {
                break;
            }
        }
    }
    catch (const std::exception& exception) {
        errorMessage = exception.what();
        return false;
    }

    return true;
}

bool RecentFiles::addPath(const std::filesystem::path& path, std::string& errorMessage)
{
    errorMessage.clear();
    if (path.empty()) {
        return true;
    }

    const std::filesystem::path normalized = normalizePath(path);
    paths_.erase(std::remove(paths_.begin(), paths_.end(), normalized), paths_.end());
    paths_.insert(paths_.begin(), normalized);
    if (paths_.size() > kMaxRecentFiles) {
        paths_.resize(kMaxRecentFiles);
    }

    return save(errorMessage);
}

bool RecentFiles::removePath(const std::filesystem::path& path, std::string& errorMessage)
{
    errorMessage.clear();
    const std::filesystem::path normalized = normalizePath(path);
    paths_.erase(std::remove(paths_.begin(), paths_.end(), normalized), paths_.end());
    return save(errorMessage);
}

std::filesystem::path RecentFiles::storagePath()
{
    if (const auto localAppData = environmentVariablePath("LOCALAPPDATA")) {
        return *localAppData / "VisiForm" / "recent_files.json";
    }

#ifdef __APPLE__
    if (const auto home = environmentVariablePath("HOME")) {
        return *home / "Library" / "Application Support" / "VisiForm" / "recent_files.json";
    }
#else
    if (const auto xdgConfigHome = environmentVariablePath("XDG_CONFIG_HOME")) {
        return *xdgConfigHome / "VisiForm" / "recent_files.json";
    }
    if (const auto home = environmentVariablePath("HOME")) {
        return *home / ".config" / "VisiForm" / "recent_files.json";
    }
#endif

    return std::filesystem::current_path() / "Generated" / "recent_files.json";
}

bool RecentFiles::save(std::string& errorMessage) const
{
    errorMessage.clear();
    const std::filesystem::path path = storagePath();
    if (path.empty()) {
        errorMessage = "Recent files storage path is empty";
        return false;
    }

    nlohmann::json json = nlohmann::json::array();
    for (const auto& entry : paths_) {
        json.push_back(FileUtils::normalizeSeparators(entry.string()));
    }

    return FileUtils::writeTextFile(path, json.dump(2), errorMessage);
}

} // namespace visiform::utils
