#pragma once

#include <filesystem>
#include <optional>

namespace visiform::utils {

[[nodiscard]] std::optional<std::filesystem::path> showOpenProjectDialog();
[[nodiscard]] std::optional<std::filesystem::path> showSaveProjectDialog(const std::filesystem::path& suggestedPath);

} // namespace visiform::utils
