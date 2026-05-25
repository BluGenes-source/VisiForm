# Phase 65 - Cross-platform build README and portability audit plan

## Phase title

Cross-platform build README and portability audit.

## Current README problem

`README.md` is Windows-focused, contains owner-specific local path examples, and does not yet provide contributor build guidance for macOS or Linux. The repository also needs a source and CMake audit so new platform instructions do not overstate support.

## Cross-platform build risk areas

- `src/ui/MainWindow.cpp` currently includes `<windows.h>` and uses `GetKeyState` for additive canvas selection.
- `src/utils/NativeFileDialogs.cpp` currently includes Win32 dialog headers directly and has no non-Windows fallback implementation.
- `src/utils/AppSettings.cpp` currently uses `_dupenv_s` with `APPDATA`, which is MSVC-oriented and Windows-specific.
- `src/utils/RecentFiles.cpp` also uses `_dupenv_s` with `LOCALAPPDATA`, which is MSVC-oriented and Windows-specific.
- `CMakePresets.json` currently hard-codes `CMAKE_MAKE_PROGRAM` to a user-specific Windows Ninja path.
- `README.md` currently documents Windows-only presets and `x64-windows-static` as if they apply generally.
- Exported project documentation in `src/generator/CMakeEmitter.cpp` and `docs/code_generation.md` should avoid reducing portability or making unverified claims.
- macOS and Linux builds cannot be validated in this Windows agent environment, so documentation must clearly state that status.

## Files to inspect

- `README.md`
- `CMakeLists.txt`
- `CMakePresets.json`
- `vcpkg.json`
- `.gitignore`
- `src/utils/NativeFileDialogs.h`
- `src/utils/NativeFileDialogs.cpp`
- `src/utils/FileUtils.h`
- `src/utils/FileUtils.cpp`
- `src/utils/AppSettings.cpp`
- `src/app/main.cpp`
- `src/app/App.cpp`
- `src/app/App.h`
- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `src/generator/CMakeEmitter.cpp`
- `src/generator/CodeGenerator.cpp`
- `docs/code_generation.md`
- `docs/settings.md`
- `docs/resources.md`
- `docs/agent_plans/phase_65_cross_platform_build_readme_plan.md`

## Audit findings

- [x] Searched the requested repository files for Windows-only build and source assumptions.
- [x] Confirmed `CMakeLists.txt` already guards `comdlg32` with `if (WIN32)`.
- [x] Confirmed `src/utils/NativeFileDialogs.cpp` is Windows-only today and needs non-Windows compile stubs.
- [x] Confirmed `src/ui/MainWindow.cpp` has an unguarded `<windows.h>` include and `GetKeyState` usage.
- [x] Confirmed `src/utils/AppSettings.cpp` uses `_dupenv_s`, which should be replaced with a portable environment lookup path.
- [x] Confirmed `src/utils/RecentFiles.cpp` also uses `_dupenv_s`, which should be replaced with a portable environment lookup path.
- [x] Confirmed committed `CMakePresets.json` contains a hard-coded Windows Ninja path under `CMAKE_MAKE_PROGRAM`.
- [x] Confirmed `README.md` still contains owner-specific `J:\` examples and lacks macOS/Linux build sections.

## TODO checklist

- [x] Inspect the requested source, CMake, and documentation files.
- [x] Document current Windows-only findings and cross-platform risks in this phase plan.
- [x] Update `CMakePresets.json` so committed presets avoid user-specific Ninja paths and add safe generic Ninja presets if practical.
- [x] Add safe non-Windows compile guards or fallbacks for Windows-specific source code that would block macOS/Linux builds.
- [x] Keep the Windows `VisiForm` build behavior intact, including the existing `build-static-debug` workflow.
- [x] Update `README.md` with accurate Windows, macOS, and Linux build instructions, platform status notes, vcpkg guidance, generated-project notes, and troubleshooting.
- [x] Update related generated-project or repository docs where portability wording changed.
- [x] Build the main `VisiForm` app with the `build-static-debug` preset on Windows.
- [x] Fix any compile errors introduced by this phase.
- [x] Update this phase plan with build validation, manual review results, final result summary, and remaining TODOs.

## Build validation checklist

- [x] Build the main `VisiForm` app with the `build-static-debug` preset.
- [x] Confirm the main `VisiForm` app built successfully on Windows.
- [x] Confirm no generated apps were launched.
- [x] Confirm `VisiForm.exe` was not run during Agent Mode.
- [x] State that macOS/Linux build instructions were not validated in this Windows environment.

## Manual review checklist

- [x] Review `README.md` for accurate platform status wording.
- [x] Review Windows setup guidance for Visual Studio 2022 and Developer Command Prompt workflows.
- [x] Review macOS and Linux instructions for dependency, vcpkg, configure, and build clarity.
- [x] Review troubleshooting additions for platform-specific limitations and missing toolchain cases.
- [x] Review generated-project portability notes for consistency with current export behavior.

## Final result summary

Completed. `README.md` was rewritten to remove owner-specific local paths and now documents Windows 10/11 as the primary tested platform plus contributor-oriented build instructions for macOS and Linux. The new README includes a platform status table, per-platform prerequisites, `vcpkg` setup guidance, toolchain examples, local `Visage` source usage, Windows Visual Studio and Developer Command Prompt workflows, macOS and Linux `cmake -S/-B` examples, generated-project notes, and expanded troubleshooting.

Portability changes were kept minimal and targeted. `CMakePresets.json` no longer hard-codes a user-specific `Ninja` path and now includes generic `ninja-debug` and `ninja-release` presets. `src/utils/NativeFileDialogs.cpp` now compiles on non-Windows platforms through safe fallback stubs, `src/ui/MainWindow.cpp` limits Win32 key-state probing to `_WIN32`, and `src/utils/AppSettings.cpp` plus `src/utils/RecentFiles.cpp` now use portable environment-variable lookups with platform-aware config paths instead of `_dupenv_s`. Generated-project documentation and preset emission in `src/generator/CMakeEmitter.cpp` and `docs/code_generation.md` were updated to reflect the same contributor-oriented portability posture. `docs/settings.md` and `docs/resources.md` were also updated to match the new behavior.

Build validation was completed successfully on Windows by entering the Visual Studio 2022 x64 developer environment, then running `cmake --preset vs2022-x64-static-debug` followed by `cmake --build --preset build-static-debug`. The main `VisiForm` app built successfully on Windows. No generated apps were launched, and `VisiForm.exe` was not run during Agent Mode. macOS and Linux instructions were added, but those build paths were not validated in this Windows environment.

## Remaining TODOs

- Validate the macOS instructions on macOS hardware.
- Validate the Linux instructions on a supported Linux environment.
- Implement native macOS and Linux file-dialog support if full non-Windows editor workflows are needed.
