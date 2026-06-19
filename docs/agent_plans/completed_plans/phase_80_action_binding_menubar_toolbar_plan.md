## Phase title

Add reusable action binding support for `MenuBar` and `ToolBar` items across the editor, project model, validation, save/load, export, and generated runtime callbacks.

## Current state

- `VisiForm` is version `1.0.0`.
- `Widget Palette` registry is repaired.
- `MenuBar` widget exists.
- `ToolBar` widget exists.
- `MenuBar` and `ToolBar` can be added, edited, saved, loaded, and exported.
- `MenuBar` and `ToolBar` currently behave mostly as visual/static widgets.
- `Button` widgets already support event/callback properties.
- Generated projects already have a callback/event preservation system.
- Undo/Redo and keyboard shortcuts work in `VisiForm`.
- The next needed feature is action binding for `MenuBar` and `ToolBar` items.

## Goal

Add action/callback binding for `MenuBar` and `ToolBar` items so exported projects can respond when menu/tool items are clicked while preserving existing widget behavior and repository constraints.

## Files to inspect

- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetDefinition.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/editors/TextEditControl.h`
- `src/ui/editors/TextEditControl.cpp`
- `src/ui/editors/DropdownControl.h`
- `src/ui/editors/DropdownControl.cpp`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/validation/ProjectValidator.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CodeGenerator.cpp`
- `docs/widget_catalog.md`
- `docs/widget_registry.md`
- `docs/project_file_format.md`
- `docs/code_generation.md`
- `docs/project_validation.md`
- `docs/component_hierarchy.md`
- `docs/agent_plans/phase_80_action_binding_menubar_toolbar_plan.md`

## Action binding design notes

- Inspect how `MenuBar` and `ToolBar` item labels are currently stored and normalized.
- Keep the storage backward-compatible for existing projects that only contain `items`.
- Prefer newline-separated `items` labels plus newline-separated `itemActions` callback names aligned by item index.
- Treat missing `itemActions` entries as empty callback bindings.
- Ignore or preserve extra `itemActions` entries safely without throwing.
- Keep model, serialization, and generator layers free of Visage UI headers.
- Reuse existing callback validation and generated handler preservation patterns where practical.
- Preserve `Button` callback behavior and existing `USER CODE BEGIN` / `USER CODE END` regions.
- Keep `MenuBar` and `ToolBar` designer rendering visual-only inside `VisiForm`.
- Generated runtime should update selected indices and invoke configured non-empty callbacks when items are clicked.
- If `WidgetEvent` is extended, do so compatibly and document the added item metadata.
- Current inspection notes:
- `MenuBar`, `ToolBar`, `ComboBox`, and `ListBox` all currently store `items` as newline-delimited text normalized by `splitItems()` / `joinItems()` in `src/model/WidgetItemUtils.cpp`.
- JSON save currently writes `items` as a JSON string array for item-list widgets, and JSON load accepts the array form and joins it back into newline-delimited text.
- The shared item editor in `MainWindow` currently edits only labels through a preview list plus a single `Item Text` field, then rewrites `items` and clamps the selected index on Apply.
- `Button` callbacks and similar widget events are stored directly in widget properties such as `onClick`, validated as C++ identifiers, and emitted as user-named generated handler overrides.
- Generated handler names are the exact callback names entered by the user; duplicate names are deduplicated by handler name and signature during export.
- Generated `USER CODE BEGIN <handler>` / `USER CODE END <handler>` markers are preserved by handler name in the emitted user subclass `.cpp` file.
- Generated `WidgetEvent` currently exposes `senderId`, `senderName`, and `senderType`; item metadata is not yet included.
- Implemented model notes:
- Added shared `supportsItemActions()`, `splitItemActions()`, `joinItemActions()`, `getWidgetItemActions()`, `getWidgetItemActionBindings()`, and selected-action helpers in `src/model/WidgetItemUtils.*`.
- `normalizeItemListProperties()` now keeps `itemActions` aligned with the normalized `items` count for `MenuBar` and `ToolBar`.
- `MenuBar` and `ToolBar` definitions now expose an `itemActions` property with backward-compatible empty default values.
- Continued inspection notes:
- `PropertyInspector` already summarizes `items`, summarizes `itemActions`, and shows a read-only `Selected Action` row for `MenuBar` and `ToolBar`.
- `MainWindow` already opens a combined item/action editor for `MenuBar` and `ToolBar`, keeps action bindings aligned while adding/removing/reordering rows, and clamps the selected item index on apply.
- JSON save/load already persists `itemActions` as a string array while keeping in-memory newline-delimited normalization.
- `ProjectValidator` already validates extra `itemActions` entries and invalid callback identifiers for `MenuBar` and `ToolBar` bindings.
- `VisageCppEmitter` already emits `WidgetEvent` item metadata plus generated runtime item-action dispatch for `MenuBar` and `ToolBar`.

## TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect current `MenuBar` item storage.
- [x] Inspect current `ToolBar` item storage.
- [x] Inspect current `ComboBox` / `ListBox` item editor behavior.
- [x] Inspect current callback storage for `Button` and related widgets.
- [x] Inspect generated callback naming and `USER CODE` preservation behavior.
- [x] Finalize the backward-compatible `items` plus `itemActions` storage design.
- [x] Add reusable action binding helpers in the model layer.
- [x] Extend `MenuBar` metadata and normalization for item actions.
- [x] Extend `ToolBar` metadata and normalization for item actions.
- [x] Extend the property inspector to show item/action editing and selected action display.
- [x] Replace or extend the item editor for `MenuBar` label/action editing.
- [x] Replace or extend the item editor for `ToolBar` label/action editing.
- [x] Preserve current `MenuBar` designer rendering and selected-item highlighting.
- [x] Preserve current `ToolBar` designer rendering and selected-item highlighting.
- [x] Clamp selected indices safely when item lists change.
- [x] Validate `MenuBar` item action names and mismatch messaging.
- [x] Validate `ToolBar` item action names and mismatch messaging.
- [x] Preserve backward-compatible save/load behavior for existing projects.
- [x] Emit generated runtime support for item labels, item actions, and selected indices.
- [x] Emit generated `MenuBar` click handling for configured callbacks.
- [x] Emit generated `ToolBar` click handling for configured callbacks.
- [x] Preserve and reuse generated callback stubs when action names repeat.
- [x] Preserve `USER CODE` blocks for generated item callbacks on re-export.
- [ ] Confirm existing `Button` callback behavior still works.
- [x] Update `docs/widget_catalog.md`.
- [x] Update `docs/project_file_format.md`.
- [x] Update `docs/code_generation.md`.
- [x] Update `docs/project_validation.md`.
- [x] Validate touched files for compile errors.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Confirm the main `VisiForm` app built successfully.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.
- [x] Update this phase plan with progress, build validation, final summary, and remaining TODOs.

## Build validation checklist

- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Confirm the main `VisiForm` app built successfully.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

## Manual test checklist

- [ ] Add a `MenuBar` and open the item editor.
- [ ] Edit `MenuBar` item labels and callback names.
- [ ] Confirm `selectedMenuIndex` highlighting still works.
- [ ] Confirm selected `MenuBar` action display updates with the selected item.
- [ ] Add a `ToolBar` and open the item editor.
- [ ] Edit `ToolBar` item labels and callback names.
- [ ] Confirm `selectedToolIndex` highlighting still works.
- [ ] Confirm selected `ToolBar` action display updates with the selected item.
- [ ] Save and reload a project containing `MenuBar` / `ToolBar` item actions.
- [ ] Load an older project that only contains `items` and confirm it still works.
- [ ] Validate a project with invalid action names and confirm clear messages appear.
- [ ] Export a project with `MenuBar` / `ToolBar` item actions.
- [ ] Build the exported generated project in Debug.
- [ ] Build the exported generated project in Release.
- [ ] Confirm generated `MenuBar` item clicks invoke configured callbacks.
- [ ] Confirm generated `ToolBar` button clicks invoke configured callbacks.
- [ ] Confirm generated callback `USER CODE` blocks survive re-export.
- [ ] Confirm existing `Button` callbacks still work.
- [ ] Confirm existing `GroupBox`, `TabControl`, `ComboBox`, `ListBox`, `TreeView`, `Table / Grid`, docking, anchors, and `StatusBar` behavior still works.

## Final result summary

- Phase 80 implementation was already largely complete in the codebase before this continuation pass.
- Confirmed the existing implementation covers shared `itemActions` model helpers, aligned normalization, `MenuBar` / `ToolBar` inspector summaries, combined item/action editor behavior, JSON save/load support, validator checks, and generated runtime callback dispatch.
- Updated `docs/widget_catalog.md`, `docs/project_file_format.md`, `docs/code_generation.md`, and `docs/project_validation.md` to describe the shipped `MenuBar` / `ToolBar` action-binding behavior.
- Built the main `VisiForm` app successfully with `build-static-debug` after invoking the build from an explicit Visual Studio x64 developer command environment.
- `VisiForm.exe` was not run, and no generated applications were launched.

## Remaining TODOs

- Manual verification checklist items remain to be executed by the developer, especially save/reload coverage, validation message review, export/re-export confirmation, and generated callback behavior checks.
