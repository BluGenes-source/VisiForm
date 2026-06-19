# Phase 70 — Repair Pass: GroupBox Selection and Movement

We are working in the existing C++ project at:

```text
J:\Dev\CeePlusPlus\VisiForm
```

Use the repository Copilot rules in:

```text
.github/copilot-instructions.md
.github/instructions/visiform.instructions.md
```

## Important Agent Mode Instruction

Do not run `VisiForm.exe`.

Do not use `Start-Process`.

Do not try to automate keyboard input.

Do not launch generated apps.

Only build the main VisiForm project with `build-static-debug` and fix compile errors.

The user will manually run VisiForm from Visual Studio.

## Phase Context

This is a Phase 70 bug-fix continuation.

Do not start a new unrelated phase.

Do not implement the command palette.

Do not implement full Panel/TabControl parenting in this repair unless required to avoid breaking existing code.

Focus only on GroupBox selection, hit testing, and movement behavior.

## Phase Plan File

Create or update this phase plan:

```text
docs/agent_plans/phase70_component_hierarchy_containers_and_tabs_plan.md
```

If the file already exists, append a new section:

```markdown
## Repair Pass - GroupBox Selection and Movement
```

The plan must include:

- Current failed behavior
- Root cause diagnosis
- Files inspected
- Step-by-step TODO checklist with markdown checkboxes
- Build validation checklist
- Manual test checklist
- Final result summary

Update the plan as work progresses.

Mark completed items complete.

Do not finish without updating the plan.

The plan must include that the main VisiForm app was built successfully.

---

# Current Bug

After dropping/adding a GroupBox on the canvas, the GroupBox itself cannot be reliably selected and moved on the canvas.

# Expected Behavior

1. A newly added GroupBox should be selected immediately after creation.
2. Clicking the GroupBox border/title/background should select the GroupBox.
3. Dragging the selected GroupBox should move the GroupBox on the root canvas.
4. Moving the GroupBox should also move its child widgets visually because children are parent-relative.
5. GroupBox children should remain selectable inside the GroupBox.
6. Clicking a child inside the GroupBox should select the child.
7. Clicking the GroupBox empty area should select the GroupBox.
8. Selection handles should draw correctly around the GroupBox.
9. ProjectTree should update selection correctly.
10. PropertyInspector should show GroupBox properties when the GroupBox is selected.
11. Undo/Redo should work for moving the GroupBox if move undo is currently supported.

---

# Do Not Change

- Main app target name `VisiForm`
- Main executable name `VisiForm.exe`
- App version `1.0.0`
- vcpkg triplet
- Static runtime settings
- Generated base class rule: `MainWindow`
- User subclass export rule
- `USER CODE` preservation markers
- Model-layer independence from Visage
- Serialization-layer independence from Visage
- Generator-layer independence from Visage
- Local Visage source support
- Generated project Debug/Release build workflow

---

# Files to Inspect First

```text
src/ui/DesignerCanvas.h
src/ui/DesignerCanvas.cpp
src/ui/MainWindow.h
src/ui/MainWindow.cpp
src/ui/ProjectTree.h
src/ui/ProjectTree.cpp
src/ui/PropertyInspector.h
src/ui/PropertyInspector.cpp
src/model/ProjectDocument.h
src/model/ProjectDocument.cpp
src/model/WidgetNode.h
src/model/WidgetNode.cpp
src/model/WidgetRegistry.h
src/model/WidgetRegistry.cpp
src/commands/UndoRedoStack.h
src/commands/UndoRedoStack.cpp
docs/component_hierarchy.md
docs/widget_catalog.md
docs/agent_plans/phase70_component_hierarchy_containers_and_tabs_plan.md
```

---

# PART A — Diagnose GroupBox Selection Failure

## 1. Inspect GroupBox creation path

Document:

- Where GroupBox is created from the palette/menu.
- Whether newly created GroupBox is selected after creation.
- Whether GroupBox is added to root or accidentally added as a child of another container.
- Whether GroupBox bounds are valid.
- Whether GroupBox z-order places it behind another widget.
- Whether ProjectTree receives the new GroupBox.

## 2. Inspect GroupBox hit testing

Document:

- Whether hit testing checks GroupBox itself.
- Whether recursive child hit testing skips parent containers.
- Whether GroupBox background is treated as transparent/non-hit-testable.
- Whether only children are considered during recursive hit testing.
- Whether GroupBox title/border area is excluded by mistake.
- Whether root-level hit testing still includes containers.

## 3. Inspect movement handling

Document:

- Whether GroupBox movement is blocked because it is a container.
- Whether move logic ignores widgets that can contain children.
- Whether parent-relative coordinate conversion is wrong for root-level GroupBox.
- Whether drag start is captured correctly when clicking GroupBox border/background.
- Whether drag delta is applied to GroupBox bounds.

---

# PART B — Fix Selection After Creation

## 4. Select GroupBox after adding

When a GroupBox is added:

- Add it as a root child unless a GroupBox is selected and explicit nested GroupBox behavior is already supported.
- Select the newly added GroupBox.
- Refresh DesignerCanvas.
- Refresh ProjectTree.
- Refresh PropertyInspector.
- Status:

```text
Added GroupBox to MainWindow
```

## 5. Do not auto-add GroupBox into itself or invalid parent

Prevent:

- `GroupBox parentId = own id`
- GroupBox being child of non-container.
- Accidental parent assignment due to stale selection.

---

# PART C — Fix GroupBox Hit Testing

## 6. Container hit test rule

Recursive hit testing should use this order:

1. Test children first, in reverse z-order, if the click is inside the container.
2. If no child is hit, test the container itself.
3. If container body/title/border contains the point, return the container.

This allows:

- Clicking child selects child.
- Clicking empty GroupBox body selects GroupBox.
- Clicking GroupBox border/title selects GroupBox.

## 7. GroupBox hit area

GroupBox hit area should include:

- Title area
- Border
- Background/content area

The GroupBox should not be unselectable because it has children.

## 8. Root hit testing

Root canvas hit testing must still select:

- Root-level GroupBox
- Root-level Button
- Root-level Image
- Other root widgets

---

# PART D — Fix GroupBox Movement

## 9. Moving GroupBox as root widget

When selected GroupBox is root-level:

- Dragging it updates GroupBox `x/y` in root coordinates.
- Children remain parent-relative and therefore move visually with it.
- PropertyInspector shows GroupBox `x/y` changing.
- ProjectTree hierarchy does not change.

## 10. Moving GroupBox with children

Moving GroupBox should not modify child local coordinates.

Example:

```text
GroupBox at x=100 y=100
Child Button at x=20 y=40 local
Move GroupBox to x=200 y=150
Child Button remains x=20 y=40 local
Child absolute draw position becomes x=220 y=190
```

## 11. Prevent accidental reparent while moving GroupBox

Dragging GroupBox itself should not reparent its children.

Dragging GroupBox should not turn it into a child of itself or a child widget.

## 12. Undo/Redo movement

If move undo is currently supported:

- Moving GroupBox should push one move command on mouse release.
- Undo restores old GroupBox `x/y`.
- Redo restores new GroupBox `x/y`.
- Children visually follow.

---

# PART E — Selection Drawing

## 13. Draw GroupBox selection handles

When GroupBox is selected:

- Draw selection border around full GroupBox bounds.
- Draw resize handles around GroupBox.
- Handles should be in root absolute coordinates.
- Handles should draw above GroupBox and children.

## 14. Multi-select behavior

If GroupBox is part of multi-selection:

- Draw multi-select outline using existing multi-select color.
- Do not hide selection behind children.

---

# PART F — ProjectTree and PropertyInspector

## 15. ProjectTree selection sync

When GroupBox is selected from canvas:

- ProjectTree highlights GroupBox.
- PropertyInspector shows GroupBox properties.

When GroupBox is selected from ProjectTree:

- Canvas selection border appears around GroupBox.
- PropertyInspector shows GroupBox properties.

## 16. PropertyInspector

When GroupBox selected, show:

- id
- type
- name
- x
- y
- width
- height
- title/text
- dock/layout properties if present
- Children section/list if already implemented

---

# PART G — Validation

## 17. Add safety validation if needed

ProjectValidator should catch:

- GroupBox with `parentId` equal to own id.
- GroupBox under non-container parent.
- Cycles.
- Missing parent references.

Do not add major new validation unless needed.

---

# PART H — Documentation

## 18. Update `docs/component_hierarchy.md`

Document:

- GroupBox can be selected by clicking border/title/body.
- GroupBox movement changes parent/root coordinates.
- Children remain local to GroupBox and move visually with parent.

## 19. Update `docs/widget_catalog.md`

Update GroupBox docs with:

- Selection behavior.
- Movement behavior.
- Child coordinate behavior.

---

# PART I — Do Not Implement Yet

Do not implement:

- General Panel container selection repair unless needed.
- TabControl parenting.
- Advanced layout manager.
- Anchors.
- Data binding.
- Command palette.
- Lasso selection.

---

# PART J — Build Validation

After changes:

- Build main VisiForm with `build-static-debug`.
- Fix compile errors.
- Do not run `VisiForm.exe`.
- Do not launch generated apps.

---

# Acceptance Criteria

- [ ] Main VisiForm builds successfully.
- [ ] Adding a GroupBox selects it immediately.
- [ ] Clicking GroupBox border selects GroupBox.
- [ ] Clicking GroupBox title selects GroupBox.
- [ ] Clicking empty GroupBox body selects GroupBox.
- [ ] Dragging GroupBox moves it on the root canvas.
- [ ] Child widgets visually move with GroupBox.
- [ ] Child local coordinates do not change when GroupBox moves.
- [ ] Clicking child inside GroupBox still selects child.
- [ ] ProjectTree highlights GroupBox when selected.
- [ ] PropertyInspector shows GroupBox properties when selected.
- [ ] Undo/Redo works for GroupBox move if move undo is supported.
- [ ] Save/load still preserves GroupBox hierarchy.
- [ ] Exported project still builds Debug and Release.
- [ ] USER CODE preservation still works.
- [ ] Phase plan is updated with final result summary.

---

# Completion Summary Required

When complete, summarize:

1. Phase plan updated.
2. Root cause of GroupBox selection/movement failure.
3. Files changed.
4. GroupBox hit-test behavior.
5. GroupBox movement behavior.
6. Selection drawing behavior.
7. Manual tests I should perform.

---

# Manual Test Checklist

1. Build > Build All.
2. Run VisiForm manually.
3. Add a GroupBox.
4. Confirm the new GroupBox is selected immediately.
5. Drag the GroupBox.
6. Confirm it moves on the canvas.
7. Click the root canvas to clear/select root.
8. Click the GroupBox border.
9. Confirm GroupBox is selected.
10. Click the GroupBox title.
11. Confirm GroupBox is selected.
12. Click empty space inside GroupBox.
13. Confirm GroupBox is selected.
14. Select GroupBox.
15. Add a Button.
16. Confirm Button appears inside GroupBox.
17. Click the Button.
18. Confirm Button is selected, not GroupBox.
19. Move the Button inside GroupBox.
20. Confirm Button x/y are local to GroupBox.
21. Select GroupBox again.
22. Move GroupBox.
23. Confirm Button visually moves with GroupBox.
24. Confirm Button local x/y do not change just because GroupBox moved.
25. Select GroupBox from ProjectTree.
26. Confirm canvas selection border appears around GroupBox.
27. Select child Button from ProjectTree.
28. Confirm canvas selection border appears around Button.
29. Press Ctrl+Z after moving GroupBox.
30. Confirm GroupBox move is undone if supported.
31. Save As and reload.
32. Confirm hierarchy and positions persist.
33. Run Chk / Validate.
34. Confirm validation passes.
35. Export to a clean folder.
36. Build exported Debug and Release.

---

# Suggested Commit Message

```text
Fix GroupBox selection and movement
```
