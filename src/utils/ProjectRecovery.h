#pragma once

#include "model/ProjectDocument.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace visiform::utils {

struct RecoveryEntry {
    std::filesystem::path projectDataPath{};
    std::filesystem::path metadataPath{};
    std::filesystem::path originalProjectPath{};
    std::string displayName{};
    std::string updatedUtc{};
    std::string visiformVersion{};
    std::string documentId{};
    bool originalWasUnsaved = true;
};

class ProjectRecovery {
public:
    [[nodiscard]] static std::string savedDocumentId(const std::filesystem::path& projectPath);
    [[nodiscard]] static std::string newUnsavedDocumentId();

    [[nodiscard]] static bool write(
        const model::ProjectDocument& document,
        const std::filesystem::path& originalProjectPath,
        const std::string& documentId,
        const std::string& visiformVersion,
        RecoveryEntry& entry,
        std::string& errorMessage);

    [[nodiscard]] static std::vector<RecoveryEntry> discover(std::string& errorMessage);
    [[nodiscard]] static std::optional<model::ProjectDocument> load(const RecoveryEntry& entry, std::string& errorMessage);
    [[nodiscard]] static bool remove(const RecoveryEntry& entry, std::string& errorMessage);

private:
    [[nodiscard]] static std::filesystem::path recoveryDirectory();
};

} // namespace visiform::utils
