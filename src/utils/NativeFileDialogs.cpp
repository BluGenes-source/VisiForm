#include "utils/NativeFileDialogs.h"

#include "utils/NativeFileDialogs.h"

#include "utils/FileUtils.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <objbase.h>

namespace visiform::utils {
namespace {

constexpr wchar_t kProjectFilter[] =
    L"VisiForm Project (*.vfb.json)\0*.vfb.json\0"
    L"All Files (*.*)\0*.*\0\0";

constexpr wchar_t kImageResourceFilter[] =
    L"Image Assets (*.png;*.jpg;*.jpeg;*.bmp;*.webp)\0*.png;*.jpg;*.jpeg;*.bmp;*.webp\0"
    L"All Files (*.*)\0*.*\0\0";

constexpr wchar_t kFontResourceFilter[] =
    L"Font Assets (*.ttf;*.otf)\0*.ttf;*.otf\0"
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
    const std::filesystem::path dialogDirectory = !initialDirectory.empty()
        ? initialDirectory
        : suggestedPath.parent_path();
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

std::optional<std::filesystem::path> showOpenFilteredFileDialog(const wchar_t* filter, const std::filesystem::path& initialDirectory)
{
    std::vector<wchar_t> buffer(4096, L'\0');

    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = nullptr;
    dialog.lpstrFilter = filter;
    dialog.nFilterIndex = 1;
    dialog.lpstrFile = buffer.data();
    dialog.nMaxFile = static_cast<DWORD>(buffer.size());
    dialog.lpstrInitialDir = initialDirectory.empty() ? nullptr : initialDirectory.c_str();
    dialog.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (!GetOpenFileNameW(&dialog)) {
        return std::nullopt;
    }

    return std::filesystem::path{ buffer.data() };
}

bool isValidColorText(const std::string& value)
{
    if (value.size() != 7 || value.front() != '#') {
        return false;
    }

    return std::all_of(value.begin() + 1, value.end(), [](unsigned char character) {
        return std::isxdigit(character) != 0;
    });
}

COLORREF parseInitialColor(const std::string& initialColor)
{
    if (!isValidColorText(initialColor)) {
        return RGB(45, 125, 255);
    }

    const unsigned int red = std::stoul(initialColor.substr(1, 2), nullptr, 16);
    const unsigned int green = std::stoul(initialColor.substr(3, 2), nullptr, 16);
    const unsigned int blue = std::stoul(initialColor.substr(5, 2), nullptr, 16);
    return RGB(red, green, blue);
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

std::optional<std::filesystem::path> showSelectExportFolderDialog(const std::filesystem::path& initialDirectory)
{
    std::optional<std::filesystem::path> resultPath;
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool comInitialized = SUCCEEDED(hr);

    IFileDialog* pfd = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr) && pfd != nullptr) {
        DWORD options = 0;
        if (SUCCEEDED(pfd->GetOptions(&options))) {
            pfd->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        }

        if (!initialDirectory.empty()) {
            IShellItem* psi = nullptr;
            if (SUCCEEDED(SHCreateItemFromParsingName(initialDirectory.wstring().c_str(), nullptr, IID_PPV_ARGS(&psi)))) {
                pfd->SetDefaultFolder(psi);
                psi->Release();
            }
        }

        if (SUCCEEDED(pfd->Show(nullptr))) {
            IShellItem* psiResult = nullptr;
            if (SUCCEEDED(pfd->GetResult(&psiResult)) && psiResult != nullptr) {
                PWSTR pszPath = nullptr;
                if (SUCCEEDED(psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)) && pszPath != nullptr) {
                    resultPath = std::filesystem::path{ pszPath };
                    CoTaskMemFree(pszPath);
                }
                psiResult->Release();
            }
        }

        pfd->Release();
    }

    if (comInitialized) {
        CoUninitialize();
    }

    return resultPath;
}

std::optional<std::filesystem::path> showOpenImageResourceDialog(const std::filesystem::path& initialDirectory)
{
    return showOpenFilteredFileDialog(kImageResourceFilter, initialDirectory);
}

std::optional<std::filesystem::path> showOpenFontResourceDialog(const std::filesystem::path& initialDirectory)
{
    return showOpenFilteredFileDialog(kFontResourceFilter, initialDirectory);
}

std::optional<std::string> showColorPickerDialog(const std::string& initialColor)
{
    static COLORREF customColors[16] = {};

    CHOOSECOLORW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = nullptr;
    dialog.rgbResult = parseInitialColor(initialColor);
    dialog.lpCustColors = customColors;
    dialog.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (!ChooseColorW(&dialog)) {
        return std::nullopt;
    }

    const unsigned int red = GetRValue(dialog.rgbResult);
    const unsigned int green = GetGValue(dialog.rgbResult);
    const unsigned int blue = GetBValue(dialog.rgbResult);

    char buffer[8] = {};
    std::snprintf(buffer, sizeof(buffer), "#%02X%02X%02X", red, green, blue);
    return std::string{ buffer };
}

} // namespace visiform::utils
