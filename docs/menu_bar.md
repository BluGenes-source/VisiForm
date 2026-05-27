# Menu bar

`VisiForm` now exposes a simple editor menu bar above the toolbar.

## Top-level menus

Current top-level menus:

- `File`
- `Edit`
- `View`
- `Insert`
- `Layout`
- `Project`
- `Export`
- `Help`

## Command routing

The menu bar and the toolbar both route into the same `MainWindow` command handlers.
Keyboard shortcuts are dispatched through that same command path after `MainWindow::keyPress(const visage::KeyEvent&)` normalizes the incoming `Visage` key event against the active keymap.
This keeps existing commands working without duplicating the underlying editor logic.

Shared command examples:

- `New`
- `Open`
- `Save`
- `Save As`
- `Export`
- `Validate / Check`
- `Undo`
- `Redo`
- `Copy`
- `Paste`
- `Delete`
- `Grid`
- `Snap`
- `Guides`
- `Multi Select`
- alignment and distribution commands

## Menu contents

### File

- `New` - opens the `New Project Wizard`
- `Open`
- `Open Sample`
- `Save`
- `Save As`
- recent files when available

### Edit

- `Undo`
- `Redo`
- `Copy`
- `Paste`
- `Duplicate`
- `Delete`

### View

- `Grid`
- `Snap`
- `Guides`
- `Multi Select`
- `Validation Report`

### Insert

The `Insert` menu uses the same add-widget flow as the `Widget Palette`.
Current insertable widget types include:

- `Frame`
- `Label`
- `Button`
- `Text Box`
- `Check Box`
- `Radio Button`
- `Slider`
- `Scroll Bar`
- `Status Bar`
- `Progress Bar`
- `Color Picker`
- `Modal Dialog`
- `Image`
- `Spacer`

### Layout

- alignment commands
- centering commands
- size matching commands
- distribution commands
- z-order commands
- `Fit Text`

### Project

- `Validate / Check`
- `Project Settings` - opens the centered modal dialog for project naming, look and feel, and export dependency settings
- `Resources` - opens the `Resource Manager` modal dialog
- `Keyboard Shortcuts` - opens the keyboard shortcut settings dialog and saves overrides in `AppSettings`
- `Export Dependencies`

### Export

- `Export`
- placeholder `Open Export Folder (TODO)` entry for future expansion

### Help

- `About VisiForm`
- `Keyboard Shortcuts` - opens the same keyboard shortcut settings dialog used by the `Project` menu
- `Generated Code Guide`

## Toolbar cleanup

The toolbar remains available for high-frequency commands only:

- `New`
- `Open`
- `Save`
- `Save As`
- `Export`
- `Chk`
- `Undo`
- `Redo`
- `Copy`
- `Paste`
- `Delete`
- `Multi`
- `Grid`
- `Snap`
- `Guides`

Less frequent layout commands now live primarily in the `Layout` menu.

## Wizard and settings flow

- `File > New` now opens a single-page `New Project Wizard`
- the wizard lets the user edit project identity, form size, look and feel, and a built-in template before creating a clean document
- `Project > Settings` opens a modal dialog for updating project naming fields stored in `.vfb.json`
- local `Visage` dependency values remain machine-specific `AppSettings` values and are edited from that same settings dialog

## Current limitations

This phase keeps the menu system editor-local and intentionally lightweight:

- no native OS menu integration
- no menu icons
- no accelerator underline system
- no tear-off menus
- placeholder entries remain for some future file-system convenience actions

## Keyboard shortcut settings

The editor now includes a keyboard shortcut settings dialog reachable from:

- `Project > Keyboard Shortcuts`
- `Help > Keyboard Shortcuts`

Current behavior:

- shortcut labels shown in menus come from the shared command registry plus any `AppSettings` override
- shortcut labels are normalized before display, so configured bindings show consistent text such as `Ctrl+Shift+Z` or `Ctrl+Alt+Left`
- toolbar hints append the current effective shortcut text
- the dialog lets the user select a command, inspect its default shortcut, edit the current shortcut, or clear it to disable that binding
- the dialog validates shortcut text and prevents duplicate bindings before saving
- saved overrides persist through `AppSettings.keyboardShortcuts`
- `Ctrl+Alt+K` opens the keyboard shortcuts settings dialog through the same command path used by the `Project` menu
