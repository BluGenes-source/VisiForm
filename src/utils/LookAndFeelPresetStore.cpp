#include "utils/LookAndFeelPresetStore.h"

#include "model/LookAndFeelRegistry.h"
#include "utils/FileUtils.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace visiform::utils {
namespace {

using Json = nlohmann::json;

std::string trimWhitespace(std::string value)
{
    const auto isWhitespace = [](unsigned char character) { return std::isspace(character) != 0; };
    value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), isWhitespace));
    value.erase(std::find_if_not(value.rbegin(), value.rend(), isWhitespace).base(), value.end());
    return value;
}

std::string lowerText(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return value;
}

bool isValidColor(const std::string& value)
{
    return (value.size() == 7 || value.size() == 9)
        && value.front() == '#'
        && std::all_of(value.begin() + 1, value.end(),
            [](unsigned char character) { return std::isxdigit(character) != 0; });
}

std::optional<std::filesystem::path> environmentVariablePath(const char* variableName)
{
    const char* value = std::getenv(variableName);
    if (value == nullptr || *value == '\0') {
        return std::nullopt;
    }
    return std::filesystem::path{ value }.lexically_normal();
}

Json styleToJson(const model::LookAndFeelDefinition& definition)
{
    return {
        { "applicationSurfaceColor", definition.panelColor },
        { "controlSurfaceColor", definition.controlFillColor },
        { "recessedSurfaceColor", definition.recessedSurfaceColor },
        { "raisedSurfaceColor", definition.raisedSurfaceColor },
        { "primaryTextColor", definition.controlTextColor },
        { "secondaryTextColor", definition.secondaryTextColor },
        { "disabledTextColor", definition.disabledTextColor },
        { "disabledSurfaceColor", definition.disabledColor },
        { "borderColor", definition.controlBorderColor },
        { "focusOutlineColor", definition.focusOutlineColor },
        { "accentColor", definition.accentColor },
        { "selectedStateColor", definition.selectedStateColor },
        { "hoverStateColor", definition.hoverStateColor },
        { "pressedStateColor", definition.pressedStateColor },
        { "checkedStateColor", definition.checkedStateColor },
        { "highlightEdgeColor", definition.highlightEdgeColor },
        { "shadowEdgeColor", definition.shadowEdgeColor },
        { "borderThickness", definition.borderThickness },
        { "cornerRadius", definition.cornerRadius },
        { "fontFamily", definition.fontFamily },
        { "fontSize", definition.fontSize },
        { "fontWeight", definition.fontWeight },
        { "italic", definition.italic },
        { "controlPadding", definition.controlPadding },
        { "textPadding", definition.textPadding },
        { "disabledTextTreatment", definition.disabledTextTreatment },
        { "splitterHighlightThickness", definition.splitterHighlightThickness },
        { "splitterShadowThickness", definition.splitterShadowThickness }
    };
}

Json presetToJson(const CustomLookAndFeelPreset& preset)
{
    Json json{
        { "formatVersion", preset.formatVersion },
        { "id", preset.definition.id },
        { "displayName", preset.definition.displayName },
        { "style", styleToJson(preset.definition) }
    };
    if (!preset.sourcePresetId.empty()) {
        json["sourcePresetId"] = preset.sourcePresetId;
    }
    return json;
}

bool readColor(const Json& style, const char* key, std::string& target, std::string& errorMessage)
{
    const auto iterator = style.find(key);
    if (iterator == style.end() || !iterator->is_string() || !isValidColor(iterator->get<std::string>())) {
        errorMessage = std::string{ "Preset style requires a valid " } + key + " color.";
        return false;
    }
    target = iterator->get<std::string>();
    return true;
}

bool readMetric(const Json& style, const char* key, float minimum, float maximum,
    float& target, std::string& errorMessage)
{
    const auto iterator = style.find(key);
    if (iterator == style.end() || !iterator->is_number()) {
        errorMessage = std::string{ "Preset style requires numeric " } + key + ".";
        return false;
    }
    const double value = iterator->get<double>();
    if (!std::isfinite(value)) {
        errorMessage = std::string{ "Preset style contains non-finite " } + key + ".";
        return false;
    }
    target = std::clamp(static_cast<float>(value), minimum, maximum);
    return true;
}

void readOptionalTypography(const Json& style, model::LookAndFeelDefinition& definition)
{
    if (const auto iterator = style.find("fontFamily"); iterator != style.end() && iterator->is_string()) {
        definition.fontFamily = trimWhitespace(iterator->get<std::string>());
        if (definition.fontFamily.empty()) {
            definition.fontFamily = "Default";
        }
    }
    if (const auto iterator = style.find("fontWeight"); iterator != style.end() && iterator->is_number_integer()) {
        definition.fontWeight = std::clamp(iterator->get<int>(), 100, 900);
    }
    if (const auto iterator = style.find("italic"); iterator != style.end() && iterator->is_boolean()) {
        definition.italic = iterator->get<bool>();
    }
    if (const auto iterator = style.find("textPadding"); iterator != style.end() && iterator->is_number()) {
        definition.textPadding = std::clamp(iterator->get<float>(), 0.0f, 40.0f);
    }
    else {
        definition.textPadding = definition.controlPadding;
    }
    if (const auto iterator = style.find("disabledTextTreatment"); iterator != style.end() && iterator->is_string()) {
        const std::string value = trimWhitespace(iterator->get<std::string>());
        definition.disabledTextTreatment = value == "Normal" ? "Normal" : "Muted";
    }
}

std::optional<CustomLookAndFeelPreset> presetFromJson(const Json& json, std::string& errorMessage)
{
    errorMessage.clear();
    if (!json.is_object()) {
        errorMessage = "Preset entry must be an object.";
        return std::nullopt;
    }
    if (!json.contains("formatVersion") || !json["formatVersion"].is_number_integer()
        || json["formatVersion"].get<int>() != CustomLookAndFeelPreset::currentFormatVersion) {
        errorMessage = "Unsupported Look and Feel preset format version.";
        return std::nullopt;
    }
    if (!json.contains("id") || !json["id"].is_string()
        || !json.contains("displayName") || !json["displayName"].is_string()
        || !json.contains("style") || !json["style"].is_object()) {
        errorMessage = "Preset requires id, displayName, and style.";
        return std::nullopt;
    }

    CustomLookAndFeelPreset preset;
    preset.definition.id = trimWhitespace(json["id"].get<std::string>());
    preset.definition.displayName = trimWhitespace(json["displayName"].get<std::string>());
    if (preset.definition.id.empty() || preset.definition.displayName.empty()) {
        errorMessage = "Preset id and display name cannot be empty.";
        return std::nullopt;
    }
    if (const auto iterator = json.find("sourcePresetId"); iterator != json.end() && iterator->is_string()) {
        preset.sourcePresetId = trimWhitespace(iterator->get<std::string>());
    }

    const Json& style = json["style"];
    auto& definition = preset.definition;
    if (!readColor(style, "applicationSurfaceColor", definition.panelColor, errorMessage)
        || !readColor(style, "controlSurfaceColor", definition.controlFillColor, errorMessage)
        || !readColor(style, "recessedSurfaceColor", definition.recessedSurfaceColor, errorMessage)
        || !readColor(style, "raisedSurfaceColor", definition.raisedSurfaceColor, errorMessage)
        || !readColor(style, "primaryTextColor", definition.controlTextColor, errorMessage)
        || !readColor(style, "secondaryTextColor", definition.secondaryTextColor, errorMessage)
        || !readColor(style, "disabledTextColor", definition.disabledTextColor, errorMessage)
        || !readColor(style, "disabledSurfaceColor", definition.disabledColor, errorMessage)
        || !readColor(style, "borderColor", definition.controlBorderColor, errorMessage)
        || !readColor(style, "focusOutlineColor", definition.focusOutlineColor, errorMessage)
        || !readColor(style, "accentColor", definition.accentColor, errorMessage)
        || !readColor(style, "selectedStateColor", definition.selectedStateColor, errorMessage)
        || !readColor(style, "hoverStateColor", definition.hoverStateColor, errorMessage)
        || !readColor(style, "pressedStateColor", definition.pressedStateColor, errorMessage)
        || !readColor(style, "checkedStateColor", definition.checkedStateColor, errorMessage)
        || !readColor(style, "highlightEdgeColor", definition.highlightEdgeColor, errorMessage)
        || !readColor(style, "shadowEdgeColor", definition.shadowEdgeColor, errorMessage)
        || !readMetric(style, "borderThickness", 0.0f, 20.0f, definition.borderThickness, errorMessage)
        || !readMetric(style, "cornerRadius", 0.0f, 50.0f, definition.cornerRadius, errorMessage)
        || !readMetric(style, "fontSize", 8.0f, 72.0f, definition.fontSize, errorMessage)
        || !readMetric(style, "controlPadding", 0.0f, 40.0f, definition.controlPadding, errorMessage)
        || !readMetric(style, "splitterHighlightThickness", 0.0f, 8.0f, definition.splitterHighlightThickness, errorMessage)
        || !readMetric(style, "splitterShadowThickness", 0.0f, 8.0f, definition.splitterShadowThickness, errorMessage)) {
        return std::nullopt;
    }
    readOptionalTypography(style, definition);
    return preset;
}

bool replaceFile(const std::filesystem::path& temporaryPath,
    const std::filesystem::path& finalPath,
    std::string& errorMessage)
{
#ifdef _WIN32
    if (MoveFileExW(temporaryPath.c_str(), finalPath.c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0) {
        return true;
    }
#else
    std::error_code errorCode;
    std::filesystem::rename(temporaryPath, finalPath, errorCode);
    if (!errorCode) {
        return true;
    }
#endif
    errorMessage = "Unable to replace Look and Feel preset store: "
        + FileUtils::normalizeSeparators(finalPath.string());
    return false;
}

bool writeJsonSafely(const std::filesystem::path& path, const Json& json, std::string& errorMessage)
{
    if (!FileUtils::ensureDirectoryExists(path.parent_path(), errorMessage)) {
        return false;
    }
    const std::filesystem::path temporaryPath = path.string() + ".tmp";
    if (!FileUtils::writeTextFile(temporaryPath, json.dump(2), errorMessage)) {
        return false;
    }
    if (!replaceFile(temporaryPath, path, errorMessage)) {
        std::error_code ignored;
        std::filesystem::remove(temporaryPath, ignored);
        return false;
    }
    return true;
}

} // namespace

LookAndFeelPresetStore::LookAndFeelPresetStore(std::filesystem::path storagePath)
    : storagePath_(storagePath.empty() ? defaultStoragePath() : std::move(storagePath))
{
}

bool LookAndFeelPresetStore::load(std::string& warningMessage)
{
    warningMessage.clear();
    presets_.clear();
    if (storagePath_.empty() || !std::filesystem::exists(storagePath_)) {
        return true;
    }

    std::string text;
    if (!FileUtils::readTextFile(storagePath_, text, warningMessage)) {
        return false;
    }

    Json json;
    try {
        json = Json::parse(text);
    }
    catch (...) {
        warningMessage = "Custom Look and Feel preset store is not valid JSON.";
        return false;
    }
    bool supportedStore = false;
    try {
        supportedStore = json.is_object()
            && json.contains("formatVersion")
            && json["formatVersion"].is_number_integer()
            && json["formatVersion"].get<int>() == CustomLookAndFeelPreset::currentFormatVersion
            && json.contains("presets")
            && json["presets"].is_array();
    }
    catch (...) {
        supportedStore = false;
    }
    if (!supportedStore) {
        warningMessage = "Custom Look and Feel preset store uses an unsupported format.";
        return false;
    }

    std::size_t skipped = 0;
    for (const auto& entry : json["presets"]) {
        std::string entryError;
        std::optional<CustomLookAndFeelPreset> preset;
        try {
            preset = presetFromJson(entry, entryError);
        }
        catch (...) {
            entryError = "Preset entry contains an unreadable value.";
        }
        if (!preset.has_value()
            || model::LookAndFeelRegistry::instance().isBuiltIn(preset->definition.id)
            || findById(preset->definition.id) != nullptr
            || containsDisplayName(preset->definition.displayName)) {
            ++skipped;
            continue;
        }
        presets_.push_back(std::move(*preset));
    }
    if (skipped > 0) {
        warningMessage = "Skipped " + std::to_string(skipped) + " invalid or conflicting custom Look and Feel preset entries.";
    }
    return true;
}

bool LookAndFeelPresetStore::save(std::string& errorMessage) const
{
    Json json{
        { "formatVersion", CustomLookAndFeelPreset::currentFormatVersion },
        { "presets", Json::array() }
    };
    for (const auto& preset : presets_) {
        json["presets"].push_back(presetToJson(preset));
    }
    return writeJsonSafely(storagePath_, json, errorMessage);
}

const std::vector<CustomLookAndFeelPreset>& LookAndFeelPresetStore::presets() const
{
    return presets_;
}

std::vector<model::LookAndFeelDefinition> LookAndFeelPresetStore::definitions() const
{
    std::vector<model::LookAndFeelDefinition> result;
    result.reserve(presets_.size());
    for (const auto& preset : presets_) {
        result.push_back(preset.definition);
    }
    return result;
}

const CustomLookAndFeelPreset* LookAndFeelPresetStore::findById(const std::string& id) const
{
    const auto iterator = std::find_if(presets_.begin(), presets_.end(),
        [&id](const CustomLookAndFeelPreset& preset) { return preset.definition.id == id; });
    return iterator == presets_.end() ? nullptr : &*iterator;
}

bool LookAndFeelPresetStore::containsDisplayName(const std::string& displayName,
    const std::string& exceptId) const
{
    const std::string normalized = lowerText(trimWhitespace(displayName));
    if (normalized.empty()) {
        return false;
    }
    const auto& registry = model::LookAndFeelRegistry::instance();
    for (const auto& definition : registry.definitions()) {
        if (registry.isBuiltIn(definition.id)
            && definition.id != exceptId
            && lowerText(definition.displayName) == normalized) {
            return true;
        }
    }
    return std::any_of(presets_.begin(), presets_.end(),
        [&normalized, &exceptId](const CustomLookAndFeelPreset& preset) {
            return preset.definition.id != exceptId
                && lowerText(preset.definition.displayName) == normalized;
        });
}

std::string LookAndFeelPresetStore::uniqueDisplayName(const std::string& preferredName,
    const std::string& exceptId) const
{
    const std::string base = trimWhitespace(preferredName).empty() ? std::string{ "Custom Preset" } : trimWhitespace(preferredName);
    if (!containsDisplayName(base, exceptId)) {
        return base;
    }
    for (int suffix = 2; suffix < 10000; ++suffix) {
        const std::string candidate = base + " " + std::to_string(suffix);
        if (!containsDisplayName(candidate, exceptId)) {
            return candidate;
        }
    }
    return base + " " + createStableId().substr(7);
}

std::optional<std::string> LookAndFeelPresetStore::addFromResolvedStyle(
    const std::string& displayName,
    const model::ResolvedLookAndFeelStyle& style,
    const std::string& sourcePresetId,
    std::string& errorMessage)
{
    errorMessage.clear();
    const std::string requestedName = trimWhitespace(displayName);
    if (requestedName.empty()) {
        errorMessage = "Preset name cannot be empty.";
        return std::nullopt;
    }
    const std::string normalizedName = uniqueDisplayName(requestedName);

    const auto previous = presets_;
    CustomLookAndFeelPreset preset;
    preset.definition = definitionFromResolvedStyle(createStableId(), normalizedName, style);
    preset.sourcePresetId = sourcePresetId;
    const std::string id = preset.definition.id;
    presets_.push_back(std::move(preset));
    if (!persistMutation(previous, errorMessage)) {
        return std::nullopt;
    }
    return id;
}

std::optional<std::string> LookAndFeelPresetStore::duplicate(
    const model::LookAndFeelDefinition& definition,
    const std::string& sourcePresetId,
    std::string& errorMessage)
{
    model::ResolvedLookAndFeelStyle style =
        model::LookAndFeelRegistry::instance().resolveProjectStyle(definition.id, {});
    return addFromResolvedStyle(uniqueDisplayName(definition.displayName + " Copy"),
        style, sourcePresetId.empty() ? definition.id : sourcePresetId, errorMessage);
}

bool LookAndFeelPresetStore::rename(const std::string& id,
    const std::string& displayName,
    std::string& errorMessage)
{
    errorMessage.clear();
    auto iterator = std::find_if(presets_.begin(), presets_.end(),
        [&id](const CustomLookAndFeelPreset& preset) { return preset.definition.id == id; });
    if (iterator == presets_.end()) {
        errorMessage = "Only custom presets can be renamed.";
        return false;
    }
    const std::string normalizedName = trimWhitespace(displayName);
    if (normalizedName.empty()) {
        errorMessage = "Preset name cannot be empty.";
        return false;
    }
    if (containsDisplayName(normalizedName, id)) {
        errorMessage = "A Look and Feel preset already uses that name.";
        return false;
    }
    if (iterator->definition.displayName == normalizedName) {
        return true;
    }
    const auto previous = presets_;
    iterator->definition.displayName = normalizedName;
    return persistMutation(previous, errorMessage);
}

bool LookAndFeelPresetStore::remove(const std::string& id, std::string& errorMessage)
{
    errorMessage.clear();
    const auto iterator = std::find_if(presets_.begin(), presets_.end(),
        [&id](const CustomLookAndFeelPreset& preset) { return preset.definition.id == id; });
    if (iterator == presets_.end()) {
        errorMessage = "Only custom presets can be deleted.";
        return false;
    }
    const auto previous = presets_;
    presets_.erase(iterator);
    return persistMutation(previous, errorMessage);
}

std::optional<std::string> LookAndFeelPresetStore::importPreset(
    const std::filesystem::path& path,
    std::string& errorMessage)
{
    std::string text;
    if (!FileUtils::readTextFile(path, text, errorMessage)) {
        return std::nullopt;
    }
    Json json;
    try {
        json = Json::parse(text);
    }
    catch (...) {
        errorMessage = "Preset file is not valid JSON.";
        return std::nullopt;
    }
    std::optional<CustomLookAndFeelPreset> imported;
    try {
        imported = presetFromJson(json, errorMessage);
    }
    catch (...) {
        errorMessage = "Preset file contains an unreadable value.";
    }
    if (!imported.has_value()) {
        return std::nullopt;
    }

    imported->definition.id = createStableId();
    const std::string importedName = imported->definition.displayName;
    imported->definition.displayName = containsDisplayName(importedName)
        ? uniqueDisplayName(importedName + " Imported")
        : importedName;
    const std::string id = imported->definition.id;
    const auto previous = presets_;
    presets_.push_back(std::move(*imported));
    if (!persistMutation(previous, errorMessage)) {
        return std::nullopt;
    }
    return id;
}

bool LookAndFeelPresetStore::exportPreset(
    const model::LookAndFeelDefinition& definition,
    const std::string& sourcePresetId,
    const std::filesystem::path& path,
    std::string& errorMessage) const
{
    CustomLookAndFeelPreset preset;
    preset.definition = definition;
    preset.sourcePresetId = sourcePresetId;
    return writeJsonSafely(path, presetToJson(preset), errorMessage);
}

std::filesystem::path LookAndFeelPresetStore::defaultStoragePath()
{
    if (const auto appData = environmentVariablePath("APPDATA")) {
        return *appData / "VisiForm" / "look_and_feel_presets.json";
    }
#ifdef __APPLE__
    if (const auto home = environmentVariablePath("HOME")) {
        return *home / "Library" / "Application Support" / "VisiForm" / "look_and_feel_presets.json";
    }
#else
    if (const auto xdgConfigHome = environmentVariablePath("XDG_CONFIG_HOME")) {
        return *xdgConfigHome / "VisiForm" / "look_and_feel_presets.json";
    }
    if (const auto home = environmentVariablePath("HOME")) {
        return *home / ".config" / "VisiForm" / "look_and_feel_presets.json";
    }
#endif
    return std::filesystem::temp_directory_path() / "VisiForm" / "look_and_feel_presets.json";
}

model::LookAndFeelDefinition LookAndFeelPresetStore::definitionFromResolvedStyle(
    const std::string& id,
    const std::string& displayName,
    const model::ResolvedLookAndFeelStyle& style)
{
    model::LookAndFeelDefinition definition;
    definition.id = id;
    definition.displayName = displayName;
    definition.panelColor = style.applicationSurfaceColor;
    definition.controlFillColor = style.controlSurfaceColor;
    definition.recessedSurfaceColor = style.recessedSurfaceColor;
    definition.raisedSurfaceColor = style.raisedSurfaceColor;
    definition.controlTextColor = style.primaryTextColor;
    definition.secondaryTextColor = style.secondaryTextColor;
    definition.disabledTextColor = style.disabledTextColor;
    definition.disabledColor = style.disabledSurfaceColor;
    definition.controlBorderColor = style.borderColor;
    definition.focusOutlineColor = style.focusOutlineColor;
    definition.accentColor = style.accentColor;
    definition.selectedStateColor = style.selectedStateColor;
    definition.hoverStateColor = style.hoverStateColor;
    definition.pressedStateColor = style.pressedStateColor;
    definition.checkedStateColor = style.checkedStateColor;
    definition.highlightEdgeColor = style.highlightEdgeColor;
    definition.shadowEdgeColor = style.shadowEdgeColor;
    definition.borderThickness = style.borderThickness;
    definition.cornerRadius = style.cornerRadius;
    definition.fontFamily = style.fontFamily;
    definition.fontSize = style.fontSize;
    definition.fontWeight = style.fontWeight;
    definition.italic = style.italic;
    definition.controlPadding = style.controlPadding;
    definition.textPadding = style.textPadding;
    definition.disabledTextTreatment = style.disabledTextTreatment;
    definition.splitterHighlightThickness = style.splitterHighlightThickness;
    definition.splitterShadowThickness = style.splitterShadowThickness;
    return definition;
}

std::string LookAndFeelPresetStore::createStableId() const
{
    static unsigned long long counter = 0;
    const auto ticks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    for (;;) {
        std::ostringstream stream;
        stream << "custom." << std::hex << ticks << '.' << ++counter;
        const std::string candidate = stream.str();
        if (model::LookAndFeelRegistry::instance().findById(candidate) == nullptr
            && findById(candidate) == nullptr) {
            return candidate;
        }
    }
}

bool LookAndFeelPresetStore::persistMutation(
    std::vector<CustomLookAndFeelPreset> previous,
    std::string& errorMessage)
{
    if (save(errorMessage)) {
        return true;
    }
    presets_ = std::move(previous);
    return false;
}

} // namespace visiform::utils
