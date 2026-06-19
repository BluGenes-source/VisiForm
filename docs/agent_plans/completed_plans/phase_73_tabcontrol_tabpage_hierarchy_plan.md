# Phase 73 TabControl TabPage Hierarchy Plan

## Phase title
- Add a usable `TabControl` widget with logical `TabPage` hierarchy support while preserving existing `GroupBox` behavior.

## Current state
- `VisiForm` remains version `1.0.0`.
- `GroupBox` is already treated as a selectable container widget.
- `WidgetNode` stores hierarchy through `children`, `parentId`, and `zOrder` metadata.
- `ProjectDocument` appends child widgets directly into the selected parent node and reapplies dock layout after hierarchy changes.
- `ProjectTree` already renders hierarchy recursively from `widget.children`.
- `DesignerCanvas` already uses recursive child-first hit testing and recursive drawing for container children.
- Existing `TabControl` support is only partial: it uses flat child widgets with `tabs` and `selectedTab` properties instead of real `TabPage` child nodes.
- Save/load and export already walk recursive `children`, so real `TabPage` nodes can reuse the existing hierarchy persistence shape.

### Baseline hierarchy notes from initial inspection
- How `GroupBox` stores children:
  - `WidgetNode` stores children in `std::vector<WidgetNode> children` and `appendChild(...)` assigns `zOrder`, `parentId`, and recurses metadata sync.
  - `ProjectDocument::addChildToParent(...)` and `reparentWidget(...)` add descendants by mutating the parent node’s `children` list.
- How `parentId` is stored:
  - `WidgetNode::syncHierarchyMetadata(...)` writes `parentId` and normalizes all descendants.
  - JSON save writes `parentId` explicitly for every widget, and JSON load reads it back before metadata refresh.
- How recursive drawing works:
  - `DesignerCanvas` already renders widget hierarchies recursively and uses parent-relative bounds accumulated to screen/form coordinates.
  - Existing tab preview behavior filters flat child widgets by `tabIndex` when the parent is a `TabControl`.
- How recursive hit testing works:
  - `WidgetNode::hitTest(...)` checks children in reverse order first, then falls back to the current widget if its bounds contain the point.
  - `DesignerCanvas` and `MainWindow` build on that child-first hit ordering.
- How `ProjectTree` displays children:
  - `ProjectTree::appendRows(...)` recursively appends every `widget.children` entry beneath its parent.
- How save/load stores children:
  - `JsonProjectWriter::widgetToJson(...)` writes `children` recursively.
  - `JsonProjectReader::parseWidget(...)` reads `children` recursively.
- How export currently walks hierarchy:
  - Generator and validation paths already traverse recursive widget trees.
  - Existing generated `TabControl` behavior still assumes flat child widgets grouped by `tabIndex`, so export must be updated for real `TabPage` nodes.

## Goal
- Add a real `TabControl` widget that owns logical `TabPage` child containers.
- Allow `TabPage` nodes to own normal widgets.
- Keep the canvas, tree, property inspector, serialization, validation, undo/redo, and export paths consistent with the new hierarchy.
- Preserve all current `GroupBox` workflows.

## Files to inspect
- [x] `.github/copilot-instructions.md`
- [x] `.github/instructions/visiform.instructions.md`
- [x] `src/model/ProjectDocument.h`
- [x] `src/model/ProjectDocument.cpp`
- [x] `src/model/WidgetNode.h`
- [x] `src/model/WidgetNode.cpp`
- [x] `src/model/WidgetDefinition.h`
- [x] `src/model/WidgetDefinition.cpp`
- [x] `src/model/WidgetRegistry.h`
- [x] `src/model/WidgetRegistry.cpp`
- [x] `src/model/FormNode.h`
- [x] `src/ui/DesignerCanvas.h`
- [x] `src/ui/DesignerCanvas.cpp`
- [x] `src/ui/MainWindow.h`
- [x] `src/ui/MainWindow.cpp`
- [ ] `src/ui/ProjectTree.h`
- [x] `src/ui/ProjectTree.cpp`
- [ ] `src/ui/PropertyInspector.h`
- [x] `src/ui/PropertyInspector.cpp`
- [ ] `src/ui/WidgetPalette.h`
- [x] `src/ui/WidgetPalette.cpp`
- [ ] `src/ui/WidgetMetrics.h`
- [ ] `src/ui/WidgetMetrics.cpp`
- [ ] `src/commands/Command.h`
- [x] `src/commands/Command.cpp`
- [ ] `src/commands/UndoRedoStack.h`
- [ ] `src/commands/UndoRedoStack.cpp`
- [x] `src/serialization/JsonProjectReader.cpp`
- [x] `src/serialization/JsonProjectWriter.cpp`
- [x] `src/validation/ProjectValidator.cpp`
- [x] `src/generator/VisageCppEmitter.cpp`
- [x] `src/generator/CodeGenerator.cpp`
- [ ] `docs/component_hierarchy.md`
- [ ] `docs/widget_catalog.md`
- [ ] `docs/project_file_format.md`
- [ ] `docs/code_generation.md`
- [ ] `docs/project_validation.md`
- [x] `docs/agent_plans/phase_73_tabcontrol_tabpage_hierarchy_plan.md`

## TabControl design notes
- Add `WidgetType::TabPage` as a logical container child type.
- Keep `TabPage` out of the normal palette unless later inspection proves the current UI requires it.
- `TabControl` may contain only `TabPage` children.
- `TabPage` may contain normal widgets, including `GroupBox` when supported by existing hierarchy behavior.
- Adding a `TabControl` should automatically create two default `TabPage` children named `Tab 1` and `Tab 2`.
- The editor should route new normal widgets into the active `TabPage` when the current selection is a `TabControl`, a `TabPage`, or a descendant inside a `TabPage`, with existing `GroupBox` insertion priority preserved.
- The canvas should draw only the selected page’s children and only hit test the selected page’s descendants.
- `ProjectTree` should expose `TabControl -> TabPage -> child widget` hierarchy directly from the model tree.
- `PropertyInspector` should expose editable selected tab state and page titles, plus add/remove tab actions with safe constraints.
- Serialization, validation, and export should use real child nodes instead of only flat `tabIndex` metadata.
- Generated code must stay safe even if generated runtime behavior is initially limited.

## Step-by-step TODO checklist
- [x] Create this phase plan file before code changes.
- [x] Inspect current hierarchy behavior for `GroupBox`, recursive drawing, recursive hit testing, tree display, save/load, and export traversal.
- [ ] Inspect remaining UI, command, and documentation files needed for `TabControl` / `TabPage` changes.
- [x] Add `TabPage` model support and tighten `TabControl` container rules in the model and registry.
- [x] Implement default `TabControl` creation with two default `TabPage` children and selected-tab metadata.
- [x] Implement insertion-parent resolution so new normal widgets target the active `TabPage` while preserving `GroupBox` priority.
- [x] Implement `TabPage` add/remove/rename workflows in the editor with safe empty-page removal limits.
- [x] Update `DesignerCanvas` rendering, selected-page child drawing, selection visuals, and tab-header hit testing.
- [ ] Update `ProjectTree` selection behavior so selecting tab nodes activates the correct page.
- [x] Update `PropertyInspector` rows and actions for `TabControl` and `TabPage` editing.
- [ ] Update undo/redo-affecting editor flows for tab selection and tab page modifications where required.
- [ ] Update JSON save/load and validation for `TabControl`, `TabPage`, selected tab index, and page titles.
- [ ] Update export/generation to preserve real `TabControl` / `TabPage` hierarchy safely.
- [ ] Update hierarchy, widget catalog, project format, code generation, and validation documentation.
- [x] Build the main `VisiForm` app with the `build-static-debug` preset.
- [ ] Update this plan with final results, build status, remaining limitations, and manual verification notes.

## Build validation checklist
- [x] Build with the existing `build-static-debug` preset.
- [x] Confirm the main `VisiForm` target built successfully.
- [x] Confirm the main `VisiForm` app was built successfully.
- [x] Confirm no compile errors remain from this phase.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

## Manual test checklist
- [ ] Add a `TabControl` from the palette and verify it creates two default `TabPage` children.
- [ ] Verify the canvas draws tab headers and only the selected tab page content.
- [ ] Click each tab header and verify the selected tab changes.
- [ ] Select the `TabControl` in the canvas and add a `Button`; verify it appears under the selected `TabPage`.
- [ ] Select the `TabPage` in `ProjectTree` and add a `Label`; verify it is inserted into that page.
- [ ] Select a child inside a `TabPage` and add another widget; verify insertion still targets the correct page.
- [ ] Add a `GroupBox` inside a `TabPage` and verify existing `GroupBox` behavior still works.
- [ ] Verify `ProjectTree` shows `TabControl -> TabPage -> child widgets`.
- [ ] Verify selecting a `TabPage` from `ProjectTree` activates that page in the canvas.
- [ ] Verify `PropertyInspector` can rename tab pages and change the selected tab.
- [ ] Verify adding a new tab page works.
- [ ] Verify removing an empty tab page works and removing the last page is blocked.
- [ ] Verify removing a non-empty tab page is blocked with a clear status message.
- [ ] Move the `TabControl` and verify page children move visually with it.
- [ ] Move a child inside a `TabPage` and verify coordinates remain page-relative.
- [ ] Save and reload a project containing `TabControl` page children.
- [ ] Validate a project and verify invalid tab hierarchy reports clear messages.
- [ ] Export a project with `TabControl` content and verify generated Debug and Release builds still work.
- [ ] Verify existing root and `GroupBox` insertion behavior still works.

## Final result summary
- Completed for this repair pass. Phase 73 now includes real `TabPage` model support, default `TabControl` page creation, inspector-driven tab add/remove/rename actions, and the repair-pass fixes that keep empty tab content selectable as the owning `TabControl`, keep new `TabControl` insertion at the root form, and prevent `TabControl` drag moves from changing parent targets. The main `VisiForm` app was rebuilt successfully with `build-static-debug` after re-running the build from the Visual Studio x64 developer environment.

## Remaining TODOs
- Run the manual editor verification checklist for `TabControl` selection, move, resize, tree selection, save/load, and export behavior in Visual Studio.
- If exported runtime tab behavior still needs refinement beyond build safety, capture it as a later phase item instead of expanding this repair pass.

## Repair Pass - TabControl Move Resize and Parenting

### Current failed behavior

- `TabControl` can be added, and the current add flow already selects the new `TabControl` after creation.
- Clicking the visible tab content area often selects the active `TabPage` instead of keeping the `TabControl` selected.
- Once the active `TabPage` becomes the primary selection, resize handles are intentionally suppressed and drag/resize start is blocked.
- `TabControl` page bounds are created once, but they are not recalculated when the owning `TabControl` is resized.
- New `TabControl` insertion still follows the generic container-parent resolver, so it can be routed under a selected `GroupBox` or `TabPage` during this repair pass.
- `ProjectTree` selection currently performs generic widget selection only, so selecting a `TabPage` descendant does not automatically activate the owning tab.

### Root cause diagnosis

- `src/ui/DesignerCanvas.cpp`
  - Recursive child-first hit testing returns the visible `TabPage` for empty tab content before the parent `TabControl` can claim the hit.
  - `DesignerCanvas::hitTestInteraction(...)` returns `nullopt` when the current primary selection is a `TabPage`, so move/resize interaction cannot start from the tab content area once the page is selected.
  - Tab page preview bounds come from `tabPageBoundsForTabControl(...)` at creation time, but no matching resize-time recomputation keeps the page bounds in sync with `TabControl` resize changes.
- `src/ui/MainWindow.cpp`
  - `handleWidgetClicked(...)` currently performs generic selection only; it does not normalize `TabPage` or tab-descendant selection to activate the owning `TabControl` page first.
  - `insertionParentIdForNewWidget(...)` still allows `TabControl` placement under the currently selected container instead of forcing the repair-pass-safe root placement.
  - `ProjectTree` click handling forwards directly to generic selection, so tab activation is not synchronized for tree-driven tab hierarchy selection.

### Files inspected

- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `docs/agent_plans/phase_73_tabcontrol_tabpage_hierarchy_plan.md`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/ProjectTree.cpp`
- `src/ui/PropertyInspector.cpp`
- `src/ui/WidgetPalette.cpp`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/WidgetRegistry.cpp`
- `src/commands/Command.cpp`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/validation/ProjectValidator.cpp`
- `src/generator/VisageCppEmitter.cpp`

### Specific code paths changed

- `src/ui/DesignerCanvas.cpp`
  - `hitTestWidgetScreenId(...)`
- `src/ui/MainWindow.cpp`
  - `insertionParentIdForNewWidget(...)`
  - `resolveDropParentId(...)`

### Step-by-step TODO checklist

- [x] Diagnose the current `TabControl` selection, hit-test, move/resize, and insertion-parent failures.
- [x] Update `DesignerCanvas` tab hit testing so empty tab content can keep `TabControl` interaction available while child widgets still win when directly hit.
- [x] Verify existing `ProjectDocument` tab selection routing already activates the owning tab before canvas or tree interaction continues.
- [x] Restrict new `TabControl` insertion to the repair-pass-safe root parent.
- [x] Verify existing `ProjectDocument` tab-page bounds refresh already recalculates `TabPage` preview bounds during `TabControl` resize refresh.
- [x] Verify `ProjectTree`, `PropertyInspector`, serialization, validation, and export behavior was inspected and left unchanged for this focused repair pass.
- [x] Build `VisiForm` with `build-static-debug`.
- [x] Update this repair-pass section with final changed paths, build status, manual verification notes, remaining TODOs, and summary.

### Build validation checklist

- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Confirm the main `VisiForm` app built successfully.
- [x] Confirm compile errors introduced by this repair pass were fixed.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

### Manual test checklist

- [ ] Add a `TabControl` and confirm it is selected immediately after creation.
- [ ] Click a tab header and confirm the `TabControl` stays selectable while the selected tab changes.
- [ ] Click empty tab content and confirm move/resize still targets the `TabControl`.
- [ ] Drag the selected `TabControl` and confirm the full control moves on the root canvas.
- [ ] Resize the selected `TabControl` and confirm the tab header and content area resize together.
- [ ] Confirm `TabPage` children move visually with the `TabControl` while their local coordinates remain unchanged.
- [ ] Select `TabControl`, `TabPage`, and tab-child widgets, then add normal widgets and confirm insertion targets the correct selected `TabPage` unless `GroupBox` priority applies.
- [ ] Confirm `ProjectTree` selection activates the correct tab page for `TabPage` and tab-child rows.
- [ ] Save, reload, validate, and export a project containing `TabControl` page children.

### Final result summary

- Completed. Empty `TabControl` content now hit-tests back to the owning `TabControl`, so header, border, and empty content clicks keep move/resize interaction on the `TabControl` while direct child-widget hits still select the child. New `TabControl` widgets now insert at the root form for this repair pass, and moving a `TabControl` no longer changes its drop-parent target. Existing `ProjectDocument` tab activation and tab-page bound refresh logic remained sufficient and did not require additional code changes. The main `VisiForm` target built successfully with `build-static-debug` from the Visual Studio x64 developer environment, and `VisiForm.exe` was not run.
