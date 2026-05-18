# App settings

`VisiForm` stores editor-level settings in a JSON file.

## Settings file location

Preferred location:

- `%APPDATA%/VisiForm/settings.json`

Fallback location when `APPDATA` is unavailable:

- `Generated/settings.json`

## Stored fields

Current settings include:

- `recentFiles`
- `lastProjectDirectory`
- `lastExportDirectory`
- `showGrid`
- `snapToGrid`
- `gridSize`
- `majorGridSize`

## Recent files

Recent files are stored inside the shared settings file.

Behavior:

- up to 10 paths
- most recent first
- duplicate paths removed
- missing paths removed when settings are loaded or when a missing recent file is clicked

## Grid and snap preferences

The designer canvas loads these preferences on startup:

- `showGrid`
- `snapToGrid`
- `gridSize`
- `majorGridSize`

If toolbar grid or snap toggles are used, the updated values are saved immediately.

## Project and export directories

Project `.vfb.json` dialogs and export folder dialogs use separate remembered directories.

- `lastProjectDirectory` is used only for project Open / Save As flows
- `lastExportDirectory` is used only for generated-project export flows

Default project folder:

- `Generated/Projects`

Default export fallback folder:

- `Generated/ExportedVisageProject`

Exporting to another folder must not overwrite `lastProjectDirectory`.
Saving or opening a project must not overwrite `lastExportDirectory`.

## Current limitations

- No full settings UI yet
- No per-project editor settings yet
- No custom unsaved-changes dialog yet
- No schema migration for settings yet
