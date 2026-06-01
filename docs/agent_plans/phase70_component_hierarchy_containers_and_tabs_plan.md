# Phase 70 Component Hierarchy Containers and Tabs Plan

## Repair Pass - GroupBox Parenting Workflow

### Current failed tests
- Adding a `Button` inside a `GroupBox` by dragging/dropping or creating inside it failed.
- `ProjectTree` did not reliably show the `Button` under `GroupBox`.
- Selecting the `Button` inside `GroupBox` from the canvas failed or behaved incorrectly.
- Moving the `Button` inside `GroupBox` failed or did not use correct relative coordinates.
- Dragging the `Button` out of `GroupBox` to the root form failed or did not update `ProjectTree` correctly.

### Root cause diagnosis
- Widget creation currently flows through `WidgetPalette::hitTestWidgetType(...)` and `MainWindow::addWidgetFromPalette(...)`.
- `MainWindow::addWidgetFromPalette(...)` always uses `document_.root.id` when constructing `AddWidgetCommand`, so selected `GroupBox` is ignored and newly created widgets are always added at the root.
- `AddWidgetCommand` already supports `parentId`, and `ProjectDocument::addChildToParent(...)` / `reparentWidget(...)` already support parent-aware insertion and reparenting.
- `IdGenerator` and `WidgetRegistry::createDefaultWidget(...)` are already preserved by `MainWindow::createDefaultWidget(...)` and should remain unchanged.
- `WidgetRegistry` currently marks `GroupBox`, `Frame`, `Panel`, `TabControl`, and `FormWindow` as containers, so repair-pass insertion rules must be narrowed in `MainWindow` instead of changing registry metadata broadly.
- `WidgetNode::bounds` are stored parent-relative for children; drawing and hit testing in `DesignerCanvas` already accumulate parent offsets recursively.
- `ProjectTree` already renders recursively from `widget.children`, so hierarchy display depends on correct parent/child updates.
- Canvas move and drop handling already has reparent support through `resolveDropParentId(...)`, `absoluteBoundsForWidget(...)`, and `boundsRelativeToParent(...)`; this repair pass narrows active drop targets to real `GroupBox`/root hits and fixes status handling after undoable document swaps.
- `PropertyInspector` currently has no GroupBox child list, no add-existing-child action, and no remove-to-root action.
- Serialization, validation, and export paths already preserve nested `children`, `parentId`, and parent-relative bounds, so the repair pass can reuse the existing hierarchy persistence model without schema changes.

### Files inspected
- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/ProjectTree.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/WidgetPalette.cpp`
- `src/commands/Command.h`
- `src/commands/Command.cpp`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/validation/ProjectValidator.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `docs/agent_plans/phase_70_component_hierarchy_containers_and_tabs_plan.md`

### Step-by-step TODO checklist
- [x] Create or update the repair-pass phase plan.
- [x] Diagnose the current widget insertion parent selection flow.
- [x] Confirm current container metadata and GroupBox-specific constraints.
- [x] Confirm current coordinate, drawing, hit testing, and tree hierarchy behavior.
- [x] Implement selected-parent insertion behavior for `GroupBox` and root only.
- [x] Implement GroupBox-relative move and drag-out reparent behavior.
- [x] Implement `PropertyInspector` GroupBox child list add/remove workflow.
- [ ] Verify `ProjectTree` refresh and selection behavior after reparenting.
- [ ] Verify save/load, validation, and export behavior remain correct for GroupBox children.
- [x] Update hierarchy documentation for the GroupBox repair pass.
- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Update this plan with final results and remaining TODOs.

### Build validation checklist
- [ ] Build with the existing `build-static-debug` preset.
- [ ] Confirm the main `VisiForm` target built successfully.
- [ ] Confirm no compile errors remain from this repair pass.
- [ ] Confirm `VisiForm.exe` was not run.
- [ ] Confirm no generated apps were launched.

### Manual test checklist
- [ ] Add a `GroupBox` to the root form.
- [ ] Select the `GroupBox` and add a `Button`; verify the `Button` is inserted into the `GroupBox`.
- [ ] Select the root form and add a `Button`; verify the `Button` is inserted at root.
- [ ] Select the child `Button` from the canvas inside the `GroupBox`.
- [ ] Select the child `Button` from `ProjectTree`.
- [ ] Move the child `Button` inside the `GroupBox` and verify local coordinates remain correct.
- [ ] Drag the child `Button` out of the `GroupBox` and verify it reparents to the root form.
- [ ] Use the `PropertyInspector` child list to add an existing root widget to the `GroupBox`.
- [ ] Use the `PropertyInspector` child list to remove a `GroupBox` child back to the root form.
- [ ] Save and reload a project with a `GroupBox` child widget.
- [ ] Validate the project and verify hierarchy errors still report clearly.
- [ ] Export a project with a `GroupBox` child widget and verify generated builds still work.

### Final result summary
- In progress. New palette/menu insertion now selects the current `GroupBox` as the parent when a `GroupBox` is selected, otherwise it inserts at the root form, new `GroupBox` children receive GroupBox-local default bounds, canvas reparenting now resolves only real `GroupBox`/root drop targets, the `PropertyInspector` now exposes GroupBox child rows with select/add/remove actions, and the hierarchy docs now describe the repair-pass behavior.

### Remaining TODOs
- Run the build and finish final validation notes.

## Repair Pass - GroupBox Box Selection and Multi-Select

### Current failed behavior
- Dragging a box-selection rectangle on the root canvas still works, but starting the same gesture from empty space inside a `GroupBox` content area does not reliably enter marquee mode.
- Because marquee mode does not start from `GroupBox` content hits, no selection rectangle is drawn there and child widgets inside the `GroupBox` are not box-selected.
- Child widget outlines already render from absolute canvas positions when ids are selected, but the current marquee completion path always evaluates a root-wide recursive selection set and has no explicit `GroupBox` scope.

### Root cause diagnosis
- Box selection starts in `MainWindow::mouseDown(...)` when `DesignerCanvas::hitTestWidgetId(...)` returns the root form id or no widget hit.
- Marquee start/end points are stored in `canvasInteraction_.dragStart` and `canvasInteraction_.currentPoint`, then normalized by `normalizedSelectionRect(...)`.
- Those coordinates are already root canvas/form coordinates because `DesignerCanvas::toFormPoint(...)` converts screen coordinates into root-form coordinates.
- `DesignerCanvas::draw(...)` draws the marquee rectangle after widget rendering, so the rectangle is not inherently hidden behind `GroupBox` children or the grid.
- `GroupBox` child hit testing does not start marquee selection itself; clicking empty `GroupBox` content returns the `GroupBox` id, which keeps the event on the widget click / move path instead of the marquee path.
- Multi-selection is stored in `ProjectDocument::selectedWidgetIds_`, supports nested child widget ids, and `ProjectTree` already lists child widgets recursively, so the main failure is start/scope logic rather than selection storage.
- The current intersection helper `collectIntersectingWidgetIds(...)` always traverses from `document_.root` and does not distinguish root scope from active-`GroupBox` scope.

### Files inspected
- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/commands/UndoRedoStack.h`
- `src/commands/UndoRedoStack.cpp`
- `src/commands/Command.h`
- `src/commands/Command.cpp`
- `docs/component_hierarchy.md`
- `docs/widget_catalog.md`
- `docs/agent_plans/phase70_component_hierarchy_containers_and_tabs_plan.md`
- `docs/agent_plans/phase_34_fix_box_selection_plan.md`

### Step-by-step TODO checklist
- [x] Append the GroupBox box-selection repair-pass section to the Phase 70 plan.
- [x] Diagnose marquee start, storage, draw order, and selection storage behavior.
- [x] Add scoped marquee-selection state for root canvas versus active `GroupBox` selection.
- [x] Allow marquee start from empty content inside a selected or active `GroupBox` without breaking child drag behavior.
- [x] Convert selection candidates to root coordinates and collect intersecting widgets recursively by scope.
- [x] Preserve root-level box selection behavior and avoid selecting unrelated widgets outside the active `GroupBox` scope.
- [x] Update status text, hierarchy docs, and widget catalog notes for GroupBox box selection.
- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Update this section with final validation results, manual-test notes, and remaining TODOs.

### Build validation checklist
- [ ] Build with the existing `build-static-debug` preset.
- [ ] Confirm the main `VisiForm` target built successfully.
- [ ] Confirm no compile errors remain from this repair pass.
- [ ] Confirm `VisiForm.exe` was not run.
- [ ] Confirm no generated apps were launched.

### Manual test checklist
- [ ] Drag a marquee rectangle on empty root-canvas space and verify the rectangle remains visible.
- [ ] Verify root-canvas marquee selection still selects root-level widgets.
- [ ] Select a `GroupBox`, drag from empty content inside it, and verify a visible marquee rectangle appears.
- [ ] Verify the `GroupBox` marquee selects only child widgets inside that `GroupBox`.
- [ ] Verify selected child widgets render the expected multi-select outlines and correctly placed handles.
- [ ] Verify clicking and dragging directly on a child widget still keeps the existing child move behavior.
- [ ] Verify moving multiple selected children inside the same `GroupBox` keeps their parent-relative coordinates.
- [ ] Verify `ProjectTree`, save/load, and export still preserve the `GroupBox` hierarchy.

### Final result summary
- In progress. `MainWindow` now stores marquee scope per drag, empty content clicks on a selected or active `GroupBox` start a scoped marquee, selection candidates are collected recursively in root coordinates with descendant-preferred selection results, the marquee rectangle now includes a translucent fill, and the hierarchy docs now describe GroupBox-local box selection.

### Remaining TODOs
- Run the required `build-static-debug` validation and finish manual verification notes.

## Repair Pass - GroupBox Selection and Movement

### Current failed behavior
- A newly added `GroupBox` is created, but it is not reliably movable from its empty body area on the designer canvas.
- Clicking or dragging in empty `GroupBox` content can enter the recently added GroupBox-local marquee path instead of the normal GroupBox move path.
- When another `GroupBox` is already selected, adding a new `GroupBox` can still inherit that selected `GroupBox` as its parent, which is not the intended root-level workflow for this repair pass.

### Root cause diagnosis
- `AddWidgetCommand::execute()` already selects the newly added widget, so immediate post-create selection is not missing in the command layer.
- `MainWindow::addWidgetFromPalette(...)` currently delegates parent choice through `insertionParentIdForNewWidget(...)`; that helper does not distinguish `GroupBox` creation from normal child-widget insertion, so a new `GroupBox` can be inserted into the currently selected `GroupBox`.
- `DesignerCanvas::hitTestWidgetId(...)` already uses child-first recursive hit testing and falls back to the container itself, so GroupBox title, border, and background are inherently hit-testable when not intercepted earlier.
- The current GroupBox-local marquee branch in `MainWindow::mouseDown(...)` runs before normal move setup and is triggered from empty GroupBox content whenever the `GroupBox` is selected or active, which steals clicks that should select or drag the `GroupBox` itself.
- GroupBox movement itself is not blocked in `mouseDrag(...)`; once move mode starts, the delta is applied to the selected widget bounds and child widgets follow visually because their bounds remain parent-relative.

### Files inspected
- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/commands/UndoRedoStack.h`
- `src/commands/UndoRedoStack.cpp`
- `src/commands/Command.h`
- `src/commands/Command.cpp`
- `docs/component_hierarchy.md`
- `docs/widget_catalog.md`
- `docs/agent_plans/phase70_component_hierarchy_containers_and_tabs_plan.md`

### Step-by-step TODO checklist
- [x] Append the GroupBox selection and movement repair-pass section to the Phase 70 plan.
- [x] Diagnose GroupBox creation, hit testing, move handling, and selection-sync behavior.
- [x] Keep new `GroupBox` creation root-level for this repair pass and preserve immediate selection after add.
- [x] Constrain GroupBox-local marquee start so normal GroupBox selection and movement still work.
- [x] Preserve child-first hit testing so children remain selectable while empty GroupBox area selects the GroupBox.
- [x] Preserve root-level GroupBox move behavior, child-relative coordinates, and existing move undo behavior.
- [x] Update hierarchy and widget documentation for GroupBox selection and movement behavior.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Update this section with final validation results, manual-test notes, and remaining TODOs.

### Build validation checklist
- [x] Build with the existing `build-static-debug` preset.
- [x] Confirm the main `VisiForm` target built successfully.
- [x] Confirm no compile errors remain from this repair pass.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

### Manual test checklist
- [ ] Add a root-level `GroupBox` and verify it is selected immediately after creation.
- [ ] Click the `GroupBox` border, title, and empty body to verify the `GroupBox` is selected.
- [ ] Drag the selected root-level `GroupBox` from its empty area and verify it moves on the root canvas.
- [ ] Verify child widgets move visually with the `GroupBox` while their local coordinates stay unchanged.
- [ ] Click a child widget inside the `GroupBox` and verify the child still becomes selected.
- [ ] Verify `ProjectTree` and `PropertyInspector` follow GroupBox canvas selection.
- [ ] Undo and redo a GroupBox move and verify the `GroupBox` returns to the previous position.
- [ ] Save, reload, and export a project containing a moved `GroupBox` with children.

### Final result summary
- Completed. `MainWindow` now keeps newly added `GroupBox` widgets at the root for this repair pass, refreshes inspector state after add, and only starts GroupBox-local marquee selection when explicit additive multi-select intent is active, which restores normal empty-area GroupBox selection and dragging while preserving child-first hit testing and existing move behavior. The main `VisiForm` app was then built successfully with `build-static-debug` from an x64 Visual Studio developer environment, and no new compile errors remained from this repair pass. This continuation revalidated the same build path with `cmake --build --preset build-static-debug`, which completed cleanly with `ninja: no work to do.`

### Remaining TODOs
- Manually verify GroupBox add/select/move behavior in the canvas, `ProjectTree`, and `PropertyInspector`.
- Manually verify undo/redo, save/load, and export behavior for moved `GroupBox` widgets with children.

## Critical Repair Pass - GroupBox Still Cannot Be Selected or Moved

### Current failed behavior
- After adding or dropping a `GroupBox` onto the root form, the `GroupBox` appears on the designer canvas but does not reliably behave like a normal selectable widget.
- Clicking the `GroupBox` title, border, or empty body does not consistently leave the `GroupBox` selected for immediate drag interaction.
- Dragging from the `GroupBox` body often fails to start a move even though the `GroupBox` is visible and root-level.
- Child widgets can still participate in recursive hit testing, which makes the regression look like the `GroupBox` is treated as only a child container instead of both a widget and a container.

### Root cause diagnosis
- `DesignerCanvas::hitTestWidgetId(...)` and the recursive helper `hitTestWidgetScreenId(...)` already walk children front-to-back and fall back to the container widget when no child is hit, so the canvas hit-test algorithm itself still resolves empty `GroupBox` area clicks to the `GroupBox` id.
- The remaining failure is in `MainWindow::mouseDown(...)`: the `GroupBox`-local marquee branch still runs before normal drag-start setup and had no additive-selection guard, so empty `GroupBox` body clicks were diverted into marquee mode instead of selecting or moving the `GroupBox`.
- The same method also only armed move or resize when `wasSelected` was true before `handleWidgetClicked(...)` updated the selection, so a newly added or newly clicked `GroupBox` could become selected in the document state without creating a same-press drag start.
- Together those two ordering bugs made the `GroupBox` look like a child-only container even though recursive hit testing, absolute-bounds math, selection overlays, child-local movement math, and move undo infrastructure were already valid for a selectable container widget.

### Files inspected
- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetDefinition.cpp`
- `src/commands/Command.h`
- `src/commands/Command.cpp`
- `src/commands/UndoRedoStack.h`
- `src/commands/UndoRedoStack.cpp`
- `docs/component_hierarchy.md`
- `docs/widget_catalog.md`
- `docs/agent_plans/phase70_component_hierarchy_containers_and_tabs_plan.md`

### Specific code paths changed
- [x] `MainWindow::mouseDown(...)`
- [x] `MainWindow` GroupBox hit/drag diagnostics via `logCanvasHitDiagnostic(...)` and `writeCanvasInteractionDebug(...)`
- [x] `MainWindow::addWidgetFromPalette(...)`
- [ ] `DesignerCanvas::hitTestWidgetId(...)` or recursive hit-test helpers if the diagnosis requires it
- [ ] `DesignerCanvas::draw(...)` selection overlay behavior if the diagnosis requires it
- [x] Phase-plan and documentation updates

### Step-by-step TODO checklist
- [x] Append this critical repair-pass section to the Phase 70 plan.
- [x] Inspect the listed canvas, selection, widget-definition, and undo files.
- [x] Confirm the current hit-test recursion and identify the drag-start ordering bug.
- [x] Add low-noise click and drag diagnostics for GroupBox hit testing.
- [x] Ensure newly added root-level `GroupBox` widgets remain selected immediately after creation.
- [x] Fix `GroupBox` click-to-select and drag-start behavior on title, border, and empty body hits.
- [x] Preserve deepest-child hit testing for child widgets inside a `GroupBox`.
- [x] Verify move math keeps child local coordinates stable while the `GroupBox` moves.
- [x] Verify selection sync with `ProjectTree` and `PropertyInspector`.
- [x] Update hierarchy and widget documentation for the repaired `GroupBox` behavior.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Update this section with final results and remaining TODOs.

### Build validation checklist
- [x] Build with the existing `build-static-debug` preset.
- [x] Confirm the main `VisiForm` target built successfully.
- [x] Confirm no compile errors remain from this critical repair pass.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

### Manual test checklist
- [ ] Add a root-level `GroupBox` and verify it is selected immediately.
- [ ] Click the `GroupBox` title and verify the `GroupBox` selects.
- [ ] Click the `GroupBox` border and verify the `GroupBox` selects.
- [ ] Click the empty `GroupBox` body and verify the `GroupBox` selects.
- [ ] Drag the selected `GroupBox` and verify it moves on the canvas.
- [ ] Verify `PropertyInspector` x/y values change when the `GroupBox` moves.
- [ ] Verify child widgets move visually with the `GroupBox` while child local x/y stay unchanged.
- [ ] Click a child inside the `GroupBox` and verify the child selects.
- [ ] Drag a child inside the `GroupBox` and verify the child moves locally without moving the `GroupBox`.
- [ ] Verify `ProjectTree` selection stays in sync with canvas selection.
- [ ] Undo and redo a `GroupBox` move and verify the position is restored.
- [ ] Save and reload a project with a moved `GroupBox` and verify positions persist.
- [ ] Export a project with a moved `GroupBox` and verify generated builds still work.

### Final result summary
- Completed. The critical root cause was not recursive hit testing itself; it was the ordering inside `MainWindow::mouseDown(...)`. Empty `GroupBox` body clicks were still diverted into GroupBox-local marquee mode before normal drag setup, and move/resize setup still depended on the widget already being selected before `handleWidgetClicked(...)` ran. This pass fixed both issues by restricting GroupBox-local marquee start to additive multi-select input only, then arming same-press move/resize after selection so a `GroupBox` behaves as both a selectable widget and a child container. The repair also added low-noise `OutputDebugStringA` diagnostics for GroupBox-related hit tests and drag-start outcomes, preserved existing deepest-child selection for child widgets, kept root-level `GroupBox` creation selected with `Added GroupBox`, and updated the hierarchy/widget docs to match the repaired interaction behavior. The main `VisiForm` app was then built successfully with `build-static-debug` from an explicit x64 Visual Studio developer command environment, and no compile errors remained from this repair.

### Remaining TODOs
- Manually verify root-level `GroupBox` add/select/move behavior on the canvas.
- Manually verify child selection and child-local movement inside a `GroupBox`.
- Manually verify `ProjectTree`/`PropertyInspector` sync, undo/redo, save/load, and export behavior for moved `GroupBox` widgets with children.
