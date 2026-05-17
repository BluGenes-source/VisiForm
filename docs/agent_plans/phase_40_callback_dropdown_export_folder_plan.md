# Phase 40 callback dropdown & export folder plan

## Goal

Fix callback suggestion selection in the Property Inspector and add a native folder dialog for selecting the export folder. Preserve `Generated/ExportedVisageProject` as a fallback and persist `lastExportDirectory` in app settings.

## Current problems

- Callback suggestion list appears but items cannot be selected by clicking.
- No native folder picker exists for export; export uses a default folder only.

## Files to inspect

- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/utils/NativeFileDialogs.h`
- `src/utils/NativeFileDialogs.cpp`
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`
- `src/generator/CodeGenerator.h`
- `src/generator/CodeGenerator.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `docs/code_generation.md`
- `docs/widget_registry.md`
- `docs/widget_catalog.md`
- `docs/agent_plans/phase_40_callback_dropdown_export_folder_plan.md`

## Step-by-step TODO list

- [x] Create this persistent phase TODO file
- [ ] Diagnose how callback suggestions are generated and drawn
- [ ] Add suggestion hit-testing to `PropertyInspector`
- [ ] Make suggestions clickable and apply on click
- [ ] Ensure suggestions commit to the model and mark project dirty
- [ ] Add native folder dialog API in `utils::NativeFileDialogs`
- [ ] Implement `MainWindow::exportGeneratedCode*` flows using folder dialog
- [ ] Persist and reuse `lastExportDirectory` through `AppSettings`
- [ ] Ensure `Generated/ExportedVisageProject` remains usable as fallback
- [ ] Update docs about export folder dialog and default fallback
- [ ] Build and fix compile errors
- [ ] Test property suggestions selection and export folder flow manually

## Current progress notes

- Plan file created and committed before code changes.
- Preliminary inspection suggests suggestion rows were rendered as read-only rows, which caused clicks to be consumed without applying suggestions.

## Build validation checklist

- [ ] Build main `VisiForm` with `build-static-debug`.
- [ ] Fix any compile errors.
- [ ] Do not run `VisiForm.exe` from the agent.

## Manual test checklist

- [ ] Click an event property, see suggestions, click a suggestion and confirm it is applied.
- [ ] Type a new callback name and press Enter to commit.
- [ ] Attempt an invalid handler name and confirm validation occurs.
- [ ] Open Export, choose a folder, confirm generated files appear in that folder.
- [ ] Cancel Export folder dialog and confirm no files written and status shows "Export cancelled".
- [ ] Export without folder dialog via fallback (if necessary) and confirm default folder written.

## Final result summary

Pending.
