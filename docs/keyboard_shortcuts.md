# Keyboard shortcuts

`VisiForm` uses a central command registry plus `AppSettings.keyboardShortcuts` overrides for editor shortcuts.

## Default shortcuts

### File

- `Ctrl+N` - New
- `Ctrl+O` - Open
- `Ctrl+S` - Save
- `Ctrl+Shift+S` - Save As
- `Ctrl+E` - Export

### Edit

- `Ctrl+Z` - Undo
- `Ctrl+Y` - Redo
- `Ctrl+Shift+Z` - Redo alternate when the default Redo binding remains `Ctrl+Y`
- `Ctrl+C` - Copy
- `Ctrl+V` - Paste
- `Delete` - Delete selected widget
- `Ctrl+D` - Duplicate

### View

- `Ctrl+G` - Toggle Grid
- `Ctrl+Shift+G` - Toggle Guides
- `Ctrl+Alt+S` - Toggle Snap

### Project

- `Ctrl+Shift+V` - Validate
- `Ctrl+Alt+R` - Resources
- `Ctrl+Alt+K` - Keyboard Shortcuts

### Layout

- `Ctrl+Alt+Left` - Align Left
- `Ctrl+Alt+Right` - Align Right
- `Ctrl+Alt+Up` - Align Top
- `Ctrl+Alt+Down` - Align Bottom
- `Ctrl+Alt+F` - Fit Text
- `Ctrl+Alt+B` - Send Backward
- `Ctrl+Alt+Shift+B` - Bring Forward

## How dispatch works

- `MainWindow::keyPress(const visage::KeyEvent&)` receives editor key events.
- The event is converted into a normalized shortcut string through `CommandRegistry`.
- The normalized shortcut is matched against the active keymap.
- The active keymap uses `AppSettings.keyboardShortcuts` overrides first and falls back to command defaults.
- Matching shortcuts call the same `executeCommand(...)` path used by menus and the toolbar.

## Shortcut string format

Shortcut strings are normalized before display and matching.

Examples:

- `ctrl+z` -> `Ctrl+Z`
- `CTRL+SHIFT+Z` -> `Ctrl+Shift+Z`
- `Del` -> `Delete`
- `Esc` -> `Escape`
- `Return` -> `Enter`

Supported normalized key names include:

- `A` through `Z`
- `0` through `9`
- `Delete`
- `Backspace`
- `Escape`
- `Enter`
- `Tab`
- `Space`
- `Left`
- `Right`
- `Up`
- `Down`
- `F1` through `F12`

Modifier order is normalized as:

- `Ctrl+Alt+Shift+Meta+Key`

## Customizing shortcuts

Open the shortcuts editor from:

- `Project > Keyboard Shortcuts`
- `Ctrl+Alt+K`

Current behavior:

- the dialog edits the same active keymap used by runtime dispatch
- `Apply` validates and saves changes immediately
- `Reset` restores the selected command to its default shortcut
- clearing a shortcut disables that binding

## Conflict behavior

Shortcut assignments are validated before saving.

- duplicate bindings are rejected
- invalid shortcut text is rejected
- settings are not saved until validation succeeds

## Settings storage

Shortcut overrides are stored in `AppSettings.keyboardShortcuts`.

They are persisted in the machine-level `settings.json` written by `AppSettings::save()`.
They are not stored in `.vfb.json` project files.

## Focus rules

Global shortcuts are intentionally blocked when a more specific editor surface owns the key event.

- `TextEditControl` consumes printable text, `Backspace`, `Delete`, arrow keys, `Enter`, and `Escape`
- `DropdownControl` consumes `Up`, `Down`, `Enter`, and `Escape` while open
- editor modal dialogs keep `Enter` and `Escape` local
- open menus keep keyboard input local until closed
- plain arrow keys nudge the selection only when no global shortcut modifier is held

This keeps `Delete` editing text while inline text entry is active and keeps layout shortcuts such as `Ctrl+Alt+Left` available when the main editor is active.

## Current limitations

This phase does not add:

- OS-level accelerator tables
- global shortcuts outside app focus
- multi-stroke shortcuts
- per-project keymaps
- keymap import or export files
