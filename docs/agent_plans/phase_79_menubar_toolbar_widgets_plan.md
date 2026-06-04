## Phase title

Add first-pass `MenuBar` and `ToolBar` application-structure widgets with editable item lists, top docking defaults, persistence, validation, and generated rendering.

## Current state

- VisiForm is version `1.0.0`.
- `Widget Palette` registry repair is already complete.
- `StatusBar` is restored in the `Widget Palette`.
- Existing widgets are visible and addable.
- `GroupBox` parenting works.
- `TabControl` and `TabPage` parenting work.
- Docking and anchors work.
- `StatusBar` dock-bottom behavior works.
- `ComboBox`, `ListBox`, `TreeView`, and `Table / Grid` work.
- Save/load/export work.
- Undo/Redo and keyboard shortcuts work.
- Image resources work.
- The next useful application-structure widgets are `MenuBar` and `ToolBar`.

## Goal

Add first-pass `MenuBar` and `ToolBar` widgets with editable item lists, default top docking, save/load, validation, export, and generated runtime rendering while preserving all existing widget behavior.

## Files to inspect

- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetDefinition.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/WidgetMetrics.h`
- `src/ui/WidgetMetrics.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
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
- `docs/agent_plans/phase_79_menubar_toolbar_widgets_plan.md`

## MenuBar/ToolBar design notes

- Add `WidgetType::MenuBar` and `WidgetType::ToolBar` through the existing `WidgetRegistry` source of truth.
- Keep model, serialization, and generator layers free of Visage UI headers.
- Reuse the existing item-list storage and editing flow used by `ComboBox`/`ListBox` where practical.
- Use newline-separated item text for `items` storage.
- `MenuBar` defaults: name prefix `menuBar`, height `32`, dock `Top`, anchor `Top Left`, root-parent preference, default items `File/Edit/View/Project/Export/Help`, `selectedMenuIndex = 0`.
- `ToolBar` defaults: name prefix `toolBar`, height `40`, dock `Top`, anchor `Top Left`, root-parent preference, default items `New/Open/Save/Export/Validate`, `selectedToolIndex = 0`.
- `MenuBar` and `ToolBar` are not general-purpose child containers in this phase.
- `MenuBar` and `ToolBar` should prefer the root `MainWindow` parent even if a nested container is selected.
- Top-docked `MenuBar` and `ToolBar` must stack correctly above regular content while `StatusBar` remains docked at the bottom.
- Designer rendering can stay static and safe; no native menu integration, dropdowns, icons, or command binding in this phase.
- Generated runtime rendering can stay static and safe; exported projects must still build.
- Current inspection notes:
- `WidgetRegistry` is the main source of widget defaults, palette visibility, and addable child rules.
- `supportsItemList()` now covers `MenuBar` and `ToolBar` in addition to `ComboBox` and `ListBox`, with widget-specific selected-index property keys.
- The palette already reads `WidgetRegistry::paletteDefinitions()`, and `Menu Bar` plus `Tool Bar` now appear through registry metadata without bespoke palette code.
- New-widget insertion now routes `MenuBar` and `ToolBar` to the root form, matching the intended application-structure behavior.
- `StatusBar` bottom-dock behavior remains intact, while `MenuBar` and `ToolBar` default to `Dock = Top` with full-width root placement.
- Save/load continues to serialize `items` generically through shared item-list support, so the new widgets persist without new JSON-specific branches.
- Validation now emits widget-specific warnings for empty item lists, out-of-range selected indices, and non-top docking on `MenuBar` and `ToolBar`.
- Generated runtime metadata and rendering now include `MenuBar` and `ToolBar`, and the editor designer preview also draws both widgets safely.

## TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect the listed model, UI, persistence, validation, and generator files.
- [x] Confirm the existing item-list editor and storage format that can be reused.
- [x] Confirm the current root-parent add flow and docking stack behavior.
- [x] Add `MenuBar` to widget type conversion and registry metadata.
- [x] Add `ToolBar` to widget type conversion and registry metadata.
- [x] Add palette entries for `Menu Bar` and `Tool Bar` without removing existing widgets.
- [x] Add default root-parent creation behavior for `MenuBar` and `ToolBar`.
- [x] Add `MenuBar` property inspector rows and item-list editing support.
- [x] Add `ToolBar` property inspector rows and item-list editing support.
- [x] Add `MenuBar` designer rendering and selection behavior.
- [x] Add `ToolBar` designer rendering and selection behavior.
- [x] Preserve top/bottom docking stack behavior with `MenuBar`, `ToolBar`, and `StatusBar`.
- [x] Save and load `MenuBar`/`ToolBar` items and selected index properties.
- [x] Validate `MenuBar` state and messages.
- [x] Validate `ToolBar` state and messages.
- [x] Add generated runtime support for `MenuBar` and `ToolBar`.
- [x] Update `docs/widget_catalog.md`.
- [x] Update `docs/widget_registry.md`.
- [x] Update `docs/component_hierarchy.md`.
- [x] Update `docs/project_file_format.md`.
- [x] Update `docs/code_generation.md`.
- [x] Update `docs/project_validation.md`.
- [x] Validate affected files for compile errors.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Fix any compile errors introduced by this phase.
- [x] Update this phase plan with completed audit notes, build validation, final result summary, and remaining TODOs.

## Build validation checklist

- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Confirm the main `VisiForm` app built successfully.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

## Manual test checklist

- [ ] Verify `Menu Bar` appears in the `Widget Palette`.
- [ ] Verify `Tool Bar` appears in the `Widget Palette`.
- [ ] Verify `Status Bar` and all previously restored widgets still appear in the palette.
- [ ] Add a `MenuBar` and confirm it prefers the root `MainWindow` parent.
- [ ] Add a `ToolBar` and confirm it prefers the root `MainWindow` parent.
- [ ] Verify `MenuBar` defaults to `Dock = Top`, `Anchor = Top Left`, height `32`, and full root width.
- [ ] Verify `ToolBar` defaults to `Dock = Top`, `Anchor = Top Left`, height `40`, and full root width.
- [ ] Verify `MenuBar` and `ToolBar` stack correctly at the top.
- [ ] Verify `StatusBar` remains docked at the bottom.
- [ ] Verify `MenuBar` item editing works and updates `selectedMenuIndex` highlight safely.
- [ ] Verify `ToolBar` item editing works and updates `selectedToolIndex` highlight safely.
- [ ] Verify dragging `MenuBar` or `ToolBar` only moves them when `Dock = None`.
- [ ] Verify save/load preserves `MenuBar` and `ToolBar` items and selected indices.
- [ ] Verify validation warnings/messages for empty items, invalid selected index, and non-top docking.
- [ ] Verify exported generated projects build in Debug and Release.
- [ ] Verify generated `MenuBar` and `ToolBar` render safely.
- [ ] Verify `USER CODE` preservation still works.
- [ ] Verify existing `GroupBox`, `TabControl`, `ComboBox`, `ListBox`, `TreeView`, `Table / Grid`, docking, anchors, and `StatusBar` behavior still works.

## Final result summary

 Completed first-pass `MenuBar` and `ToolBar` support across widget metadata, shared item-list helpers, root-form insertion, property-inspector editing, designer rendering, validation, generated runtime export, and the requested documentation updates.

- File-level diagnostics on the touched implementation files reported no compile errors.
- The required `cmake --build --preset build-static-debug` validation completed successfully with `ninja: no work to do`.
- `VisiForm.exe` was not launched, and no generated applications were run.

## Remaining TODOs

- Manual verification remains for the unchecked items in the manual test checklist.
