# Phase 33 box select, group move, copy/paste plan

## Goal

Add marquee box selection, group move for multi-selected widgets, and internal copy/paste support while preserving the current editor workflow and export behavior.

## Current editor state

- Multi Select mode works from the toolbar.
- Multiple widgets can be selected.
- Primary selection uses the main outline color and resize handles.
- Secondary selection uses a red outline.
- Layout tools work on multi-selection.
- Save/load/export and user-code preservation work.

## Files to inspect

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
- `src/utils/IdGenerator.h`
- `src/utils/IdGenerator.cpp`
- `src/commands/Command.h`
- `src/commands/Command.cpp`
- `src/commands/UndoRedoStack.h`
- `src/serialization/JsonProjectWriter.cpp`
- `src/serialization/JsonProjectReader.cpp`
- `docs/layout_tools.md`
- `docs/selection_model.md`
- `docs/agent_plans/phase_33_box_select_group_move_copy_paste_plan.md`

## Planned box-select behavior

1. Start a marquee selection when clicking empty canvas space.
2. Draw a visible selection rectangle while dragging.
3. Select widgets whose bounds intersect the marquee rectangle on mouse release.
4. Ignore the root form in marquee selection.
5. Use replace-selection behavior for marquee selection in this phase.

## Planned group-move behavior

1. Dragging the primary selected widget body moves the whole selected non-root set.
2. All selected widgets move by the same delta.
3. Snap-to-grid remains respected when enabled.
4. Primary selection remains the same after group move.
5. Group resize is out of scope for this phase.

## Planned copy/paste behavior

1. Add an internal widget clipboard in `MainWindow`.
2. Copy selected non-root widgets as deep copies.
3. Paste duplicates with new unique ids and offset positions.
4. Select pasted widgets and make the last pasted widget primary.
5. Keep clipboard/editor state out of the saved project schema.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Drag a marquee rectangle over multiple widgets and confirm they are selected.
- Confirm dragging the primary selected widget moves the entire selected group.
- Confirm snap-to-grid still affects group move.
- Use Copy and Paste and confirm new widgets get unique ids and an offset.
- Confirm multiple pasted widgets are selected and the last pasted widget is primary.
- Confirm delete, layout tools, save/load, export, and user-code preservation still work.

## Final result summary

Completed.

- Added marquee box selection on the designer canvas using intersection-based selection.
- Added group move for selected non-root widgets when dragging the primary selected widget body.
- Added an internal widget clipboard with toolbar and keyboard copy/paste support.
- Pasted widgets receive new unique ids recursively and an increasing paste offset.
- Selection remains editor-only state and save/load/export behavior stays compatible.
- Verified the main project builds successfully with `build-static-debug`.
