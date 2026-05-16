#include "utils/AppSettings.h"

#include "utils/FileUtils.h"

#include <algorithm>
#include <cstdlib>
#include <nlohmann/json.hpp>

namespace visiform::utils {
namespace {

constexpr std::size_t kMaxRecentFiles = 10;

std::filesystem::path normalizePath(const std::filesystem::path& path)
{
    return path.lexically_normal();
}

} // namespace

AppSettings AppSettings::load(std::string& errorMessage)
{
    errorMessage.clear();
    AppSettings settings;

    const std::filesystem::path path = storagePath();
    if (path.empty() || !std::filesystem::exists(path)) {
        return settings;
    }

    std::string text;
    if (!FileUtils::readTextFile(path, text, errorMessage)) {
        errorMessage.clear();
        return settings;
    }

    try {
        const auto json = nlohmann::json::parse(text);
        if (const auto iterator = json.find("recentFiles"); iterator != json.end() && iterator->is_array()) {
            for (const auto& entry : *iterator) {
                if (!entry.is_string()) {
                    continue;
                }
                settings.recentFiles.push_back(normalizePath(entry.get<std::string>()));
                if (settings.recentFiles.size() >= kMaxRecentFiles) {
                    break;
                }
            }
        }
        if (const auto iterator = json.find("lastProjectDirectory"); iterator != json.end() && iterator->is_string()) {
            settings.lastProjectDirectory = normalizePath(iterator->get<std::string>());
        }
        if (const auto iterator = json.find("lastExportDirectory"); iterator != json.end() && iterator->is_string()) {
            settings.lastExportDirectory = normalizePath(iterator->get<std::string>());
        }
        if (const auto iterator = json.find("showGrid"); iterator != json.end() && iterator->is_boolean()) {
            settings.showGrid = iterator->get<bool>();
        }
        if (const auto iterator = json.find("snapToGrid"); iterator != json.end() && iterator->is_boolean()) {
            settings.snapToGrid = iterator->get<bool>();
        }
        if (const auto iterator = json.find("smartGuidesEnabled"); iterator != json.end() && iterator->is_boolean()) {
            settings.smartGuidesEnabled = iterator->get<bool>();
        }
        if (const auto iterator = json.find("gridSize"); iterator != json.end() && iterator->is_number_integer()) {
            settings.gridSize = std::max(1, iterator->get<int>());
        }
        if (const auto iterator = json.find("majorGridSize"); iterator != json.end() && iterator->is_number_integer()) {
            settings.majorGridSize = std::max(1, iterator->get<int>());
        }
    }
    catch (...) {
        errorMessage.clear();
        return AppSettings{};
    }

    settings.removeMissingRecentFiles();
    return settings;
}

bool AppSettings::save(std::string& errorMessage) const
{
    errorMessage.clear();
    const std::filesystem::path path = storagePath();
    if (path.empty()) {
        errorMessage = "Settings storage path is empty";
        return false;
    }

    nlohmann::json json;
    json["recentFiles"] = nlohmann::json::array();
    for (const auto& entry : recentFiles) {
        json["recentFiles"].push_back(FileUtils::normalizeSeparators(entry.string()));
    }
    json["lastProjectDirectory"] = lastProjectDirectory.empty() ? std::string{} : FileUtils::normalizeSeparators(lastProjectDirectory.string());
    json["lastExportDirectory"] = lastExportDirectory.empty() ? std::string{} : FileUtils::normalizeSeparators(lastExportDirectory.string());
    json["showGrid"] = showGrid;
    json["snapToGrid"] = snapToGrid;
    json["smartGuidesEnabled"] = smartGuidesEnabled;
    json["gridSize"] = gridSize;
    json["majorGridSize"] = majorGridSize;

    return FileUtils::writeTextFile(path, json.dump(2), errorMessage);
}

void AppSettings::addRecentFile(const std::filesystem::path& path)
{
    if (path.empty()) {
        return;
    }

    const std::filesystem::path normalized = normalizePath(path);
    recentFiles.erase(std::remove(recentFiles.begin(), recentFiles.end(), normalized), recentFiles.end());
    recentFiles.insert(recentFiles.begin(), normalized);
    if (recentFiles.size() > kMaxRecentFiles) {
        recentFiles.resize(kMaxRecentFiles);
    }
}

void AppSettings::removeRecentFile(const std::filesystem::path& path)
{
    const std::filesystem::path normalized = normalizePath(path);
    recentFiles.erase(std::remove(recentFiles.begin(), recentFiles.end(), normalized), recentFiles.end());
}

void AppSettings::removeMissingRecentFiles()
{
    recentFiles.erase(std::remove_if(recentFiles.begin(), recentFiles.end(),
        [](const std::filesystem::path& path) { return !std::filesystem::exists(path); }), recentFiles.end());
}

std::filesystem::path AppSettings::storagePath()
{
    char* appData = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&appData, &length, "APPDATA") == 0 && appData != nullptr) {
        const std::filesystem::path path = std::filesystem::path{ appData } / "VisiForm" / "settings.json";
        free(appData);
        return path;
    }

    return std::filesystem::current_path() / "Generated" / "settings.json";
}

} // namespace visiform::utils
