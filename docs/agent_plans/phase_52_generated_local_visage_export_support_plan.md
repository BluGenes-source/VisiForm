# Phase 52 - Generated local Visage export support
# Phase 52 - Generated local Visage export support

## Phase title
Generated local Visage export support

## Current dependency problem
Exported/generated projects may still use `FetchContent` and re-download Visage even when a local Visage source tree is available. Generated projects should prefer a configured local Visage source folder and only fall back to `FetchContent` when no valid local source is configured.

## Files to inspect
- `CMakeLists.txt`
- `CMakePresets.json`
- `.gitignore`
- `src/generator/CMakeEmitter.h`
- `src/generator/CMakeEmitter.cpp`
- `src/generator/CodeGenerator.cpp`
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `docs/code_generation.md`
- `docs/settings.md`
- `docs/copilot_rules.md`

## TODO checklist
- [x] Inspect current main app dependency configuration and generated project export flow.
- [x] Verify `.gitignore` keeps `CMakeUserPresets.json` untracked.
- [x] Add or verify `AppSettings` fields for local Visage source and fallback Git metadata.
- [x] Decide whether root `FormWindow` dependency settings are practical in this phase.
- [x] Update generated `CMakeLists.txt` emission to support `VISIFORM_VISAGE_SOURCE_DIR` with `FetchContent` fallback.
- [x] Update generated `CMakePresets.json` emission to include `VISIFORM_VISAGE_SOURCE_DIR` when configured.
- [x] Update generated `README.md` content for local source mode, fallback mode, and build instructions.
- [x] Update repository documentation for generated dependency behavior and settings.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Record final results and remaining TODOs.

## Build validation checklist
- [x] Confirm main app CMake configuration still supports local Visage without committing a machine-specific path.
- [x] Confirm generated project CMake preserves the existing `visage` link target.
- [x] Build the main `VisiForm` project with `build-static-debug` successfully.
- [x] Confirm no changes launch `VisiForm.exe` or generated apps.

## Manual test checklist
- [ ] Export a project with `localVisageSourceDirectory` set and verify generated `CMakeLists.txt` uses `add_subdirectory` when the path is valid.
- [ ] Export a project with `localVisageSourceDirectory` empty and verify generated `CMakeLists.txt` falls back to `FetchContent`.
- [ ] Open an exported project in Visual Studio 2022 and verify Debug preset configuration.
- [ ] Open an exported project in Visual Studio 2022 and verify Release preset configuration.
- [ ] Verify generated files preserve existing `USER CODE` regions.

## Final result summary
Completed. `AppSettings` now persists `localVisageSourceDirectory`, `visageGitRepository`, and `visageGitTag`. Export generation now threads those settings into generated output so exported `CMakeLists.txt` declares `VISIFORM_VISAGE_SOURCE_DIR`, prefers `add_subdirectory(...)` for a valid local Visage checkout, and falls back to `FetchContent` otherwise. Generated `CMakePresets.json` now includes `VISIFORM_VISAGE_SOURCE_DIR` when configured, using forward-slash paths. Generated `README.md` and repository docs now describe the local-source workflow, fallback behavior, and Debug/Release preset commands. The main `VisiForm` app was configured and built successfully with `build-static-debug` from a VS 2022 x64 developer environment, and no apps were launched.

## Remaining TODOs
- Manual export verification in Visual Studio 2022 for Debug and Release generated projects.
- Optional future UI work to expose export dependency settings directly in the root `FormWindow` property inspector.
