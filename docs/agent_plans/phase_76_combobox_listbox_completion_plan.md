## Phase title

Complete `ComboBox` and `ListBox` support by finishing persistence, validation, designer/runtime generation, documentation, and build validation on top of the existing in-progress phase-75 changes.

## Current state

- Phase 75 introduced uncommitted groundwork for `ComboBox` and `ListBox`.
- `WidgetType`, `WidgetRegistry`, `IdGenerator`, `CMakeLists.txt`, and `MainWindow` already contain partial implementation changes.
- `WidgetItemUtils` exists but still needs final integration verification.
- Serialization, validation, generator/runtime support, docs, and final build validation are still incomplete.

## Goal

Finish safe first-pass `ComboBox` and `ListBox` support without regressing existing editor, export, or generated runtime behavior.

## TODO checklist

- [x] Inspect the current in-progress `ComboBox` and `ListBox` changes.
- [x] Create this continuation phase plan.
- [x] Complete item-list model helper integration.
- [x] Complete property inspector and modal item editor workflow.
- [x] Complete project save/load support for `items` and selection state.
- [x] Complete project validation for `items`, `selectedIndex`, and callbacks.
- [x] Complete designer rendering support for `ComboBox` and `ListBox`.
- [x] Complete generated export/runtime support for `ComboBox` and `ListBox`.
- [x] Update documentation for the new widgets and file format behavior.
- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [x] Fix any compile errors introduced by this phase.
- [x] Update this phase plan with build validation, final result summary, and remaining TODOs.

## Build validation checklist

- [x] Use the `build-static-debug` workflow.
- [ ] Confirm the main `VisiForm` app built successfully.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

## Final result summary

- Completed first-pass `ComboBox` and `ListBox` support across model helpers, JSON persistence, property-inspector item editing, validation, designer preview, and generated runtime export behavior.
- Fixed follow-up compile issues introduced during the pass in `DesignerCanvas.cpp`, `PropertyInspector.cpp`, and `ProjectValidator.cpp`.
- `build-static-debug` now reaches link stage, but final app build is still blocked in this environment by pre-existing x86/x64 linker-library mismatches and unrelated unresolved externals.

## Remaining TODOs

- Resolve the existing `build-static-debug` environment/link failure so the main `VisiForm` executable can link successfully.
- Re-run `cmake --build --preset build-static-debug` after the linker environment issue is corrected.
- Confirm explicitly that `VisiForm.exe` and generated apps remain unlaunched during validation.
