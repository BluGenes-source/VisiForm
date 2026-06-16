#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace visiform::utils {

void setNativeDialogOwnerHandle(void* nativeHandle);

[[nodiscard]] std::optional<std::filesystem::path> showOpenProjectDialog(const std::filesystem::path& initialDirectory = {});
[[nodiscard]] std::optional<std::filesystem::path> showSaveProjectDialog(
    const std::filesystem::path& suggestedPath,
    const std::filesystem::path& initialDirectory = {});

[[nodiscard]] std::optional<std::filesystem::path> showSelectExportFolderDialog(const std::filesystem::path& initialDirectory = {});
[[nodiscard]] std::optional<std::filesystem::path> showOpenImageResourceDialog(const std::filesystem::path& initialDirectory = {});
[[nodiscard]] std::optional<std::filesystem::path> showOpenFontResourceDialog(const std::filesystem::path& initialDirectory = {});
[[nodiscard]] std::optional<std::string> showColorPickerDialog(const std::string& initialColor = {});

} // namespace visiform::utils
