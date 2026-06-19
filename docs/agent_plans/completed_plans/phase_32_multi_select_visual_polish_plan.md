# Phase 32 multi-select visual polish plan

## Goal

Improve multi-selection visual feedback so primary and secondary selections are clearly distinguishable on the designer canvas.

## Current editor state

- Multi Select mode works from the toolbar.
- Multiple widgets can be selected.
- Layout tools work on the selected set.
- Single selection already has a visible outline and resize handles.

## Files to inspect

- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `docs/layout_tools.md`
- `docs/agent_plans/phase_32_multi_select_visual_polish_plan.md`

## Planned visual changes

1. Keep the current primary-selection appearance for single selection and primary multi-selection.
2. Draw secondary selected widgets with a red outline.
3. Keep resize handles only on the primary selected widget.
4. Add simple selection helper methods if they improve clarity.
5. Update documentation for the new primary/secondary selection visuals.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Single-select one widget and confirm appearance is unchanged.
- Turn on Multi Select and select several widgets.
- Confirm the primary widget keeps the normal outline and resize handles.
- Confirm secondary widgets use a red outline and no resize handles.
- Confirm layout tools still work on the selected set.
- Confirm save/load/export still work.

## Final result summary

Completed.

- Kept the existing primary-selection blue outline and resize handles for both single selection and primary multi-selection.
- Changed secondary multi-selection visuals to a red outline with no resize handles.
- Added explicit `ProjectDocument` helpers for primary and secondary selection queries.
- Moved selection-outline drawing to occur after widget content and children so outlines remain clearly visible.
- Verified the main project builds successfully with `build-static-debug`.
