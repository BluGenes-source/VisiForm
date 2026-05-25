# Phase 63 property editor controls and button events plan

## Phase title
Phase 63 property editor controls and button events

## Current problem
The `Resource Manager` and `Property Inspector` need stronger reusable editing controls before continuing more resource work. Text editing behavior is still split between one-off inspector state and a single `visage::TextEditor`, dropdown behavior is duplicated between property rows, callback suggestions, and modal field choices, the `Resource Manager` does not yet show a scaled image preview, and `Button` widgets only expose `onClick` without the fuller event and visual state support needed for later generation work.

## Goal
Build a reusable property editor control layer for text and dropdown editing, migrate the `Property Inspector` and shared editor modal fields onto it, improve `Resource Manager` image resource display with stable ID visibility and preview feedback, add `Button` event and visual state properties, and keep save, load, validation, export, and generated project behavior working.

## Files to inspect
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/ui/editors/TextEditControl.h`
- `src/ui/editors/TextEditControl.cpp`
- `src/ui/editors/DropdownControl.h`
- `src/ui/editors/DropdownControl.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/validation/ProjectValidator.h`
- `src/validation/ProjectValidator.cpp`
- `src/generator/CodeGenerator.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `docs/resources.md`
- `docs/widget_catalog.md`
- `docs/project_file_format.md`
- `docs/code_generation.md`
- `docs/property_inspector.md`
- `docs/agent_plans/phase_63_property_editor_controls_and_button_events_plan.md`

## Diagnostic notes
- `PropertyInspector` currently stores ad-hoc edit state in `activeKey_`, `activeEditKind_`, and `editBuffer_` in `src/ui/PropertyInspector.h`.
- `PropertyInspector::beginEditing()` seeds `editBuffer_` from the current row display text and uses `activeCallbackPropertyKey_` plus `suggestions_` for callback and choice suggestions in `src/ui/PropertyInspector.cpp`.
- Text cursor position, selection range, scrolling, commit, and cancel are not tracked inside `PropertyInspector`; they currently come from the shared `visage::TextEditor propertyEditor_` owned by `MainWindow`.
- `MainWindow` currently reuses one `visage::TextEditor propertyEditor_` for both inspector editing and modal dialog field editing in `src/ui/MainWindow.h`.
- `MainWindow::beginInspectorEdit()` and `MainWindow::beginEditorModalFieldEdit()` call `propertyEditor_.setText(...)`, `selectAll()`, and `requestKeyboardFocus()` to start ad-hoc text editing.
- `MainWindow::commitInspectorEdit()` and `commitEditorModalFieldEdit()` handle commit, parse, and validation paths separately, so text editing orchestration is duplicated across inspector and modal flows.
- `MainWindow::keyPress()` and `textInput()` currently leave text entry behavior to the embedded `visage::TextEditor`; `MainWindow::receivesTextInput()` returns `false`.
- Explicit reusable text editing state such as `cursorIndex`, `selectionStart`, `selectionEnd`, `scrollX`, and committed or cancelled result flags does not yet exist in the UI layer.
- `PropertyInspector` currently has multiple one-off dropdown systems: property `choices`, dynamic image `resourceId` choices, callback suggestion popups, and editor modal choice fields in `MainWindow::editorModalFields()`.
- Current Phase 63 in-progress code already adds `src/ui/editors/TextEditControl.*` and `src/ui/editors/DropdownControl.*`, but `MainWindow` still contains legacy `propertyEditor_` references in a few commit and modal cleanup paths.
- `MainWindow::handleEditorModalMouseDown()` still uses the old click-to-cycle choice behavior for modal choice fields instead of always opening the shared dropdown control.
- `MainWindow` currently routes `keyPress()`, `textInput()`, and top-level `mouseDown()` into the reusable controls first, so the remaining migration work is mainly cleanup and modal integration rather than starting from scratch.
- `PropertyInspector::hitTestSuggestion()` is now stubbed because callback choice rows are being migrated onto the shared dropdown control, so the old one-click suggestion guard in `MainWindow::mouseUp()` should also be removed.
- `PropertyInspector.cpp` still needs a forward declaration or reordered helper for `collectMatchingHandlers(...)` because `callbackChoices(...)` now calls it before the helper definition.
- The `Resource Manager` already shows a choice row that stores `selectedResourceId` and displays a combined label, but it only exposes read-only text rows for type, display name, source path, and export path.
- The `Resource Manager` currently shows resource IDs in the combined label text, but it does not provide a dedicated resource ID field or an image preview area.
- `WidgetRegistry::makeButtonDefinition()` currently exposes only `text` and `hint` properties plus a single `onClick` event.
- `DesignerCanvas` currently draws `Button` with a single fill and text state and does not preview pressed or toggle visuals.
- `ProjectValidator` already validates callback names by signature kind and image resource references, so new button events and resource editor changes should extend existing validation paths rather than bypass them.
- `VisageCppEmitter` already generates `void_event` handler declarations and runtime event emitters, which should be extended carefully for `Button` release and double-click support.

## TODO checklist
- [x] Create the phase 63 plan before code changes.
- [x] Inspect the current text editing, dropdown, resource manager, button, validation, and generation infrastructure.
- [x] Document the current text editing flow, cursor or selection behavior, duplicated ad-hoc fields, and dropdown systems in this plan.
- [x] Add reusable `TextEditControl` state and behavior in the UI layer.
- [x] Add reusable `DropdownControl` state and behavior in the UI layer.
- [x] Migrate `PropertyInspector` text editing to the shared text editor behavior.
- [x] Migrate `PropertyInspector` enum, resource, look-and-feel, and callback dropdowns to the shared dropdown behavior.
- [x] Migrate shared modal dialog field editing to the reusable editor controls where applicable.
- [x] Improve property hover hints and readable property labels for edited fields.
- [x] Improve `Resource Manager` resource ID display and image preview feedback.
- [x] Keep image resource binding stored as the stable resource ID while showing readable labels.
- [x] Add `Button` event properties for `onClick`, `onRelease`, and `onDoubleClick`.
- [x] Add simple `Button` visual state properties for normal versus pressed or toggle state.
- [x] Update designer preview for `Button` visual states where practical.
- [x] Keep save and load behavior working for the new button properties and events.
- [x] Extend validation for new button callback properties and preserve existing validation paths.
- [x] Extend code generation and runtime emission for the new button properties and callbacks.
- [ ] Update `docs/resources.md`.
- [x] Update `docs/widget_catalog.md`.
- [x] Update `docs/project_file_format.md`.
- [x] Update `docs/code_generation.md`.
- [ ] Update `docs/property_inspector.md`.
- [x] Build the main `VisiForm` app with the `build-static-debug` workflow.
- [x] Fix compile errors introduced by this phase.
- [x] Confirm the main `VisiForm` app builds successfully.
- [x] Update this plan with the final result summary, build result, and remaining TODOs.

## Build validation checklist
- [x] Build only the main `VisiForm` project.
- [x] Use the `build-static-debug` workflow.
- [x] Fix compile errors introduced by this phase.
- [x] Confirm the build completes successfully.
- [x] Record the successful build result in this plan.
- [x] Do not run `VisiForm.exe`.
- [x] Do not launch generated apps.

Build result:

- `build-static-debug` was run for the main `VisiForm` target.
- The Phase 63 `MainWindow.cpp` compile error caused by `std::optional::transform(...)` was fixed.
- A plain shell build exposed an x86-versus-x64 linker environment mismatch, so the build was rerun from `VsDevCmd.bat -arch=x64 -host_arch=x64`.
- The corrected x64 developer environment successfully linked `VisiForm.exe` with `cmake --build --preset build-static-debug --target VisiForm`.

## Manual test checklist
- [ ] Open `Project > Resources` and confirm the `Resource Manager` dialog opens.
- [ ] Select an image resource and confirm the manager shows a clear stable resource ID value.
- [ ] Select an image resource and confirm the manager shows a scaled preview or a clear placeholder state.
- [ ] Add an image resource and confirm the list still shows readable labels while preserving the stable `image_N` id.
- [ ] Select an `Image` widget and confirm the `Resource` property uses the shared dropdown behavior and stores the stable resource ID.
- [ ] Confirm `ScrollBar.orientation`, `Scale Mode`, `Look and Feel`, and callback choice fields use the shared dropdown behavior.
- [ ] Confirm string, numeric, callback, path, color, and resource fallback text fields use the shared text edit behavior.
- [ ] Confirm text fields support click focus, typing, Backspace, Delete, arrow keys, Enter commit, and Escape cancel.
- [ ] Confirm property hover hints still appear in only one status pane.
- [ ] Select a `Button` widget and confirm `On Click`, `On Release`, and `On Double Click` properties appear.
- [ ] Confirm button callback suggestions still offer compatible handlers.
- [ ] Confirm `toggleMode`, `checked`, `normalText`, `pressedText`, `normalFillColor`, and `pressedFillColor` are available for `Button`.
- [ ] Confirm the designer preview reflects button pressed or toggled visuals where applicable.
- [ ] Save and reload a project and confirm the new button properties and events persist.
- [ ] Validate a project and confirm invalid callback names or signatures for button events are reported.
- [ ] Export a project and confirm generated button callback stubs include the new event hooks.
- [ ] Confirm generated projects still build Debug and Release manually after export.
- [ ] Confirm existing save, load, export, validation, menu, wizard, project settings, and local `Visage` dependency flows still work.

## Final result summary
- Phase 63 now uses the shared text and dropdown editor controls in the inspector and editor modal flows, keeps Resource Manager stable resource-id display and image preview feedback in place, and extends `Button` with `onRelease`, `onDoubleClick`, toggle-mode visual state properties, designer preview support, and generated runtime support.
- The phase also fixed the `MainWindow.cpp` C++20 compatibility issue caused by `std::optional::transform(...)` and validated the main `VisiForm` target successfully with the required `build-static-debug` workflow when run from the proper x64 Visual Studio developer environment.

## Remaining TODOs
- Update `docs/resources.md` if additional Resource Manager behavior needs to be documented for this phase.
- Update `docs/property_inspector.md` with the shared editor-control behavior if Phase 63 closes in this pass.
- Complete the manual verification checklist for shared editor controls, Resource Manager flows, button events, and export behavior.
