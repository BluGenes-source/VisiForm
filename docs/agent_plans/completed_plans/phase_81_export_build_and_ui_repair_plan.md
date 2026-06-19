## Phase title

Repair generated export build scripts and high-impact editor usability issues for Phase 81.

## Current state

- `VisiForm` is version `1.0.0`.
- Phase 80 added or partially added `MenuBar` / `ToolBar` action binding through `itemActions`.
- Main `VisiForm` currently builds successfully with `build-static-debug`.
- Existing generated project build scripts fail before compilation with `-products was unexpected at this time.`
- The failure appears to come from generated `.cmd` scripts that run `vswhere` inside a parenthesized batch `for (...) do (...)` block.
- The generated command contains the Visual Studio version range `[17.0,18.0)`, and the closing parenthesis breaks `cmd.exe` parsing unless it is escaped or otherwise handled safely.
- The `Insert` menu is hard-coded and is missing newer palette widgets.
- The widget palette is sorted by `paletteOrder` first, then display name, so it is not globally alphabetical.
- `Button` exposes `Text`, `Normal Text`, and `Pressed Text`; this likely needs clearer inspector labeling or documentation.
- `cornerRadius` is stored and editable, but button preview rendering still draws square rectangles.
- The `TreeView` node editor can show more nodes than fit, but there is no usable scrolling path.
- `tests/CMakeLists.txt` exists, but the root CMake file does not wire the test target into normal configuration.

## Goal

Make generated export build validation reliable first, then repair the highest-impact editor usability issues without breaking existing save/load, export, validation, or runtime behavior.

## Files to inspect

- `src/generator/CMakeEmitter.h`
- `src/generator/CMakeEmitter.cpp`
- `src/generator/CodeGenerator.cpp`
- `Generated/menu/scripts/build_static_debug.cmd`
- `Generated/menu/scripts/configure_static_debug.cmd`
- `Generated/Validate/scripts/build_static_debug.cmd`
- `Generated/Export-Test/scripts/build_static_debug.cmd`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/WidgetPalette.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/model/WidgetItemUtils.h`
- `src/model/WidgetItemUtils.cpp`
- `CMakeLists.txt`
- `tests/CMakeLists.txt`
- `docs/widget_catalog.md`
- `docs/menu_bar.md`
- `docs/code_generation.md`
- `docs/project_file_format.md`
- `docs/project_validation.md`
- `docs/agent_plans/phase_81_export_build_and_ui_repair_plan.md`

## Root-cause notes

- Pending investigation.

## TODO checklist

- [x] Create the new phase plan before changing code.
- [ ] Inspect generator script emission and existing generated script outputs.
- [ ] Reproduce the generated build-script failure without launching apps.
- [ ] Fix generated `.cmd` script emission for safe `vswhere` version-range handling.
- [ ] Check generated PowerShell script quoting for related issues.
- [ ] Validate at least one generated project Debug build path after the script fix.
- [ ] Capture and fix any first real generated configure/compile failure caused by generated output.
- [ ] Inspect current shared item/action editor behavior for `MenuBar` and `ToolBar`.
- [ ] Improve `MenuBar` item/action editor clarity.
- [ ] Improve `ToolBar` item/action editor clarity.
- [ ] Keep `items` and `itemActions` aligned through add/remove/move actions.
- [ ] Inspect the hard-coded `Insert` menu implementation.
- [ ] Make the `Insert` menu reflect addable palette widgets.
- [ ] Change widget palette ordering to alphabetical by display name.
- [ ] Clarify `Button` text property meanings in the inspector and docs.
- [ ] Implement `Button` corner-radius preview if safe.
- [ ] Repair `TreeView` node editor overflow handling.
- [ ] Inspect root and test CMake wiring.
- [ ] Add optional test wiring to the root CMake configuration.
- [ ] Update documentation for shipped behavior changes.
- [ ] Validate touched files for compile errors.
- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Record generated project build validation attempt and result.
- [ ] Confirm `VisiForm.exe` was not run.
- [ ] Confirm no generated applications were launched.
- [ ] Update this phase plan with progress, validation results, final summary, and remaining TODOs.

## Build validation checklist

- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Record whether the main `VisiForm` app built successfully.
- [ ] Attempt generated project build validation.
- [ ] Record the generated project build validation result.
- [ ] Confirm `VisiForm.exe` was not run.
- [ ] Confirm no generated applications were launched.

## Manual test checklist

- [ ] Run the generated sample Debug build scripts from an existing exported project and confirm the batch scripts no longer fail at the `vswhere` line.
- [ ] If practical, run generated sample Release build validation without launching the app.
- [ ] Open the `MenuBar` item editor and confirm the item/action workflow is obvious.
- [ ] Open the `ToolBar` item editor and confirm the item/action workflow is obvious.
- [ ] Add, remove, and move `MenuBar` and `ToolBar` items and confirm labels/actions stay aligned.
- [ ] Confirm the `Insert` menu includes the intended addable widgets.
- [ ] Confirm the widget palette appears alphabetically by display name.
- [ ] Confirm `Button` text fields are clearly understood in the inspector.
- [ ] Confirm `Button` preview visibly responds to `cornerRadius` if implemented.
- [ ] Confirm the `TreeView` node editor can access nodes beyond the initially visible area.
- [ ] Confirm existing `Button` callbacks still work.
- [ ] Confirm existing `ComboBox`, `ListBox`, `TreeView`, `TableGrid`, `GroupBox`, `TabControl`, docking, anchors, `StatusBar`, and resource behavior still works.
- [ ] If tests are enabled, configure/build them and run only non-UI safe tests.

## Final result summary

- Pending.

## Remaining TODOs

- Pending.
