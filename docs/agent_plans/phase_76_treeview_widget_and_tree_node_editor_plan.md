# Phase 76 - TreeView widget and tree node editor plan

## Phase title

Add a `TreeView` widget with editable hierarchical nodes, selected-node support, expand/collapse state, save/load, validation, export integration, generated runtime behavior, documentation updates, and build validation while preserving existing layout, container, and generated-code rules.

## Current state

- VisiForm version is `1.0.0`.
- `GroupBox` parenting works.
- `TabControl` and `TabPage` parenting work.
- Docking and anchors work.
- `StatusBar` dock bottom works.
- `ComboBox` and `ListBox` work.
- `ComboBox` and `ListBox` item editing works.
- ProjectTree hierarchy works.
- Save/load/export work.
- Undo/Redo and keyboard shortcuts work.
- Image resources work.
- The next useful data/list widget is `TreeView`.

## Goal

Add safe first-pass `TreeView` support with a simple hierarchical node text format, an inspector-based node editor, selected-node and expanded-node state, validation, save/load, designer preview, generated runtime rendering/interaction, documentation, and successful `build-static-debug` validation.

## Files to inspect

- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetDefinition.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetItemUtils.h`
- `src/model/WidgetItemUtils.cpp`
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
- `docs/agent_plans/phase_76_treeview_widget_and_tree_node_editor_plan.md`

## TreeView design notes

### Planned widget defaults

- `TreeView`
  - Name prefix: `treeView`
  - Default size: `240 x 180`
  - Default nodes text:
    - `Root`
    - `  Child 1`
    - `  Child 2`
    - `    Grandchild 1`
  - Default `selectedNodePath`: `Root/Child 1`
  - Default `expandedNodePaths`: `Root,Root/Child 2`
  - Default `showRoot`: `true`
  - Default `showLines`: `true`
  - Default `onChanged`: empty
  - Default `onDoubleClick`: empty
  - Default `dock`: `None`
  - Default `anchor`: `Top Left`
  - Hint: `Displays a hierarchical list of expandable tree nodes.`
  - Not a container

### Planned node data format

- Store tree nodes as newline-delimited indented text in the widget property map.
- Two spaces represent one indent level.
- Empty lines are ignored.
- Leading and trailing whitespace around node text is trimmed.
- Invalid indentation jumps must be normalized or handled safely without throwing.
- Node paths are derived from visible hierarchy text, for example `Root/Child 2/Grandchild 1`.

### Planned helper behavior

- Add shared helpers for parsing, serializing, flattening, normalizing, selected-path clamping, selected-node text lookup, and validation-safe normalization.
- Keep parsing non-throwing.
- Keep helper code in model layer without `Visage` dependencies.
- Reuse the same normalized representation across designer, validation, persistence, and generation paths.

### Planned editor behavior

- Expose a `Nodes` row in `PropertyInspector` showing a compact node count plus `Edit...` action.
- Reuse the existing modal/editor framework in `MainWindow`.
- Minimum editor provides an editable text area using the indented tree format plus `Apply` and `Cancel` buttons.
- Applying changes reparses nodes, stores normalized text, clamps `selectedNodePath`, normalizes `expandedNodePaths`, refreshes inspector/canvas, marks project dirty, and uses undoable property changes where current infrastructure allows.

### Planned designer and runtime behavior

- Designer preview should draw a bordered tree area, indentation, optional tree lines, expand/collapse markers, selected-node highlight, visible rows only, and a simple scrollbar placeholder when rows overflow.
- `TreeView` must work in the root form, inside `GroupBox`, and inside `TabPage`, using existing parent-relative layout, dock, and anchor behavior.
- Generated runtime support should render visible rows, allow row selection, toggle expanded nodes from marker clicks, and emit `onChanged`.
- `onDoubleClick` should be generated when the runtime input path safely supports it.
- Preserve `USER CODE` regions and keep model/serialization/generator layers free of `Visage` UI headers.

### Initial inspection findings

- `WidgetType`, widget string conversion, registry defaults, palette population, designer rendering, validation, and code generation all use explicit widget-type switches, so `TreeView` must be added consistently across multiple existing paths.
- `WidgetItemUtils` already centralizes safe newline-based item helpers for `ComboBox` and `ListBox`, making it the safest place to add shared tree-node text helpers without introducing UI dependencies.
- `PropertyInspector` and `MainWindow` already contain reusable modal editor patterns that can be extended for a tree-node editor instead of introducing a new editor subsystem.
- `JsonProjectReader` and `JsonProjectWriter` already handle targeted property-format conversions, so `TreeView` node text and expanded-path data can use focused persistence rules while remaining backward compatible.
- `DesignerCanvas` and generated runtime code already implement widget-specific rendering and input switches, so `TreeView` behavior should follow the established `ComboBox` and `ListBox` pattern.

## TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect the requested model, UI, serialization, validation, generator, and documentation touchpoints.
- [x] Record `TreeView` design and storage decisions in this phase plan.
- [ ] Add the `TreeView` widget type, string conversion, defaults, registry metadata, and palette entry.
- [ ] Add shared tree-node helper logic for parsing, normalization, flattening, selected-path clamping, and validation-safe lookups.
- [ ] Add `PropertyInspector` support for `Nodes`, `Selected Node`, `Expanded Nodes`, `Show Root`, `Show Lines`, and callbacks.
- [ ] Add the modal tree-node editor workflow in `MainWindow`.
- [ ] Render `TreeView` safely in `DesignerCanvas`.
- [ ] Preserve placement, selection, movement, resize, parent-relative layout, dock, and anchor behavior in root, `GroupBox`, and `TabPage`.
- [ ] Preserve undo/redo behavior for `TreeView` add, delete, move, resize, and property edits.
- [ ] Persist `TreeView` node and state properties through save and load.
- [ ] Add validation for tree-node indentation, selected node, expanded nodes, and callback names.
- [ ] Export and generate runtime `TreeView` support safely.
- [ ] Preserve generated `USER CODE` callback regions on re-export.
- [ ] Update the requested documentation files.
- [ ] Build the main `VisiForm` app with the `build-static-debug` workflow.
- [ ] Fix any compile errors introduced by this phase.
- [ ] Update this phase plan with build validation, final result summary, and remaining TODOs.

## Build validation checklist

- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Confirm the main `VisiForm` app built successfully.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

## Manual test checklist

- [ ] Confirm `TreeView` appears in the Widget Palette and can be added to the root form.
- [ ] Confirm `TreeView` can be added inside `GroupBox`.
- [ ] Confirm `TreeView` can be added inside the selected `TabPage`.
- [ ] Confirm ProjectTree hierarchy is correct for `TreeView`.
- [ ] Confirm `PropertyInspector` shows `Nodes`, `Selected Node`, `Expanded Nodes`, `Show Root`, `Show Lines`, and callback fields.
- [ ] Confirm the tree-node editor opens, applies, cancels, and normalizes indentation safely.
- [ ] Confirm selected-node changes update the highlighted row.
- [ ] Confirm expand/collapse updates designer rendering when supported.
- [ ] Save and reload a project and confirm `nodes`, `selectedNodePath`, `expandedNodePaths`, `showRoot`, and `showLines` persist.
- [ ] Run project validation and confirm invalid node paths or indentation issues are reported clearly.
- [ ] Export a project and confirm generated `TreeView` code builds in Debug and Release.
- [ ] Confirm generated runtime `TreeView` rendering and interaction match documented support.
- [ ] Confirm generated callbacks are present and `USER CODE` preservation still works after re-export.
- [ ] Confirm existing `GroupBox`, `TabControl`, dock, anchor, `ComboBox`, and `ListBox` behavior still works.

## Final result summary

- `TreeView` support is now present in the editor-side registry, inspector, designer, persistence, validation, and current repair-pass workflow.
- The Phase 76 repair pass completed the inspector dropdown collapse fix and replaced the raw node-text modal with a visual `TreeView` node editor.
- The main `VisiForm` app builds successfully with the required `build-static-debug` workflow.

## Remaining TODOs

- Run the remaining manual verification items listed in the checklists below.

## Repair Pass - TreeView Node Editor and Dropdown Collapse

### Current failed behavior

- Opening the `PropertyInspector` `Selected Node` dropdown and then scrolling the inspector leaves the popup floating on screen.
- Any inspector-owned dropdown currently remains open when the inspector scroll offset changes.
- The `TreeView` `Nodes` editor is still a raw multiline text editor.
- The raw editor draws helper text inside the same rectangle as the active multiline text control, causing overlap.
- Building tree structure currently requires manually typing indentation.

### Root cause diagnosis

- `PropertyInspector` scrolling only changes `scrollOffsetY_`; popup state is owned separately by `MainWindow::dropdownControl_`, so wheel and scrollbar scrolling do not automatically collapse active inspector dropdowns.
- `MainWindow::mouseWheel()` routes wheel input to `dropdownControl_` before the inspector, which lets an open dropdown consume wheel input instead of collapsing when the inspector is being scrolled.
- `PropertyInspector::mouseDown()` and `PropertyInspector::mouseDrag()` handle scrollbar paging and thumb dragging without any central popup-cancel path.
- `MainWindow::openSelectedTreeNodeEditor()` still starts a multiline `TextEditControl` over the whole tree editor body, and `drawEditorModalDialog()` also paints the helper instruction text inside that same bounds region.
- The current `TreeView` editor has no editor-side node model or visual row interaction; it only edits serialized indented text.

### Files inspected

- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/editors/TextEditControl.h`
- `src/ui/editors/TextEditControl.cpp`
- `src/ui/editors/DropdownControl.h`
- `src/ui/editors/DropdownControl.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetItemUtils.h`
- `src/model/WidgetItemUtils.cpp`
- `src/model/WidgetRegistry.cpp`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/validation/ProjectValidator.cpp`
- `CMakePresets.json`
- `README.md`
- `docs/widget_catalog.md`
- `docs/project_file_format.md`
- `docs/code_generation.md`
- `docs/project_validation.md`
- `docs/agent_plans/phase_76_treeview_widget_and_tree_node_editor_plan.md`

### Specific code paths changed

- `PropertyInspector` now records scroll interactions from mouse-wheel scrolling, scrollbar paging, and scrollbar dragging so popup collapse can happen through one central path.
- `MainWindow` now cancels inspector popups when the inspector scrolls and routes `TreeView` node editing through a visual modal editor with selection, rename, add, remove, and move actions.
- Documentation now reflects the visual `TreeView` editor, stable stored node properties, `TreeView` validation behavior, export data flow, and repository-level feature summary updates.

### TODO checklist

- [x] Inspect `PropertyInspector` scroll behavior and popup ownership.
- [x] Inspect the current raw `TreeView` node editor and overlap cause.
- [x] Add a central inspector popup-close path for scroll-driven collapse.
- [x] Collapse inspector dropdowns on mouse wheel, scrollbar paging, and scrollbar drag.
- [x] Replace the raw multiline `TreeView` node editor with a visual tree editor.
- [x] Add visual node selection, rename, add child, add sibling, remove, and move up/down behavior.
- [x] Keep stored `TreeView` node serialization stable through apply, save/load, validation, and export.
- [x] Update the requested documentation files for the visual editor behavior.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Fix any compile errors introduced by this repair pass.
- [x] Update this repair-pass section with build validation, final summary, and remaining TODOs.

### Build validation checklist

- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Confirm the main `VisiForm` app built successfully.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

### Manual test checklist

- [ ] Open an inspector dropdown, scroll the inspector wheel, and confirm the popup collapses immediately.
- [ ] Drag the inspector scrollbar with an open dropdown and confirm the popup collapses immediately.
- [ ] Confirm `Selected Node` no longer floats after inspector scrolling.
- [ ] Open `TreeView` `Nodes/Edit` and confirm a visual tree list appears instead of raw indented text.
- [ ] Confirm node rows do not overlap helper text or labels.
- [ ] Confirm selecting a node updates the rename field.
- [ ] Confirm rename, add child, add sibling, remove, and move up/down work before `Apply`.
- [ ] Confirm `Apply` updates the designer `TreeView` immediately.
- [ ] Confirm `Cancel` leaves the previous tree unchanged.
- [ ] Save and reload a project and confirm `TreeView` nodes and selected-node state persist.
- [ ] Validate a project and confirm `TreeView` node data still validates cleanly.
- [ ] Export a project and confirm generated output uses the edited tree data.

### Final result summary

- Added a central inspector popup-cancel path so inspector-owned dropdowns collapse immediately when the `PropertyInspector` scrolls by mouse wheel, scrollbar paging, or scrollbar thumb drag.
- Replaced the raw `TreeView` multiline text editor with a visual tree editor in `MainWindow`, including node selection, rename through the `Node Text` field, add child, add sibling, remove subtree, and move up/down among siblings.
- Kept `TreeView` storage stable by serializing the visual editor back into the existing `nodes`, `selectedNodePath`, and `expandedNodePaths` properties already used by save/load, validation, preview, and export flows.
- Updated `docs/widget_catalog.md`, `docs/project_file_format.md`, `docs/code_generation.md`, `docs/project_validation.md`, and `README.md` to document the repaired behavior.
- Verified the main `VisiForm` app builds successfully with the required `build-static-debug` workflow.

### Remaining TODOs

- Run the manual editor verification items in the checklist above inside Visual Studio.
