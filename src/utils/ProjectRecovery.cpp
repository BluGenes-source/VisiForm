#include "utils/ProjectRecovery.h"

#include "serialization/JsonProjectReader.h"
#include "serialization/JsonProjectWriter.h"
#include "utils/FileUtils.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <system_error>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace visiform::utils {
namespace {

constexpr std::string_view kMetadataSuffix = ".recovery.meta.json";

std::string normalizedPathText(const std::filesystem::path& path)
{
    return FileUtils::normalizeSeparators(path.lexically_normal().string());
}

std::string hexadecimalHash(std::string_view value)
{
    std::uint64_t hash = 14695981039346656037ull;
    for (const unsigned char character : value) {
        hash ^= character;
        hash *= 1099511628211ull;
    }

    std::ostringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(16) << hash;
    return stream.str();
}

std::string utcTimestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif

    std::ostringstream stream;
    stream << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return stream.str();
}

bool writeAndFlush(const std::filesystem::path& path, const std::string& text, std::string& errorMessage)
{
    errorMessage.clear();
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        errorMessage = "Unable to open recovery temporary file: " + normalizedPathText(path);
        return false;
    }

    output.write(text.data(), static_cast<std::streamsize>(text.size()));
    output.flush();
    if (!output.good()) {
        errorMessage = "Failed while writing recovery temporary file: " + normalizedPathText(path);
        return false;
    }

    output.close();
    if (output.fail()) {
        errorMessage = "Failed while closing recovery temporary file: " + normalizedPathText(path);
        return false;
    }

    return true;
}

bool replaceFile(const std::filesystem::path& temporaryPath, const std::filesystem::path& finalPath, std::string& errorMessage)
{
    errorMessage.clear();
#ifdef _WIN32
    if (MoveFileExW(temporaryPath.c_str(), finalPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0) {
        return true;
    }

    errorMessage = "Unable to replace recovery file: " + normalizedPathText(finalPath);
    return false;
#else
    std::error_code errorCode;
    std::filesystem::rename(temporaryPath, finalPath, errorCode);
    if (!errorCode) {
        return true;
    }

    errorMessage = "Unable to replace recovery file: " + normalizedPathText(finalPath);
    return false;
#endif
}

bool safeWriteText(const std::filesystem::path& finalPath, const std::string& text, std::string& errorMessage)
{
    if (!FileUtils::ensureDirectoryExists(finalPath.parent_path(), errorMessage)) {
        return false;
    }

    std::filesystem::path temporaryPath = finalPath;
    temporaryPath += ".tmp";
    if (!writeAndFlush(temporaryPath, text, errorMessage)) {
        return false;
    }

    if (!replaceFile(temporaryPath, finalPath, errorMessage)) {
        std::error_code ignored;
        std::filesystem::remove(temporaryPath, ignored);
        return false;
    }

    return true;
}

std::optional<std::filesystem::path> environmentPath(const char* variableName)
{
    const char* value = std::getenv(variableName);
    if (value == nullptr || *value == '\0') {
        return std::nullopt;
    }

    return std::filesystem::path{ value }.lexically_normal();
}

bool isNewerThanOriginal(const RecoveryEntry& entry)
{
    if (entry.originalWasUnsaved || entry.originalProjectPath.empty()) {
        return true;
    }

    std::error_code errorCode;
    if (!std::filesystem::exists(entry.originalProjectPath, errorCode) || errorCode) {
        return true;
    }

    const auto recoveryTime = std::filesystem::last_write_time(entry.projectDataPath, errorCode);
    if (errorCode) {
        return false;
    }
    const auto originalTime = std::filesystem::last_write_time(entry.originalProjectPath, errorCode);
    return !errorCode && recoveryTime > originalTime;
}

} // namespace

std::string ProjectRecovery::savedDocumentId(const std::filesystem::path& projectPath)
{
    return "saved-" + hexadecimalHash(normalizedPathText(projectPath));
}

std::string ProjectRecovery::newUnsavedDocumentId()
{
    const auto ticks = std::chrono::system_clock::now().time_since_epoch().count();
    return "unsaved-" + hexadecimalHash(std::to_string(ticks));
}

bool ProjectRecovery::write(
    const model::ProjectDocument& document,
    const std::filesystem::path& originalProjectPath,
    const std::string& documentId,
    const std::string& visiformVersion,
    RecoveryEntry& entry,
    std::string& errorMessage)
{
    errorMessage.clear();
    const std::filesystem::path directory = recoveryDirectory();
    if (!FileUtils::ensureDirectoryExists(directory, errorMessage)) {
        return false;
    }

    const std::string safeDisplayName = FileUtils::sanitizeFileName(
        document.projectName.empty() ? std::string{ "Untitled" } : document.projectName);
    const std::string effectiveDocumentId = documentId.empty()
        ? (originalProjectPath.empty() ? newUnsavedDocumentId() : savedDocumentId(originalProjectPath))
        : documentId;
    const std::string fileBase = safeDisplayName + "-" + effectiveDocumentId;

    RecoveryEntry nextEntry;
    nextEntry.projectDataPath = directory / (fileBase + ".recovery.vfb.json");
    nextEntry.metadataPath = directory / (fileBase + std::string{ kMetadataSuffix });
    nextEntry.originalProjectPath = originalProjectPath.lexically_normal();
    nextEntry.displayName = document.projectName.empty() ? std::string{ "Untitled" } : document.projectName;
    nextEntry.updatedUtc = utcTimestamp();
    nextEntry.visiformVersion = visiformVersion;
    nextEntry.documentId = effectiveDocumentId;
    nextEntry.originalWasUnsaved = originalProjectPath.empty();

    serialization::JsonProjectWriter writer;
    if (!safeWriteText(nextEntry.projectDataPath, writer.writeToString(document), errorMessage)) {
        return false;
    }

    nlohmann::json metadata;
    metadata["originalProjectPath"] = nextEntry.originalProjectPath.empty()
        ? std::string{}
        : normalizedPathText(nextEntry.originalProjectPath);
    metadata["projectDisplayName"] = nextEntry.displayName;
    metadata["updatedUtc"] = nextEntry.updatedUtc;
    metadata["visiformVersion"] = nextEntry.visiformVersion;
    metadata["originalWasUnsaved"] = nextEntry.originalWasUnsaved;
    metadata["documentId"] = nextEntry.documentId;
    metadata["projectDataFile"] = nextEntry.projectDataPath.filename().string();
    if (!safeWriteText(nextEntry.metadataPath, metadata.dump(2), errorMessage)) {
        return false;
    }

    entry = std::move(nextEntry);
    return true;
}

std::vector<RecoveryEntry> ProjectRecovery::discover(std::string& errorMessage)
{
    errorMessage.clear();
    std::vector<RecoveryEntry> entries;
    const std::filesystem::path directory = recoveryDirectory();

    std::error_code errorCode;
    if (!std::filesystem::exists(directory, errorCode)) {
        return entries;
    }
    if (errorCode || !std::filesystem::is_directory(directory, errorCode) || errorCode) {
        errorMessage = "Unable to inspect the recovery directory.";
        return entries;
    }

    for (std::filesystem::directory_iterator iterator(directory, errorCode), end; iterator != end && !errorCode; iterator.increment(errorCode)) {
        std::error_code fileTypeError;
        if (!iterator->is_regular_file(fileTypeError) || fileTypeError) {
            continue;
        }

        const std::string filename = iterator->path().filename().string();
        if (!filename.ends_with(kMetadataSuffix)) {
            continue;
        }

        std::string metadataText;
        std::string readError;
        if (!FileUtils::readTextFile(iterator->path(), metadataText, readError)) {
            continue;
        }

        try {
            const auto metadata = nlohmann::json::parse(metadataText);
            if (!metadata.contains("projectDataFile") || !metadata["projectDataFile"].is_string()) {
                continue;
            }

            const std::filesystem::path dataFilename = std::filesystem::path{ metadata["projectDataFile"].get<std::string>() }.filename();
            RecoveryEntry entry;
            entry.projectDataPath = directory / dataFilename;
            entry.metadataPath = iterator->path();
            entry.originalProjectPath = metadata.value("originalProjectPath", std::string{});
            entry.displayName = metadata.value("projectDisplayName", std::string{ "Untitled" });
            entry.updatedUtc = metadata.value("updatedUtc", std::string{});
            entry.visiformVersion = metadata.value("visiformVersion", std::string{});
            entry.documentId = metadata.value("documentId", std::string{});
            entry.originalWasUnsaved = metadata.value("originalWasUnsaved", entry.originalProjectPath.empty());

            std::error_code dataFileError;
            if (!std::filesystem::exists(entry.projectDataPath, dataFileError)
                || dataFileError
                || !isNewerThanOriginal(entry)) {
                continue;
            }

            serialization::JsonProjectReader reader;
            std::string loadError;
            if (!reader.readFromFile(entry.projectDataPath, loadError).has_value()) {
                continue;
            }

            entries.push_back(std::move(entry));
        }
        catch (...) {
            continue;
        }
    }

    if (errorCode) {
        errorMessage = "Unable to enumerate all recovery files.";
    }

    std::sort(entries.begin(), entries.end(), [](const RecoveryEntry& left, const RecoveryEntry& right) {
        std::error_code leftError;
        std::error_code rightError;
        const auto leftTime = std::filesystem::last_write_time(left.projectDataPath, leftError);
        const auto rightTime = std::filesystem::last_write_time(right.projectDataPath, rightError);
        if (leftError || rightError) {
            return left.updatedUtc > right.updatedUtc;
        }
        return leftTime > rightTime;
    });
    return entries;
}

std::optional<model::ProjectDocument> ProjectRecovery::load(const RecoveryEntry& entry, std::string& errorMessage)
{
    serialization::JsonProjectReader reader;
    return reader.readFromFile(entry.projectDataPath, errorMessage);
}

bool ProjectRecovery::remove(const RecoveryEntry& entry, std::string& errorMessage)
{
    errorMessage.clear();
    bool success = true;
    for (const auto& path : { entry.projectDataPath, entry.metadataPath }) {
        if (path.empty()) {
            continue;
        }

        std::error_code errorCode;
        std::filesystem::remove(path, errorCode);
        if (errorCode) {
            success = false;
            if (errorMessage.empty()) {
                errorMessage = "Unable to remove recovery data: " + normalizedPathText(path);
            }
        }
    }
    return success;
}

std::filesystem::path ProjectRecovery::recoveryDirectory()
{
    if (const auto appData = environmentPath("APPDATA")) {
        return *appData / "VisiForm" / "recovery";
    }

#ifdef __APPLE__
    if (const auto home = environmentPath("HOME")) {
        return *home / "Library" / "Application Support" / "VisiForm" / "recovery";
    }
#else
    if (const auto xdgConfigHome = environmentPath("XDG_CONFIG_HOME")) {
        return *xdgConfigHome / "VisiForm" / "recovery";
    }
    if (const auto home = environmentPath("HOME")) {
        return *home / ".config" / "VisiForm" / "recovery";
    }
#endif

    return std::filesystem::current_path() / "Generated" / "recovery";
}

} // namespace visiform::utils
