# Phase 53 - Export local Visage setting propagation

## Phase title
Export local Visage setting propagation

## Current bug
The exported project `CMakeLists.txt` already supports `VISIFORM_VISAGE_SOURCE_DIR`, but exported projects still default to an empty local path because the configured local Visage source setting is not being propagated into generated project presets and export UI flow.

## Files to inspect
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/generator/CMakeEmitter.h`
- `src/generator/CMakeEmitter.cpp`
- `src/generator/CodeGenerator.h`
- `src/generator/CodeGenerator.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `docs/settings.md`
- `docs/code_generation.md`
- `docs/agent_plans/phase_53_export_local_visage_setting_propagation_plan.md`

## TODO checklist
- [x] Inspect current settings, inspector, and export propagation flow.
- [x] Verify `AppSettings` contains local Visage dependency fields with defaults.
- [x] Verify `MainWindow` passes dependency settings into export generation.
- [x] Expose root `FormWindow` export dependency fields in the property inspector.
- [x] Persist edited export dependency values in `AppSettings` instead of project JSON.
- [x] Validate local Visage source directory editing and warn when `CMakeLists.txt` is missing.
- [x] Update generated `CMakePresets.json` emission to include dependency cache variables for Debug and Release.
- [x] Update export completion status text for local-source mode or fallback mode.
- [x] Confirm generated `CMakeLists.txt` remains portable and keeps fallback behavior.
- [x] Update `docs/settings.md` and `docs/code_generation.md`.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Record final results and remaining TODOs.

## Build validation checklist
- [x] Confirm main app still builds successfully with `build-static-debug`.
- [x] Confirm generated presets include dependency cache variables when configured.
- [x] Confirm generated `CMakeLists.txt` still supports local-source and `FetchContent` fallback behavior.
- [x] Confirm no app or generated project is launched.

## Manual test checklist
- [ ] Select the root `FormWindow` and verify the property inspector shows an Export / Dependencies section.
- [ ] Set `localVisageSourceDirectory` to `J:/Dev/CeePlusPlus/visage` and verify the value persists in app settings.
- [ ] Enter `J:\Dev\CeePlusPlus\visage` and verify generated output normalizes the path to forward slashes.
- [ ] Export a project with a valid local Visage path and verify generated `CMakePresets.json` includes `VISIFORM_VISAGE_SOURCE_DIR`.
- [ ] Export a project with an empty local Visage path and verify generated presets and generated `CMakeLists.txt` fall back cleanly.
- [ ] Verify generated `README.md` explains local-source mode and fallback mode.
- [ ] Re-export over an existing generated project and verify `USER CODE` regions remain intact.

## Final result summary
Completed. The root `FormWindow` property inspector now exposes an `Export / Dependencies` section for `localVisageSourceDirectory`, `visageGitRepository`, and `visageGitTag`. Those values remain machine-local in `AppSettings` instead of `.vfb.json` files. Export generation already passed `AppSettings` through `MainWindow`, `CodeGenerator`, and `CMakeEmitter`; this phase completed the missing user-facing propagation so configured values can be edited in-app and emitted into generated presets. Generated `CMakePresets.json` now includes `VISIFORM_VISAGE_GIT_REPOSITORY` and `VISIFORM_VISAGE_GIT_TAG` for Debug and Release, and includes `VISIFORM_VISAGE_SOURCE_DIR` when configured, using forward-slash paths such as `J:/Dev/CeePlusPlus/visage`. Generated `CMakeLists.txt` remains portable by defaulting `VISIFORM_VISAGE_SOURCE_DIR` to empty and preserving local-source plus `FetchContent` fallback behavior. Export completion status now reports either local-source mode or FetchContent fallback mode. The main `VisiForm` app was configured and built successfully with `build-static-debug`, and no apps were launched.

## Remaining TODOs
- Manually export a project and verify generated `CMakePresets.json` contains the configured `VISIFORM_VISAGE_SOURCE_DIR` value.
- Manually open an exported project in Visual Studio 2022 and verify both Debug and Release generated presets.
- Manually confirm a valid local Visage path avoids repeated dependency downloads in the exported project.
