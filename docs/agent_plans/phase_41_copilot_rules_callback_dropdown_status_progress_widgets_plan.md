# Phase 41 - Copilot rules, callback dropdown, status & progress, widgets

## Goal

1. Fix callback suggestion selection (make suggestions clickable and applied).
2. Add StatusBar and ProgressBar widget types and use them in the editor UI for status and export progress.
3. Add repository-level Copilot instruction files for future agent runs.

## Current problems

- Callback suggestion item shows but cannot be selected.
- Export needs visible progress status.
- Missing StatusBar and ProgressBar widgets.

## Files to inspect

- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`
- `src/generator/CodeGenerator.h`
- `src/generator/CodeGenerator.cpp`
- `docs/copilot_rules.md`

## Step-by-step TODO list

- [x] Create repository-level Copilot instructions
- [x] Create path-specific Copilot instructions
- [x] Create docs/copilot_rules.md
- [x] Create this phase plan file
- [ ] Diagnose callback suggestion generation and hit-testing
- [ ] Ensure suggestion hit-testing is evaluated before row selection
- [ ] Add StatusBar and ProgressBar widget definitions in WidgetRegistry
- [ ] Add enum entries and string mappings for new widget types
- [ ] Render StatusBar and ProgressBar previews in DesignerCanvas
- [ ] Add StatusBar and ProgressBar to Widget Palette automatically via WidgetRegistry
- [ ] Add MainWindow export progress state and display in status area
- [ ] Add CodeGenerator progress callback and call it at stages
- [ ] Wire MainWindow export to use the progress callback
- [ ] Build and fix compile issues
- [ ] Manual testing checklist

## Current progress notes

- Copilot instruction files and docs added.
- PropertyInspector suggestion hit-testing was previously extended; further diagnosis pending.

## Build validation checklist

- [ ] Build main `VisiForm` with `build-static-debug` after implementing changes
- [ ] Fix any compile issues

## Manual test checklist

- [ ] Click an event property, see suggestions, click a suggestion and confirm it is applied
- [ ] Export with folder dialog, observe progress updates in the status area
- [ ] Add StatusBar widget from palette, edit its properties, save, load, and export
- [ ] Add ProgressBar widget from palette, edit properties, save, load, and export

## Final result summary

Pending
