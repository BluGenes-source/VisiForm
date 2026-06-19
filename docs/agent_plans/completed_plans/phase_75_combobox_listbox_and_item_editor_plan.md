# Phase 75 - ComboBox, ListBox, and item editor plan

## Phase title

Add `ComboBox` and `ListBox` widgets with editable item lists, selected-index support, callbacks, persistence, validation, export support, and generated runtime behavior while preserving existing container and layout behavior.

## Current state

- VisiForm version is `1.0.0`.
- `GroupBox` parenting works.
- `TabControl` and `TabPage` parenting work.
- Docking and anchors work.
- `StatusBar` dock bottom works.
- ProjectTree hierarchy works.
- Save/load/export work.
- Undo/Redo and keyboard shortcuts work.
- Image resources work.
- `PropertyInspector` already has reusable editors.
- The next needed widgets are data/list selection widgets.

## Goal

Add safe first-pass support for `ComboBox` and `ListBox` widgets, including reusable item-list editing in the `PropertyInspector`, `selectedIndex`, editable items, callback fields, persistence, validation, export integration, generated runtime behavior, documentation, and build validation.

## Files to inspect

- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetDefinition.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/ui/WidgetMetrics.h`
- `src/ui/WidgetMetrics.cpp`
- `src/ui/editors/TextEditControl.h`
- `src/ui/editors/TextEditControl.cpp`
- `src/ui/editors/DropdownControl.h`
- `src/ui/editors/DropdownControl.cpp`
- `src/commands/Command.h`
- `src/commands/UndoRedoStack.h`
- `src/commands/UndoRedoStack.cpp`
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
- `docs/agent_plans/phase_75_combobox_listbox_and_item_editor_plan.md`

## Widget design notes

### Planned widget defaults

- `ComboBox`
  - Name prefix: `comboBox`
  - Default size: `180 x 32`
  - Default items: `Apple`, `Orange`, `Banana`
  - Default `selectedIndex`: `0`
  - Default `text`: `Apple`
  - Default `onChanged`: empty
  - Default `dock`: `None`
  - Default `anchor`: `Top Left`
  - Hint: `Selects one item from a dropdown list.`
  - Not a container
- `ListBox`
  - Name prefix: `listBox`
  - Default size: `220 x 140`
  - Default items: `Item 1`, `Item 2`, `Item 3`
  - Default `selectedIndex`: `0`
  - Default `multiSelect`: `false`
  - Default `onChanged`: empty
  - Default `onDoubleClick`: empty
  - Default `dock`: `None`
  - Default `anchor`: `Top Left`
  - Hint: `Displays a selectable list of items.`
  - Not a container

### Planned property format

- Prefer stable project-file storage for `items` as a JSON array when existing model support allows it.
- For this phase, newline-delimited internal storage is acceptable if it keeps the model, serialization, and generator layers simple and safe.
- Empty item lists are allowed.
- `selectedIndex` must become `-1` for empty item lists and otherwise be clamped or validated safely.
- Helper paths may include `splitItems(...)`, `joinItems(...)`, `sanitizeSelectedIndex(...)`, and `getSelectedItemText(...)`.

### Inspection findings

- `WidgetType`, string conversion, duplicate-name generation, id-prefix generation, registry defaults, palette entries, and designer rendering all use explicit widget-type switches, so `ComboBox` and `ListBox` support must be added in several existing switch paths.
- `PropertyValue` currently supports only scalar values, so the simplest safe model representation for `items` is newline-delimited text inside the widget property map.
- `JsonProjectReader` and `JsonProjectWriter` already serialize arbitrary properties, which means `items`, `selectedIndex`, `multiSelect`, and callbacks can piggyback on existing property persistence with a focused special case if `items` should be written as a JSON array.
- `PropertyInspector` already supports read-only, text, integer, slider, bool, color, and choice rows plus callback-choice reuse, but it does not yet have an item-list-specific editor row.
- `MainWindow` already has a reusable editor-modal system with editable fields, dropdown support, status text, and custom buttons. This is the best fit for a reusable item editor without introducing `Visage` dependencies into non-UI layers.
- `DesignerCanvas` uses a widget-type render switch and generic parent-relative recursion, so new widget visuals can inherit existing root, `GroupBox`, `TabPage`, dock, and anchor behavior once added to the draw path.
- Generated runtime export already supports shared `items` vectors, callbacks, helper methods, draw-time widget switches, and pointer interaction, so `ComboBox` and `ListBox` can be added by extending existing runtime widget metadata instead of introducing a new export architecture.

### Chosen implementation direction

- Store `items` internally as newline-delimited text on the widget property map.
- Add shared helper functions in the model layer for splitting, joining, selected-index sanitization, and selected-item lookup.
- Persist `items` as a JSON array in project files while still accepting legacy string input on load.
- Add a reusable item editor through the existing `MainWindow` editor-modal system.
- Use `void_event` callback signatures for `onChanged` and `onDoubleClick` in this phase unless an existing typed signature is already required by an established path.

### Planned editor behavior

- Reuse the `PropertyInspector` to edit `items` through a reusable editor.
- Minimum acceptable behavior is newline-separated item editing through a modal or multiline-style editor.
- Preferred inspector row shows item count plus an `Edit...` action.
- `selectedIndex` should be editable and should expose the current selected item when practical.
- Callback fields should reuse existing callback editing behavior.
- `ComboBox` uses `onChanged`.
- `ListBox` uses `onChanged` and `onDoubleClick` if runtime input safely supports it.

### Planned designer and runtime behavior

- `ComboBox` should render as a collapsed field with current item text and a dropdown arrow in the designer.
- `ListBox` should render rows, current selection, and a simple scrollbar placeholder when needed in the designer.
- Both widgets must work in the root form, `GroupBox`, and `TabPage`.
- Both widgets must respect dock and anchor behavior.
- Generated runtime support should safely render both widgets and preserve callbacks.
- Safe minimum runtime behavior is acceptable for this phase if documented.

## TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect the requested model, UI, serialization, validation, generator, and documentation files.
- [x] Record concrete widget design and storage decisions in this phase plan.
- [ ] Add `ComboBox` and `ListBox` widget types, defaults, registry metadata, and palette entries.
- [ ] Add shared item helper logic for item parsing, selected index sanitization, and selected item text lookup.
- [ ] Add `PropertyInspector` support for `items`, `selectedIndex`, and callback editing for the new widgets.
- [ ] Add reusable item-list editor UI behavior.
- [ ] Render `ComboBox` and `ListBox` safely in the designer.
- [ ] Keep placement, selection, movement, resize, parent-relative layout, dock, and anchor behavior working in root, `GroupBox`, and `TabPage`.
- [ ] Preserve undo/redo behavior for add, delete, move, resize, and property edits.
- [ ] Persist `ComboBox` and `ListBox` properties through save and load.
- [ ] Add validation for `items`, `selectedIndex`, and callback fields.
- [ ] Export and generate runtime support for `ComboBox` and `ListBox` safely.
- [ ] Preserve `USER CODE` callback regions on re-export.
- [ ] Update requested documentation files.
- [ ] Build the main `VisiForm` app with the `build-static-debug` workflow.
- [ ] Fix any compile errors introduced by this phase.
- [ ] Update this phase plan with build validation, final result summary, and remaining TODOs.

## Build validation checklist

- [ ] Configure with the preset used by `build-static-debug` if needed.
- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Confirm the main `VisiForm` app built successfully.
- [ ] Confirm `VisiForm.exe` was not run.
- [ ] Confirm no generated apps were launched.

## Manual test checklist

- [ ] Confirm `ComboBox` appears in the Widget Palette and can be added to the root form.
- [ ] Confirm `ListBox` appears in the Widget Palette and can be added to the root form.
- [ ] Confirm `ComboBox` and `ListBox` can be added inside `GroupBox`.
- [ ] Confirm `ComboBox` and `ListBox` can be added inside the selected `TabPage`.
- [ ] Confirm ProjectTree hierarchy is correct for the new widgets.
- [ ] Confirm `PropertyInspector` shows `items`, `selectedIndex`, and callback fields.
- [ ] Confirm the item-list editor edits, reorders, applies, and cancels safely.
- [ ] Confirm selected index changes update displayed selected item text.
- [ ] Save and reload a project and confirm items and `selectedIndex` persist.
- [ ] Run project validation and confirm invalid `selectedIndex` values are reported clearly.
- [ ] Export a project and confirm generated `ComboBox` and `ListBox` code builds in Debug and Release.
- [ ] Confirm generated runtime `ComboBox` and `ListBox` behavior matches documented support.
- [ ] Confirm `USER CODE` preservation still works after re-export.
- [ ] Confirm existing `GroupBox`, `TabControl`, docking, and anchor behavior still works.

## Final result summary

- Pending implementation.

## Remaining TODOs

- Pending implementation and validation.

## Repair Pass - ComboBox and ListBox Items Editing

### Current failed behavior

- `ComboBox` and `ListBox` can be added and their item-related properties appear in `PropertyInspector`.
- The `Items` workflow is failing in the inspector: the row may look read-only, the click target may not open the editor, or edited values may not commit back to the selected widget.
- After attempted item changes, `selectedIndex` and the displayed selected item may not refresh immediately.

### Root cause diagnosis

- [x] Inspect the current `items` property definition metadata for `ComboBox` and `ListBox`.
- [x] Confirm how the inspector renders the `items` row and whether the click target is editable.
- [x] Confirm whether the existing item editor modal opens, applies changes, clamps `selectedIndex`, and refreshes the canvas.

#### Diagnosis notes

- `ComboBox` and `ListBox` both define the `items` property with the key `items`, the inspector label `Items`, and `PropertyEditKind::Text` in `WidgetRegistry`; the stored widget value is newline-delimited text and `selectedIndex` is stored separately as an integer.
- `PropertyInspector` currently renders a read-only `Items` summary row plus a second synthetic `__edit_items` choice row instead of a single actionable `Items` row.
- The current inspector-to-modal wiring is broken: the synthetic row uses the key `__edit_items` with the choice value `edit`, but `MainWindow::applyInspectorDropdownSelection(...)` only opens the editor for `key == "items" && value == "__edit_items"`.
- Because of that key/value mismatch, clicking the current edit affordance does not open the item editor, and clicking the visible `Items` row does nothing because it is read-only.
- The existing modal item editor does commit through `applyItemListEditor()`, but it always starts its preview selection at row `0` instead of the widget's current `selectedIndex`.

### Files inspected

- [x] `docs/agent_plans/phase_75_combobox_listbox_and_item_editor_plan.md`
- [x] `src/ui/PropertyInspector.h`
- [x] `src/ui/PropertyInspector.cpp`
- [x] `src/ui/MainWindow.h`
- [x] `src/ui/MainWindow.cpp`
- [x] `src/model/WidgetNode.h`
- [x] `src/model/WidgetNode.cpp`
- [x] `src/model/WidgetRegistry.cpp`
- [x] `src/model/WidgetItemUtils.h`
- [x] `src/model/WidgetItemUtils.cpp`
- [x] `src/ui/DesignerCanvas.h`
- [x] `src/ui/DesignerCanvas.cpp`
- [x] `src/ui/editors/TextEditControl.h`
- [x] `src/ui/editors/TextEditControl.cpp`
- [x] `src/ui/editors/DropdownControl.h`
- [x] `src/ui/editors/DropdownControl.cpp`
- [x] `src/model/WidgetDefinition.h`
- [ ] `src/model/WidgetDefinition.cpp`
- [x] `src/model/WidgetRegistry.h`
- [x] `src/serialization/JsonProjectReader.cpp`
- [x] `src/serialization/JsonProjectWriter.cpp`
- [x] `src/validation/ProjectValidator.cpp`
- [x] `src/generator/VisageCppEmitter.cpp`
- [x] `docs/widget_catalog.md`
- [x] `docs/project_file_format.md`
- [x] `docs/code_generation.md`

### Specific code paths changed

- [x] `src/ui/PropertyInspector.cpp`
- [x] `src/ui/MainWindow.cpp`
- [x] `src/model/WidgetItemUtils.cpp`
- [ ] `src/serialization/JsonProjectReader.cpp`
- [ ] `src/serialization/JsonProjectWriter.cpp`
- [ ] `src/validation/ProjectValidator.cpp`
- [ ] `src/generator/VisageCppEmitter.cpp`
- [x] `docs/widget_catalog.md`
- [x] `docs/project_file_format.md`
- [x] `docs/code_generation.md`

#### Implemented path notes

- `src/ui/PropertyInspector.cpp` now exposes a single actionable `Items` row with an inline `Edit...` affordance instead of the broken synthetic `__edit_items` row.
- `src/ui/MainWindow.cpp` now opens the item editor directly when `Items` is clicked, still tolerates the legacy synthetic dropdown route, and initializes the editor preview on the widget's current `selectedIndex`.
- `src/model/WidgetItemUtils.cpp` now trims item lines, ignores blank lines, adds shared `getWidgetItems(...)`, `setWidgetItems(...)`, and `clampSelectedIndex(...)` helpers, and keeps `ComboBox.text` synchronized with the selected item.
- Documentation now reflects the repaired inspector workflow, normalized item storage rules, and generated export behavior.

### Step-by-step TODO checklist

- [x] Diagnose the current `items` property metadata and storage format for `ComboBox` and `ListBox`.
- [x] Diagnose the `PropertyInspector` item-row rendering, hit testing, and modal open path.
- [x] Diagnose the current item helper behavior for splitting, joining, lookup, and selected-index clamping.
- [x] Repair the inspector `Items` row so the value area and edit affordance open the item editor reliably.
- [x] Repair item apply behavior so `items`, `selectedIndex`, and displayed content update immediately.
- [x] Verify save/load, validation, and export continue to use the edited item list safely.
- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Record the final repair summary and remaining manual checks.

### Build validation checklist

- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Confirm the main `VisiForm` app built successfully.
- [ ] Confirm `VisiForm.exe` was not run.
- [ ] Confirm no generated apps were launched.

### Manual test checklist

- [ ] Add a `ComboBox` and confirm `PropertyInspector` shows `Items` with a clear edit action.
- [ ] Add a `ListBox` and confirm `PropertyInspector` shows `Items` with a clear edit action.
- [ ] Click the `Items` value area for `ComboBox` and confirm the item editor opens.
- [ ] Click the `Items` value area for `ListBox` and confirm the item editor opens.
- [ ] Edit newline-separated `ComboBox` items and confirm the displayed selected item refreshes immediately.
- [ ] Edit newline-separated `ListBox` items and confirm the visible rows refresh immediately.
- [ ] Confirm `selectedIndex` clamps to `-1`, `0`, or the last valid item index after item edits.
- [ ] Save, reload, and confirm edited item lists persist.
- [ ] Export and confirm generated code uses the edited items.
- [ ] Confirm GroupBox, TabControl, docking, and anchor behavior still work.

### Final result summary

- Pending repair implementation.
