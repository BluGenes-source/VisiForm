## Phase title

Finish keyboard shortcut key dispatch so assigned shortcuts execute commands.

## Current bug

Keyboard shortcuts are displayed or assigned, but pressing the key combinations does not execute the mapped command.

Examples:

- `Ctrl+Z` does not trigger Undo.
- `Ctrl+Y` and `Ctrl+Shift+Z` do not trigger Redo.
- `Delete` and other assigned shortcuts may not route through the command path.

## Goal

Finish the keyboard shortcut system so normalized key events are matched against the active keymap and execute the same command path used by menus and the toolbar, while still respecting text edit fields, dropdowns, modal dialogs, and normal editor focus behavior.

## Files to inspect

- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/editors/TextEditControl.h`
- `src/ui/editors/TextEditControl.cpp`
- `src/ui/editors/DropdownControl.h`
- `src/ui/editors/DropdownControl.cpp`
- `src/commands/CommandIds.h`
- `src/commands/CommandRegistry.h`
- `src/commands/CommandRegistry.cpp`
- `src/commands/Command.h`
- `src/commands/UndoRedoStack.h`
- `src/commands/UndoRedoStack.cpp`
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`
- `docs/keyboard_shortcuts.md`
- `docs/menu_bar.md`
- `docs/property_inspector.md`
- `README.md`
- `docs/agent_plans/phase_69_finish_keyboard_shortcuts_key_dispatch_plan.md`
- local Visage headers under `out/build/x64-debug/_deps/visage-src/`

## Diagnosis notes

- `MainWindow` already implements `bool keyPress(const visage::KeyEvent& e)` and `void textInput(const std::string& text)`.
- Local Visage routes keyboard input through `Frame::keyPress(const KeyEvent&)`, with modifier state exposed by `KeyEvent::isCtrlDown()`, `isShiftDown()`, `isAltDown()`, `isCmdDown()`, and `isMetaDown()`.
- `MainWindow::mouseDown()` calls `requestKeyboardFocus()`, so the window should be able to receive key events when the editor surface is active.
- `TextEditControl` and `DropdownControl` already consume their own key events first.
- `MainWindow::keyPress()` already guards dropdowns, text editing, modal dialogs, open menus, and property inspector editing before attempting global shortcut dispatch.
- Shortcut strings are already stored in `AppSettings.keyboardShortcuts`, shown in menus and toolbar hints, and validated through `CommandRegistry`.
- The current dispatch path in `MainWindow::keyPress()` parses the displayed shortcut text and matches it against the incoming `visage::KeyEvent` before calling `executeCommand(...)`.
- The confirmed root cause is in `CommandRegistry::parseShortcutString(...)`: letter tokens are normalized to uppercase text and then cast directly to `visage::KeyCode`, but Visage letter `KeyCode` values are lowercase ASCII (`'a'` through `'z'`). This makes letter shortcuts like `Ctrl+Z`, `Ctrl+Y`, `Ctrl+S`, and `Ctrl+D` fail to match even though the command metadata and UI labels exist.
- Existing parsing/formatting coverage also appears incomplete for some requested keys such as digits, `Backspace`, `Tab`, and `F10` through `F12`, so shortcut normalization should be completed while fixing dispatch.
- `Delete` can already be consumed by `TextEditControl`, so widget deletion must remain blocked while editing text.

## Step-by-step TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect the current `MainWindow`, focused controls, command registry, settings, and local Visage key APIs.
- [x] Document the confirmed root cause of keyboard shortcuts not firing.
- [x] Repair shortcut parsing and formatting so Visage key codes normalize correctly.
- [x] Add a normalized shortcut-from-event helper for active keymap lookup.
- [x] Route keyboard shortcut dispatch through the existing command execution path using the active `AppSettings` keymap.
- [x] Preserve text edit, dropdown, modal dialog, and menu key ownership.
- [x] Verify alternate Redo bindings such as `Ctrl+Shift+Z` still route correctly.
- [x] Update shortcut documentation and related menu/property inspector notes.
- [x] Validate touched files for compile issues.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Fix any compile errors introduced by this phase.
- [x] Confirm the main `VisiForm` target built successfully.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.
- [x] Update this phase plan with the final result summary and remaining TODOs.

## Build validation checklist

- [x] Configure and build with the existing `build-static-debug` preset flow.
- [x] Confirm the main `VisiForm` target built successfully.
- [x] Confirm no new compile errors remain from this phase.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

## Manual test checklist

- [ ] Verify `Ctrl+Z` triggers Undo.
- [ ] Verify `Ctrl+Y` triggers Redo.
- [ ] Verify `Ctrl+Shift+Z` triggers Redo.
- [ ] Verify `Delete` deletes the selected widget when no text editor is active.
- [ ] Verify `Delete` edits text instead of deleting a widget while `TextEditControl` is active.
- [ ] Verify `Ctrl+S` triggers Save.
- [ ] Verify `Ctrl+D` triggers Duplicate.
- [ ] Verify `Ctrl+G` toggles Grid.
- [ ] Verify `Ctrl+Shift+G` toggles Guides.
- [ ] Verify `Ctrl+Alt+K` opens Keyboard Shortcuts.
- [ ] Verify dropdown navigation keys are not intercepted by global shortcuts.
- [ ] Verify modal dialogs keep `Enter` and `Escape` handling local.
- [ ] Verify custom shortcut changes take effect immediately after Apply.
- [ ] Verify custom shortcut overrides persist after app restart.
- [ ] Verify duplicate shortcut assignment is detected.
- [ ] Verify menu items display current shortcut labels.
- [ ] Verify toolbar hints display current shortcuts.
- [ ] Verify Help/About shortcut information is accurate or points to the Keyboard Shortcuts dialog.
- [ ] Verify existing menu and toolbar Undo/Redo still work.
- [ ] Verify existing save/load/export/validation still work.

## Final result summary

- Phase implementation is complete.
- Confirmed the real Visage editor callback path uses `keyPress(const visage::KeyEvent&)` plus `textInput(const std::string&)`, and that `MainWindow` already received keyboard events when the editor held focus.
- Fixed the main root cause in `CommandRegistry`: letter shortcuts were being parsed as uppercase ASCII while Visage letter `KeyCode` values are lowercase, so bindings like `Ctrl+Z`, `Ctrl+Y`, `Ctrl+S`, `Ctrl+D`, and `Ctrl+G` never matched runtime key events.
- Added normalized event-to-shortcut conversion and completed shortcut parsing/formatting coverage for letters, digits, `Delete`, `Backspace`, `Escape`, `Enter`, `Tab`, `Space`, arrows, and `F1` through `F12`.
- Updated `MainWindow::keyPress()` to match normalized events against the active keymap from `AppSettings`, route handled shortcuts through the existing `executeCommand(...)` path, and consume unavailable mapped shortcuts with status feedback.
- Prevented plain arrow-key nudging from stealing modified shortcut combinations such as `Ctrl+Alt+Left` and `Ctrl+Alt+Right`.
- Kept text edit fields, dropdowns, modal dialogs, and open menus ahead of global shortcut dispatch so local editing keys still win.
- Updated the `ShowKeyboardShortcuts` command to open the actual keyboard shortcuts settings dialog, which makes `Ctrl+Alt+K` route to the expected UI.
- Added `docs/keyboard_shortcuts.md`, added `docs/property_inspector.md`, updated `docs/menu_bar.md`, and updated `README.md` to reflect the working shortcut system.
- `build-static-debug` completed successfully after invoking the build from the Visual Studio x64 developer environment; no compile errors from the touched files remained, `VisiForm.exe` was not run, and no generated apps were launched.

## Remaining TODOs

- Manually verify the scenarios in the checklist above, especially `Ctrl+Z`, `Ctrl+Y`, `Ctrl+Shift+Z`, `Delete` during inline text editing, `Ctrl+Alt+K`, custom shortcut persistence, and menu/toolbar shortcut labels.
