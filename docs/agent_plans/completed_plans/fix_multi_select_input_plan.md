# Fix multi-select input plan

## Goal

Make multi-selection usable even without reliable keyboard modifier state by adding a toolbar-driven fallback and centralizing additive selection handling.

## Current bug

- Normal single selection still works.
- Ctrl+click does not currently add or remove widgets from the selection.
- Multi-select needs a reliable non-keyboard fallback.

## Files inspected

- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `docs/layout_tools.md`
- `docs/agent_plans/multi_select_foundation_plan.md`
- `docs/agent_plans/fix_multi_select_input_plan.md`

## Modifier-key support found or not found

Direct Visage mouse modifier helpers were not found in the current project input path.

Working result:

- the editor now uses a toolbar `Multi` toggle as the reliable fallback
- click handling also checks Windows key state for `Ctrl` and `Shift` during mouse clicks

## Planned fallback behavior

1. Add a `Multi` toolbar toggle.
2. When `Multi` is on, clicking widgets toggles them in the selection.
3. Keep one primary selection for resize handles and Property Inspector.
4. Use Ctrl+click too if modifier support is actually available.
5. Document a TODO if reliable modifier state is not exposed by the current input path.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Toggle `Multi` on and off from the toolbar.
- With `Multi` off, clicking selects only one widget.
- With `Multi` on, clicking adds and removes widgets from selection.
- Confirm multiple widgets show selection outlines.
- Confirm primary widget still shows resize handles.
- Confirm layout tools still work on multiple selected widgets.
- Confirm delete removes multiple selected widgets.
- Confirm save/load/export still work.

## Final result summary

Completed.

- Added a toolbar `Multi` toggle that enables additive click selection without depending only on keyboard modifiers.
- Centralized widget click routing in `MainWindow::handleWidgetClicked(...)`.
- Extended `ProjectDocument` to track ordered multi-selection while preserving `selectedWidgetId` as the primary selection.
- Multiple selected widgets now draw outlines; the primary selection still gets resize handles.
- Layout tools now operate across the selection where intended, while `Front`/`Back` and `Duplicate` remain primary-only.
- Delete now removes multiple selected non-root widgets.
- Verified the main project builds successfully with `build-static-debug`.
