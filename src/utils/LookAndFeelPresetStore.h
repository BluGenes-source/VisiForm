#pragma once

#include "model/LookAndFeelDefinition.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace visiform::utils {

struct CustomLookAndFeelPreset {
    static constexpr int currentFormatVersion = 1;

    int formatVersion = currentFormatVersion;
    model::LookAndFeelDefinition definition{};
    std::string sourcePresetId{};

    bool operator==(const CustomLookAndFeelPreset&) const = default;
};

class LookAndFeelPresetStore {
public:
    explicit LookAndFeelPresetStore(std::filesystem::path storagePath = {});

    [[nodiscard]] bool load(std::string& warningMessage);
    [[nodiscard]] bool save(std::string& errorMessage) const;

    [[nodiscard]] const std::vector<CustomLookAndFeelPreset>& presets() const;
    [[nodiscard]] std::vector<model::LookAndFeelDefinition> definitions() const;
    [[nodiscard]] const CustomLookAndFeelPreset* findById(const std::string& id) const;
    [[nodiscard]] bool containsDisplayName(const std::string& displayName,
        const std::string& exceptId = {}) const;
    [[nodiscard]] std::string uniqueDisplayName(const std::string& preferredName,
        const std::string& exceptId = {}) const;

    [[nodiscard]] std::optional<std::string> addFromResolvedStyle(
        const std::string& displayName,
        const model::ResolvedLookAndFeelStyle& style,
        const std::string& sourcePresetId,
        std::string& errorMessage);
    [[nodiscard]] std::optional<std::string> duplicate(
        const model::LookAndFeelDefinition& definition,
        const std::string& sourcePresetId,
        std::string& errorMessage);
    [[nodiscard]] bool rename(const std::string& id,
        const std::string& displayName,
        std::string& errorMessage);
    [[nodiscard]] bool remove(const std::string& id, std::string& errorMessage);
    [[nodiscard]] std::optional<std::string> importPreset(
        const std::filesystem::path& path,
        std::string& errorMessage);
    [[nodiscard]] bool exportPreset(
        const model::LookAndFeelDefinition& definition,
        const std::string& sourcePresetId,
        const std::filesystem::path& path,
        std::string& errorMessage) const;

    [[nodiscard]] static std::filesystem::path defaultStoragePath();
    [[nodiscard]] static model::LookAndFeelDefinition definitionFromResolvedStyle(
        const std::string& id,
        const std::string& displayName,
        const model::ResolvedLookAndFeelStyle& style);

private:
    [[nodiscard]] std::string createStableId() const;
    [[nodiscard]] bool persistMutation(std::vector<CustomLookAndFeelPreset> previous,
        std::string& errorMessage);

    std::filesystem::path storagePath_{};
    std::vector<CustomLookAndFeelPreset> presets_{};
};

} // namespace visiform::utils
