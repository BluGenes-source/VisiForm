#include "generator/CMakeEmitter.h"
#include "generator/CMakeEmitter.h"

#include "utils/FileUtils.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace visiform::generator {
namespace {

constexpr const char* kVsWhereVersionRange = "[17.0,18.0)";

std::string sanitizeProjectNameForCMake(const std::string& value)
{
    const std::string source = value.empty() ? std::string{"VisiFormProject"} : value;
    std::string sanitized;
    sanitized.reserve(source.size());
    for (char character : source) {
        if (std::isalnum(static_cast<unsigned char>(character)) != 0) {
            sanitized.push_back(character);
        }
        else if (character == '_' || character == '-' || std::isspace(static_cast<unsigned char>(character)) != 0) {
            sanitized.push_back('_');
        }
    }

    if (sanitized.empty()) {
        return "VisiFormProject";
    }
    if (std::isdigit(static_cast<unsigned char>(sanitized.front())) != 0) {
        sanitized.insert(sanitized.begin(), '_');
    }

    return sanitized;
}

std::string sanitizeExecutableName(const std::string& value, const std::string& fallback)
{
    const std::string source = value.empty() ? fallback : value;
    std::string sanitized;
    sanitized.reserve(source.size());
    for (char character : source) {
        if (std::isalnum(static_cast<unsigned char>(character)) != 0 || character == '_' || character == '-') {
            sanitized.push_back(character);
        }
        else {
            sanitized.push_back('_');
        }
    }

    if (sanitized.empty()) {
        sanitized = fallback.empty() ? std::string{"VisiFormProject"} : fallback;
    }
    if (std::isdigit(static_cast<unsigned char>(sanitized.front())) != 0) {
        sanitized.insert(sanitized.begin(), '_');
    }

    return sanitized;
}

std::string sanitizeDisplayName(const std::string& value)
{
    return value.empty() ? std::string{ "VisiFormProject" } : value;
}

std::string sanitizeClassName(const std::string& value, const std::string& fallback)
{
    std::string sanitized;
    for (char character : value.empty() ? fallback : value) {
        if (std::isalnum(static_cast<unsigned char>(character)) != 0 || character == '_') {
            sanitized.push_back(character);
        }
    }
    if (sanitized.empty()) {
        return fallback;
    }
    if (sanitized == "MainWindow" && fallback != "MainWindow") {
        return fallback;
    }
    return sanitized;
}

std::string escapeCMakeString(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size());
    for (char character : value) {
        if (character == '\\' || character == '"') {
            escaped.push_back('\\');
        }
        escaped.push_back(character);
    }

    return escaped;
}

std::string escapeJsonString(const std::string& value)
{
    std::ostringstream stream;
    for (unsigned char character : value) {
        switch (character) {
        case '\\':
            stream << "\\\\";
            break;
        case '"':
            stream << "\\\"";
            break;
        case '\b':
            stream << "\\b";
            break;
        case '\f':
            stream << "\\f";
            break;
        case '\n':
            stream << "\\n";
            break;
        case '\r':
            stream << "\\r";
            break;
        case '\t':
            stream << "\\t";
            break;
        default:
            stream << static_cast<char>(character);
            break;
        }
    }

    return stream.str();
}

std::string configuredVisageGitRepository(const utils::AppSettings& settings)
{
    return settings.visageGitRepository.empty()
        ? std::string{ utils::AppSettings::defaultVisageGitRepository }
        : settings.visageGitRepository;
}

std::string configuredVisageGitTag(const utils::AppSettings& settings)
{
    return settings.visageGitTag.empty()
        ? std::string{ utils::AppSettings::defaultVisageGitTag }
        : settings.visageGitTag;
}

std::string configuredLocalVisageSourceDirectory(const utils::AppSettings& settings)
{
    return settings.localVisageSourceDirectory.empty()
        ? std::string{}
        : utils::FileUtils::normalizeSeparators(settings.localVisageSourceDirectory.string());
}

void emitGenericNinjaPreset(std::ostringstream& stream,
    const char* presetName,
    const char* displayName,
    const char* binaryDir,
    const char* buildType,
    const std::string& visageGitRepository,
    const std::string& visageGitTag,
    const std::string& localVisageSourceDirectory)
{
    stream << "    {\n";
    stream << "      \"name\": \"" << presetName << "\",\n";
    stream << "      \"displayName\": \"" << displayName << "\",\n";
    stream << "      \"generator\": \"Ninja\",\n";
    stream << "      \"binaryDir\": \"${sourceDir}/" << binaryDir << "\",\n";
    stream << "      \"cacheVariables\": {\n";
    stream << "        \"CMAKE_BUILD_TYPE\": \"" << buildType << "\",\n";
    stream << "        \"VISIFORM_VISAGE_GIT_REPOSITORY\": \"" << escapeJsonString(visageGitRepository) << "\",\n";
    stream << "        \"VISIFORM_VISAGE_GIT_TAG\": \"" << escapeJsonString(visageGitTag) << "\"";
    if (!localVisageSourceDirectory.empty()) {
        stream << ",\n";
        stream << "        \"VISIFORM_VISAGE_SOURCE_DIR\": \"" << escapeJsonString(localVisageSourceDirectory) << "\"\n";
    }
    else {
        stream << "\n";
    }
    stream << "      }\n";
    stream << "    }";
}

std::string emitVsDevCmdBatchScript(const char* cmakeCommand)
{
    std::ostringstream stream;
    stream << "@echo off\r\n";
    stream << "setlocal\r\n\r\n";
    stream << "cd /d \"%~dp0\\..\"\r\n\r\n";
    stream << "set \"VSWHERE=%ProgramFiles(x86)%\\Microsoft Visual Studio\\Installer\\vswhere.exe\"\r\n\r\n";
    stream << "set \"VSWHERE_VERSION_RANGE=" << kVsWhereVersionRange << "\"\r\n\r\n";
    stream << "if not exist \"%VSWHERE%\" (\r\n";
    stream << "    echo ERROR: vswhere.exe not found.\r\n";
    stream << "    echo Install Visual Studio 2022 with Desktop development with C++.\r\n";
    stream << "    exit /b 1\r\n";
    stream << ")\r\n\r\n";
    stream << "set \"VSINSTALL=\"\r\n";
    stream << "for /f \"usebackq tokens=*\" %%i in (`\"%VSWHERE%\" -latest -version \"%VSWHERE_VERSION_RANGE%\" -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (\r\n";
    stream << "    set \"VSINSTALL=%%i\"\r\n";
    stream << ")\r\n\r\n";
    stream << "if \"%VSINSTALL%\"==\"\" (\r\n";
    stream << "    echo ERROR: Visual Studio 2022 C++ tools were not found.\r\n";
    stream << "    echo Install Desktop development with C++.\r\n";
    stream << "    exit /b 1\r\n";
    stream << ")\r\n\r\n";
    stream << "set \"VSDEVCMD=%VSINSTALL%\\Common7\\Tools\\VsDevCmd.bat\"\r\n";
    stream << "if not exist \"%VSDEVCMD%\" (\r\n";
    stream << "    echo ERROR: VsDevCmd.bat not found in the selected Visual Studio installation.\r\n";
    stream << "    exit /b 1\r\n";
    stream << ")\r\n\r\n";
    stream << "call \"%VSDEVCMD%\" -arch=x64 -host_arch=x64\r\n";
    stream << "if errorlevel 1 exit /b 1\r\n\r\n";
    stream << cmakeCommand << "\r\n";
    stream << "if errorlevel 1 exit /b 1\r\n\r\n";
    stream << "endlocal & exit /b 0\r\n";
    return stream.str();
}

std::string emitVsDevCmdPowerShellScript(const char* cmakeCommand)
{
    std::ostringstream stream;
    stream << "$ErrorActionPreference = 'Stop'\n\n";
    stream << "$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path\n";
    stream << "Set-Location (Join-Path $scriptRoot '..')\n\n";
    stream << "$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\\Installer\\vswhere.exe'\n";
    stream << "if (-not (Test-Path $vswhere)) {\n";
    stream << "    Write-Error 'vswhere.exe not found. Install Visual Studio 2022 with Desktop development with C++.'\n";
    stream << "}\n\n";
    stream << "$vswhereVersionRange = '" << kVsWhereVersionRange << "'\n";
    stream << "$vsInstall = & $vswhere -latest -version $vswhereVersionRange -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath\n";
    stream << "if ([string]::IsNullOrWhiteSpace($vsInstall)) {\n";
    stream << "    Write-Error 'Visual Studio 2022 C++ tools were not found. Install Desktop development with C++.'\n";
    stream << "}\n\n";
    stream << "$vsDevCmd = Join-Path $vsInstall 'Common7\\Tools\\VsDevCmd.bat'\n";
    stream << "if (-not (Test-Path $vsDevCmd)) {\n";
    stream << "    Write-Error 'VsDevCmd.bat not found in the selected Visual Studio installation.'\n";
    stream << "}\n\n";
    stream << "$command = \"call `\"$vsDevCmd`\" -arch=x64 -host_arch=x64 && " << cmakeCommand << "\"\n";
    stream << "cmd.exe /c $command\n";
    stream << "if ($LASTEXITCODE -ne 0) {\n";
    stream << "    exit $LASTEXITCODE\n";
    stream << "}\n";
    return stream.str();
}

} // namespace

std::string CMakeEmitter::emitCMakeLists(const model::ProjectDocument& document, const utils::AppSettings& settings) const
{
    const std::string projectName = sanitizeProjectNameForCMake(document.projectName);
    const std::string executableName = sanitizeExecutableName(document.executableName, projectName);
    const std::string baseClassName = "MainWindow";
    const std::string userClassName = sanitizeClassName(document.userSubclassName.empty() ? document.mainFormClassName : document.userSubclassName, "AppMainWindow");
    const std::string visageGitRepository = configuredVisageGitRepository(settings);
    const std::string visageGitTag = configuredVisageGitTag(settings);
    std::ostringstream stream;
    stream << "# Generated by VisiForm - Visage Form Builder.\n";
    stream << "# This file is generated.\n";
    stream << "# Manual changes may be overwritten.\n\n";
    stream << "cmake_minimum_required(VERSION 3.24)\n";
    stream << "project(" << projectName << " VERSION 0.1.0 LANGUAGES CXX)\n\n";
    stream << "set(CMAKE_CXX_STANDARD 20)\n";
    stream << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
    stream << "set(CMAKE_CXX_EXTENSIONS OFF)\n\n";
    stream << "if (MSVC)\n";
    stream << "    set(CMAKE_MSVC_RUNTIME_LIBRARY \"MultiThreaded$<$<CONFIG:Debug>:Debug>\")\n";
    stream << "endif()\n\n";
    stream << "include(FetchContent)\n\n";
    stream << "set(VISIFORM_VISAGE_SOURCE_DIR \"\" CACHE PATH \"Optional local Visage source directory\")\n";
    stream << "set(VISIFORM_VISAGE_GIT_REPOSITORY \"" << escapeCMakeString(visageGitRepository) << "\" CACHE STRING \"Visage Git repository\")\n";
    stream << "set(VISIFORM_VISAGE_GIT_TAG \"" << escapeCMakeString(visageGitTag) << "\" CACHE STRING \"Visage Git tag or commit\")\n\n";
    stream << "set(VISAGE_BUILD_EXAMPLES OFF CACHE BOOL \"\" FORCE)\n";
    stream << "set(VISAGE_BUILD_TESTS OFF CACHE BOOL \"\" FORCE)\n";
    stream << "set(VISAGE_ENABLE_WIDGETS ON CACHE BOOL \"\" FORCE)\n";
    stream << "set(VISAGE_AMALGAMATED_BUILD ON CACHE BOOL \"\" FORCE)\n\n";
    stream << "if (VISIFORM_VISAGE_SOURCE_DIR AND EXISTS \"${VISIFORM_VISAGE_SOURCE_DIR}/CMakeLists.txt\")\n";
    stream << "    message(STATUS \"Using local Visage source: ${VISIFORM_VISAGE_SOURCE_DIR}\")\n\n";
    stream << "    add_subdirectory(\n";
    stream << "        \"${VISIFORM_VISAGE_SOURCE_DIR}\"\n";
    stream << "        \"${CMAKE_BINARY_DIR}/_deps/visage-build\"\n";
    stream << "    )\n";
    stream << "else()\n";
    stream << "    message(STATUS \"Using FetchContent Visage fallback\")\n";
    stream << "    message(STATUS \"Visage repository: ${VISIFORM_VISAGE_GIT_REPOSITORY}\")\n";
    stream << "    message(STATUS \"Visage tag/commit: ${VISIFORM_VISAGE_GIT_TAG}\")\n\n";
    stream << "    FetchContent_Declare(\n";
    stream << "        visage\n";
    stream << "        GIT_REPOSITORY \"${VISIFORM_VISAGE_GIT_REPOSITORY}\"\n";
    stream << "        GIT_TAG \"${VISIFORM_VISAGE_GIT_TAG}\"\n";
    stream << "        GIT_SHALLOW TRUE\n";
    stream << "    )\n\n";
    stream << "    FetchContent_MakeAvailable(visage)\n";
    stream << "endif()\n\n";
    stream << "add_executable(" << executableName << "\n";
    stream << "    src/main.cpp\n";
    stream << "    src/" << baseClassName << ".cpp\n";
    stream << "    src/" << userClassName << ".cpp\n";
    stream << ")\n\n";
    stream << "target_include_directories(" << executableName << " PRIVATE src)\n\n";
    stream << "target_link_libraries(" << executableName << " PRIVATE visage)\n\n";
    stream << "if (MSVC)\n";
    stream << "    target_compile_options(" << executableName << " PRIVATE /W4 /permissive-)\n";
    stream << "endif()\n\n";
    stream << "# TODO: Generated project may later support vcpkg manifest mode if needed.\n";
    return stream.str();
}

std::string CMakeEmitter::emitCMakePresets(const utils::AppSettings& settings) const
{
    const std::string localVisageSourceDirectory = configuredLocalVisageSourceDirectory(settings);
    const std::string visageGitRepository = configuredVisageGitRepository(settings);
    const std::string visageGitTag = configuredVisageGitTag(settings);
    std::ostringstream stream;
    stream << "{\n";
    stream << "  \"version\": 3,\n";
    stream << "  \"configurePresets\": [\n";
    stream << "    {\n";
    stream << "      \"name\": \"vs2022-x64-static-debug\",\n";
    stream << "      \"displayName\": \"VS2022 x64 Static Debug\",\n";
    stream << "      \"generator\": \"Ninja\",\n";
    stream << "      \"binaryDir\": \"${sourceDir}/build/vs2022-x64-static-debug\",\n";
    stream << "      \"cacheVariables\": {\n";
    stream << "        \"CMAKE_BUILD_TYPE\": \"Debug\",\n";
    stream << "        \"CMAKE_MSVC_RUNTIME_LIBRARY\": \"MultiThreadedDebug\",\n";
    stream << "        \"VISIFORM_VISAGE_GIT_REPOSITORY\": \"" << escapeJsonString(visageGitRepository) << "\",\n";
    stream << "        \"VISIFORM_VISAGE_GIT_TAG\": \"" << escapeJsonString(visageGitTag) << "\"";
    if (!localVisageSourceDirectory.empty()) {
        stream << ",\n";
        stream << "        \"VISIFORM_VISAGE_SOURCE_DIR\": \"" << escapeJsonString(localVisageSourceDirectory) << "\"\n";
    }
    else {
        stream << "\n";
    }
    stream << "      }\n";
    stream << "    },\n";
    stream << "    {\n";
    stream << "      \"name\": \"vs2022-x64-static-release\",\n";
    stream << "      \"displayName\": \"VS2022 x64 Static Release\",\n";
    stream << "      \"generator\": \"Ninja\",\n";
    stream << "      \"binaryDir\": \"${sourceDir}/build/vs2022-x64-static-release\",\n";
    stream << "      \"cacheVariables\": {\n";
    stream << "        \"CMAKE_BUILD_TYPE\": \"Release\",\n";
    stream << "        \"CMAKE_MSVC_RUNTIME_LIBRARY\": \"MultiThreaded\",\n";
    stream << "        \"VISIFORM_VISAGE_GIT_REPOSITORY\": \"" << escapeJsonString(visageGitRepository) << "\",\n";
    stream << "        \"VISIFORM_VISAGE_GIT_TAG\": \"" << escapeJsonString(visageGitTag) << "\"";
    if (!localVisageSourceDirectory.empty()) {
        stream << ",\n";
        stream << "        \"VISIFORM_VISAGE_SOURCE_DIR\": \"" << escapeJsonString(localVisageSourceDirectory) << "\"\n";
    }
    else {
        stream << "\n";
    }
    stream << "      }\n";
    stream << "    }\n";
    stream << "  ],\n";
    stream << "  \"buildPresets\": [\n";
    stream << "    {\n";
    stream << "      \"name\": \"build-static-debug\",\n";
    stream << "      \"displayName\": \"Build Static Debug\",\n";
    stream << "      \"configurePreset\": \"vs2022-x64-static-debug\"\n";
    stream << "    },\n";
    stream << "    {\n";
    stream << "      \"name\": \"build-static-release\",\n";
    stream << "      \"displayName\": \"Build Static Release\",\n";
    stream << "      \"configurePreset\": \"vs2022-x64-static-release\"\n";
    stream << "    }\n";
    stream << "  ]\n";
    stream << "}\n";
    return stream.str();
}

std::string CMakeEmitter::emitReadme(const model::ProjectDocument& document, const utils::AppSettings& settings) const
{
    const std::string projectName = sanitizeDisplayName(document.projectName);
    const std::string executableName = sanitizeExecutableName(document.executableName, sanitizeProjectNameForCMake(document.projectName));
    const std::string baseClassName = "MainWindow";
    const std::string userClassName = sanitizeClassName(document.userSubclassName.empty() ? document.mainFormClassName : document.userSubclassName, "AppMainWindow");
    const std::string localVisageSourceDirectory = configuredLocalVisageSourceDirectory(settings);
    std::ostringstream stream;
    stream << "# " << projectName << "\n\n";
    stream << "Generated by VisiForm - Visage Form Builder.\n\n";
    stream << "## Requirements\n\n";
    stream << "- Windows 10/11 with Visual Studio 2022 for the primary tested workflow\n";
    stream << "- macOS or Linux with Clang or GCC for contributor builds\n";
    stream << "- CMake\n";
    stream << "- Ninja\n";
    stream << "- Git\n\n";
    stream << "## Platform status\n\n";
    stream << "| Platform | Status | Notes |\n";
    stream << "| --- | --- | --- |\n";
    stream << "| Windows 10/11 | Primary supported | Developed and tested with Visual Studio 2022. |\n";
    stream << "| macOS | Experimental | Contributor build path depends on Visage platform support and local toolchain setup. |\n";
    stream << "| Linux | Experimental | Contributor build path depends on Visage platform support and local toolchain setup. |\n\n";
    stream << "## Visage dependency modes\n\n";
    stream << "### Fast local source mode\n\n";
    stream << "Set `VISIFORM_VISAGE_SOURCE_DIR` to a local Visage checkout, for example `C:/dev/visage` on Windows or `$HOME/dev/visage` on macOS/Linux.\n\n";
    if (!localVisageSourceDirectory.empty()) {
        stream << "This export was generated with `VISIFORM_VISAGE_SOURCE_DIR` configured in `CMakePresets.json` as `" << localVisageSourceDirectory << "`.\n\n";
    }
    stream << "### Portable fallback\n\n";
    stream << "If `VISIFORM_VISAGE_SOURCE_DIR` is empty or does not point to a valid Visage source tree, CMake falls back to `FetchContent` using the configured repository and tag values.\n\n";
    stream << "You can set or override `VISIFORM_VISAGE_SOURCE_DIR` in `CMakePresets.json`, `CMakeUserPresets.json`, or on the CMake command line.\n\n";
    stream << "## Build workflows\n\n";
    stream << "The generated presets continue to use `Ninja`. Windows helper scripts remain the main tested workflow. The generic `ninja-debug` and `ninja-release` presets can also be used on macOS or Linux when the required compiler, Ninja, CMake, and Visage dependencies are available.\n\n";
    stream << "## Open in Visual Studio\n\n";
    stream << "1. Open `Visual Studio 2022`.\n";
    stream << "2. Use `File > Open > Folder` and choose this exported project directory.\n";
    stream << "3. Select the `vs2022-x64-static-debug` or `vs2022-x64-static-release` preset in Visual Studio.\n";
    stream << "4. Build and run from the IDE.\n\n";
    stream << "Visual Studio loads the required MSVC environment for the `Ninja` presets automatically when the folder is opened.\n\n";
    stream << "## x64 Native Tools Command Prompt\n\n";
    stream << "Open `x64 Native Tools Command Prompt for VS 2022`, then run:\n\n";
    stream << "- Debug configure: `cmake --preset vs2022-x64-static-debug`\n";
    stream << "- Debug build: `cmake --build --preset build-static-debug`\n";
    stream << "- Release configure: `cmake --preset vs2022-x64-static-release`\n";
    stream << "- Release build: `cmake --build --preset build-static-release`\n\n";
    stream << "## Normal PowerShell\n\n";
    stream << "Use the generated helper scripts from a normal PowerShell or Command Prompt window. They locate Visual Studio with `vswhere`, call `VsDevCmd.bat` for `x64`, switch to the generated project root, and then invoke CMake.\n\n";
    stream << "Required `.cmd` scripts:\n\n";
    stream << "- `scripts\\configure_static_debug.cmd`\n";
    stream << "- `scripts\\build_static_debug.cmd`\n";
    stream << "- `scripts\\configure_static_release.cmd`\n";
    stream << "- `scripts\\build_static_release.cmd`\n\n";
    stream << "Optional `.ps1` wrappers:\n\n";
    stream << "- `scripts/configure_static_debug.ps1`\n";
    stream << "- `scripts/build_static_debug.ps1`\n";
    stream << "- `scripts/configure_static_release.ps1`\n";
    stream << "- `scripts/build_static_release.ps1`\n\n";
    stream << "## Direct command usage notes\n\n";
    stream << "If you want to run `cmake --preset ...` directly from PowerShell, load `VsDevCmd.bat` first or use the `x64 Native Tools Command Prompt`. The helper scripts exist to make that setup automatic.\n\n";
    stream << "On macOS or Linux, use the generic Ninja presets or direct `cmake -S . -B ... -G Ninja` commands. Those builds are not validated by this export step and may require additional platform packages from Visage.\n\n";
    stream << "## Generated source structure\n\n";
    stream << "- `src/" << baseClassName << ".*` is regenerated by VisiForm\n";
    stream << "- `src/" << userClassName << ".*` is the user subclass layer\n";
    stream << "- Put custom logic in `src/" << userClassName << ".cpp` USER CODE regions\n";
    if (!document.resources.empty()) {
        stream << "- Managed assets are copied into the generated `assets/` folders during export\n";
    }
    stream << "\n";

    if (!document.resources.empty()) {
        stream << "## Exported asset folders\n\n";
        stream << "- `assets/images/` for managed image resources\n";
        stream << "- `assets/fonts/` for managed font resources\n";
        stream << "- `assets/icons/` for managed icon resources\n";
        stream << "- `assets/themes/` for managed theme resources\n\n";
        stream << "Image widgets use managed `resourceId` values when assigned, and fall back to direct `imagePath` values only when no managed resource is selected.\n";
        stream << "Generated image placeholders use relative asset paths such as `assets/images/logo.png`.\n\n";
    }
    stream << "## Generated executable\n\n";
    stream << "- CMake project name: `" << sanitizeProjectNameForCMake(document.projectName) << "`\n";
    stream << "- Executable target name: `" << executableName << "`\n";
    stream << "- Produced executable: `" << executableName << ".exe` on Windows\n\n";
    stream << "## Debug and release notes\n\n";
    stream << "- Debug uses the `vs2022-x64-static-debug` configure preset and `build-static-debug` build preset\n";
    stream << "- Release uses the `vs2022-x64-static-release` configure preset and `build-static-release` build preset\n";
    stream << "- Contributor-oriented cross-platform builds can use `ninja-debug`, `build-ninja-debug`, `ninja-release`, and `build-ninja-release`\n\n";
    stream << "## Known limitations\n\n";
    stream << "- Generated UI interaction is intentionally lightweight and not a full retained-mode widget toolkit\n";
    stream << "- Runtime theme switching is future work\n";
    stream << "- Color picker interaction is still minimal\n";
    stream << "- No generated native layout manager yet\n";
    stream << "- Visage commit is not pinned yet\n";
    stream << "- macOS and Linux builds should be treated as contributor paths until verified on those platforms\n";
    return stream.str();
}

std::string CMakeEmitter::emitGitIgnore() const
{
    return
        "build/\n"
        "out/\n"
        ".vs/\n"
        "CMakeCache.txt\n"
        "CMakeFiles/\n"
        "cmake_install.cmake\n"
        "compile_commands.json\n"
        "logs/\n"
        "*.log\n"
        "*.user\n"
        "*.suo\n"
        ".DS_Store\n"
        "Thumbs.db\n";
}

std::string CMakeEmitter::emitConfigureScript(bool release) const
{
    const char* presetName = release ? "vs2022-x64-static-release" : "vs2022-x64-static-debug";
    std::ostringstream command;
    command << "cmake --preset " << presetName;
    return emitVsDevCmdBatchScript(command.str().c_str());
}

std::string CMakeEmitter::emitBuildScript(bool release) const
{
    const char* presetName = release ? "build-static-release" : "build-static-debug";
    std::ostringstream command;
    command << "cmake --build --preset " << presetName;
    return emitVsDevCmdBatchScript(command.str().c_str());
}

std::string CMakeEmitter::emitConfigurePowerShellScript(bool release) const
{
    const char* presetName = release ? "vs2022-x64-static-release" : "vs2022-x64-static-debug";
    std::ostringstream command;
    command << "cmake --preset " << presetName;
    return emitVsDevCmdPowerShellScript(command.str().c_str());
}

std::string CMakeEmitter::emitBuildPowerShellScript(bool release) const
{
    const char* presetName = release ? "build-static-release" : "build-static-debug";
    std::ostringstream command;
    command << "cmake --build --preset " << presetName;
    return emitVsDevCmdPowerShellScript(command.str().c_str());
}

} // namespace visiform::generator
