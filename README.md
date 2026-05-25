# VisiForm
# VisiForm

*A C++ / Visage UI form builder that generates Visage-based C++ projects.*

## What VisiForm does

`VisiForm` is a form builder inspired by tools like `wxFormBuilder`.
It uses the `Visage` graphics and UI library to let you design forms visually, save projects as `.vfb.json`, validate the in-memory project model, and export complete generated C++ projects.

Exported projects include:

- generated `MainWindow` base class files
- generated user subclass files
- `USER CODE BEGIN` / `USER CODE END` preservation blocks
- generated `CMakeLists.txt`, `CMakePresets.json`, and helper scripts
- optional local `Visage` source support with `FetchContent` fallback

## Platform status

| Platform | Status | Notes |
| --- | --- | --- |
| Windows 10/11 | Primary supported | Developed and tested with `Visual Studio 2022`. |
| macOS | Experimental/build instructions provided | Requires `Clang`, `CMake`, `Ninja`, `vcpkg`, `Visage` platform support, and additional native dialog work. |
| Linux | Experimental/build instructions provided | Requires `GCC` or `Clang`, `CMake`, `Ninja`, `vcpkg`, `Visage` platform support, and additional native dialog work. |

Windows is the primary tested platform.
macOS and Linux instructions are provided for contributors, but may require additional native dialog or platform package work.
These non-Windows build paths were not verified in this Windows agent environment, so this README should not be treated as a claim of full production support there.

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

## Core build requirements

- `CMake 3.24` or newer
- `Ninja`
- `Git`
- `vcpkg`
- local `Visage` source checkout recommended for faster iteration
- C++20-capable compiler toolchain

Current repository dependencies from `vcpkg.json` include:

- `nlohmann_json`
- `fmt`
- `spdlog`
- `Catch2`

The primary Windows presets keep the static runtime strategy and use the `x64-windows-static` triplet.
That Windows triplet does not apply to macOS or Linux.

## Clone the repositories

Repository URL:

- `https://github.com/BluGenes-source/VisiForm.git`

### Windows PowerShell example

```powershell
git clone https://github.com/BluGenes-source/VisiForm.git $env:USERPROFILE\dev\VisiForm
git clone https://github.com/VitalAudio/visage.git $env:USERPROFILE\dev\visage
```

### macOS or Linux example

```bash
git clone https://github.com/BluGenes-source/VisiForm.git ~/dev/VisiForm
git clone https://github.com/VitalAudio/visage.git ~/dev/visage
```

## `vcpkg` setup

### Windows

Clone and bootstrap `vcpkg`:

```powershell
git clone https://github.com/microsoft/vcpkg.git $env:USERPROFILE\dev\vcpkg
& $env:USERPROFILE\dev\vcpkg\bootstrap-vcpkg.bat
```

Set `VCPKG_ROOT`:

```powershell
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "$env:USERPROFILE/dev/vcpkg", "User")
```

Restart your terminal and `Visual Studio 2022` after setting `VCPKG_ROOT`.

The committed Windows presets use:

- `CMAKE_TOOLCHAIN_FILE=$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake`
- `VCPKG_TARGET_TRIPLET=x64-windows-static`

### macOS and Linux

Clone and bootstrap `vcpkg`:

```bash
git clone https://github.com/microsoft/vcpkg.git ~/dev/vcpkg
~/dev/vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=~/dev/vcpkg
```

Use the vcpkg toolchain when configuring CMake:

```bash
-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
```

Common non-Windows triplets depend on your platform:

- Linux: `x64-linux`
- macOS Intel: `x64-osx`
- macOS Apple Silicon: `arm64-osx`

If needed, pass the triplet explicitly with `-DVCPKG_TARGET_TRIPLET=<triplet>`.

## `CMakePresets.json`

Committed repository presets include:

- Windows primary presets
  - `vs2022-x64-static-debug`
  - `vs2022-x64-static-release`
  - `build-static-debug`
  - `build-static-release`
- Generic Ninja presets
  - `ninja-debug`
  - `ninja-release`
  - `build-ninja-debug`
  - `build-ninja-release`

The Windows presets keep the current static MSVC runtime and `x64-windows-static` configuration.
The generic Ninja presets avoid a forced Windows triplet and avoid user-specific `Ninja` paths.

## `CMakeUserPresets.json`

`CMakeUserPresets.json` is local-only and should not be committed.
Use it for machine-specific values such as `VISIFORM_VISAGE_SOURCE_DIR`.

Example Windows local preset file:

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "vs2022-x64-static-debug-local-visage",
      "inherits": "vs2022-x64-static-debug",
      "cacheVariables": {
        "VISIFORM_VISAGE_SOURCE_DIR": "C:/dev/visage",
        "VISIFORM_VISAGE_GIT_TAG": "main"
      }
    },
    {
      "name": "vs2022-x64-static-release-local-visage",
      "inherits": "vs2022-x64-static-release",
      "cacheVariables": {
        "VISIFORM_VISAGE_SOURCE_DIR": "C:/dev/visage",
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

On macOS or Linux you can use the same `VISIFORM_VISAGE_SOURCE_DIR` variable in a local preset, or pass it directly on the command line.

## `.gitignore`

`CMakeUserPresets.json` is ignored by the repository because it stores machine-specific paths.

## Windows 10/11 build instructions

### Required software

- `Visual Studio 2022`
  - workload: `Desktop development with C++`
  - `MSVC v143` toolset
  - `Windows 10/11 SDK`
- `Git`
- `CMake`
- `Ninja`
- `vcpkg`
- optional local `Visage` checkout

### Suggested `winget` installs

```powershell
winget install --id Git.Git -e
winget install --id Kitware.CMake -e
winget install --id Ninja-build.Ninja -e
winget install --id Microsoft.VisualStudio.2022.Community -e
```

### Build from `Visual Studio 2022`

1. Open `Visual Studio 2022`.
2. Select `File > Open > Folder`.
3. Open your local `VisiForm` repository folder.
4. Select `vs2022-x64-static-debug` or your local `vs2022-x64-static-debug-local-visage` preset.
5. If needed, use `Project > Delete Cache and Reconfigure`.
6. Use `Build > Build All`.
7. If needed, set `VisiForm.exe` as the startup item from `Solution Explorer > CMake Targets`.
8. Run manually from Visual Studio when you want to test the app.

### Build from `x64 Native Tools Command Prompt for VS 2022`

Debug:

```bat
cd /d %USERPROFILE%\dev\VisiForm
cmake --preset vs2022-x64-static-debug
cmake --build --preset build-static-debug
```

Release:

```bat
cd /d %USERPROFILE%\dev\VisiForm
cmake --preset vs2022-x64-static-release
cmake --build --preset build-static-release
```

If you created local-user presets for a local `Visage` checkout, use the `-local-visage` preset names instead.

### Normal PowerShell warning

Because the project uses `Ninja`, the MSVC compiler environment must already be available.
A normal PowerShell session may fail with:

```text
No CMAKE_CXX_COMPILER could be found.
```

Use one of these instead:

- `Visual Studio 2022`
- `x64 Native Tools Command Prompt for VS 2022`
- scripts that call `VsDevCmd` before invoking CMake

## macOS build instructions

macOS contributor builds depend on `Visage` platform support and local platform packages.
If native file dialogs are not implemented on macOS yet, some open and save workflows may be unavailable until that work is completed.

### Prerequisites

- Xcode Command Line Tools
- `Clang`
- `CMake`
- `Ninja`
- `Git`
- `vcpkg`
- optional local `Visage` checkout

Install command line tools:

```bash
xcode-select --install
```

Optional `Homebrew` installs:

```bash
brew install cmake ninja git
```

### Configure and build

Debug:

```bash
cd ~/dev/VisiForm
cmake -S . -B build/macos-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVISIFORM_VISAGE_SOURCE_DIR=$HOME/dev/visage

cmake --build build/macos-debug
```

Release:

```bash
cd ~/dev/VisiForm
cmake -S . -B build/macos-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVISIFORM_VISAGE_SOURCE_DIR=$HOME/dev/visage

cmake --build build/macos-release
```

If you need an explicit vcpkg triplet, add one such as `-DVCPKG_TARGET_TRIPLET=arm64-osx` or `-DVCPKG_TARGET_TRIPLET=x64-osx`.

## Linux build instructions

Linux contributor builds depend on `Visage` platform support and the required graphics or windowing packages for your distro.
If native file dialogs are not implemented on Linux yet, some open and save workflows may be unavailable until that work is completed.

### Ubuntu or Debian example prerequisites

```bash
sudo apt update
sudo apt install -y build-essential git cmake ninja-build pkg-config
```

You may also choose `Clang` instead of `GCC`.
If `Visage` reports additional missing platform packages, install the required graphics and windowing dependencies documented by `Visage`.

### Configure and build

Debug:

```bash
cd ~/dev/VisiForm
cmake -S . -B build/linux-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVISIFORM_VISAGE_SOURCE_DIR=$HOME/dev/visage

cmake --build build/linux-debug
```

Release:

```bash
cd ~/dev/VisiForm
cmake -S . -B build/linux-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVISIFORM_VISAGE_SOURCE_DIR=$HOME/dev/visage

cmake --build build/linux-release
```

If you need an explicit vcpkg triplet, add `-DVCPKG_TARGET_TRIPLET=x64-linux`.

## Running `VisiForm`

On Windows, run the app manually from `Visual Studio 2022` after a successful build.
In Debug builds, a console window may also appear for logs.

macOS and Linux runtime behavior has not been validated in this environment.
Even if those builds configure successfully, final runtime support depends on `Visage` and the remaining native dialog work.

## Export workflow

`VisiForm` project files are saved as `.vfb.json`.
To generate a standalone C++ app:

1. Create or open a `.vfb.json` project.
2. Use the `File` or `Export` menu to export.
3. Select the export folder.

Generated projects currently include files such as:

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

## Generated project build notes

Exported projects use similar CMake logic:

- optional `VISIFORM_VISAGE_SOURCE_DIR` local source support
- `FetchContent` fallback for `Visage`
- Windows-specific static presets for the main tested path
- generic Ninja presets for contributor-oriented cross-platform builds

Windows generated-project builds are the primary tested export workflow.
macOS and Linux generated-project builds depend on the same `Visage` platform support and toolchain availability as the main repository.

## Local `Visage` support

`VISIFORM_VISAGE_SOURCE_DIR` can be used in both the main repository and exported projects.
If the path is empty or invalid, CMake falls back to `FetchContent` using the configured repository and tag.

## Project files

- `VisiForm` projects are saved as `.vfb.json`
- the default project folder may be `Generated/Projects`
- exported generated C++ projects are separate from the `.vfb.json` source project files

## Troubleshooting

### `CMake` error: unrecognized preset `version`

Use a `CMake` release that supports preset `version` `3`.

### Windows: no `CMAKE_CXX_COMPILER` could be found

Use `Visual Studio 2022` or the `x64 Native Tools Command Prompt for VS 2022` so the MSVC toolchain is available.

### macOS: `xcrun` error or compiler not found

Run:

```bash
xcode-select --install
```

### Linux: compiler not found

Install `build-essential` or `clang`.

### `Ninja` not found

Install `Ninja` and make sure it is available in `PATH`.
On Ubuntu or Debian the package name is commonly `ninja-build`.

### `vcpkg` toolchain or packages fail

Verify:

- `VCPKG_ROOT` points to a valid `vcpkg` checkout
- `CMAKE_TOOLCHAIN_FILE` points to `$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake`
- your selected triplet matches the current platform

### Visual Studio instance not found by `vcpkg`

Install the `Desktop development with C++` workload in `Visual Studio 2022`.

### `Visage` downloads every time

Set `VISIFORM_VISAGE_SOURCE_DIR` to a valid local `Visage` checkout.

### Native file dialogs are missing on macOS or Linux

Non-Windows dialog support is currently limited.
Some project, export, image, font, or color-picker workflows may remain unavailable until native dialog implementations are added.

### `Visage` platform dependency errors

Install the graphics and windowing packages required by `Visage` for your platform.
If platform-specific packages are missing, use `Visage` documentation for the required dependency list.

### Duplicate console window in Debug

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
