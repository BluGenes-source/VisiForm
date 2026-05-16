#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace visiform::utils {

class RecentFiles {
public:
    [[nodiscard]] const std::vector<std::filesystem::path>& paths() const;
    [[nodiscard]] bool load(std::string& errorMessage);
    [[nodiscard]] bool addPath(const std::filesystem::path& path, std::string& errorMessage);
    [[nodiscard]] bool removePath(const std::filesystem::path& path, std::string& errorMessage);

private:
    [[nodiscard]] static std::filesystem::path storagePath();
    [[nodiscard]] bool save(std::string& errorMessage) const;

    std::vector<std::filesystem::path> paths_{};
};

} // namespace visiform::utils
