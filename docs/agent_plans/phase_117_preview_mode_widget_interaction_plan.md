# Phase 117 Preview Mode Widget Interaction Plan

## Scope

- Add editor-only Preview Mode interaction for supported controls without mutating `WidgetNode` or persisted project data.
- Use temporary preview values over stored model values and discard them when Preview Mode ends.
- Cover Button, Text Box, Check Box, Radio Button, Combo Box, List Box, Slider, Scroll Bar, and Tab Control interaction where the current editor rendering supports it.
- Preserve Preview Mode as non-event-running and non-persistent.
- Update authoritative version declarations from `1.0.22` to `1.0.23`.

## Requirements

- Use Phase 117 and version `1.0.23`.
- Use no subagents.
- Do not execute generated event handlers.
- Do not launch `VisiForm.exe` or generated applications from an automated agent.
- Do not add event-handler execution, script simulation, data binding, modal-dialog execution, persistent preview state, animations, accessibility overhaul, or new widget types.

## Version Notes

- Previous Phase 116 version: `1.0.22`.
- Phase 117 version: `1.0.23`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, `docs/versioning.md`, and the current-version line in `README.md`.

## Architecture Decisions

- Keep Preview Mode state inside the editor UI layer, owned by `DesignerCanvas`, so model, serialization, validation, and generator layers remain untouched.
- Reuse existing Phase 115-116 text layout/rendering concepts and the existing `TextEditControl` for preview Text Box text editing rather than introducing another editor widget.
- Route Preview Mode mouse and keyboard input before design selection, drag, resize, creation, and layout commands.
- Treat disabled and hidden widgets as non-interactive; treat read-only Text Boxes as focusable/selectable but not editable when practical.
- Keep event handlers inert in editor Preview Mode; interaction only updates temporary visuals and values.
- Confirmed shared failure: Preview Mode had visual state and some widget-specific rendering, but the canvas mouse path stopped at selection/press bookkeeping for several controls. It did not dispatch Combo Box clicks into the existing dropdown popup control, and Table/Grid did not update any temporary selection state.
- Preview input policy: canvas hit testing stays screen-based at the edge, converts through the existing zoom/pan-aware widget bounds helpers exactly once, then routes to editor-only temporary state owned by `DesignerCanvas` or the shared `DropdownControl`.
- Combo Box popup ownership is in `MainWindow` through the existing `DropdownControl`, keyed with a Preview-only prefix and writing selection back to `DesignerCanvas` temporary selected-index state.
- Table/Grid Preview selection mirrors the current runtime model: one temporary selected row/column pair. The current model has no separate cell/row/column selection-mode property, so Phase 117 does not invent one.

## TODO Checklist

- [x] Inspect Git state, project status, Phase 116 plan, and Phase 117 instructions.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.23`.
- [x] Inspect targeted Preview Mode, widget input, temporary preview state, Text Box editing, and supported widget rendering paths.
- [x] Extend Preview Mode state for temporary values and safe cleanup.
- [x] Route Preview Mode mouse interaction for supported widgets.
- [x] Route Preview Mode keyboard and text input to the focused preview widget.
- [x] Support preview Text Box editing with temporary text.
- [x] Fix the follow-up Preview Text Box keyboard-focus handoff so the shared editor requests native focus after it is mounted.
- [x] Preserve Preview Text Box caret, selection, preferred horizontal caret position, and scroll offsets for the active Preview session.
- [x] Support Combo Box Preview dropdown selection through existing popup/list infrastructure.
- [x] Support Table/Grid Preview row/column selection through temporary canvas state.
- [x] Preserve model, undo, dirty, autosave, serialization, and generated-event integrity.
- [x] Run focused static validation.
- [x] Update final project status and this plan.
- [ ] Run approved validation once, if an exact approved path is available.

## Validation Plan

- Run focused static checks such as `git diff --check`.
- Run focused tests if an exact relevant existing test command is available.
- Run the normal Windows Debug build only through an exact developer-authorized command or approved Visual Studio workspace pipeline.
- Manual runtime validation remains developer-run unless explicitly performed outside automation: version display, Preview Mode entry/exit, Text Box editing, temporary state reset, dirty flag, save/reload/autosave cleanliness, supported widget interactions, disabled/read-only behavior, Tab focus traversal, blocked design editing, zoom, and pan.

## Compatibility Considerations

- `.vfb.json` schema remains unchanged.
- Temporary Preview Mode values are not serialized, copied into `WidgetNode`, emitted into generated projects, or stored in undo history.
- Existing Design Mode behavior and generated runtime behavior remain unchanged except where shared helpers require a targeted fix.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains unrelated session-instruction archival changes plus the active Phase 117 instruction file; they are preserved.
- Focused static validation: `git diff --check` passed; output contained only existing LF-to-CRLF working-copy normalization warnings.
- Follow-up focused static validation: `git diff --check` passed; output contained only existing LF-to-CRLF working-copy normalization warnings.
- Preview Text Box overlay fix focused static validation: `git diff --check` passed; output contained only existing LF-to-CRLF working-copy normalization warnings.
- Focused tests: not run; no narrow existing unit target was identified for editor Preview Mode widget interaction, and no exact approved test command was supplied.
- Windows Debug build: not run because repository instructions prohibit terminal build commands unless the developer asks for that exact command, and no unambiguous Visual Studio workspace pipeline was available to the agent.
- Manual runtime validation: not performed.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_117_preview_mode_widget_interaction_plan.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `src/ui/editors/TextEditControl.cpp`
- `src/ui/editors/TextEditControl.h`

## Follow-up Text Box Bug Fix Pass

- Traced input chain: Preview Mode click reaches `DesignerCanvas::beginPreviewInteraction()`, hit testing assigns the Text Box ID to `previewFocusedWidgetId_`, and `MainWindow::beginPreviewTextBoxEdit()` mounts the shared `TextEditControl`. The first static failure point was keyboard-focus ownership timing: `MainWindow::mouseDown()` requested native keyboard focus before the Preview Text Box editor existed, unlike working modal text editing paths that request focus after editor activation.
- Keyboard-focus fix: `beginPreviewTextBoxEdit()` now calls `requestKeyboardFocus()` after `TextEditControl::begin()`, state restore, click caret placement, temporary text sync, and caret timer update.
- Text-input callback used: printable characters continue through `MainWindow::textInput()` into `TextEditControl::textInput()`, then sync back to `DesignerCanvas::setPreviewText()`. Navigation and editing keys continue through `MainWindow::keyPress()` and `TextEditControl::keyPress()`.
- Shared editor logic reused: Preview Text Boxes still use the existing `TextEditControl` for insertion, Backspace, Delete, caret movement, selection, Enter/newline, wrapping, and scrolling. The follow-up added `TextEditControl::State` snapshot/restore rather than a separate Preview editing engine.
- Temporary state reset finding: Preview text itself was not being overwritten from the model on repaint; `DesignerCanvas::previewText()` falls back to model text only until a Preview text value exists. However, caret, selection, preferred horizontal caret position, and scroll offsets were previously owned only by the transient `TextEditControl`, so they were lost whenever Preview editing was cleared and later remounted in the same Preview session.
- Temporary state fix: `MainWindow` now keeps editor-only `previewTextEditStates_` snapshots keyed by widget ID for the active Preview session. The map is cleared when Preview Mode changes, so re-entering Preview initializes from the stored model text again.
- Files changed in this follow-up pass: `src/ui/MainWindow.cpp`, `src/ui/MainWindow.h`, `src/ui/editors/TextEditControl.cpp`, and `src/ui/editors/TextEditControl.h`.
- Build result: not run; the repository safety instructions require an exact developer-authorized build command or an unambiguous Visual Studio workspace pipeline.
- Manual validation actually performed: none; runtime validation remains developer-run because automated agents must not launch `VisiForm.exe`.
- Superseded note: this pass did not expand Text Box clipboard operations; the later overlay fix pass below adds shared `TextEditControl` Ctrl+C, Ctrl+X, and Ctrl+V support through Visage clipboard APIs.

## Follow-up Preview Text Box Overlay Fix Pass

- Confirmed architectural cause: Preview Text Boxes are canvas-rendered widget representations, not native child text controls. Updating `previewFocusedWidgetId_` only changes editor-owned visual state; printable text is delivered by Visage only when `MainWindow::receivesTextInput()` reports that the shared `TextEditControl` is focused.
- Confirmed input path: the canvas/main window can receive printable text through `MainWindow::textInput()`, but only after the reusable `TextEditControl` is mounted, focused, and `requestKeyboardFocus()` has been called. A manually painted caret or Preview focus ID does not prove text delivery.
- Overlay design update: `MainWindow` now binds the reusable Preview Text Box editor by stable widget ID, initializes it from temporary Preview text, restores the active-session editor state, and can place the caret from the click point or mount it from Tab focus without relying on raw widget pointers.
- Geometry synchronization: active Preview Text Box editor bounds are refreshed on resize, DPI changes, draw/layout updates, zoom, pan, and Project Tree / Property Inspector splitter drags. Invalid, deleted, disabled, or non-Text Box targets hide the overlay.
- Rendering update: `DesignerCanvas` tracks the active Preview Text Box overlay ID and skips only that Text Box's canvas text while the editor is active, preserving the runtime border and styling while avoiding stacked duplicate glyphs.
- Focus navigation update: Tab / Shift+Tab commits the outgoing overlay value to temporary Preview state, advances Preview focus, and mounts the same reusable editor when the incoming focus target is a Text Box.
- Shared text editor update: `TextEditControl` now handles Ctrl+C, Ctrl+X, and Ctrl+V through Visage clipboard APIs in addition to existing Ctrl+A, caret, selection, Backspace/Delete, Enter, multiline, word-wrap, mouse placement, and scrolling behavior. Read-only fields allow copy/select-all and block mutation.
- Model integrity: Preview edits continue to call only `DesignerCanvas::setPreviewText()` and `previewTextEditStates_`; they do not write `WidgetNode`, dirty the document, create undo entries, serialize, autosave, or affect generated output.
- Files changed in this pass: `src/ui/DesignerCanvas.cpp`, `src/ui/DesignerCanvas.h`, `src/ui/MainWindow.cpp`, `src/ui/MainWindow.h`, and `src/ui/editors/TextEditControl.cpp`.
- Focused static validation: `git diff --check` passed; output contained only existing LF-to-CRLF working-copy normalization warnings.
- Build result: not run; repository safety instructions require an exact developer-authorized build command or an unambiguous Visual Studio workspace pipeline.
- Manual validation actually performed: none; runtime validation remains developer-run because automated agents must not launch `VisiForm.exe`.

## Final Result Summary

- Updated the phase version from `1.0.22` to `1.0.23`.
- Extended the editor-only `DesignerCanvas` Preview Mode state with temporary Text Box text, numeric values for Slider and Scroll Bar, selected indexes, active tabs, focus, hover, press, checked, and selected state.
- Preview Mode now routes mouse input to supported widgets instead of design selection, drag, resize, or creation.
- Text Box clicks in Preview Mode mount the existing `TextEditControl` over the rendered Text Box, reuse multiline/word-wrap layout behavior, request native keyboard focus after editor activation, sync edits into temporary preview text, preserve editor caret/selection/scroll state for the active Preview session, and discard the value on Preview exit.
- Read-only Preview Text Boxes allow focus/caret/selection behavior while blocking Backspace, Delete, Return mutation, and text input.
- Check Box and Radio Button clicks update temporary checked/selected state; Radio Button grouping follows the existing group property behavior.
- List Box clicks select the clicked visible row.
- Combo Box clicks or keyboard activation open the existing `DropdownControl` popup above the canvas; the popup is clipped to available canvas viewport space, supports scrolling through the existing dropdown behavior, and writes only a temporary selected index when an item is chosen.
- Table/Grid clicks identify the clicked visible cell and update a temporary selected row/column pair used by the existing renderer. The current widget model exposes `selectedRow` and `selectedColumn` only, so the result is current-runtime cell selection rather than a new row/column selection-mode feature.
- Slider and Scroll Bar clicks/drags update temporary values from the pointer while respecting current minimum, maximum, and step metadata.
- Tab Control clicks update the temporary active tab.
- Tab and Shift+Tab move Preview focus through supported controls. Space and Return activate focused toggles, item controls, and tab controls where supported.
- Preview Mode state is cleared on mode changes and remains outside the project model, undo stack, dirty state, serialization, autosave/recovery, generated event handlers, and generated output.

## Remaining TODOs

- Run the normal Windows Debug build through the approved Visual Studio workspace pipeline or an exact developer-authorized command.
- Manually validate the Phase 117 checklist from the session instructions, including version display, Text Box editing, dirty flag preservation, temporary state reset, supported widget interaction, disabled/read-only behavior, Tab traversal, blocked design editing, zoom/pan, save/reload, autosave/recovery cleanliness, and generated-event non-execution.
- Future work: if Table/Grid later gains explicit row-only or column-only selection modes, wire Preview dispatch to those model properties; Phase 117 preserves the existing selected row/column behavior.
