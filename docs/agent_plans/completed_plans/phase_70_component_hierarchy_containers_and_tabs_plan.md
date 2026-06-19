## Phase title

Component hierarchy containers, docking foundations, and tab containers.

## Current design problem

VisiForm currently mixes recursive widget storage with mostly flat designer behavior and incomplete container semantics. The root form already owns child widgets and `WidgetNode` already stores `children`, but widgets do not store an explicit `parentId`, container capability is not standardized in `WidgetDefinition`, and the editor/export pipeline does not yet treat GroupBox, Panel, docking, and TabControl as first-class hierarchy features.

## Architectural goal

Establish a clear component hierarchy where every widget is a component with parent-relative bounds, optional parent ownership, container metadata, recursive drawing and hit testing, hierarchy-aware project tree display, reparenting support, hierarchy-preserving save/load/export, and backward-compatible loading for existing flat projects.

## Files to inspect

- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetDefinition.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/FormNode.h`
- `src/model/FormNode.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/ui/WidgetMetrics.h`
- `src/ui/WidgetMetrics.cpp`
- `src/commands/Command.h`
- `src/commands/Command.cpp`
- `src/commands/UndoRedoStack.h`
- `src/commands/UndoRedoStack.cpp`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/validation/ProjectValidator.h`
- `src/validation/ProjectValidator.cpp`
- `src/generator/CodeGenerator.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `docs/widget_catalog.md`
- `docs/widget_registry.md`
- `docs/project_file_format.md`
- `docs/code_generation.md`
- `docs/project_validation.md`
- `docs/agent_plans/phase_70_component_hierarchy_containers_and_tabs_plan.md`

## Component hierarchy design notes

### Initial diagnosis

- `ProjectDocument` currently stores a single root form as `root` and uses recursive traversal for `findWidgetById`, selection, resource lookups, duplication, and sibling reordering.
- `WidgetNode` already stores `children`, recursive lookup helpers, and parent discovery by searching the tree, but it does not currently store an explicit `parentId` or standardized z-order metadata.
- `ProjectTree` already renders nested rows recursively by walking `widget.children`, so it has basic nested-node support.
- `DesignerCanvas` already draws and hit-tests recursively using parent-local accumulation, so the hierarchy foundation exists, but container-specific behavior and reparenting semantics are incomplete.
- Current widget serialization writes nested `children` arrays and requires `children` during read, so hierarchy already exists in the file format, but backward-compatible flat migration still needs review.
- Export currently walks the widget model recursively through emitter logic, but generated runtime hierarchy and container behavior need inspection and likely expansion for true container widgets, docking, and tabs.

### Confirmed current behavior

- `ProjectDocument` stores the form root in `document.root` and all editing helpers operate by recursive traversal from that root.
- `WidgetNode` already owns `children`, but widgets do not store a `parentId`; parent lookup is derived at runtime with `findParentOf(...)`.
- `ProjectTree` already supports nested rows because `appendRows(...)` recurses through `widget.children` and increases indentation depth.
- `DesignerCanvas` already draws recursively with accumulated parent-local offsets and already performs recursive front-to-back hit testing.
- Current canvas movement logic edits `widget->bounds` directly in parent-local coordinates, then commits move/resize undo commands on mouse release.
- `JsonProjectWriter` serializes nested `children` arrays recursively.
- `JsonProjectReader` currently requires a `root` object and a `children` array on every widget; there is no flat legacy `widgets` migration path yet.
- Export currently has mixed hierarchy behavior: preview drawing is recursive, but generated runtime widget specs are flattened to absolute coordinates rather than preserving explicit parent-child runtime links.

### Component principles for this phase

- Every visible widget is a component.
- The root `FormWindow` is the root component.
- Each component should have an id, name, type, parent-relative bounds, optional parent relationship, child widgets if the type is a container, and layout metadata.
- Drawing and hit testing should remain recursive.
- Save/load/export should preserve hierarchy.
- Existing flat projects must remain loadable by attaching legacy widgets to the root form.
- Model, serialization, and generator layers must remain independent of Visage.

## Resume Notes

- Resume target: `docs/agent_plans/phase_70_component_hierarchy_containers_and_tabs_plan.md`
- Existing modified files already contain explicit hierarchy persistence support in `WidgetNode`, `ProjectDocument`, `JsonProjectReader`, and `JsonProjectWriter`.
- Existing modified files already contain hierarchy-aware validation in `ProjectValidator` and export/runtime metadata support in `VisageCppEmitter`.
- Existing documentation updates are already present in `docs/component_hierarchy.md`, `docs/widget_catalog.md`, `docs/project_file_format.md`, `docs/code_generation.md`, and `docs/project_validation.md`.
- Remaining work for this resume: reconcile stale checklist items, revalidate the main `build-static-debug` build, and refresh the final phase summary plus remaining manual TODOs.

## Step-by-step TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect the current hierarchy, canvas, tree, serialization, validation, and export paths.
- [x] Confirm and document the current root storage, parent tracking, recursive drawing, hit testing, serialization, and export behavior.
- [x] Extend the hierarchy model with explicit parent/layout metadata and consistent container semantics.
- [x] Add container metadata to `WidgetDefinition` and register new container widgets.
- [x] Add `GroupBox` foundation while preserving `Frame` compatibility.
- [x] Add `Panel` foundation as a generic container.
- [x] Add `TabControl` foundation and a safe first-phase tab model.
- [x] Add `dock`, `anchor`, and `layoutMode` properties with minimal root docking support.
- [x] Default `StatusBar` to bottom docking on the root form.
- [ ] Update designer hit testing, drawing, selection, drag/drop, and movement for child widgets and containers.
- [ ] Add widget reparenting support with undo/redo-safe command flow.
- [ ] Update `ProjectTree` to show and select the real hierarchy.
- [ ] Update the `PropertyInspector` for container, parent, dock, anchor, and tab metadata.
- [x] Update save/load to preserve hierarchy and migrate older flat layouts to the root form.
- [x] Update validation for hierarchy integrity, container rules, and docking/tab metadata.
- [x] Update code generation/export to preserve parent-child relationships and recursive container behavior.
- [x] Update documentation for component hierarchy, containers, tabs, project format, validation, and export behavior.
- [x] Validate touched files for compile issues.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Fix any compile errors introduced by this phase.
- [x] Confirm the main `VisiForm` target built successfully.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.
- [x] Update this phase plan with the final result summary and remaining TODOs.

## Build validation checklist

- [x] Configure and build with the existing `build-static-debug` preset flow.
- [x] Confirm the main `VisiForm` target built successfully.
- [x] Confirm no new compile errors remain from this phase.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

## Manual test checklist

- [ ] Verify `GroupBox` appears in the palette and can be added to the form.
- [ ] Verify `Panel` appears in the palette and can be added to the form.
- [ ] Verify `TabControl` appears in the palette and can be added to the form.
- [ ] Verify widgets can be dragged into `GroupBox`.
- [ ] Verify widgets can be dragged into `Panel`.
- [ ] Verify widgets can be dragged back to the root form.
- [ ] Verify selecting a child widget inside a container selects the child.
- [ ] Verify clicking container background selects the container.
- [ ] Verify `ProjectTree` shows nested hierarchy.
- [ ] Verify selecting a tree item selects the same widget in the designer.
- [ ] Verify `StatusBar` defaults to bottom dock and spans the form width.
- [ ] Verify the `dock` property can be changed from the property inspector.
- [ ] Verify `TabControl` tab headers render and the selected tab can change in the designer.
- [ ] Verify child widgets inside the selected tab render correctly.
- [ ] Verify save and reload preserve parent-child hierarchy.
- [ ] Verify older flat projects still load with widgets attached to the root form.
- [ ] Verify validation reports hierarchy issues such as invalid parenting or cycles.
- [ ] Verify export preserves container hierarchy and generated projects still build in Debug and Release.
- [ ] Verify existing save/load/export/image resources/menu/toolbar/undo-shortcut flows still work.

## Final result summary

- Repaired the current `MainWindow` hierarchy interaction build break by moving the new absolute/relative bounds helpers back to namespace scope and restoring the `resolveDropParentId(...)` `MainWindow` declaration. The main `VisiForm` target now builds successfully in Debug without launching the app.
- Resumed `docs/agent_plans/phase_70_component_hierarchy_containers_and_tabs_plan.md`, audited the current modified hierarchy files, and confirmed that persistence, hierarchy validation, export/runtime hierarchy metadata, and the requested hierarchy documentation updates were already present in the active Phase 70 workspace changes.
- Reconciled the stale checklist by marking the confirmed persistence, validation, export, and documentation items complete, added `Resume Notes`, and corrected the `docs/project_validation.md` range text to match the current validator behavior.
- Revalidated the main app with `cmake --build --preset build-static-debug`; the preset completed successfully with `ninja: no work to do`. `VisiForm.exe` was not run and no generated apps were launched.

## Remaining TODOs

- Confirm whether the remaining editor interaction checklist items can now be marked complete after manual verification:
  - `Update designer hit testing, drawing, selection, drag/drop, and movement for child widgets and containers.`
  - `Add widget reparenting support with undo/redo-safe command flow.`
  - `Update ProjectTree to show and select the real hierarchy.`
  - `Update the PropertyInspector for container, parent, dock, anchor, and tab metadata.`
- Complete the manual test checklist, especially hierarchy drag/drop, save/reload migration, validation edge cases, and exported-project build verification.
