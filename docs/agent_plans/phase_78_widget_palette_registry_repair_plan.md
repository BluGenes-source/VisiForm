## Phase title

Repair the `Widget Palette` and `WidgetRegistry` coverage so every supported existing user-placeable widget is visible and addable again in the main `VisiForm` app.

## Current bug

More than one existing widget is missing from the `Widget Palette`.

## Current state

- `Status Bar` is currently missing from the `Widget Palette`.
- Other previously available widgets may also be missing after recent widget additions and registry changes.
- Initial inspection shows `WidgetPalette` already queries `WidgetRegistry` definitions and excludes only `FormWindow` and `TabPage`.
- Initial inspection also shows `WidgetPalette` draws and hit-tests only the rows that fit inside the visible panel height and currently has no scrolling support.
- Because the palette is clipped to the visible area, lower registry entries can become unreachable even though they still exist in the registry and add flow.

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
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/validation/ProjectValidator.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `docs/widget_catalog.md`
- `docs/widget_registry.md`
- `docs/agent_plans/phase_78_widget_palette_registry_repair_plan.md`

## Widget registry/palette audit table

| Widget | WidgetType enum | Type string conversion | WidgetRegistry definition | WidgetPalette visible list | DesignerCanvas drawing | PropertyInspector support | Json reader/writer | ProjectValidator | Code generator | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Frame | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Palette-visible container |
| Group Box | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Existing behavior preserved |
| Panel | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Palette-visible container |
| Tab Control | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Adds internal `TabPage` children by default |
| Tab Page | Yes | Yes | Yes | Hidden | Yes | Yes | Yes | Yes | Yes | Logical/internal only |
| Label | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | |
| Button | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | |
| Text Box | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | |
| Combo Box | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | |
| List Box | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | |
| Table / Grid | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | |
| Tree View | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | |
| Check Box | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | |
| Radio Button | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | |
| Slider | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | |
| Scroll Bar | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Restored to reachable palette range |
| Status Bar | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Restored to reachable palette range; add path already sets dock `Bottom` |
| Progress Bar | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Restored to reachable palette range |
| Modal Dialog | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Restored to reachable palette range |
| Color Picker | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Restored to reachable palette range |
| Image | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Restored to reachable palette range |
| Spacer | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Restored to reachable palette range |

## Root cause diagnosis

- The palette bug was not caused by removed widget definitions.
- `WidgetPalette` already derived entries from `WidgetRegistry`, but it relied on an implicit registry order and only rendered rows that fit in the current panel height.
- `FormWindow` and `TabPage` are intentionally hidden from the user-placeable palette.
- The pre-fix palette had no scrolling state, no mouse-wheel support, and no draggable scrollbar thumb.
- Lower widgets such as `Status Bar`, `Progress Bar`, `Modal Dialog`, `Color Picker`, `Image`, and `Spacer` could therefore remain registered but never become visible or clickable in shorter editor layouts.
- The repair formalizes palette visibility and ordering in `WidgetRegistry`, then makes `WidgetPalette` scrollable so registry-visible entries remain reachable.

## TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect the initial widget registry, widget definition, widget palette, and main window palette layout sources.
- [x] Audit every expected widget across enum, string conversion, registry, palette, designer, inspector, serialization, validation, and generator layers.
- [x] Confirm whether any supported widgets are genuinely missing from `WidgetRegistry` metadata or only hidden by the palette viewport.
- [x] Repair the palette source of truth with minimal changes.
- [x] Repair palette scrolling or clipping so every palette-visible widget can be reached.
- [x] Verify palette ordering is stable and `TabPage` remains hidden unless intentionally internal.
- [x] Add a lightweight palette consistency check if practical.
- [x] Update `docs/widget_registry.md`.
- [x] Update `docs/widget_catalog.md`.
- [ ] Validate affected files for compile errors.
- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Fix any compile errors introduced by this phase.
- [ ] Update this phase plan with completed audit results, build validation, final result summary, and remaining TODOs.

## Build validation checklist

- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Confirm the main `VisiForm` app built successfully.
- [ ] Confirm `VisiForm.exe` was not run.
- [ ] Confirm no generated apps were launched.

## Manual test checklist

- [ ] Open the `Widget Palette` and verify every expected user-placeable widget is visible.
- [ ] Verify `Tab Page` does not appear in the palette unless intentionally designed.
- [ ] Verify palette scrolling reaches the last entry and does not clip the last row.
- [ ] Add each visible palette widget and confirm the correct widget type is created.
- [ ] Verify the added widget becomes selected.
- [ ] Verify the `ProjectTree` updates after each add.
- [ ] Verify the `PropertyInspector` shows the correct type and expected properties.
- [ ] Verify `Status Bar` adds at the root, docks `Bottom`, defaults to height `50`, fills root width, and shows section text.
- [ ] Verify `Progress Bar` adds correctly and preserves its value or text behavior.
- [ ] Verify `Scroll Bar` adds correctly and its orientation property still works.
- [ ] Verify `Modal Dialog` adds correctly and does not open full-screen unless already fixed elsewhere.
- [ ] Verify `Color Picker`, `Image`, and `Spacer` add correctly and preserve save or load behavior.
- [ ] Verify existing `GroupBox`, `TabControl`, `ComboBox`, `ListBox`, `TreeView`, and `Table / Grid` behavior still works.
- [ ] Verify save and load still work.
- [ ] Verify exported projects still build in Debug and Release.
- [ ] Verify `USER CODE` preservation still works.

## Final result summary

- In progress. Registry palette metadata, ordering, visibility, and palette scrolling have been repaired.
- Build validation and final manual verification notes are still pending.

## Remaining TODOs

- Run the required `build-static-debug` validation.
- Record the final build result and remaining manual verification steps.
