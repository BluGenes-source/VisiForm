# Layout tools plan

## Goal

Add basic single-selection layout tools for the currently selected widget, including alignment, size matching, and z-order changes.

## Current editor state

- Single-widget selection is working.
- Designer canvas rendering is working.
- Property inspector editing is working.
- Save/load/export are working.
- User-code preservation is working.
- Label and CheckBox sizing is fixed.

## Files to inspect

- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/PropertyValue.h`
- `src/commands/Command.h`
- `src/commands/Command.cpp`
- `src/commands/UndoRedoStack.h`
- `src/commands/UndoRedoStack.cpp`
- `docs/agent_plans/layout_tools_plan.md`

## Planned layout tools

1. Add toolbar buttons for `Align L`, `Align T`, `Same W`, `Same H`, `Front`, and `Back`.
2. Add model helpers for parent lookup, previous sibling lookup, and z-order movement.
3. Add UI actions for alignment and size changes using the selected widget.
4. Use the shared widget metrics source of truth for minimum width and height enforcement.
5. Keep root form excluded from layout actions.
6. Refresh the UI after each layout action.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Select a widget and use `Align L`.
- Select a widget and use `Align T`.
- Use `Same W` and `Same H` and confirm size changes are applied.
- Use `Front` and `Back` and confirm drawing order changes.
- Confirm root form cannot be changed by layout tools.
- Confirm Property Inspector updates after layout operations.
- Confirm save/load/export keep the updated bounds and order.

## Final result summary

Completed.

- Added toolbar buttons for `AlignL`, `AlignT`, `SameW`, `SameH`, `Front`, and `Back` using compact labels.
- Added `ProjectDocument` helpers for previous-sibling lookup and z-order changes.
- Implemented single-selection layout actions in `MainWindow`.
- `SameW` uses the previous sibling width when available, otherwise falls back to root form width minus `40`.
- `SameH` uses the previous sibling height when available, otherwise falls back to widget default height from shared metrics.
- `Front` and `Back` now swap the selected widget within its parent child vector, changing draw order.
- Current layout actions use direct document edits and mark the document dirty; dedicated undo commands remain a future TODO.
- Verified the main project builds successfully with `build-static-debug`.
