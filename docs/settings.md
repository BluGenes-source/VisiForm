# App settings

`VisiForm` stores editor-level settings in a JSON file.

## Settings file location

Preferred locations:

- Windows: `%APPDATA%/VisiForm/settings.json`
- macOS: `$HOME/Library/Application Support/VisiForm/settings.json`
- Linux and other Unix-like environments: `$XDG_CONFIG_HOME/VisiForm/settings.json`
- Linux fallback when `XDG_CONFIG_HOME` is unavailable: `$HOME/.config/VisiForm/settings.json`

Fallback location when `APPDATA` is unavailable:

- `Generated/settings.json`

## Stored fields

Current settings include:

- `recentFiles`
- `lastProjectDirectory`
- `lastExportDirectory`
- `localVisageSourceDirectory`
- `visageGitRepository`
- `visageGitTag`
- `showGrid`
- `snapToGrid`
- `smartGuidesEnabled`
- `gridSize`
- `majorGridSize`
- `propertyInspectorWidth`

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
- `smartGuidesEnabled`
- `gridSize`
- `majorGridSize`

If toolbar grid, snap, or smart-guide toggles are used, the updated values are saved immediately.

## Editor shell layout state

The editor shell also persists the Property Inspector width through:

- `propertyInspectorWidth`

Behavior:

- the Designer Canvas / Property Inspector splitter restores the last saved inspector width on startup
- restored width is clamped against the current window size and pane minimums
- dragging the splitter saves the updated width after the drag completes

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

## Export dependency settings

Generated-project export also stores dependency settings used for emitted CMake files.

- `localVisageSourceDirectory`
  - default: empty
  - when non-empty, generated `CMakePresets.json` can prefill `VISIFORM_VISAGE_SOURCE_DIR`
- use forward slashes for configured paths, for example `C:/dev/visage` or `/home/you/dev/visage`
  - the root `FormWindow` property inspector now exposes this as an app-level `Export / Dependencies` field
- `visageGitRepository`
  - default: `https://github.com/VitalAudio/visage.git`
  - used by generated `CMakeLists.txt` as the `FetchContent` fallback repository
  - generated `CMakePresets.json` also emits this as `VISIFORM_VISAGE_GIT_REPOSITORY`
- `visageGitTag`
  - default: `main`
  - used by generated `CMakeLists.txt` as the `FetchContent` fallback tag or commit
  - generated `CMakePresets.json` also emits this as `VISIFORM_VISAGE_GIT_TAG`

Generated project dependency behavior:

- valid `localVisageSourceDirectory` prefers local `add_subdirectory(...)`
- if that setting is empty or invalid, generated CMake checks the
  `VISIFORM_VISAGE_SOURCE_DIR` environment variable
- generated CMake then checks nearby `visage` sibling folders such as
  `../visage`, `../../visage`, and `../../../visage`
- if no valid local source is found, generated CMake falls back to
  `FetchContent`
- this avoids re-downloading Visage for repeated local exports when a shared checkout already exists
- when editing `localVisageSourceDirectory`, an invalid path shows a warning status if `CMakeLists.txt` is missing, but export is still allowed

## Current limitations

- No full settings UI yet
- No per-project editor settings yet
- No custom unsaved-changes dialog yet
- No schema migration for settings yet
