# Phase 49 repair generated interactivity and project naming plan

## Phase title

Phase 49 repair generated interactivity and project naming

## Current bugs

1. Exported generated project compiles and runs, but controls are not interactive:
   - Button does not click or animate.
   - CheckBox does not toggle.
   - RadioButton does not select.
   - Slider does not drag.
   - ScrollBar does not respond.
   - TextBox does not accept input.
2. Exported project always builds and runs as `untitledVisFormProject.exe`.
3. `VisiForm` needs user-editable project naming so generated projects have correct names.

## Files to inspect

- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `src/generator/VisageCppEmitter.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CodeGenerator.h`
- `src/generator/CodeGenerator.cpp`
- `src/generator/CMakeEmitter.h`
- `src/generator/CMakeEmitter.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `Generated/ExportedVisageProject/src/main.cpp`
- `Generated/ExportedVisageProject/src/MainWindow.h`
- `Generated/ExportedVisageProject/src/MainWindow.cpp`
- `Generated/ExportedVisageProject/src/MyAppWindow.h`
- `Generated/ExportedVisageProject/src/MyAppWindow.cpp`
- `docs/code_generation.md`
- `docs/project_file_format.md`
- `docs/widget_catalog.md`
- `docs/agent_plans/phase_49_repair_generated_interactivity_and_project_naming_plan.md`

## Diagnosis notes

- Initial generator inspection shows the exported `Generated/ExportedVisageProject/src/MainWindow.cpp` currently emits static draw calls only.
- The exported `MainWindow` header currently overrides only `draw()` and exposes no mouse, keyboard, or text-input overrides.
- Existing generated event handlers are emitted as stubs, but the generated drawing path contains TODO comments instead of runtime dispatch.
- Generated widget drawing currently reads export-time literals instead of mutable runtime widget state.
- Current generated project naming flows still center on `projectName` and user subclass naming, but generated target naming needs deeper inspection.
- Main editor input API names must be mirrored from the working `src/ui/MainWindow` implementation before generated interaction is repaired.
- Confirmed working editor Visage hooks are `mouseDown`, `mouseMove`, `mouseDrag`, `mouseUp`, `mouseWheel`, `keyPress`, `receivesTextInput`, and `textInput` on `visage::ApplicationWindow`.
- Confirmed the working editor requests keyboard focus with `requestKeyboardFocus()` and refreshes interaction changes with `redraw()`.
- Main application execution remains prohibited in Agent Mode.

## Step-by-step TODO checklist with markdown checkboxes

- [x] Create the persistent phase plan file before code changes
- [x] Inspect the generator flow and current exported sources for runtime interaction gaps
- [x] Inspect the working editor input and redraw patterns used by the main `VisiForm` window
- [x] Document the generated interactivity failure cause in this plan
- [x] Add mutable generated runtime widget state and sender metadata in emitted code
- [x] Repair generated hit testing and coordinate conversion
- [x] Repair generated mouse interaction for `Button`, `CheckBox`, `RadioButton`, `Slider`, and `ScrollBar`
- [x] Repair generated keyboard and text input for `TextBox`
- [x] Ensure generated drawing reads mutable runtime state and requests redraw after state changes
- [x] Add project naming fields and defaults to `ProjectDocument`
- [x] Save and load project naming fields through JSON serialization
- [x] Expose root form project naming fields in the editor UI
- [x] Apply project naming to generated `CMakeLists.txt`, `README.md`, source files, and window title output
- [x] Preserve generated base class naming, user subclass export, and USER CODE regions
- [x] Update generation and project format documentation
- [x] Build the main `VisiForm` app successfully with `build-static-debug`
- [x] Write the final result summary and remaining manual test notes

## Build validation checklist

- [x] Build only the main `VisiForm` project
- [x] Use the `build-static-debug` workflow
- [x] Fix compile errors introduced by this phase
- [x] Do not run `VisiForm.exe`
- [x] Do not use `Start-Process`
- [x] Do not launch generated applications
- [x] Record successful build completion in this phase plan

## Manual generated-project test checklist

- [ ] Export a generated project after setting `projectName`, `executableName`, `userSubclassName`, and `windowTitle`
- [ ] Configure generated Debug with `vs2022-x64-static-debug`
- [ ] Build generated Debug successfully in Visual Studio 2022
- [ ] Configure generated Release with `vs2022-x64-static-release`
- [ ] Build generated Release successfully in Visual Studio 2022
- [ ] Verify the generated executable name matches `executableName`
- [ ] Verify generated window title uses `windowTitle` or `projectName`
- [ ] Verify generated `Button` clicks and fires `onClick`
- [ ] Verify generated `CheckBox` toggles visually and fires `onToggle`
- [ ] Verify generated `RadioButton` group selection updates visually and passes sender info
- [ ] Verify generated `Slider` drags, redraws, and fires `onChanged`
- [ ] Verify generated `ScrollBar` responds to arrows, paging, and thumb dragging where supported
- [ ] Verify generated `TextBox` accepts basic input, redraws, and fires `onTextChanged`
- [ ] Verify generated runtime drawing follows mutable widget state instead of static export literals
- [ ] Verify USER CODE regions are preserved across repeated export

## Final result summary

Completed.

- Cause of generated interactivity failure: `VisageCppEmitter` was exporting static draw-only widget code plus unused handler stubs, with no mutable runtime widget state and no actual Visage mouse or text-input overrides.
- Actual Visage input methods used for the generated runtime: `mouseDown`, `mouseMove`, `mouseDrag`, `mouseUp`, `keyPress`, `receivesTextInput`, `textInput`, `requestKeyboardFocus()`, and `redraw()`.
- Generated runtime state model: exported widgets now initialize into mutable `RuntimeWidget` records with runtime bounds, current values, text, selection state, callback names, and style colors.
- Repaired widget behaviors: generated `Button`, `CheckBox`, `RadioButton`, `Slider`, `ScrollBar`, and `TextBox` now update runtime state and dispatch sender-aware callbacks; `ColorPicker` now emits a minimal current-value callback path.
- Project naming fields added and wired through the editor and serialization: `projectName`, `executableName`, `userSubclassName`, and `windowTitle`.
- Generated naming behavior now uses sanitized `projectName` for `project(...)`, `executableName` for `add_executable(...)` and the produced `.exe`, `userSubclassName` for user-layer filenames, and `windowTitle` for the generated runtime title.
- Documentation updated: `docs/code_generation.md`, `docs/project_file_format.md`, and `docs/widget_catalog.md` now describe generated runtime interaction and generated project naming.
- Validation: the main `VisiForm` workspace build now succeeds with `build-static-debug`.

Remaining TODOs:

- Perform the manual generated-project export, Debug/Release build, and runtime interaction verification steps in Visual Studio.
- Verify generated `onLoad` and any desired `onClose` behavior in exported projects if lifecycle callbacks need deeper coverage beyond this repair phase.
