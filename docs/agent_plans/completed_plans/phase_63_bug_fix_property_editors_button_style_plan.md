## Phase title
Phase 63 Bug Fix - Property Editors and Button Style Repair

## Current bugs
- Text property editing clips text at the bottom of the edit box.
- `borderThickness` and `cornerRadius` still use fragile text editing instead of a slider editor.
- The intended slider range for `borderThickness` and `cornerRadius` is 1 through 25.
- The property editor UI should treat slider value `25` as `100%` when percent display is shown.
- Newly added `Button` widgets do not show usable text immediately.
- `Button` preview currently depends too heavily on `normalText` instead of a stable fallback order.
- Selecting or editing `cornerRadius` can throw.
- Shared `TextEditControl` and `DropdownControl` behavior must stay stable.
- Save, load, export, and validation behavior must continue working.

## Files to inspect
- `src/ui/editors/TextEditControl.h`
- `src/ui/editors/TextEditControl.cpp`
- `src/ui/editors/DropdownControl.h`
- `src/ui/editors/DropdownControl.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/WidgetMetrics.h`
- `src/ui/WidgetMetrics.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetDefinition.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetNode.h`
- `src/model/ProjectDocument.h`
- `src/validation/ProjectValidator.h`
- `src/validation/ProjectValidator.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `docs/widget_catalog.md`
- `docs/property_inspector.md`
- `docs/code_generation.md`
- `docs/agent_plans/phase_63_bug_fix_property_editors_button_style_plan.md`

## Diagnosis notes
- `TextEditControl` currently draws text with `visage::Font::kTopLeft`, so the y value passed to `canvas.text(...)` is a top-left coordinate rather than a text baseline.
- `TextEditControl::draw()` currently uses `baselineY = bounds_.y + 5.0f`, but that value is really the text top position. With the current clipped inner height and `bounds_.height - 8.0f`, glyph descenders can sit too close to the lower clip edge.
- `PropertyInspector` currently uses `kRowHeight = 30.0f` and `activeEditorBounds()` returns `kRowHeight - 6.0f`, so the shared text edit box height is only `24.0f`.
- `TextEditControl` currently uses horizontal padding only. It has no dedicated vertical text padding helper and no explicit centered single-line text placement helper.
- `borderThickness` and `cornerRadius` still come from `commonStyleProperties()` with `PropertyValue{}` defaults, so they display as `<unset>` through `PropertyValue::toDisplayString()`.
- `PropertyInspector` still maps those properties to plain float editing because no slider edit kind or slider metadata exists yet in `WidgetDefinition` or `PropertyInspector`.
- `MainWindow::setSelectedWidgetPropertyFromString()` uses local `parseFloat` and `parseInt` lambdas that call `std::stof` and `std::stoi` inside try/catch. Invalid text is caught, but first-chance exceptions still occur for bad input such as `<unset>`.
- `DesignerCanvas` resolves `borderThickness` and `cornerRadius` through `WidgetNode::getFloatProperty(...)`, so missing values do not throw during drawing, but they stay in the UI as unset text rather than a stable numeric default.
- `Button` defaults currently set `text = "Button"`, but also set `normalText = ""`, so the current preview fallback path is still wrong for empty-but-present string properties.
- `DesignerCanvas` currently reads `normalText` by calling `getStringProperty("normalText", getStringProperty("text", widgetLabel(widget)))`, but `getStringProperty(...)` returns the stored empty string when the property exists, so the fallback never reaches `text`.
- `VisageCppEmitter` currently has the same empty-string fallback flaw for generated button `normalText`, so generated output would mirror the blank-button regression unless repaired.

## TODO checklist
- [x] Create the new Phase 63 bug-fix plan before code changes.
- [x] Inspect the requested editor, widget metadata, validation, generator, and documentation files.
- [x] Document how `TextEditControl` bounds, padding, clipping, and baseline math currently work.
- [x] Document the current `cornerRadius` exception path and parsing assumptions.
- [x] Document the current `Button` default property values and preview fallback behavior.
- [x] Fix `TextEditControl` vertical clipping for single-line property editing.
- [ ] Ensure the text edit fix applies to inspector and shared modal fields without one-off handling.
- [x] Add slider editor support for `borderThickness` and `cornerRadius`.
- [x] Prefer property metadata for slider editor kind and range support.
- [x] Clamp the style sliders to `1..25` and keep `25` equivalent to `100%` in UI percent display where shown.
- [x] Add safe numeric parsing for style property editing and prevent exceptions.
- [x] Ensure unset, empty, invalid, and `<unset>` style values do not throw.
- [x] Keep `DesignerCanvas` style rendering safe for unset or invalid `cornerRadius` values.
- [x] Fix `Button` default text and fallback order in the editor preview.
- [x] Fix generated `Button` text fallback behavior.
- [ ] Keep save, load, export, and validation working for the repaired properties.
- [x] Extend or confirm validation coverage for style numeric fields and new `Button` event or visual properties.
- [x] Update `docs/property_inspector.md`.
- [x] Update `docs/widget_catalog.md`.
- [x] Update `docs/code_generation.md`.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Fix compile errors introduced by this phase.
- [x] Confirm the main `VisiForm` app builds successfully.
- [ ] Update this plan with the final result summary and remaining TODOs.

## Build validation checklist
- [x] Build only the main `VisiForm` project.
- [x] Use the `build-static-debug` workflow.
- [x] Fix compile errors introduced by this phase.
- [x] Confirm the build completes successfully.
- [x] Record the successful build result in this plan.
- [x] Do not run `VisiForm.exe`.
- [x] Do not launch generated apps.

Build result:
- Repaired the `PropertyInspector.cpp` helper-scope compile break introduced during the slider-editor work.
- Built the main `VisiForm` target successfully with `cmake --build --preset build-static-debug --target VisiForm` from the proper x64 Visual Studio developer environment.

## Manual test checklist
- [ ] Edit text properties such as `name`, `text`, `hint`, callback names, color strings, and numeric fields and confirm the text is vertically centered and not clipped.
- [ ] Open `Project Settings` and confirm shared text fields remain readable and not clipped.
- [ ] Select a widget with `borderThickness` and confirm the property inspector shows a slider editor.
- [ ] Select a widget with `cornerRadius` and confirm the property inspector shows a slider editor.
- [ ] Confirm the style slider range is `1..25`.
- [ ] Confirm slider max `25` corresponds to `100%` if percent text is shown.
- [ ] Confirm selecting or editing `cornerRadius` no longer throws.
- [ ] Confirm empty, invalid, and `<unset>` style values are handled safely.
- [ ] Add a new `Button` and confirm it immediately displays `Button` on the canvas.
- [ ] Confirm a `Button` still displays text when only `text` is set.
- [ ] Confirm a `Button` displays `normalText` when `normalText` is set.
- [ ] Confirm a `Button` displays `pressedText` only in the pressed or toggle-preview case.
- [ ] Save and reload a project and confirm repaired button and style properties persist.
- [ ] Validate and export a project and confirm the repaired properties do not break validation or export.
- [ ] Confirm generated projects keep the intended button text fallback behavior.

## Final result summary
- Pending.

## Remaining TODOs
- Pending inspection and repair work.

## Repair Pass 2 - Remaining Bugs

### Current remaining bugs
- Text editing fields still cut text off at the bottom.
- `borderThickness` still shows as `<unset>`.
- `cornerRadius` still shows as `<unset>`.
- Selecting or editing `borderThickness` or `cornerRadius` can still throw `std::invalid_argument`.
- The status bar still shows `Invalid value for borderThickness` during selection or editing.
- `borderThickness` and `cornerRadius` are still not functioning as slider editors.
- A newly added `Button` still displays no text initially.
- `Button` text still appears only when `normalText` is populated.
- `Button` drawing is still not using the intended fallback text order.
- Existing generated and export behavior must not regress.

### Diagnosis notes
- The remaining `std::invalid_argument` report is consistent with first-chance exceptions thrown inside `std::stof` or `std::stoi` before local catch blocks return `false`.
- The visible `<unset>` values in the inspector are coming from `PropertyValue::toDisplayString()` for monostate style properties rather than from rendering code.
- The slider editor is not active because the current property metadata and inspector edit-kind mapping still expose style fields as plain float text edits.
- The blank new-button preview is caused by empty-but-present `normalText`, not by add-widget flow overwriting `text`.

### Exact root cause
- `borderThickness` and `cornerRadius` are still defined with monostate defaults in `commonStyleProperties()`, so the inspector displays `<unset>` and starts fragile text editing instead of stable numeric slider editing.
- The current numeric parse path in `MainWindow::setSelectedWidgetPropertyFromString()` still reaches `std::stof` or `std::stoi` for invalid user-facing text such as `<unset>`, which produces first-chance `std::invalid_argument` exceptions even though the lambdas catch them later.
- `TextEditControl` draws single-line text too low inside a 24-pixel editor box because it uses a fixed top offset instead of centered vertical placement with explicit vertical padding.
- `Button` preview and generated fallback logic incorrectly treat an existing empty `normalText` property as a valid final value, so the intended fallback to `text` or `"Button"` never happens.

### TODO checklist
- [x] Search all remaining unsafe numeric parsing paths affecting property values.
- [x] Document the actual root cause of the `std::invalid_argument` path for `borderThickness` or `cornerRadius`.
- [x] Replace or guard unsafe style numeric parsing so selection, drawing, and commit paths cannot throw.
- [x] Stop using `<unset>` as the active editable value for `borderThickness` and `cornerRadius`.
- [x] Make `borderThickness` and `cornerRadius` function as slider editors with range `1..25`.
- [ ] Ensure style slider values update safely on mouse interaction and show clear status text.
- [x] Fix global property text vertical alignment so text is not clipped.
- [x] Document the actual text baseline or top-left draw behavior used by the edit control.
- [x] Repair `Button` defaults so a new button visibly shows `Button` immediately.
- [x] Repair `Button` drawing fallback order in the editor preview.
- [x] Repair generated `Button` fallback order without regressing export behavior.
- [ ] Keep dropdown, text editing, save, load, validation, and export behavior working.
- [x] Update `docs/property_inspector.md` for Repair Pass 2 behavior.
- [x] Update `docs/widget_catalog.md` for Repair Pass 2 button behavior.
- [x] Update `docs/code_generation.md` for Repair Pass 2 generated button behavior.
- [x] Build the main `VisiForm` app successfully with `build-static-debug`.
- [x] Record the successful build result in this plan.

### Build validation checklist
- [x] Build only the main `VisiForm` project.
- [x] Use the `build-static-debug` workflow.
- [x] Fix compile errors introduced by Repair Pass 2.
- [x] Confirm the build completes successfully.
- [x] Record that the main `VisiForm` app was built successfully.
- [x] Do not run `VisiForm.exe`.
- [x] Do not launch generated apps.

Build result:
- The current Repair Pass 2 source changes compile and link successfully in the main `VisiForm` app.
- The verified command was `cmake --build --preset build-static-debug --target VisiForm` from `VsDevCmd.bat -arch=x64 -host_arch=x64`.

### Manual test checklist
- [ ] Select `borderThickness` and confirm no exception is thrown.
- [ ] Select `cornerRadius` and confirm no exception is thrown.
- [ ] Confirm `borderThickness` no longer displays `<unset>` for editable widgets.
- [ ] Confirm `cornerRadius` no longer displays `<unset>` for editable widgets.
- [ ] Confirm `borderThickness` uses a working slider editor with range `1..25`.
- [ ] Confirm `cornerRadius` uses a working slider editor with range `1..25`.
- [ ] Drag each style slider and confirm values update safely.
- [ ] Confirm invalid numeric values no longer throw and show a clear status message.
- [ ] Confirm property text editing fields no longer clip text at the bottom.
- [ ] Add a new `Button` and confirm it immediately shows `Button` on the canvas.
- [ ] Confirm button text fallback works for `text`, `normalText`, `pressedText`, and empty legacy cases.
- [ ] Confirm save, load, validation, and export still work for repaired button and style properties.

### Final result summary
- Repair Pass 2 fixed the immediate Phase 63 regressions that kept style fields at `<unset>`, threw first-chance numeric parsing exceptions, clipped shared property text at the bottom, and left new Buttons blank when `normalText` was empty.
- The code now compiles successfully, the phase plan records the required build validation, and the remaining work is manual in-editor verification of slider interaction, text alignment, and export behavior.
