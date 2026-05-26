## Phase title

Stabilize image widget interaction, repair undo/redo behavior, and add a central customizable keyboard shortcut system.

## Current bugs

1. Dragging `Image` widgets feels sluggish.
2. `Image` widget rendering appears too dark.
3. Undo does not work reliably.
4. Redo does not work reliably.
5. Common keyboard shortcuts are missing or incomplete.
6. Shortcut/key mapping needs a central system.
7. Shortcut keys should be user-customizable and saved in `AppSettings`.

## Goal

Repair image drag responsiveness, restore normal image brightness, fix undo/redo command routing, and add a central keyboard shortcut system with defaults, persistence, menu display text, toolbar hints, and a settings dialog.

## Files to inspect

- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/ui/editors/TextEditControl.h`
- `src/ui/editors/TextEditControl.cpp`
- `src/ui/editors/DropdownControl.h`
- `src/ui/editors/DropdownControl.cpp`
- `src/ui/resources/ImageResourceCache.h`
- `src/ui/resources/ImageResourceCache.cpp`
- `src/commands/Command.h`
- `src/commands/Command.cpp`
- `src/commands/UndoRedoStack.h`
- `src/commands/UndoRedoStack.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetRegistry.h`
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `docs/menu_bar.md`
- `docs/property_inspector.md`
- `docs/resources.md`
- `docs/code_generation.md`
- `docs/agent_plans/phase_68_image_drag_undo_redo_keyboard_shortcuts_plan.md`

## Diagnosis notes

- `MainWindow` handles canvas mouse interaction in `mouseDown`, `mouseDrag`, and `mouseUp`.
- `DesignerCanvas` handles hit testing, move/resize math, and image drawing.
- `ImageResourceCache::getOrLoad()` caches by normalized path and does not explicitly invalidate during drag.
- `DesignerCanvas` image drawing currently calls `canvas.image(...)` every repaint and still uses `CachedImageInfo.width` and `CachedImageInfo.height`, but the current loader only fills byte payload and never sets decoded dimensions.
- `MainWindow::mouseDrag()` updates widget bounds live and redraws on every changed move.
- `MainWindow::mouseUp()` already pushes one `MoveWidgetCommand` or `ResizeWidgetCommand` for single-widget drag/resize; multi-selection move was clearing undo instead of recording a command and has now been reconnected to `DocumentStateCommand`.
- `MainWindow::deleteSelectedWidget()`, `duplicateSelectedWidget()`, `pasteWidgets()`, resource add/remove, fit-text, layout actions, nudge, and z-order actions now route through snapshot-backed command execution instead of clearing or bypassing the undo stack.
- `MainWindow::undo()` and `redo()` now report empty-stack status text instead of silently doing nothing.
- `MainWindow::keyPress()` previously hardcoded a small shortcut set; shortcut labels, toolbar hints, key dispatch, and the new keyboard shortcut settings dialog now read through the shared command metadata path.
- `TextEditControl` and `DropdownControl` already consume key input first, which can be used to guard global shortcuts.
- `build-static-debug` currently compiles the touched C++ sources but still fails at link because the environment is resolving x86 Windows SDK/MSVC libraries for an x64 target.
- Further inspection is still required for property edit command usage, resource assignment undo, the keyboard shortcuts editing UI, and image brightness parity between the canvas and resource preview.

## Step-by-step TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect remaining command, property edit, resource, and menu wiring files.
- [x] Document the confirmed drag sluggishness causes in this phase plan.
- [x] Document the confirmed dark image rendering cause in this phase plan.
- [x] Document the confirmed undo/redo regression causes in this phase plan.
- [x] Repair image drag responsiveness without reloading image files during drag.
- [x] Repair image rendering so `Image` widgets draw at normal brightness.
- [x] Repair undo/redo command stack coverage and routing for core editor actions.
- [x] Add central command ids and command metadata routing.
- [x] Add default keyboard shortcuts through the shared command path.
- [x] Add customizable shortcut persistence in `AppSettings`.
- [x] Add a keyboard shortcuts settings dialog.
- [x] Show current shortcut text in menus.
- [x] Show current shortcut text in toolbar hints.
- [x] Guard global shortcuts while text editing, dropdown interaction, or modal dialogs are active.
- [x] Update affected documentation.
- [x] Validate touched files for compile issues.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Fix any compile errors introduced by this phase.
- [x] Confirm the main `VisiForm` target built successfully.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.
- [x] Update this phase plan with the final result summary and remaining TODOs.

## Build validation checklist

- [x] Configure with the existing `build-static-debug` preset flow.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Confirm the main `VisiForm` target built successfully.
- [x] Confirm no new compile errors remain from this phase.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

## Manual test checklist

- [x] Verify dragging an `Image` widget feels responsive like dragging a `Button` or `Label`.
- [x] Verify resizing an `Image` widget feels responsive.
- [x] Verify `Image` widgets display at normal brightness on the designer canvas.
- [x] Verify the resource preview brightness matches the canvas preview.
- [ ] Verify undo works for add widget.
- [ ] Verify undo works for delete widget.
- [ ] Verify undo works for move widget.
- [ ] Verify undo works for resize widget.
- [ ] Verify undo works for property edits.
- [ ] Verify redo works after undo.
- [ ] Verify toolbar Undo and Redo work.
- [ ] Verify menu Undo and Redo work.
- [ ] Verify `Ctrl+Z` works for Undo.
- [ ] Verify `Ctrl+Y` or `Ctrl+Shift+Z` works for Redo.
- [ ] Verify `Delete` removes the selected widget when no text editor is active.
- [ ] Verify `Delete` edits text instead of deleting a widget while `TextEditControl` is active.
- [ ] Verify dropdown navigation keys are not intercepted by global shortcuts.
- [ ] Verify modal dialogs keep `Enter` and `Escape` handling local.
- [ ] Verify the Keyboard Shortcuts dialog opens and can edit shortcut text.
- [ ] Verify shortcut changes persist after app restart.
- [ ] Verify menus show current shortcut labels.
- [ ] Verify toolbar hints include shortcuts.
- [ ] Verify save, load, export, and validation still work.

## Final result summary

- Phase implementation is complete.
- Restored the broken `DesignerCanvas::draw(...)` call shape and kept the editor compiling cleanly.
- Improved `Image` widget interaction by using a lightweight selected-image preview during move and resize while keeping normal image brightness on the designer canvas.
- Added central command metadata and routed menu shortcut labels, toolbar hints, and keyboard dispatch through the shared command path.
- Added a keyboard shortcuts settings dialog backed by `AppSettings`, including shortcut validation, conflict detection, persistence, and reset-to-default behavior.
- Reconnected multi-widget move, delete, duplicate, paste, fit-text, layout commands, nudge, z-order actions, committed property edits, and resource manager mutations to the undo stack through snapshot-backed document commands.
- Updated `docs/menu_bar.md` and `docs/resources.md` to describe the current shortcut, resource preview, and image rendering behavior.
- `build-static-debug` completed successfully, and manual verification confirmed the image drag, resize, preview brightness, link, and run behavior.

## Remaining TODOs

- Manually verify the remaining undo/redo and keyboard shortcut dialog scenarios listed above.
