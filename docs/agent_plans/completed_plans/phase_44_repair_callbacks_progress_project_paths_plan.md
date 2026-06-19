# Phase 44 repair callbacks, progress, and project paths plan

## Goal

Repair callback suggestion selection for multi-item lists, add sender-aware generated callback signatures, fix ProgressBar text contrast, keep project and export dialog directories separate, enforce one hint pane in the status bar, and update Copilot rules so every phase maintains a persistent TODO plan file.

## Current remaining bugs

- Callback suggestions can still appear as one comma-separated entry, making only the first handler reliably selectable.
- Generated callback signatures do not yet provide sender metadata.
- ProgressBar text is still unreadable at mixed fill percentages.
- Project Save As can drift to the export folder instead of staying in the dedicated project folder.
- Hints can still appear in more than one status pane.
- Copilot instruction files do not yet explicitly require phase TODO plan maintenance details for every run.

## Files to inspect

- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetNode.h`
- `src/model/ProjectDocument.h`
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`
- `src/utils/NativeFileDialogs.h`
- `src/utils/NativeFileDialogs.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CodeGenerator.cpp`
- `docs/code_generation.md`
- `docs/widget_catalog.md`
- `docs/settings.md`
- `docs/project_file_format.md`
- `docs/copilot_rules.md`
- `docs/agent_plans/phase_44_repair_callbacks_progress_project_paths_plan.md`

## Step-by-step TODO list with checkboxes

- [x] Create persistent phase TODO plan file before code changes
- [x] Inspect callback suggestion generation, hit testing, and apply flow
- [x] Inspect project and export dialog initial-directory flow
- [x] Update repository Copilot instruction files with mandatory plan maintenance rules
- [x] Replace comma-joined callback suggestions with separate suggestion items
- [x] Add a direct callback suggestion apply path into the selected widget property
- [x] Add sender-aware generated callback signatures and `WidgetEvent`
- [x] Update callback compatibility kinds to sender-aware signature groups
- [x] Fix ProgressBar text contrast in designer preview
- [x] Fix ProgressBar text contrast in the bottom export progress pane
- [x] Keep hints in one status pane only
- [x] Keep project dialog folders separate from export folder memory
- [x] Update docs for callback API, contrast behavior, directory separation, and Copilot TODO rules
- [x] Build with `build-static-debug`
- [x] Record final result summary

## Current progress notes

- Phase plan file created before edits.
- Confirmed the old callback picker stored suggestions as one comma-joined display string and relied on row hit-testing rather than discrete suggestion items.
- Confirmed project Save As drifted because the native file dialog prioritized the suggested file path parent over the explicit initial directory.
- Callback suggestions are now rebuilt as separate stacked items, applied through a direct selected-widget property path, and consumed before normal row handling.
- Generated callbacks now use sender-aware `WidgetEvent` signatures and the widget registry groups callback suggestions by `void_event`, `bool_event`, `float_event`, and `string_event`.
- ProgressBar text now uses a dedicated text area in the designer preview and in the bottom export progress pane for reliable contrast.
- Hints now stay in the left status pane only, while pane 1 shows selection info and pane 2 shows export progress only.
- Project dialogs now prefer `Generated/Projects` or `lastProjectDirectory`, while export continues to use `lastExportDirectory` separately.

## Build validation checklist

- [x] Build the main `VisiForm` project with `build-static-debug`
- [x] Fix any compile errors introduced by this phase
- [x] Do not run `VisiForm.exe`
- [x] Do not launch the generated app

## Manual test checklist

- [ ] Edit a `RadioButton` `onSelected` field when multiple existing callbacks exist and verify each suggestion is separately selectable
- [ ] Verify selecting any callback suggestion immediately updates the event property row and marks the project dirty
- [ ] Verify manual callback typing still supports Backspace, Enter apply, Escape cancel, and invalid name rejection
- [ ] Export a project and inspect generated sender-aware callback signatures and preserved USER CODE blocks
- [ ] Verify ProgressBar text is readable at 10%, 50%, and 90% in designer preview
- [ ] Verify the bottom export progress pane text is readable during export
- [ ] Verify hints appear only in pane 0 of the bottom status bar
- [ ] Verify Open/Save As project dialogs start in `Generated/Projects` or `lastProjectDirectory`
- [ ] Verify export folder dialog starts in `lastExportDirectory`
- [ ] Verify saving/opening project files does not change `lastExportDirectory`

## Final result summary

Completed.

- Callback suggestions are now rebuilt and drawn as separate clickable items instead of one comma-joined display string, and clicking any visible suggestion applies that exact callback to the selected widget property.
- Generated callbacks now use sender-aware `WidgetEvent` signatures and the widget registry uses sender-aware signature groups for callback compatibility.
- Progress text is now kept readable by using dedicated text areas beside the progress fill in the designer preview, export progress pane, and generated static previews.
- Project Open and Save As now prefer `lastProjectDirectory` or `Generated/Projects`, while export continues to use `lastExportDirectory` independently.
- The bottom status bar now keeps hints in pane 0 only, selection info in pane 1, and export progress in pane 2.
- Repository Copilot instruction files now explicitly require persistent phase TODO creation, checkbox updates, final summaries, and remaining TODO notes.

Remaining TODOs:

- Manual Visual Studio verification is still required for multi-suggestion callback selection, sender-aware generated stubs, progress text readability at several values, and project/export dialog folder memory behavior.
