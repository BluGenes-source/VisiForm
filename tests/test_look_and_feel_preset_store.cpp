#include "model/LookAndFeelRegistry.h"
#include "utils/FileUtils.h"
#include "utils/LookAndFeelPresetStore.h"

#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>
#include <string>

namespace {

std::filesystem::path uniqueTestPath(const std::string& name)
{
    const auto ticks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path()
        / ("visiform_" + name + "_" + std::to_string(ticks) + ".json");
}

void removeTestFile(const std::filesystem::path& path)
{
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
    std::filesystem::remove(path.string() + ".tmp", ignored);
}

} // namespace

TEST_CASE("Custom Look and Feel presets persist complete styles and stable renamed identifiers")
{
    auto& registry = visiform::model::LookAndFeelRegistry::instance();
    registry.setCustomDefinitions({});
    const std::filesystem::path path = uniqueTestPath("preset_store");
    removeTestFile(path);

    visiform::utils::LookAndFeelPresetStore store(path);
    std::string errorMessage;
    auto style = registry.resolveProjectStyle("VisiFormLight", {});
    style.controlSurfaceColor = "#123456";
    const auto id = store.addFromResolvedStyle("My Preset", style, "VisiFormLight", errorMessage);
    REQUIRE(id.has_value());
    CHECK(errorMessage.empty());
    REQUIRE(store.rename(*id, "Renamed Preset", errorMessage));
    CHECK(store.findById(*id)->definition.displayName == "Renamed Preset");

    visiform::utils::LookAndFeelPresetStore reloaded(path);
    std::string warningMessage;
    REQUIRE(reloaded.load(warningMessage));
    CHECK(warningMessage.empty());
    REQUIRE(reloaded.findById(*id) != nullptr);
    CHECK(reloaded.findById(*id)->definition.controlFillColor == "#123456");
    CHECK(reloaded.findById(*id)->sourcePresetId == "VisiFormLight");

    registry.setCustomDefinitions(reloaded.definitions());
    const auto resolved = registry.resolveProjectStyle(*id, {});
    CHECK(resolved.controlSurfaceColor == "#123456");
    CHECK(resolved.id == *id);

    registry.setCustomDefinitions({});
    removeTestFile(path);
}

TEST_CASE("Built-in presets duplicate as custom presets with unique names")
{
    auto& registry = visiform::model::LookAndFeelRegistry::instance();
    registry.setCustomDefinitions({});
    const std::filesystem::path path = uniqueTestPath("preset_duplicate");
    removeTestFile(path);

    visiform::utils::LookAndFeelPresetStore store(path);
    std::string errorMessage;
    const auto* builtIn = registry.findById("VisiFormDark");
    REQUIRE(builtIn != nullptr);
    const auto firstId = store.duplicate(*builtIn, builtIn->id, errorMessage);
    REQUIRE(firstId.has_value());
    const auto secondId = store.duplicate(*builtIn, builtIn->id, errorMessage);
    REQUIRE(secondId.has_value());
    CHECK(*firstId != *secondId);
    CHECK(store.findById(*firstId)->definition.displayName == "VisiForm Dark Copy");
    CHECK(store.findById(*secondId)->definition.displayName == "VisiForm Dark Copy 2");
    CHECK_FALSE(store.remove("VisiFormDark", errorMessage));

    removeTestFile(path);
}

TEST_CASE("Preset import validates atomically and resolves collisions")
{
    auto& registry = visiform::model::LookAndFeelRegistry::instance();
    registry.setCustomDefinitions({});
    const std::filesystem::path storePath = uniqueTestPath("preset_import_store");
    const std::filesystem::path exportPath = uniqueTestPath("preset_export");
    const std::filesystem::path invalidPath = uniqueTestPath("preset_invalid");
    removeTestFile(storePath);
    removeTestFile(exportPath);
    removeTestFile(invalidPath);

    visiform::utils::LookAndFeelPresetStore store(storePath);
    std::string errorMessage;
    const auto* builtIn = registry.findById("FlatClassic");
    REQUIRE(builtIn != nullptr);
    REQUIRE(store.exportPreset(*builtIn, builtIn->id, exportPath, errorMessage));

    const auto importedId = store.importPreset(exportPath, errorMessage);
    REQUIRE(importedId.has_value());
    CHECK(importedId->starts_with("custom."));
    CHECK(store.findById(*importedId)->definition.displayName == "Flat Classic Imported");
    CHECK(store.presets().size() == 1);

    REQUIRE(visiform::utils::FileUtils::writeTextFile(
        invalidPath,
        R"({"formatVersion":1,"id":"VisiFormDark","displayName":"Broken","style":{"applicationSurfaceColor":"nope"}})",
        errorMessage));
    CHECK_FALSE(store.importPreset(invalidPath, errorMessage).has_value());
    CHECK(store.presets().size() == 1);

    removeTestFile(storePath);
    removeTestFile(exportPath);
    removeTestFile(invalidPath);
}

TEST_CASE("Preset store skips malformed entries while preserving valid presets")
{
    auto& registry = visiform::model::LookAndFeelRegistry::instance();
    registry.setCustomDefinitions({});
    const std::filesystem::path sourcePath = uniqueTestPath("preset_partial_source");
    const std::filesystem::path storePath = uniqueTestPath("preset_partial_store");
    removeTestFile(sourcePath);
    removeTestFile(storePath);

    visiform::utils::LookAndFeelPresetStore source(sourcePath);
    std::string errorMessage;
    const auto style = registry.resolveProjectStyle("ImGuiDark", {});
    const auto id = source.addFromResolvedStyle("Valid Preset", style, "ImGuiDark", errorMessage);
    REQUIRE(id.has_value());

    std::string storeText;
    REQUIRE(visiform::utils::FileUtils::readTextFile(sourcePath, storeText, errorMessage));
    const std::string marker = R"("presets": [)";
    const auto markerPosition = storeText.find(marker);
    REQUIRE(markerPosition != std::string::npos);
    storeText.insert(markerPosition + marker.size(),
        R"({"formatVersion":1,"id":"bad","displayName":"Bad","style":{}},)");
    REQUIRE(visiform::utils::FileUtils::writeTextFile(storePath, storeText, errorMessage));

    visiform::utils::LookAndFeelPresetStore loaded(storePath);
    std::string warningMessage;
    REQUIRE(loaded.load(warningMessage));
    CHECK(loaded.presets().size() == 1);
    CHECK(loaded.findById(*id) != nullptr);
    CHECK(warningMessage.find("Skipped 1") != std::string::npos);

    removeTestFile(sourcePath);
    removeTestFile(storePath);
}
