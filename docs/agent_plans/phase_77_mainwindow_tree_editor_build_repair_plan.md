## Phase title

Repair the `MainWindow` modal editor integration after the `TreeView` editor work so the main `VisiForm` app builds again with the required `build-static-debug` workflow.

## Current state

- The main `VisiForm` build currently fails in `src/ui/MainWindow.cpp`.
- The reported errors indicate broken function boundaries near the modal editor and tree node editor code.
- The failure prevents full build validation for the recent `TreeView` editor changes.

## Goal

Restore valid `MainWindow` function structure, preserve intended tree node editor behavior, validate the file compiles, and complete a successful `build-static-debug` build without launching `VisiForm.exe`.

## Files to inspect

- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `docs/agent_plans/phase_77_mainwindow_tree_editor_build_repair_plan.md`

## Initial inspection findings

- Build logs show `openSelectedTreeNodeEditor` is being parsed as a local function inside `beginEditorModalFieldEdit`.
- Build logs also show stray `if` logic before `handleEditorModalMouseDown`, which suggests misplaced or unbalanced braces in the modal editor code.
- The break appears localized to `MainWindow` modal/tree editor integration.
- After repairing the `MainWindow` structure, the build advanced to additional code-side blockers in `src/ui/editors/TextEditControl.cpp` and a missing `treeNodeEditorTextBounds()` definition.
- The final IDE build completed successfully after restoring the missing text editor helpers and the missing `MainWindow` bounds helper.

## TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect the reported `MainWindow.cpp` error regions.
- [x] Repair the broken modal editor function structure.
- [x] Validate `src/ui/MainWindow.cpp` for compile errors.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Fix any compile errors introduced by this phase.
- [x] Update this phase plan with build validation, final result summary, and remaining TODOs.

## Build validation checklist

- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Confirm the main `VisiForm` app built successfully.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

## Final result summary

- Repaired `MainWindow` modal editor function boundaries so `openSelectedTreeNodeEditor()` and `handleEditorModalMouseDown()` compile in the intended scope.
- Restored missing multiline `TextEditControl` helpers needed by the tree node editor path, including line navigation, hit testing, and line height support.
- Added the missing `MainWindow::treeNodeEditorTextBounds()` definition used by modal editor layout updates and drawing.
- Verified the main `VisiForm` app builds successfully with the required `build-static-debug` workflow.

## Remaining TODOs

- None.
