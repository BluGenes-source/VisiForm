# Phase 116 Multiline Text Box Editing Plan

## Scope

- Extend the existing Text Box widget so the `Multiline` property enables practical multiline editing.
- Preserve current single-line Text Box behavior when `Multiline` is false.
- Reuse the Phase 115 shared text-layout system for multiline drawing, caret placement, hit testing, selection, wrapping, and scrolling.
- Keep editor scroll offsets, caret, and selection as runtime/editor-only state.
- Update generated Text Box output for multiline text, wrapping, and scrolling parity where practical.
- Update authoritative version declarations from `1.0.21` to `1.0.22`.

## Requirements

- Use Phase 116 and version `1.0.22`.
- Use no subagents.
- Do not create a new Text Editor widget.
- Preserve `.vfb.json` compatibility; Text remains a normal string and layout properties remain sparse overrides.
- Do not launch `VisiForm.exe` or generated applications from an automated agent.

## Version Notes

- Previous Phase 115 version: `1.0.21`.
- Phase 116 version: `1.0.22`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, `docs/versioning.md`, and the current-version line in `README.md`.

## Architecture Decisions

- Keep `TextEditControl` as the shared focused text editor control and extend it rather than introducing a second editing path.
- Add layout-backed visual line metadata inside `TextEditControl` so text drawing, caret placement, hit testing, selection rectangles, and line navigation share Phase 115 layout behavior.
- Treat Design Mode text edits as the model-mutating path through the existing undoable property-edit flow.
- Treat Preview Mode Text Box interaction as temporary preview/runtime state only if safely supported; otherwise document the limitation.
- Keep scroll offsets outside the model and reset them when editing starts, selection/project context changes, or the editor clears.

## TODO Checklist

- [x] Inspect Git state, project status, Phase 115 plan, and Phase 116 instructions.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.22`.
- [x] Inspect targeted Text Box model/properties, shared layout, editing, scrolling, Preview Mode, serialization, generator, and tests.
- [x] Extend `TextEditControl` for multiline editing, navigation, selection, wrapping, and scrolling.
- [x] Wire Design Mode Text Box editing to multiline and word-wrap layout state without regressing single-line behavior.
- [x] Document Preview Mode multiline Text Box interaction policy.
- [x] Update generated Text Box output for multiline text newlines and existing wrapping render behavior where practical.
- [x] Add focused static validation coverage.
- [x] Update final project status and this plan.
- [ ] Run approved validation once, if an exact approved path is available.

## Validation Plan

- Run focused static checks such as `git diff --check`.
- Run focused tests if an exact approved test command is available.
- Run the normal Windows Debug build only through an exact developer-authorized command or approved Visual Studio workspace pipeline.
- Manual runtime validation remains developer-run unless explicitly performed outside automation: version display, single-line editing, multiline input, navigation, selection, copy/cut/paste, wrapping, scrolling, resize/repaint, Delete behavior, undo/redo, save/reload, Preview Mode policy, export, generated output, and autosave/recovery preservation.

## Compatibility Considerations

- Old single-line projects continue loading unchanged.
- `Multiline` and `Word Wrap` remain sparse appearance overrides.
- Single-line Text Box rendering sanitizes stored newline content by using the first logical line.
- Scroll position, caret, and selection remain non-serialized editor/runtime state.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains unrelated session-instruction archival changes; they are preserved.
- Focused static validation: `git diff --check` passed; output contained only existing LF-to-CRLF working-copy normalization warnings.
- Bug-fix pass static validation: `git diff --check` passed after the Text Box behavior row fix; output contained only existing LF-to-CRLF working-copy normalization warnings.
- Multiline Boolean commit static validation: `git diff --check` passed after the Boolean click routing fix; output contained only existing LF-to-CRLF working-copy normalization warnings.
- Focused tests: not run; no narrow existing unit target was identified for `TextEditControl`, and no exact approved test command was supplied.
- Windows Debug build: not run because repository instructions prohibit terminal build commands unless the developer asks for the exact command, and no exact build command was supplied for this bug-fix pass.
- Manual runtime validation: not performed.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_116_multiline_text_box_editing_plan.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.cpp`
- `src/ui/editors/TextEditControl.cpp`
- `src/ui/editors/TextEditControl.h`

## Bug-Fix Pass: Text Box Multiline / Word Wrap Inspector Rows

- Root cause: `Multiline` and `Word Wrap` were emitted from the Appearance override loop with `__appearance_` keys, labels such as `(Inh)`, inheritance hints, and the Appearance override commit path. Property rows are rebuilt from fresh `PropertyRow` values, so the observed inherited state was confirmed as incorrect row construction and routing rather than stale reused row/editor state.
- Row/editor reset finding: the inspected row construction path uses value-initialized `PropertyRow` instances, and direct Text Box behavior rows now use plain `multiline` / `wordWrap` keys, direct labels, direct hints, Bool edit kind, no dropdown choices, no reset action, no state-specific styling, and no Appearance prefix. This prevents Appearance metadata from leaking into the normal Text Box behavior rows.
- Property-scope distinction: Text Box `Multiline` and `Word Wrap` are now skipped in the Appearance override row list and are exposed under a focused `Text Box Behavior` section. Existing Appearance and Typography rows, including real inherited overrides, keep their existing `__appearance_` routing and reset behavior.
- Commit behavior: direct `multiline` and `wordWrap` commits update the selected Text Box's existing sparse text-layout storage through an undoable document command, repaint the editor, and avoid dirtying the project on no-op value selections. `Word Wrap` is only exposed when `Multiline` is active or an existing word-wrap value needs to be cleared; disabling `Multiline` clears the direct word-wrap setting.
- Nearby-property check: `Overflow Mode`, horizontal alignment, vertical alignment, and text padding remain in the Appearance/Typography override path by design for this phase; they were not changed in this narrow pass.
- Manual validation: not performed by the agent. The manual checklist in the bug-fix prompt remains developer-run.

## Bug-Fix Pass: Boolean Commit Routing

- Failed-commit root cause: the Property Inspector correctly rebuilt `Multiline` and `Word Wrap` as direct `PropertyEditKind::Bool` rows, but the mouse click handler treated every Boolean row as a normal widget property. It read `document_.selectedWidget()->getBoolProperty(row->key, false)` and called `setSelectedWidgetProperty(row->key, !currentValue)`. Text Box behavior values are not stored in the generic widget property map; they are sparse text-layout overrides in `WidgetLookAndFeelOverrides`. The click therefore wrote a separate generic `multiline` property while the refreshed row continued reading `appearanceOverrides.multiline`, so the visible value stayed `false`.
- Boolean editor fix: Boolean rows now invert the row's displayed Boolean value and route the result through `setSelectedWidgetPropertyFromString()`. This preserves the existing inspector Boolean interaction while ensuring special Boolean rows, including direct Text Box behavior and Appearance preview rows, use their correct parsing and commit callbacks instead of the generic model-property setter.
- Parsing and model-update path: `setSelectedWidgetPropertyFromString()` parses `true` / `false` through the shared Boolean parser, handles direct Text Box `multiline` / `wordWrap`, stores the value in `appearanceOverrides`, applies the change through `applyUndoableDocumentChange()`, refreshes inspector bounds, and repaints. No-op selections return without creating a new undo command.
- Status hint correction: successful direct Text Box behavior commits now report the actual value, for example `Multiline set to true.` or `Word Wrap set to false.`, instead of a generic changed/edited message.
- Word Wrap path: `Word Wrap` uses the same Boolean routing and direct override storage as `Multiline`; it remains guarded so it cannot be enabled while `Multiline` is false.
- Validation: focused static validation passed with `git diff --check`; the Windows Debug build and manual runtime checklist remain developer-run because no exact approved build command or automated launch permission was supplied.

## Final Result Summary

- Updated the phase version from `1.0.21` to `1.0.22`.
- Extended `TextEditControl` with opt-in multiline and word-wrap state, mouse drag selection, mouse-wheel vertical scrolling, Shift-based selection extension, `Ctrl+A`, `Ctrl+Home`, `Ctrl+End`, Enter newline insertion in multiline mode, measured-font Up/Down preferred-X navigation, per-line multiline selection drawing, and caret-follow scrolling.
- Routed active text editor mouse drag, mouse-up, and wheel events before canvas interactions so text editing does not trigger widget drag/resize while active.
- Wired Text Box `Text` property editing to the selected Text Box resolved `Multiline` and `Word Wrap` style.
- Preserved raw newline-containing Text Box `text` values only when the selected Text Box resolves to `Multiline=true`; other string properties keep the existing trimmed behavior.
- Updated generated runtime Text Box input so Enter appends a newline in multiline mode and pasted newline characters are preserved only for multiline Text Boxes.
- Preview Mode remains temporary runtime interaction only in generated output; the editor Preview Mode renders multiline Text Boxes but does not persist preview text edits into the project model.
- Fixed the bug-fix pass issue where Text Box `Multiline` and `Word Wrap` were shown and committed as inherited Appearance overrides instead of direct Text Box behavior rows.
- Fixed the follow-up failed-commit issue where Boolean row clicks bypassed the Text Box behavior commit callback and wrote `multiline` / `wordWrap` into the generic widget property map instead of sparse text-layout overrides.

## Remaining TODOs

- Run the normal Windows Debug build through the approved Visual Studio workspace pipeline or an exact developer-authorized command.
- Manually validate the Phase 116 checklist, including version display, single-line editing, multiline entry, navigation, selection, copy/cut/paste behavior, wrapping, vertical/horizontal scrolling, resize/repaint, Delete behavior, undo/redo, save/reload, Preview Mode rendering policy, export, generated output, and autosave/recovery preservation.
- Manually validate the bug-fix pass checklist: Text Box `Multiline` label/hint/editor, direct commit/no-op behavior, undo/redo, repaint, `Word Wrap` visibility and hint, selection/tab/scroll refresh stability, real Appearance inheritance rows, save/reload persistence, and generated Text Box properties.
- Add dedicated automated coverage if a suitable text-editor test harness is introduced.
