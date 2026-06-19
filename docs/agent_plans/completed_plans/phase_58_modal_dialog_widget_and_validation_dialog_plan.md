# Phase 58 Modal Dialog Widget and Validation Dialog Plan

## Goal
Add a reusable modal dialog system that works in both the VisiForm editor and exported/generated projects, use it for validation results, add a `ModalDialog` widget type, and keep generated code buildable with USER CODE preservation intact.

## Current State
- The `Chk / Validate` command works.
- Validation appears to run correctly.
- Validation currently behaves mostly as a behind-the-scenes status/report operation.
- VisiForm needs a reusable modal dialog system.
- Exported/generated projects should also be able to show modal dialogs from event callbacks.
- Generated widgets are interactive.
- Generated runtime state API exists.
- USER CODE preservation works.

## Files to Inspect
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetDefinition.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetNode.h`
- `src/model/ProjectDocument.h`
- `src/validation/ProjectValidator.h`
- `src/validation/ProjectValidator.cpp`
- `src/generator/VisageCppEmitter.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CodeGenerator.cpp`
- `docs/project_validation.md`
- `docs/widget_catalog.md`
- `docs/widget_registry.md`
- `docs/code_generation.md`
- `docs/project_file_format.md`

## TODO Checklist
- [x] Create the Phase 58 plan file.
- [x] Inspect the existing editor validation flow, export validation guard, widget registry, and generator runtime hooks.
- [x] Design the editor modal dialog state and blocking behavior.
- [x] Implement the editor modal dialog overlay in `MainWindow`.
- [x] Route validation summaries through the editor modal dialog while preserving report generation.
- [x] Block export on validation errors with a modal dialog.
- [x] Add `WidgetType::ModalDialog` model and registry support.
- [x] Add `ModalDialog` palette, designer preview, property inspector, save/load, and export support.
- [x] Add generated runtime modal helpers and modal rendering/input behavior.
- [x] Generate `ModalDialog` callback wiring and new USER CODE examples safely.
- [ ] Update documentation for validation, widgets, generation, and file format changes.
- [ ] Build the main `VisiForm` app with `build-static-debug` and fix introduced compile errors.
- [ ] Record final validation status, remaining TODOs, and result summary.

## Build Validation Checklist
- [ ] Configure/build using the `build-static-debug` workflow only.
- [ ] Confirm the main `VisiForm` application builds successfully.
- [ ] Fix any compile errors introduced by this phase.
- [ ] Do not run `VisiForm.exe`.
- [ ] Do not launch generated applications.

## Manual Test Checklist
- [ ] Click `Chk / Validate` and verify a validation modal summary appears.
- [ ] Verify validation still writes the full report when applicable.
- [ ] Verify validation errors block export and show a modal dialog.
- [ ] Verify warnings do not block export.
- [ ] Verify the modal overlay blocks underlying editor clicks.
- [ ] Add a `ModalDialog` from the widget palette.
- [ ] Edit `ModalDialog` properties in the property inspector.
- [ ] Save and reload a project containing a `ModalDialog`.
- [ ] Export a project containing a `ModalDialog`.
- [ ] Build exported Debug and Release projects manually.
- [ ] Verify `showMessageDialog(...)` works from a button callback USER CODE block.
- [ ] Verify `showModalDialog(...)` works with a `ModalDialog` widget.
- [ ] Verify `onAccepted` and `onCancelled` callbacks fire correctly.
- [ ] Verify USER CODE preservation still works after re-export.

## Final Result Summary
- Status: In progress.
- Build validation: Pending.
- Remaining TODOs: Documentation updates, build validation, and final summary remain pending.

## Progress Notes
- Inspected `MainWindow`, `DesignerCanvas`, `PropertyInspector`, `WidgetRegistry`, `WidgetNode`, `ProjectValidator`, `VisageCppEmitter`, `CodeGenerator`, and JSON project reader paths.
- Confirmed validation currently writes `Generated/validation_report.md`, updates status text, and blocks export on errors without a dialog.
- Confirmed widget metadata is registry-driven, editor size limits come from `WidgetRegistry` via `WidgetMetrics`, and save/load already serializes arbitrary widget properties.
- Confirmed generated runtime widgets and event dispatch are emitted from `VisageCppEmitter`, so modal runtime support belongs in the generated base header/cpp emission path.
- Added editor modal dialog state and helpers to `MainWindow`, including centered overlay drawing, modal button hit testing, and keyboard dismissal for `Escape` / `Enter`.
- Routed `Chk / Validate` to show a modal summary after validation while preserving status text and validation report generation.
- Updated export validation handling so validation errors still block export and now also show a modal error summary.
- Added `WidgetType::ModalDialog` to the core model, string conversion helpers, widget registry metadata, and editor event property validation.
- Added a design-time `ModalDialog` preview in `DesignerCanvas`; palette and inspector exposure now flow automatically from `WidgetRegistry` metadata.
- Confirmed `ModalDialog` save/load persistence continues to flow through generic widget property serialization without schema changes.
- Added generated runtime modal helpers to the emitted base window, including `showMessageDialog(...)`, `showModalDialog(...)`, `closeModalDialog()`, and `activeModalDialogId()` support.
- Added generated modal overlay rendering, modal button layout, keyboard dismissal/accept handling, and modal input blocking so dialogs capture interaction above the exported form.
- Added generated `ModalDialog` accept/cancel callback dispatch and expanded default USER CODE examples so button callbacks can show modal dialogs safely.
