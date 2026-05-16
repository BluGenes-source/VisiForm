# File workflow

`VisiForm` stores projects as `.vfb.json` files.

## Open

Use the toolbar `Open` button or `Ctrl+O` to open a native Windows file dialog filtered to `*.vfb.json`.

If the dialog is cancelled, the editor reports `Open cancelled`.
If the current project is dirty, `VisiForm` prompts to save changes before opening another file.

## Save

Use the toolbar `Save` button or `Ctrl+S`.

Behavior:

- If the current project already has a normal project path, save writes to that file.
- If the current path is empty or points into `templates/examples`, save routes to `Save As`.

This prevents accidentally overwriting the sample template file.
If the current project path is empty or points into `templates/examples`, `Save` routes to `Save As`.

## Save As

Use the toolbar `Save As` button or `Ctrl+Shift+S` to open a native Windows save dialog.

Saved project files use the `.vfb.json` extension.

## Open Sample

Use the toolbar `Sample` button to load:

- `templates/examples/BasicWindow.vfb.json`

If the current project is dirty, `VisiForm` prompts to save changes first.

## Debug Save

Use the toolbar `Debug Save` button to save a developer/testing copy to:

- `Generated/debug_saved_project.vfb.json`

This shortcut does not replace the main project path workflow.

## Recent files

`VisiForm` stores up to 10 recent project paths through the shared app settings file.

Current settings storage location:

- `%APPDATA%/VisiForm/settings.json`

If `APPDATA` is unavailable, the editor falls back to:

- `Generated/settings.json`

Recent files are shown in the left panel under the project tree when that panel is visible.
Clicking a recent file row attempts to load that project.
Missing recent files are removed from the stored list when clicked.

## Unsaved changes

Before `New`, `Open`, `Open Sample`, or opening a recent file, `VisiForm` prompts when the current document has unsaved changes.

Current choices:

- `Save`
- `Don't Save`
- `Cancel`

If `Save` is chosen, the normal `Save` workflow runs first.
If that save fails or is cancelled, the original operation stops.

## Related file format

Project file structure is documented in `docs/project_file_format.md`.
