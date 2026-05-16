#include "utils/NativeFileDialogs.h"

#include "utils/NativeFileDialogs.h"

#include "utils/FileUtils.h"

#include <array>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>

namespace visiform::utils {
namespace {

constexpr wchar_t kProjectFilter[] =
    L"VisiForm Project (*.vfb.json)\0*.vfb.json\0"
    L"All Files (*.*)\0*.*\0\0";

std::filesystem::path normalizeProjectSavePath(std::filesystem::path path)
{
    if (FileUtils::hasProjectExtension(path)) {
        return path;
    }
    if (path.extension() == L".vfb") {
        return std::filesystem::path{ path.native() + std::wstring{ L".json" } };
    }
    if (path.extension() == L".json") {
        return path.parent_path() / std::filesystem::path{ path.stem().native() + std::wstring{ L".vfb.json" } };
    }

    return std::filesystem::path{ path.native() + std::wstring{ L".vfb.json" } };
}

std::optional<std::filesystem::path> showProjectDialog(bool saveDialog,
    const std::filesystem::path& suggestedPath,
    const std::filesystem::path& initialDirectory)
{
    std::vector<wchar_t> buffer(4096, L'\0');
    const std::wstring initialFileName = suggestedPath.filename().native();
    if (!initialFileName.empty()) {
        const std::size_t copyLength = std::min(initialFileName.size(), buffer.size() - 1);
        std::copy_n(initialFileName.c_str(), copyLength, buffer.data());
    }

    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = nullptr;
    dialog.lpstrFilter = kProjectFilter;
    dialog.nFilterIndex = 1;
    dialog.lpstrFile = buffer.data();
    dialog.nMaxFile = static_cast<DWORD>(buffer.size());
    const std::filesystem::path dialogDirectory = !suggestedPath.parent_path().empty() ? suggestedPath.parent_path() : initialDirectory;
    dialog.lpstrInitialDir = dialogDirectory.empty() ? nullptr : dialogDirectory.c_str();
    dialog.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
    if (saveDialog) {
        dialog.Flags |= OFN_OVERWRITEPROMPT;
        dialog.lpstrDefExt = L"json";
    }
    else {
        dialog.Flags |= OFN_FILEMUSTEXIST;
    }

    const BOOL result = saveDialog ? GetSaveFileNameW(&dialog) : GetOpenFileNameW(&dialog);
    if (!result) {
        return std::nullopt;
    }

    std::filesystem::path selectedPath{ buffer.data() };
    return saveDialog ? normalizeProjectSavePath(std::move(selectedPath)) : selectedPath;
}

} // namespace

std::optional<std::filesystem::path> showOpenProjectDialog(const std::filesystem::path& initialDirectory)
{
    return showProjectDialog(false, {}, initialDirectory);
}

std::optional<std::filesystem::path> showSaveProjectDialog(const std::filesystem::path& suggestedPath,
    const std::filesystem::path& initialDirectory)
{
    return showProjectDialog(true, suggestedPath, initialDirectory);
}

} // namespace visiform::utils
