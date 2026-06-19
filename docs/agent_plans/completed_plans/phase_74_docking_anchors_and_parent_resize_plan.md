# Phase 74 - Docking, anchors, and parent resize plan
# Phase 74 - Docking, anchors, and parent resize plan

## Phase title

Add the first formal docking and anchor layout pass for root form, `GroupBox`, and `TabPage` parents while preserving current `GroupBox` and `TabControl` editing behavior.

## Current state

- VisiForm version is `1.0.0`.
- `GroupBox` selection, movement, parenting, and child editing work.
- `TabControl` movement, resizing, `TabPage` parenting, and child insertion work.
- ProjectTree hierarchy works.
- Save/load/export work.
- Undo/Redo and keyboard shortcuts work.
- Image resources work.
- `StatusBar` exists, but docking/attachment behavior is not yet formalized.
- The app needs proper docking and anchoring so widgets attach to parent edges and resize predictably.

## Goal

Add a safe first real docking and anchoring pass for the root form, `GroupBox`, and `TabPage` containers with editor support, persistence, validation, export integration, documentation, and build validation.

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
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/WidgetMetrics.h`
- `src/ui/WidgetMetrics.cpp`
- `src/commands/Command.h`
- `src/commands/UndoRedoStack.h`
- `src/commands/UndoRedoStack.cpp`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/validation/ProjectValidator.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CodeGenerator.cpp`
- `docs/component_hierarchy.md`
- `docs/widget_catalog.md`
- `docs/project_file_format.md`
- `docs/code_generation.md`
- `docs/project_validation.md`
- `docs/agent_plans/phase_74_docking_anchors_and_parent_resize_plan.md`

## Docking/anchor design notes

### Current inspection findings

- [x] Widgets already have a `dock` property in `WidgetRegistry` definitions. Current stored choices are `None`, `Top`, `Bottom`, `Left`, `Right`, and `Fill`.
- [x] Widgets already have an `anchor` property in `WidgetRegistry`, but it is currently stored only as plain text with default `Left,Top` and no implemented resize behavior.
- [x] `StatusBar` already defaults to `dock = Bottom` in the root-creation paths that explicitly special-case it, and template insertion also sets `fillWidth = true`.
- [x] Bounds are stored parent-relative. `DesignerCanvas` draw and hit-test recursion adds `parentLocalX/Y + child.bounds.x/y`, and `MainWindow::boundsRelativeToParent(...)` also converts absolute drop positions back to parent-relative bounds.
- [x] Parent resize is not centrally detected today. Layout is currently recalculated only through `ProjectDocument::applyDockLayout()` on child insert/reparent and through `refreshHierarchyMetadata()` for `TabControl` page bounds. General anchor-based parent resize behavior does not exist yet.
- [x] Form/root size can be changed in the editor through widget bounds/property editing. `MainWindow::setSelectedWidgetBounds(...)` allows root width/height edits, but the designer does not expose resize handles for the root form.
- [x] `GroupBox` resize does not currently relayout children. `TabControl` resize already updates owned `TabPage` bounds through `syncTabPageBoundsRecursive(...)`, but child widgets inside the selected `TabPage` do not yet react to parent resize through dock/anchor rules.

### Initial implementation notes from inspection

- `ProjectDocument::applyDockLayoutRecursive(...)` currently docks children against the full parent bounds, without container-specific client area calculations.
- `JsonProjectReader` and `JsonProjectWriter` already persist arbitrary widget properties, so `dock`, `anchor`, and margin metadata can ride through existing property serialization without schema changes.
- `ProjectValidator` already validates `dock`, but it does not validate `anchor` or report multiple `Fill` conflicts yet.
- `PropertyInspector` already supports choice/dropdown rows when property definitions provide `choices`, so improved `Dock` and `Anchor` dropdowns can be driven from widget property definitions plus focused labels/hints.
- Generated runtime export currently emits absolute widget bounds and parent relationships, but it does not yet emit or apply dock/anchor layout metadata.

### Planned model

- `Dock` values: `None`, `Top`, `Bottom`, `Left`, `Right`, `Fill`.
- `Dock = None` keeps normal absolute positioning.
- `Dock != None` owns layout and takes priority over anchor behavior.
- Preferred readable `Anchor` choices in the Property Inspector:
  - `Top Left`
  - `Top Right`
  - `Bottom Left`
  - `Bottom Right`
  - `Stretch Width Top`
  - `Stretch Width Bottom`
  - `Stretch Height Left`
  - `Stretch Height Right`
  - `Fill`
  - `None`
- Anchor behavior should preserve edge distances and resize when opposite edges are anchored.
- Parent client area must be calculated for:
  - root form
  - `GroupBox` content area
  - `TabPage` content area
- Keep child coordinates parent-relative.
- Avoid introducing `Visage` dependencies into model, serialization, or generator layers.
- Keep nested layout handling simple and stable for this phase.

## TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect the requested model, UI, serialization, validation, generator, and documentation files.
- [x] Record the current dock, anchor, bounds, resize, and `StatusBar` findings in this phase plan.
- [x] Add a shared layout helper for dock and anchor recalculation.
- [x] Apply dock layout for supported parent containers.
- [x] Apply anchor resize behavior for non-docked children.
- [x] Recalculate layout after parent resize, layout property changes, and project load.
- [x] Make `StatusBar` default to bottom dock behavior.
- [x] Keep `GroupBox` parenting and editing behavior working.
- [x] Keep `TabControl` and `TabPage` behavior working.
- [x] Add Property Inspector dropdown support for `Dock` and `Anchor`.
- [x] Make layout property edits update immediately and remain undoable.
- [x] Preserve dock and anchor metadata through save and load.
- [x] Add validation for dock and anchor values plus layout warnings.
- [ ] Export dock and anchor metadata and generated layout behavior safely.
- [ ] Update the requested documentation files.
- [x] Build the main `VisiForm` app with the `build-static-debug` workflow.
- [x] Fix any compile errors introduced by this phase.
- [x] Update this phase plan with build validation, final result summary, and remaining TODOs.

## Build validation checklist

- [x] Configure with the preset used by `build-static-debug` if needed.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Confirm the main `VisiForm` app built successfully.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

## Manual test checklist

- [ ] Add a `StatusBar` to the root form and verify it defaults to `Dock = Bottom`.
- [ ] Resize the root form and verify the `StatusBar` stays at the bottom and fills width.
- [ ] Set child widgets to `Dock = Top`, `Bottom`, `Left`, `Right`, and `Fill` and verify parent-relative layout.
- [ ] Set non-docked child widgets to different anchor presets and verify expected resize or reposition behavior.
- [ ] Resize a `GroupBox` and verify docked and anchored children update predictably.
- [ ] Resize a `TabControl` and verify the selected `TabPage` content area updates child layout predictably.
- [ ] Save and reload a project with dock and anchor properties and verify values persist.
- [ ] Run project validation and verify invalid dock or anchor values are reported clearly.
- [ ] Export a project using docked and anchored widgets and verify generated output behavior matches the documented support level.
- [ ] Verify `GroupBox` and `TabControl` editing behavior still works.

## Final result summary

- Added shared layout helpers in `src/model/LayoutEngine.h` and `src/model/LayoutEngine.cpp` to calculate parent client bounds and apply dock and anchor behavior for root form, `GroupBox`, `TabPage`, and related container layouts.
- Extended `src/model/WidgetNode.h` and `src/model/WidgetNode.cpp` with normalized `AnchorMode` parsing so readable inspector values and legacy stored strings map to stable runtime behavior.
- Updated `src/model/ProjectDocument.h` and `src/model/ProjectDocument.cpp` so layout is recalculated after insert/reparent, after parent size or layout-property edits, and after project load.
- Updated `src/ui/MainWindow.cpp` so widget bounds edits and property edits relayout children immediately, remain undoable through the existing document-state command path, and keep `StatusBar` defaulted to `Dock = Bottom`.
- Updated `src/model/WidgetRegistry.cpp` and `src/ui/PropertyInspector.cpp` so `Dock` and `Anchor` appear as readable dropdowns, and anchor editing becomes read-only while docking is active.
- Updated `src/serialization/JsonProjectReader.cpp` to apply layout after load; existing generic property serialization continues preserving `dock` and `anchor` values through save and load.
- Updated `src/validation/ProjectValidator.cpp` to validate anchor values and warn about multiple `Fill` siblings that may overlap.
- Updated `CMakeLists.txt` to compile the new shared layout engine.
- The main `VisiForm` app was built successfully with the `build-static-debug` workflow, and no app execution was performed by Agent Mode.
- Export/runtime dock and anchor metadata work and documentation updates remain for a follow-up pass.

## Remaining TODOs

- Add generated export/runtime dock and anchor metadata support in the code generator path.
- Update `docs/component_hierarchy.md`, `docs/widget_catalog.md`, `docs/project_file_format.md`, `docs/code_generation.md`, and `docs/project_validation.md` for the finalized layout behavior.
- Run the manual verification checklist for dock, anchor, `StatusBar`, `GroupBox`, and `TabControl` behavior in the editor and exported output.
