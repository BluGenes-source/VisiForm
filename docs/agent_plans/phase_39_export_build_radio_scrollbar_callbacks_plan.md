# Phase 39 export build, radio group, scrollbar, and callback plan

## Goal

Improve generated-project build usability, enforce the new export naming rule, make `ScrollBar` look like a real scrollbar, enforce `RadioButton` group behavior, and surface/export matching callback hooks more clearly.

## Current problems

- Generated project naming and structure need to be clearer for Visual Studio build and run workflows.
- `ScrollBar` currently resembles a slider too closely.
- `RadioButton` group exclusivity is not fully enforced.
- Event handler fields need matching callback suggestions.
- Exported callback code and subclass/base naming need refinement.

## Files to inspect

- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/generator/CodeGenerator.cpp`
- `src/generator/CMakeEmitter.cpp`
- `src/generator/VisageCppEmitter.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `docs/code_generation.md`
- `docs/widget_catalog.md`
- `docs/widget_registry.md`
- `docs/project_file_format.md`
- `docs/agent_plans/phase_39_export_build_radio_scrollbar_callbacks_plan.md`

## Step-by-step TODO list

- [x] Create persistent phase TODO file
- [x] Inspect export, rendering, radio-group, and callback-suggestion paths
- [x] Enforce generated project file structure and class naming rule
- [x] Update generated CMake, presets, scripts, README, and gitignore output
- [x] Improve `ScrollBar` rendering in the editor
- [x] Improve exported `ScrollBar` rendering
- [x] Enforce `RadioButton` group exclusivity in editor state updates
- [x] Normalize invalid radio groups on load
- [x] Surface matching callback suggestions in `PropertyInspector`
- [x] Update export handler generation and preservation behavior
- [x] Update docs and TODO progress
- [x] Run `build-static-debug`
- [x] Record final result summary

## Current progress notes

- Phase TODO file created before code changes.
- Export now fixes the generated base class to `MainWindow` and uses `userSubclassName` only for the user-edit layer.
- `ScrollBar` rendering now uses arrow-button regions, track, and thumb shapes in both the editor and generated export drawing.
- `RadioButton` group exclusivity is enforced on relevant property edits, duplication and paste cleanup, and project load normalization.
- Event handler rows now show same-signature callback suggestions while editing.

## Build validation checklist

- [x] Build with `build-static-debug`
- [x] Fix compile errors if any appear
- [x] Do not run `VisiForm.exe`
- [x] Do not launch generated app

## Manual test checklist

- [ ] Verify root form shows editable `userSubclassName` and non-editable or fixed `MainWindow` base naming behavior
- [ ] Verify generated export writes `MainWindow.*` plus the user subclass files
- [ ] Verify generated README, presets, scripts, and CMake flow look complete for VS 2022
- [ ] Verify `ScrollBar` looks like a scrollbar in the editor
- [ ] Verify `RadioButton` group selection keeps only one selected per group
- [ ] Verify callback suggestions appear for matching event signature kinds
- [ ] Verify export preserves user code in the user subclass `.cpp`
- [ ] Re-check existing selection, layout, smart guides, save/load/export, and hints behavior

## Final result summary

Completed.

- Generated export now writes a fixed `MainWindow.h` and `MainWindow.cpp` base layer plus user-subclass files named from `userSubclassName`.
- Generated project build metadata remains complete for VS 2022 folder-open workflows, including presets, scripts, README, and `.gitignore`.
- `ScrollBar` visuals now read as scrollbars instead of sliders in both editor and generated drawing code.
- `RadioButton` widgets now enforce one selected item per group during relevant edits and load normalization.
- `PropertyInspector` now surfaces same-signature callback suggestions for active event fields.
- Export still generates virtual callback hooks plus preserved USER CODE blocks in the user subclass implementation.
- Verified the main VisiForm project builds successfully with `build-static-debug`.
