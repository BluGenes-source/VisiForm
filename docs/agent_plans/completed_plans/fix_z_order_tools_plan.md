# Fix z-order tools plan

## Goal

Fix `Front` and `Back` so overlapping widgets visibly change z-order on `DesignerCanvas` while preserving the same selected widget id across reorder operations.

## Current bug

- `Front` and `Back` appear to change something in the tree, but overlapping widgets do not visibly reorder on the canvas.
- The same selected widget must remain selected after reordering.

## Files inspected

- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `docs/layout_tools.md`
- `docs/agent_plans/fix_z_order_tools_plan.md`

## Cause found

- `DesignerCanvas` already drew children from first to last, and hit testing already searched children from last to first.
- The visible problem was the `Front` and `Back` actions only moving the selected widget one sibling step in the parent child vector.
- When overlapping widgets were not adjacent siblings, one-step movement could change tree order without visibly changing which widget was on top.
- `MainWindow` also used a selected widget pointer across reorder operations, which was unsafe once the parent child vector was modified.

## Planned changes

1. Inspect and document the actual draw-order and hit-test-order conventions.
2. Fix child draw order and hit testing so they match a single z-order convention.
3. Verify and correct `bringWidgetForward` and `sendWidgetBackward` behavior if needed.
4. Preserve `selectedWidgetId` across reorder operations.
5. Update docs with the z-order rules.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Overlap two widgets and confirm `Front` moves the selected widget visually on top.
- Overlap two widgets and confirm `Back` moves the selected widget visually behind.
- Confirm the same widget id remains selected after reordering.
- Confirm the Property Inspector still shows the same widget after reordering.
- Confirm the topmost overlapping widget is selected when clicking the overlap area.
- Confirm save/load preserves z-order.
- Confirm export preserves z-order.

## Final result summary

Completed.

- Kept the z-order convention explicit: `children[0]` is backmost and `children.back()` is frontmost.
- Kept `DesignerCanvas` drawing children from first to last and hit testing from last to first.
- Changed `Front` to move the selected widget to the end of the parent child vector and `Back` to move it to the beginning.
- Preserved the same `selectedWidgetId` explicitly after reorder so the same widget stays selected in the canvas, tree, and inspector.
- Verified the main project builds successfully with `build-static-debug`.
