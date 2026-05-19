# Phase 50 generated widget state API plan

## Phase title

Phase 50 generated widget state API

## Goal

Add a generated runtime widget state API to exported projects so user subclass callback code can read and modify generated widget state safely by widget id or name.

## Scope

- Generate protected runtime helper methods on the exported `MainWindow` base class.
- Support id-or-name lookup for generated widgets.
- Support text, checked, selected, numeric, progress, and status-bar field state mutation helpers.
- Request repaint after generated runtime state changes.
- Keep generated user subclass export and `USER CODE` preservation working.
- Update generator documentation for the new runtime API.

## Files to inspect

- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `src/generator/VisageCppEmitter.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CodeGenerator.h`
- `src/generator/CodeGenerator.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetNode.h`
- `src/model/ProjectDocument.h`
- `docs/code_generation.md`
- `docs/widget_catalog.md`
- `docs/widget_registry.md`
- `docs/agent_plans/phase_50_generated_widget_state_api_plan.md`

## Step-by-step TODO checklist with markdown checkboxes

- [x] Create the persistent phase plan file before code changes
- [x] Inspect generator runtime API sources and current generated runtime state model
- [x] Add protected generated widget lookup helpers for id and name access
- [x] Add generated state getter and setter helpers for text, bool, numeric, progress, and status-bar fields
- [x] Ensure generated state setters request redraw and keep radio-group exclusivity and numeric clamping rules
- [x] Improve generated user subclass TODO stubs with short runtime API examples for new handlers only
- [x] Preserve existing user code blocks unchanged on re-export
- [x] Update generated runtime API documentation in `docs/code_generation.md`
- [x] Update widget runtime usage documentation in `docs/widget_catalog.md`
- [x] Update registry guidance in `docs/widget_registry.md`
- [x] Validate changed generator files for compile issues
- [x] Build the main `VisiForm` app successfully with `build-static-debug`
- [x] Write the final result summary and remaining manual test notes

## Build validation checklist

- [x] Build only the main `VisiForm` project
- [x] Use the `build-static-debug` workflow
- [x] Fix compile errors introduced by this phase
- [x] Do not run `VisiForm.exe`
- [x] Do not use `Start-Process`
- [x] Do not launch the generated app
- [x] Record successful build completion in this phase plan

## Manual generated-project test checklist

- [ ] Export a generated project after setting `projectName`, `executableName`, `userSubclassName`, and `windowTitle`
- [ ] Build generated Debug with `build-static-debug` or the `vs2022-x64-static-debug` preset flow
- [ ] Build generated Release with the `vs2022-x64-static-release` preset flow
- [ ] Verify generated `MainWindow` exposes the protected runtime helper API
- [ ] Verify a user callback can call `setText(...)` and the target widget redraws with updated text
- [ ] Verify a user callback can call `setChecked(...)` for `CheckBox`
- [ ] Verify a user callback can call `setSelected(...)` for `RadioButton` and group exclusivity is preserved
- [ ] Verify a user callback can call `setValue(...)` for `Slider`, `ScrollBar`, and `ProgressBar`
- [ ] Verify a user callback can call `setProgressValue(...)` for `ProgressBar`
- [ ] Verify a user callback can call `setStatusBarField(...)` for `StatusBar`
- [ ] Verify id lookup resolves before name lookup when both exist
- [ ] Verify missing widget lookups return safe defaults without exceptions
- [ ] Verify existing `USER CODE` blocks survive re-export unchanged

## Final result summary

Completed.

- The generator now emits protected `MainWindow` runtime helpers for widget lookup by exact `id` or exact `name`, plus state accessors and mutators for text, checked state, radio selection, numeric values, progress values, and status-bar fields.
- Helper setters use safe return values, request generated repaint through `requestGeneratedUiRepaint()`, and avoid re-firing the same generated callback when user callback code mutates widget state.
- `setSelected(...)` now enforces generated `RadioButton` group exclusivity, while `setValue(...)` clamps `Slider`, `ScrollBar`, and `ProgressBar` values to the widget runtime `min` and `max` range.
- Generated user subclass stubs now add short runtime-helper examples only for new empty handler bodies; preserved `USER CODE` blocks still remain untouched across re-export.
- Documentation was updated in `docs/code_generation.md`, `docs/widget_catalog.md`, and `docs/widget_registry.md` to describe the generated runtime widget state API and future extension rules.
- Validation: file-level validation for `src/generator/VisageCppEmitter.cpp` reported no errors, and the main `VisiForm` app built successfully with `build-static-debug` after invoking the preset from an x64 Visual Studio developer environment.

## Remaining TODOs

- Perform manual generated-project export and runtime validation in Visual Studio.
- Confirm the generated helper API against a fresh export by editing user callback code to call `setText(...)`, `setStatusBarField(...)`, and `setProgressValue(...)`.
- Verify re-export still preserves existing user callback bodies after helper usage is added.
