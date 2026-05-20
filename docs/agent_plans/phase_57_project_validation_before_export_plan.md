# Phase 57 - Project validation before export plan

## Phase title
Project validation before export

## Goal
Add a project validation system that checks the current `.vfb.json` project before export, reports warnings and errors to the user inside `VisiForm`, blocks export on critical validation errors, allows export with warnings, and preserves generated export behavior when validation passes.

## Files to inspect
- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/utils/CppIdentifier.h`
- `src/utils/CppIdentifier.cpp`
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/DesignerCanvas.h`
- `src/generator/CodeGenerator.h`
- `src/generator/CodeGenerator.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `docs/code_generation.md`
- `docs/project_file_format.md`
- `docs/widget_registry.md`
- `docs/agent_plans/phase_57_project_validation_before_export_plan.md`

## TODO checklist
- [x] Create this phase plan file with the required sections before changing code.
- [x] Inspect the listed model, utility, UI, generator, and documentation files.
- [x] Design the validation model, report format, and non-UI integration boundaries.
- [x] Add `src/validation/ProjectValidator.h` and `src/validation/ProjectValidator.cpp`.
- [x] Validate project naming and local Visage settings.
- [x] Validate widget ids, names, bounds, colors, enum properties, numeric properties, and unsupported export cases.
- [x] Validate callback names, callback signature conflicts, and `RadioButton` group consistency.
- [x] Add a Validate or Check command to the `VisiForm` UI.
- [x] Show validation status to the user and produce a full validation report.
- [x] Run validation automatically before export.
- [x] Block export on validation errors and allow export with warnings.
- [x] Keep generated export behavior unchanged when validation passes.
- [x] Update `docs/code_generation.md`.
- [x] Update `docs/project_file_format.md`.
- [x] Create `docs/project_validation.md`.
- [x] Build the main `VisiForm` project with the `build-static-debug` workflow.
- [x] Fix any compile errors introduced by this phase.
- [x] Update this plan with build validation results, remaining TODOs, and the final result summary before finishing.

## Build validation checklist
- [x] Build the main `VisiForm` project with `build-static-debug`.
- [x] Confirm the main app build completed successfully.
- [x] Confirm `VisiForm.exe` was not launched.
- [x] Confirm no generated-project applications were launched.

### Latest build validation
- [x] Revalidated on 2026-05-20 by loading `VsDevCmd.bat` for `x64`, running `cmake --preset vs2022-x64-static-debug`, and then running `cmake --build --preset build-static-debug` from `J:\Dev\CeePlusPlus\VisiForm`.
- [x] Fixed the temporary phase 57 compile break caused by the missing `MainWindow::validateProject` declaration and rebuilt successfully.
- [x] Confirmed the main `VisiForm` target linked successfully as `VisiForm.exe` without launching the app.
- [x] Confirmed no generated-project applications were launched during validation.

## Manual test checklist
- [ ] Open an existing `.vfb.json` project and run the new Validate or Check command.
- [ ] Confirm the validation report summary appears in the UI status area.
- [ ] Confirm a full validation report is produced and can be reviewed.
- [ ] Introduce an invalid callback name and confirm it is reported as an error.
- [ ] Reuse the same callback name across incompatible event signatures and confirm it is reported as an error.
- [ ] Introduce an invalid color string and confirm it is reported as an error.
- [ ] Create duplicate widget ids and confirm export is blocked.
- [ ] Create duplicate widget names and confirm they are reported as warnings.
- [ ] Set an invalid `ScrollBar` orientation and confirm it is reported.
- [ ] Create a conflicting `RadioButton` group selection and confirm it is reported.
- [ ] Confirm export is blocked when validation errors exist.
- [ ] Confirm export proceeds when only warnings exist.
- [ ] Confirm save, load, and export still work when validation passes.
- [ ] Confirm a generated project still builds when validation passes.

## Final result summary
Added `src/validation/ProjectValidator.h` and `src/validation/ProjectValidator.cpp` as a non-UI validation layer that checks project naming, local Visage dependency settings, widget ids and names, bounds, colors, enum-like values, numeric ranges, callback identifiers, callback signature conflicts, `RadioButton` groups, and selected export-compatibility cases such as empty image paths and static-only `ColorPicker` runtime behavior. Integrated validation into `src/ui/MainWindow.cpp` and `src/ui/MainWindow.h` with a new toolbar `Chk` action that writes `Generated/validation_report.md`, reports validation status in `VisiForm`, blocks export on validation errors, and allows export with warnings while leaving normal generator behavior unchanged when validation passes cleanly. Updated `CMakeLists.txt`, `docs/code_generation.md`, `docs/project_file_format.md`, and created `docs/project_validation.md`; then revalidated the main `VisiForm` app successfully with the required `build-static-debug` workflow in this session without launching `VisiForm.exe` or any generated app.

## Remaining TODOs
- Manual in-app verification remains pending:
  - Run the new toolbar `Chk` action on a valid project and inspect `Generated/validation_report.md`.
  - Create invalid callback names, duplicate widget ids, duplicate widget names, invalid colors, invalid `ScrollBar` orientation values, and conflicting `RadioButton` groups to confirm the expected warnings and errors.
  - Confirm export is blocked when validation errors exist.
  - Confirm export proceeds when only warnings exist.
  - Confirm save, load, and export still work when validation passes.
  - Confirm a generated project still builds when validation passes.
