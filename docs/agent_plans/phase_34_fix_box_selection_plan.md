# Phase 34 fix box selection plan

## Goal

Fix box selection so dragging on empty form area draws a visible marquee rectangle and selects widgets whose bounds intersect the rectangle.

## Current bug

- Dragging on empty form area does not visibly start marquee selection.
- No marquee rectangle appears.
- Mouse release does not select widgets.

## Files inspected

- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/PropertyInspector.h`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `docs/agent_plans/phase_33_box_select_group_move_copy_paste_plan.md`
- `docs/agent_plans/phase_34_fix_box_selection_plan.md`
- `docs/selection_model.md`
- `docs/layout_tools.md`

## Mouse event flow found

- Mouse input enters through `MainWindow::mouseDown`, `mouseDrag`, and `mouseUp`.
- `MainWindow` decides whether the click is handled by the toolbar, palette, inspector, project tree, widget hit testing, or marquee selection.
- `DesignerCanvas` provides form-local coordinate conversion through `toFormPoint(...)` and widget hit testing through `hitTestWidgetId(...)`.
- Marquee drawing already existed through the optional selection rectangle passed into `DesignerCanvas::draw(...)`.

## Cause found

- `DesignerCanvas::hitTestWidgetId(...)` returns the root form id when clicking empty form background inside the preview.
- `MainWindow::mouseDown(...)` treated any hit widget id as a normal widget-selection path, so root-form background clicks never reached the marquee-start path.
- Because marquee mode never started, drag updates, marquee drawing, and mouse-up selection finalization never ran for empty form drags.

## Planned changes

1. Inspect where marquee start is blocked in the current mouse-down path.
2. Fix empty-form detection so root form background can start marquee selection.
3. Ensure marquee state updates during drag and renders visibly on the designer canvas.
4. Finalize selection in form-local coordinates using intersection.
5. Keep group move, resize, normal click selection, and multi-select mode working.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Drag on empty form area and confirm the marquee rectangle appears.
- Release over widgets and confirm they become selected.
- Drag in reverse directions and confirm selection still works.
- Confirm root form is ignored by box selection.
- Confirm group move still works when dragging selected widgets.
- Confirm resize handles still resize the primary widget.
- Confirm save/load/export and copy/paste still work.

## Final result summary

Completed.

- Fixed marquee start by treating root-form background hits as empty form area in `MainWindow::mouseDown(...)`.
- Kept marquee coordinates in form-local space and used `DesignerCanvas::toFormPoint(...)` for both drag start and drag updates.
- Kept marquee drawing through `DesignerCanvas::draw(...)` and made box-selection completion report explicit status text.
- Box selection now replaces selection, ignores the root form, and selects widgets by intersection.
- Verified the main project builds successfully with `build-static-debug`.
