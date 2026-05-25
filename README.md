# VisiForm

*A C++ / Visage UI form builder that generates Visage-based C++ projects.*

## What VisiForm does

`VisiForm` is a Windows-focused form builder inspired by tools like `wxFormBuilder`.
It uses the `Visage` graphics/UI library to let you design UI forms visually, save those projects as `.vfb.json`, validate the in-memory project model, and export complete generated C++ projects.

Exported projects are intended to open cleanly in `Visual Studio 2022` and include:

- generated `MainWindow` base class files
- generated user subclass files
- `USER CODE BEGIN` / `USER CODE END` preservation blocks
- generated CMake presets and helper scripts
- optional local `Visage` source support with `FetchContent` fallback

## Current status

Current major features include:

- Widget Palette
- Designer Canvas
- Property Inspector
- Project Tree
- Resource Manager
- Menu bar
- Validation
- Modal dialogs
- Save/load `.vfb.json`
- Export generated C++ projects
- Interactive generated widgets
- Local `Visage` dependency support

## OS requirements

- Primary supported OS: `Windows 10` or `Windows 11`, 64-bit
- Primary IDE: `Visual Studio 2022`
- The project is currently developed and tested on Windows
- Other platforms may be possible later, but they are not the primary target yet

## Required software

- `Visual Studio 2022` Community, Professional, or Enterprise
  - workload: `Desktop development with C++`
  - `MSVC v143` toolset
  - `Windows 10/11 SDK`
  - `CMake tools for Windows` if you want to use Visual Studio's bundled CMake support
- `Git`
- `CMake 3.24` or newer
- `Ninja`
- `vcpkg`
- Recommended local `Visage` source clone: `J:\Dev\CeePlusPlus\visage`

`VisiForm` uses static runtime settings.
The configured `vcpkg` triplet is `x64-windows-static`.
Current runtime settings are:

- Debug: `MultiThreadedDebug` / `/MTd`
- Release: `MultiThreaded` / `/MT`

## Suggested `winget` installs

```powershell
winget install --id Git.Git -e
winget install --id Kitware.CMake -e
winget install --id Ninja-build.Ninja -e
winget install --id Microsoft.VisualStudio.2022.Community -e
```

After installing Visual Studio, make sure the `Desktop development with C++` workload is selected in the Visual Studio Installer.
You can also install Visual Studio manually instead of using `winget`.

## Recommended development folder layout

```text
J:\Dev\CeePlusPlus\VisiForm
J:\Dev\CeePlusPlus\visage
J:\Dev\vcpkg
```

Recommended layout notes:

- the `VisiForm` repository is separate from `Visage`
- `Visage` is commonly used as a local source dependency during development
- `vcpkg` provides third-party packages used by the repository
- `CMakeUserPresets.json` stores machine-specific paths and should stay local

## Clone VisiForm

Replace the placeholder URL with your actual repository URL:

```powershell
git clone <your-visiform-repo-url> J:\Dev\CeePlusPlus\VisiForm
```

## Clone Visage

```powershell
git clone https://github.com/VitalAudio/visage.git J:\Dev\CeePlusPlus\visage
```

## `vcpkg` setup

Clone and bootstrap `vcpkg`:

```powershell
git clone https://github.com/microsoft/vcpkg.git J:\Dev\vcpkg
J:\Dev\vcpkg\bootstrap-vcpkg.bat
```

Set `VCPKG_ROOT` to the same location:

```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "J:\Dev\vcpkg", "User")
```

After setting `VCPKG_ROOT`, restart your terminal and `Visual Studio 2022` so the environment variable is visible to CMake and `vcpkg` integration.

## `CMakePresets.json`

`CMakePresets.json` is committed to the repository.
It contains the shared base presets used for the main app:

- `vs2022-x64-static-debug`
- `vs2022-x64-static-release`
- `build-static-debug`
- `build-static-release`

These presets use `Ninja`, keep the static runtime settings, and use the repository's `x64-windows-static` `vcpkg` configuration.

## `CMakeUserPresets.json`

`CMakeUserPresets.json` is local-only.
Do not commit it.
Use it to store machine-specific values such as your local `Visage` source path.

Example `CMakeUserPresets.json`:

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "vs2022-x64-static-debug-local-visage",
      "displayName": "VS2022 x64 Static Debug - Local Visage",
      "inherits": "vs2022-x64-static-debug",
      "cacheVariables": {
        "VISIFORM_VISAGE_SOURCE_DIR": "J:/Dev/CeePlusPlus/visage",
        "VISIFORM_VISAGE_GIT_TAG": "main"
      }
    },
    {
      "name": "vs2022-x64-static-release-local-visage",
      "displayName": "VS2022 x64 Static Release - Local Visage",
      "inherits": "vs2022-x64-static-release",
      "cacheVariables": {
        "VISIFORM_VISAGE_SOURCE_DIR": "J:/Dev/CeePlusPlus/visage",
        "VISIFORM_VISAGE_GIT_TAG": "main"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "build-static-debug-local-visage",
      "configurePreset": "vs2022-x64-static-debug-local-visage"
    },
    {
      "name": "build-static-release-local-visage",
      "configurePreset": "vs2022-x64-static-release-local-visage"
    }
  ]
}
```

If you prefer to let the project download `Visage` through `FetchContent`, you can skip the local-user presets and use the shared committed presets directly.

## `.gitignore` note

`CMakeUserPresets.json` should be ignored by Git because it contains machine-specific paths.
The repository `.gitignore` already includes this ignore rule.

## Build from Visual Studio 2022

Recommended `Visual Studio 2022` workflow:

1. Open `Visual Studio 2022`.
2. Select `File > Open > Folder`.
3. Open `J:\Dev\CeePlusPlus\VisiForm`.
4. Select the CMake preset `vs2022-x64-static-debug-local-visage`.
5. If needed, use `Project > Delete Cache and Reconfigure`.
6. Use `Build > Build All`.
7. Select `VisiForm.exe` as the startup item if needed.
8. Run with the green button.

If you are not using a local `Visage` checkout, select the committed `vs2022-x64-static-debug` preset instead.

## If the startup target is missing

If `VisiForm.exe` does not appear as the active startup item:

- open `Solution Explorer > CMake Targets`
- right-click `VisiForm.exe`
- choose `Set as Startup Item`

## Build from command line

Use the `x64 Native Tools Command Prompt for VS 2022` for the smoothest command-line workflow.

Debug:

```bat
cd /d J:\Dev\CeePlusPlus\VisiForm
cmake --preset vs2022-x64-static-debug-local-visage
cmake --build --preset build-static-debug-local-visage
```

Release:

```bat
cd /d J:\Dev\CeePlusPlus\VisiForm
cmake --preset vs2022-x64-static-release-local-visage
cmake --build --preset build-static-release-local-visage
```

If you are not using a local `Visage` checkout, use the committed base presets instead of the `-local-visage` presets.

## Normal PowerShell warning

Because the project uses `Ninja`, the Visual Studio C++ compiler environment must already be available.
A normal PowerShell session may fail with an error like:

```text
No CMAKE_CXX_COMPILER could be found.
```

Use one of these instead:

- `Visual Studio 2022`
- `x64 Native Tools Command Prompt for VS 2022`
- scripts that call `VsDevCmd` before running CMake when such scripts are available

## Running VisiForm

Run the app from the `Visual Studio 2022` green button after a successful build.
In Debug builds, a console window may also appear depending on build settings.
The GUI window is the actual `VisiForm` application.

## Console window note

- a console window may appear in Debug builds
- it is useful for logs and diagnostics
- Release builds can hide it later with `WIN32_EXECUTABLE` or another CMake option if that becomes desirable

## Export workflow

`VisiForm` project files are saved as `.vfb.json`.
To generate a standalone C++ app:

1. Create or open a `.vfb.json` project.
2. Use the `File` or `Export` menu to export.
3. Select the export folder.

The generated project currently includes files such as:

- `CMakeLists.txt`
- `CMakePresets.json`
- `README.md`
- `scripts/`
- `src/main.cpp`
- `src/MainWindow.h`
- `src/MainWindow.cpp`
- `src/<UserSubclassName>.h`
- `src/<UserSubclassName>.cpp`
- `assets/` when resources exist

The generated base class keeps the `MainWindow` naming rule, while the user-edit layer is emitted through the configured user subclass files.
Generated `USER CODE` blocks are preserved across re-export for recognized handler regions.

## Generated project build

After export, you can build the generated project by:

- opening the exported folder in `Visual Studio 2022`
- selecting the generated Debug or Release preset
- building and running from the IDE

You can also use the generated helper scripts in the exported `scripts/` folder.

## Local Visage in exports

`VisiForm` can write `VISIFORM_VISAGE_SOURCE_DIR` into the exported project's `CMakePresets.json`.
That allows exported projects to reuse a local `Visage` checkout instead of downloading the dependency each time.

If the local path is empty or invalid, the exported project falls back to `FetchContent` using the configured repository and tag.

## VisiForm project files

- `VisiForm` projects are saved as `.vfb.json`
- the default project folder may be `Generated/Projects`
- exported generated C++ projects are separate from the `.vfb.json` source project files

## Common troubleshooting

### CMake error: Unrecognized `version` field

Use `CMakePresets` version `3` support or update your CMake installation.

### No `CMAKE_CXX_COMPILER` could be found

Use `Visual Studio 2022` or the `x64 Native Tools Command Prompt for VS 2022` so the MSVC toolchain is available.

### `vcpkg` toolchain not found

Check that `VCPKG_ROOT` is set correctly and that `${env:VCPKG_ROOT}` points to a valid `vcpkg` checkout.

### `Ninja` not found

Install `Ninja` and make sure it is available in `PATH`.

### Visual Studio instance not found by `vcpkg`

Install the `Desktop development with C++` workload in `Visual Studio 2022`.

### `Visage` downloads every time

Configure `VISIFORM_VISAGE_SOURCE_DIR` in your local preset so the project can use a local `Visage` checkout.

### Generated project uses `FetchContent`

Check the generated `CMakePresets.json` and confirm that `VISIFORM_VISAGE_SOURCE_DIR` is present and points to a valid `Visage` source tree.

### Duplicate console window

Debug builds may create a console window in addition to the main GUI window.

## Documentation index

Additional repository documentation:

- `docs/code_generation.md`
- `docs/project_validation.md`
- `docs/resources.md`
- `docs/menu_bar.md`
- `docs/new_project_wizard.md`
- `docs/widget_catalog.md`
- `docs/widget_registry.md`
- `docs/copilot_rules.md`
