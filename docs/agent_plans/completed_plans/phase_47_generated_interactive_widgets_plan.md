# Phase 47 generated interactive widgets plan

## Goal

Make exported generated projects interactive so generated widgets behave like their intended control types while keeping the generated code simple, sender-aware, and extensible for future runtime improvements.

## Current problems

- Exported generated widgets are still mostly static previews.
- The generated project does not yet keep runtime widget state for interactive controls.
- Hit testing, mouse dispatch, keyboard input, and callback dispatch are not yet generated for real widget behavior.
- Documentation does not yet describe generated interactive widget behavior and current limitations.

## Files to inspect

- `src/generator/VisageCppEmitter.cpp`
- `src/generator/VisageCppEmitter.h`
- `src/generator/CodeGenerator.cpp`
- `src/generator/CodeGenerator.h`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetNode.h`
- `src/model/ProjectDocument.h`
- `docs/code_generation.md`
- `docs/widget_registry.md`
- `docs/widget_catalog.md`
- `docs/agent_plans/phase_47_generated_interactive_widgets_plan.md`
- `Generated/ExportedVisageProject/src/MainWindow.h`
- `Generated/ExportedVisageProject/src/MainWindow.cpp`

## Step-by-step TODO list with checkboxes

- [x] Create persistent phase plan file before code changes
- [x] Inspect current Visage input, focus, and redraw usage in the main app and generated project
- [x] Design generated runtime widget state structs and containers
- [x] Add generated hit testing and active/focused widget tracking
- [x] Add generated Button interaction
- [x] Add generated CheckBox interaction
- [x] Add generated RadioButton group behavior
- [x] Add generated Slider dragging behavior
- [x] Add generated ScrollBar interaction
- [x] Add generated TextBox focus and typing behavior
- [x] Keep ProgressBar and StatusBar display state driven by runtime data
- [x] Preserve sender-aware callback behavior and shared callback support
- [x] Preserve conflict detection and USER CODE regions
- [x] Update interactive widget documentation
- [x] Build with `build-static-debug`
- [x] Write final result summary

## Current progress notes

- Phase plan file created before edits.
- Confirmed generated interaction should use the same Visage window methods already used in the main app: `mouseDown`, `mouseMove`, `mouseDrag`, `mouseUp`, `keyPress`, `receivesTextInput`, `textInput`, `requestKeyboardFocus`, and `redraw`.
- Reworked generated `MainWindow.h` output to declare a small runtime widget model, interaction overrides, active/focused widget tracking, and runtime state helpers.
- Reworked generated `MainWindow.cpp` output to initialize runtime widget state directly from the exported document, draw from runtime state, hit test in form-local coordinates, and dispatch sender-aware callbacks.
- Added generated interaction for `Button`, `CheckBox`, `RadioButton`, `Slider`, `ScrollBar`, and `TextBox`.
- Kept `ProgressBar` and `StatusBar` display-only while moving them to runtime-state-driven drawing.
- Preserved sender-aware callbacks, shared callback generation, conflict detection, and USER CODE preservation behavior.
- Updated documentation for the new generated interactive runtime behavior and current limitations.

## Build validation checklist

- [x] Build the main `VisiForm` project with `build-static-debug`
- [x] Fix any compile errors introduced by this phase
- [x] Do not run `VisiForm.exe`
- [x] Do not launch the generated app

## Manual test checklist

- [ ] Export a project and verify generated runtime widget data is emitted into C++
- [ ] Verify generated Button click fires `onClick`
- [ ] Verify generated CheckBox toggles and fires `onToggle`
- [ ] Verify generated RadioButton group behavior works and fires `onSelected`
- [ ] Verify generated Slider drag updates value and fires `onChanged`
- [ ] Verify generated ScrollBar arrows, track, and thumb update value and fire `onChanged`
- [ ] Verify generated TextBox accepts basic text and fires `onTextChanged`
- [ ] Verify generated ProgressBar shows readable runtime value text
- [ ] Verify generated StatusBar shows its runtime fields
- [ ] Verify shared callbacks receive distinct `WidgetEvent` sender metadata
- [ ] Verify USER CODE preservation still works after re-export
- [ ] Build the exported project manually in Debug and Release

## Final result summary

Completed.

- Generated exports now emit a simple runtime widget model, runtime drawing, hit testing, mouse dispatch, basic text input, and sender-aware callback dispatch.
- `Button`, `CheckBox`, `RadioButton`, `Slider`, `ScrollBar`, and `TextBox` now have generated interactive behavior in exported projects.
- `ProgressBar` and `StatusBar` remain display-only but now render from generated runtime state.
- Shared callbacks and conflict detection remain intact, and USER CODE preservation behavior was kept unchanged.

Remaining TODOs:

- Manual export verification is still needed to confirm generated Debug and Release builds, runtime interaction behavior, shared sender-aware callback behavior, and re-export preservation in Visual Studio.
