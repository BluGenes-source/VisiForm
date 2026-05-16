# Phase 37 hints and tooltips TODO

## Goal

Add reliable hints for crowded toolbar and editor controls, plus an editable widget `hint` property that is preserved through save/load and available for status-bar help text.

## Files to inspect

- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/PropertyValue.h`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `docs/widget_catalog.md`
- `docs/project_file_format.md`
- `docs/layout_tools.md`
- `docs/settings.md`
- `docs/agent_plans/phase_37_hints_tooltips_todo.md`

## Step-by-step TODO list

- [x] Create persistent phase TODO file
- [x] Inspect toolbar, palette, property inspector, serialization, sample data, and generator hint paths
- [x] Add toolbar button hint metadata
- [x] Add widget palette hint metadata
- [x] Add default widget `hint` properties
- [x] Show toolbar hints in the status bar using hover or reliable fallback behavior
- [x] Show widget palette hints in the status bar using hover or reliable fallback behavior
- [x] Expose editable `hint` property in `PropertyInspector`
- [x] Verify hint save/load compatibility
- [x] Update sample project hint data
- [x] Add generated code hint comments if practical
- [x] Update docs
- [x] Run `build-static-debug`
- [x] Record final result summary

## Current progress notes

- Phase TODO file created before code changes.
- Toolbar buttons now carry hint metadata and expose status-bar hints on hover through `MainWindow::mouseMove(...)`.
- Widget palette items now expose status-bar hints on hover.
- Widgets now use a normal string `hint` property with default values for new widgets and the default project.
- `PropertyInspector` now shows and edits the `hint` property.
- Existing serialization already preserves generic widget properties, so `hint` save/load compatibility did not require schema code changes.
- Generated export now emits `// Hint: ...` comments when a widget hint is present.

## Build validation checklist

- [x] Build with `build-static-debug`
- [x] Fix compile errors if any appear
- [x] Do not run `VisiForm.exe`

## Manual test checklist

- [ ] Hover toolbar buttons and confirm useful hints appear, or verify the documented fallback behavior
- [ ] Hover widget palette items and confirm useful hints appear, or verify the documented fallback behavior
- [ ] Select a widget and verify the `hint` property appears in `PropertyInspector`
- [ ] Edit the `hint` property and confirm the document becomes dirty
- [ ] Save and reload a project and confirm hint properties persist
- [ ] Open the sample project and confirm hint properties exist
- [ ] Export and confirm existing behavior still works
- [ ] Re-check selection, multi-select, box-select, group move, copy/paste, layout tools, smart guides, and user-code preservation

## Final result summary

Completed.

- Added toolbar and widget-palette hint metadata with status-bar hover hints.
- Added a regular widget `hint` property with default values for newly created widgets and the default project.
- Added editable `hint` rows to `PropertyInspector`.
- Verified that existing generic property serialization preserves `hint` through save and load.
- Updated the sample project with hint properties.
- Export now includes widget hint comments when present.
- The main project builds successfully with `build-static-debug`.
