#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace visiform::utils {

class AppSettings {
public:
    static constexpr const char* defaultVisageGitRepository = "https://github.com/VitalAudio/visage.git";
    static constexpr const char* defaultVisageGitTag = "main";
    static constexpr int defaultPropertyInspectorWidth = 430;

    std::vector<std::filesystem::path> recentFiles{};
    std::filesystem::path lastProjectDirectory{};
    std::filesystem::path lastExportDirectory{};
    std::filesystem::path localVisageSourceDirectory{};
    std::string visageGitRepository = defaultVisageGitRepository;
    std::string visageGitTag = defaultVisageGitTag;
    bool showGrid = true;
    bool snapToGrid = true;
    bool smartGuidesEnabled = true;
    int gridSize = 10;
    int majorGridSize = 50;
    int propertyInspectorWidth = defaultPropertyInspectorWidth;
    std::map<std::string, std::string> keyboardShortcuts{};

    [[nodiscard]] static AppSettings load(std::string& errorMessage);
    [[nodiscard]] bool save(std::string& errorMessage) const;

    void addRecentFile(const std::filesystem::path& path);
    void removeRecentFile(const std::filesystem::path& path);
    void removeMissingRecentFiles();

private:
    [[nodiscard]] static std::filesystem::path storagePath();
};

} // namespace visiform::utils
