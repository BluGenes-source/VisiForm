# Phase 60 new project wizard and project settings plan

## Phase title
Phase 60 new project wizard and project settings

## Goal
Add a `New Project Wizard` and a `Project Settings` dialog using the existing `VisiForm` editor modal dialog system.

## Current state
- `VisiForm` builds and runs.
- Menu bar works.
- Validation modal works.
- Modal dialog system exists.
- Generated projects build and run.
- Generated widgets are interactive.
- Project naming exists.
- Export settings and local `Visage` path support exist.
- Project validation exists.
- Save, load, and export work.

## Files to inspect
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/ProjectTree.h`
- `src/ui/WidgetPalette.h`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/LookAndFeelRegistry.h`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/generator/CodeGenerator.cpp`
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`
- `src/validation/ProjectValidator.h`
- `src/validation/ProjectValidator.cpp`
- `docs/project_file_format.md`
- `docs/code_generation.md`
- `docs/menu_bar.md`
- `docs/project_validation.md`
- `docs/new_project_wizard.md`
- `docs/agent_plans/phase_60_new_project_wizard_and_project_settings_plan.md`

## TODO checklist
- [x] Inspect current menu, modal, project settings, serialization, export, and validation flow.
- [x] Define wizard state, project settings state, validation helpers, and template list.
- [x] Add modal UI state and rendering support for editable wizard and project settings dialogs.
- [x] Integrate `File > New` with the new project wizard.
- [x] Integrate `Project > Settings` with the project settings dialog.
- [x] Add project creation helpers for a blank project and built-in templates.
- [x] Apply wizard-created project settings to a new `ProjectDocument`.
- [x] Apply project settings edits to `ProjectDocument` and `AppSettings`.
- [x] Keep save and load compatibility for project naming, form size, and look-and-feel values.
- [x] Keep export and validation behavior working with edited project settings.
- [x] Update related documentation.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Write the final result summary and remaining TODOs.

## Build validation checklist
- [x] Build only the main `VisiForm` project.
- [x] Use the `build-static-debug` workflow.
- [x] Confirm the build completes successfully.
- [x] Fix compile errors introduced by this phase.
- [x] Record the successful build result in this plan.
- [x] Do not run `VisiForm.exe`.
- [x] Do not launch generated apps.

Build result:

- Initial `scripts/build_static_debug.cmd` attempt failed with the known x86 SDK and library mismatch warnings and link errors.
- Retried the same `build-static-debug` workflow from `VsDevCmd.bat -arch=x64 -host_arch=x64`.
- The main `VisiForm` app then built successfully.

## Manual test checklist
- [ ] Open `File > New` and confirm the new project wizard opens as a centered modal-sized dialog.
- [ ] Edit `projectName`, `executableName`, `windowTitle`, and `userSubclassName` in the wizard.
- [ ] Change form width, form height, look and feel, and template in the wizard.
- [ ] Confirm invalid wizard input keeps the wizard open and shows an error.
- [ ] Create a blank project from the wizard.
- [ ] Create at least three non-blank templates from the wizard.
- [ ] Confirm the created project updates `ProjectTree`, `DesignerCanvas`, and `PropertyInspector`.
- [ ] Confirm the root `FormWindow` is selected after project creation.
- [ ] Open `Project > Settings` and confirm the settings dialog opens as a centered modal-sized dialog.
- [ ] Edit `projectName`, `executableName`, `windowTitle`, `userSubclassName`, and `lookAndFeelId` in project settings.
- [ ] Edit local `Visage` dependency settings in project settings and confirm they persist through `AppSettings`.
- [ ] Confirm save and load preserve project naming, look and feel, and form size.
- [ ] Confirm export uses the updated executable name, subclass name, window title, and local `Visage` settings.
- [ ] Confirm validation passes for default wizard templates.
- [ ] Confirm existing save, load, export, validation, menu, and toolbar behavior still works.

## Final result summary
- Added a centered modal `New Project Wizard` that opens from `File > New`.
- Added wizard state, inline modal row editing, validation, and template selection inside `MainWindow` using the existing editor modal overlay and shared text editor.
- Added built-in `blank`, `basic_app`, `form_with_status`, `control_panel`, and `dialog_test` project templates using `WidgetRegistry` defaults and generated widget ids.
- Added a centered modal `Project Settings` dialog from `Project > Settings` for project naming, look and feel, and export dependency values.
- Kept project document naming, form size, and look-and-feel persistence on the existing `.vfb.json` fields and root form bounds.
- Kept local `Visage` dependency settings in `AppSettings` so export still reads machine-specific values from the existing settings store.
- Updated `docs/new_project_wizard.md`, `docs/menu_bar.md`, `docs/project_file_format.md`, and `docs/code_generation.md`.
- Built the main `VisiForm` app successfully with the required `build-static-debug` workflow after retrying from the x64 Visual Studio developer environment.

## Remaining TODOs
- Manually verify the new project wizard flow inside the running editor.
- Manually verify the built-in templates, project settings modal edits, and save/load/export/validation behavior from Visual Studio.
