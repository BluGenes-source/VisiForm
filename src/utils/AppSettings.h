#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace visiform::utils {

class AppSettings {
public:
    std::vector<std::filesystem::path> recentFiles{};
    std::filesystem::path lastProjectDirectory{};
    std::filesystem::path lastExportDirectory{};
    bool showGrid = true;
    bool snapToGrid = true;
    bool smartGuidesEnabled = true;
    int gridSize = 10;
    int majorGridSize = 50;

    [[nodiscard]] static AppSettings load(std::string& errorMessage);
    [[nodiscard]] bool save(std::string& errorMessage) const;

    void addRecentFile(const std::filesystem::path& path);
    void removeRecentFile(const std::filesystem::path& path);
    void removeMissingRecentFiles();

private:
    [[nodiscard]] static std::filesystem::path storagePath();
};

} // namespace visiform::utils
