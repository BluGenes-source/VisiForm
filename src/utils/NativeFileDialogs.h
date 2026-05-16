#pragma once

#pragma once

#include <filesystem>
#include <optional>

namespace visiform::utils {

[[nodiscard]] std::optional<std::filesystem::path> showOpenProjectDialog(const std::filesystem::path& initialDirectory = {});
[[nodiscard]] std::optional<std::filesystem::path> showSaveProjectDialog(
    const std::filesystem::path& suggestedPath,
    const std::filesystem::path& initialDirectory = {});

} // namespace visiform::utils
