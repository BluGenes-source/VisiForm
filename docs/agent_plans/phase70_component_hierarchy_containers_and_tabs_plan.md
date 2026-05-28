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
