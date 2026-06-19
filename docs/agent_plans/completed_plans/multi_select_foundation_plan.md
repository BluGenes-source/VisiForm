# Multi-select foundation plan

## Goal

Add a basic multi-select foundation so multiple non-root widgets can be selected and used with the existing layout tools while keeping one widget as the primary selection.

## Current editor state

- Single selection is working across canvas, tree, and property inspector.
- Layout tools exist for single selection.
- Z-order tools now work.
- Save/load/export and user-code preservation are working.

## Files to inspect

- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/commands/Command.h`
- `src/commands/Command.cpp`
- `src/commands/UndoRedoStack.h`
- `src/commands/UndoRedoStack.cpp`
- `docs/layout_tools.md`
- `docs/agent_plans/multi_select_foundation_plan.md`

## Planned selection model changes

1. Extend `ProjectDocument` to track multiple selected widget ids while preserving `selectedWidgetId` as the primary selection.
2. Add helpers for adding, removing, toggling, and querying selection membership.
3. Keep root selection exclusive from child multi-selection.
4. Keep existing `selectedWidget()` compatibility intact.

## Planned layout behavior changes

1. Apply `Align Left` and `Align Top` across all selected widgets.
2. Apply `Same Width` and `Same Height` using the primary selected widget as the reference when multiple widgets are selected.
3. Keep `Front` / `Back` on the primary selection only.
4. Let delete remove multiple selected non-root widgets.
5. Keep duplicate operating on the primary selection only for now.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Normal click selects one widget.
- Ctrl+click adds and removes widgets from selection.
- Multiple selected widgets show outlines.
- Primary selection still shows resize handles.
- Property Inspector indicates multi-selection and still shows the primary widget.
- Layout tools affect all selected widgets where intended.
- Delete removes multiple selected widgets.
- Duplicate still duplicates only the primary widget.
- Save/load/export still work.

## Final result summary

Pending.
