## Phase title

Add a `TableGrid` widget with editable columns and rows, selected-cell support, save/load, validation, export integration, generated runtime behavior, documentation updates, and build validation while preserving existing layout, container, and generated-code rules.

## Current state

- VisiForm version is `1.0.0`.
- `GroupBox` parenting works.
- `TabControl` and `TabPage` parenting work.
- Docking and anchors work.
- `StatusBar` dock bottom works.
- `ComboBox` and `ListBox` work.
- `ComboBox` and `ListBox` item editing works.
- `TreeView` works.
- `TreeView` visual node editor works.
- `PropertyInspector` dropdowns close when scrolled.
- `ProjectTree` hierarchy works.
- Save/load/export work.
- Undo/Redo and keyboard shortcuts work.
- Image resources work.
- The next useful data widget is `Table/Grid`.

## Goal

Add safe first-pass `TableGrid` support with editable column and row data, selected row and column state, an inspector-driven visual table editor, validation, save/load, designer preview, generated runtime rendering and interaction, documentation, and successful `build-static-debug` validation.

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
- `docs/agent_plans/phase_77_table_grid_widget_and_data_editor_plan.md`

## Table/Grid design notes

### Planned widget defaults

- `TableGrid`
  - Display name: `Table / Grid`
  - Name prefix: `tableGrid`
  - Default size: `360 x 220`
  - Default `columns` text:
    - `Name`
    - `Type`
    - `Value`
  - Default `rows` text:
    - `Row 1\tText\tHello`
    - `Row 2\tNumber\t100`
    - `Row 3\tBool\ttrue`
  - Default `selectedRow`: `0`
  - Default `selectedColumn`: `0`
  - Default `showHeader`: `true`
  - Default `showGridLines`: `true`
  - Default `rowHeight`: `28`
  - Default `headerHeight`: `30`
  - Default `onSelectionChanged`: empty
  - Default `onCellDoubleClick`: empty
  - Default `dock`: `None`
  - Default `anchor`: `Top Left`
  - Hint: `Displays editable rows and columns of data.`
  - Not a container

### Planned table data format

- Store `columns` as newline-delimited column names.
- Store `rows` as newline-delimited row strings with tab-delimited cells.
- Empty columns are allowed but should warn during validation.
- Rows may contain fewer cells than columns; missing cells display empty.
- Rows may contain more cells than columns; extra cells should be preserved safely in storage while clipped for preview when needed.
- Parsing and normalization must never throw.

### Planned helper behavior

- Add shared helpers for splitting, joining, normalizing, clamping selected row and column values, reading cell text, and writing cell text.
- Keep helper code in the model layer without `Visage` dependencies.
- Reuse the same normalized representation across designer, validation, persistence, and generation paths.

### Planned editor behavior

- Expose `Columns` and `Rows` rows in `PropertyInspector` showing compact counts plus `Edit...` actions.
- Expose `Selected Row`, `Selected Column`, `Show Header`, `Show Grid Lines`, `Row Height`, `Header Height`, `On Selection Changed`, and `On Cell Double Click`.
- Reuse the existing modal/editor framework in `MainWindow`.
- The visual editor should include instructions, a grid preview area, selected-cell text editing, row and column action buttons, and `Apply` / `Cancel` buttons.
- The editor should target at least `760 x 580` so controls are not clipped.

### Planned designer and runtime behavior

- Designer preview should draw a bordered table area, optional header row, equal-width columns, visible rows only, grid lines, selected cell or row highlight, and a simple scrollbar placeholder when rows overflow.
- `TableGrid` must work in the root form, inside `GroupBox`, and inside `TabPage`, using existing parent-relative layout, dock, and anchor behavior.
- Generated runtime support should render visible headers, rows, and selection state, allow cell selection from clicks, and emit `onSelectionChanged`.
- `onCellDoubleClick` should be generated when the runtime input path safely supports it.
- Preserve `USER CODE` regions and keep model, serialization, and generator code free of `Visage` UI headers.

### Initial inspection findings

- `WidgetType`, widget string conversion, registry defaults, inspector rows, designer rendering, validation, persistence, and code generation all use explicit widget-type branches, so `TableGrid` must be added consistently across multiple existing paths.
- `WidgetItemUtils` already centralizes safe editor-independent helpers for item lists and tree nodes, making it the safest model-layer location for shared table column and row parsing, normalization, and cell helpers.
- `PropertyInspector` already uses summary rows with `Edit...` actions for `items` and `nodes`, and `MainWindow` already owns reusable modal editor state, button handling, dropdowns, and apply/cancel flows that can be extended for a visual table editor.
- `JsonProjectReader`, `JsonProjectWriter`, and `ProjectValidator` already normalize widget-specific property formats after load and before reporting issues, so `TableGrid` should follow the same non-throwing normalization and warning patterns.
- `DesignerCanvas` and generated runtime initialization both already implement widget-specific preview and interaction branches for list-style widgets, so `TableGrid` should follow the established `ListBox` and `TreeView` rendering approach.

## TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect the requested model, UI, serialization, validation, generator, and documentation touchpoints.
- [x] Record `TableGrid` design and storage decisions in this phase plan.
- [x] Add the `TableGrid` widget type, string conversion, defaults, registry metadata, and palette entry.
- [x] Add shared table-data helper logic for parsing, normalization, clamping, and validation-safe lookups.
- [x] Add `PropertyInspector` support for table/grid properties and editor launch rows.
- [x] Add the visual table data editor workflow in `MainWindow`.
- [x] Render `TableGrid` safely in `DesignerCanvas`.
- [ ] Preserve placement, selection, movement, resize, parent-relative layout, dock, and anchor behavior in root, `GroupBox`, and `TabPage`.
- [ ] Preserve undo/redo behavior for `TableGrid` add, delete, move, resize, and property edits.
- [x] Persist `TableGrid` data and selection properties through save and load.
- [x] Add validation for table data, selection state, row and header sizes, and callback names.
- [ ] Export and generate runtime `TableGrid` support safely.
- [ ] Preserve generated `USER CODE` callback regions on re-export.
- [ ] Update the requested documentation files.
- [ ] Build the main `VisiForm` app with the `build-static-debug` workflow.
- [ ] Fix any compile errors introduced by this phase.
- [ ] Update this phase plan with build validation, final result summary, and remaining TODOs.

## Build validation checklist

- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Confirm the main `VisiForm` app built successfully.
- [ ] Confirm `VisiForm.exe` was not run.
- [ ] Confirm no generated apps were launched.

## Manual test checklist

- [ ] Confirm `Table / Grid` appears in the Widget Palette and can be added to the root form.
- [ ] Confirm `Table / Grid` can be added inside `GroupBox`.
- [ ] Confirm `Table / Grid` can be added inside the selected `TabPage`.
- [ ] Confirm ProjectTree hierarchy is correct for `TableGrid`.
- [ ] Confirm `PropertyInspector` shows table/grid properties.
- [ ] Confirm the table data editor opens, applies, cancels, and keeps layout readable.
- [ ] Confirm columns can be added, removed, and renamed.
- [ ] Confirm rows can be added, removed, and moved.
- [ ] Confirm selected cell text editing updates the grid preview.
- [ ] Confirm `selectedRow` and `selectedColumn` update selected cell display.
- [ ] Save and reload a project and confirm `columns`, `rows`, `selectedRow`, `selectedColumn`, `showHeader`, `showGridLines`, `rowHeight`, and `headerHeight` persist.
- [ ] Run project validation and confirm invalid table data is reported clearly.
- [ ] Export a project and confirm generated `TableGrid` code builds in Debug and Release.
- [ ] Confirm generated runtime `TableGrid` rendering and interaction match documented support.
- [ ] Confirm generated callbacks are present and `USER CODE` preservation still works after re-export.
- [ ] Confirm existing `GroupBox`, `TabControl`, dock, anchor, `ComboBox`, `ListBox`, and `TreeView` behavior still works.

## Final result summary

- Pending implementation.

## Remaining TODOs

- Complete the unchecked items in the TODO checklist and manual validation checklist.
