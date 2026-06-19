# Phase 105 Runtime Visual Styling Baseline Plan

## Scope

- Add one restrained, reusable beveled visual baseline for Button, Text Box, Check Box, Combo Box, Slider, and the reusable shell Splitter.
- Apply the baseline to the shared Design/Preview canvas renderer and directly relevant generated runtime renderer.
- Preserve project data, editing behavior, selection overlays, resize affordances, and generated user-code regions.
- Update authoritative version declarations from `1.0.10` to `1.0.11`.

## Requirements

- Use Phase 105 and version `1.0.11`.
- Use no subagents.
- Keep the implementation to fixed application defaults; do not add a theme editor or new serialized style properties.
- Reuse existing widget/model state and generated runtime interaction state.
- Do not launch `VisiForm.exe` or generated applications from an automated agent.
- Build only through an exact command explicitly requested by the developer.

## Version Notes

- Previous Phase 104 version: `1.0.10`.
- Phase 105 version: `1.0.11`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Architecture Decisions

- Add a compact header-only `ui/VisualStyleBaseline.h` helper so Designer Canvas widgets and reusable shell Splitters share the same color derivation, edge geometry, and raised/recessed drawing behavior.
- Keep Design and Preview rendering on the existing shared `DesignerCanvas` path. Stored checked/disabled/read-only values are rendered without adding Preview-only model mutation or a duplicate interaction state machine.
- Extend the generated runtime's existing interaction state with hover and use its existing pressed/focused state for transient styling.
- Emit a small equivalent runtime style helper because generated projects are standalone and cannot include VisiForm editor source.
- Use fixed screen/rendered-pixel edge widths clamped to available geometry. Existing canvas zoom transforms the widget bounds, while edge widths remain readable and avoid half-pixel placement where practical.
- Keep selection outlines, handles, grip arcs, guides, and other editor overlays drawn after widget styling.
- Style the reusable application Splitter only; Splitter is not a serialized/exported project widget type.

## State Mapping

- Normal: raised controls use a light top/left edge and dark bottom/right edge.
- Hover: generated runtime controls receive a modest accent blend; reusable shell splitters use existing divider hit testing.
- Pressed: generated runtime Button, Check Box, Combo Box arrow area, and Slider thumb reverse edge emphasis for a recessed appearance.
- Focused: generated runtime Text Box receives an accent focus border.
- Checked: Button toggle and Check Box retain existing stored/runtime checked state with stronger accent treatment.
- Disabled/read-only: use existing properties where present; no new schema fields are introduced.
- Design/Preview: stored model states are shown; transient interaction remains runtime-only because Phase 98 intentionally made Preview visual-only.

## TODO Checklist

- [x] Inspect branch, worktree, Phase 104 plan, Phase 105 prompt, and targeted rendering/version files.
- [x] Confirm Phase 105 is unused and select version `1.0.11`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.11`.
- [x] Add shared visual-style primitives.
- [x] Update Designer/Preview rendering for the five project widget types.
- [x] Update reusable Splitter rendering and hover/drag feedback.
- [x] Update generated runtime rendering and existing interaction-state mapping.
- [x] Run focused static validation.
- [ ] Run the normal Windows Debug build once if the developer supplies or approves an exact command.
- [x] Record final validation, files changed, parity gaps, and remaining issues.

## Validation Plan

- Inspect the shared helper for centralized colors, borders, highlight/shadow edges, focus, disabled, and raised/recessed primitives.
- Inspect all six scoped renderers for readable text, compact geometry, and state mapping.
- Confirm Designer selection outlines, resize handles, grip arcs, and Preview decoration suppression remain after widget rendering.
- Confirm generated runtime hover, pressed, checked, focused, and slider drag state reuse existing event flow.
- Confirm no `.vfb.json`, model, serialization, validation, or command schema changes.
- Run `git diff --check`.
- Build the normal Windows Debug `VisiForm` target once only through an explicitly approved exact command.
- Leave runtime/manual validation to the developer because automated agents may not launch VisiForm or generated applications.

## Compatibility

- No `.vfb.json` schema change.
- Existing widget properties and generated runtime APIs remain compatible.
- Existing USER CODE regions and export file ownership rules remain unchanged.
- No CMake preset, vcpkg triplet, or runtime-library change.

## Build / Test Status

- Branch: `main`.
- Most recent relevant starting commit: `fd2f719 Update to version 1.0.10 with Phase 104 improvements`.
- Starting worktree contains user-owned Phase 104 session-instruction archive moves and the untracked Phase 105 prompt; they are preserved.
- Focused static validation: `git diff --check` passed with line-ending normalization warnings only. Targeted inspection confirmed that one editor helper owns the fill, border, highlight, shadow, hover, pressed, recessed, disabled, focus-color, and text-contrast derivation used by the five project widgets and shell Splitter. Designer selection and resize overlays remain after widget rendering, and Preview still suppresses editor decorations. Generated runtime rendering reuses its existing pressed/focused state, adds hover bookkeeping to the existing mouse flow, and exports existing `enabled` / `disabled` / `readOnly` properties without changing project serialization.
- Cleanup-pass static validation: `git diff --check` passed with line-ending normalization warnings only. Targeted searches found no remaining generated assignment or draw path that copies `widget.name` into runtime text, captions, or modal titles. A focused Catch2 generator invariant was added but not run because no approved exact build command or Visual Studio workspace build tool was available.
- Windows Debug build: pending an explicitly approved exact command.
- Manual runtime validation: not performed; automated agents may not launch `VisiForm.exe` or generated applications.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_105_runtime_visual_styling_baseline_plan.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.cpp`
- `src/ui/Splitter.cpp`
- `src/ui/Splitter.h`
- `src/ui/VisualStyleBaseline.h`
- `tests/CMakeLists.txt`
- `tests/test_generated_runtime_labels.cpp`

## Designer / Preview / Export Consistency

- Design and Preview use the same `DesignerCanvas` baseline for Button, Text Box, Check Box, Combo Box, and Slider.
- Generated runtime output emits equivalent raised/recessed primitives and applies them to those same five project widget types.
- The editor-shell Splitter uses the editor helper directly and is not an exported project widget.
- Existing model values, text, item selection, slider values, and checked state remain the source of truth.

## Known Parity Gaps

- Designer Preview remains visual-only and therefore cannot show transient hover, pressed, or focus states.
- Splitter is an editor-shell control rather than an exported project widget.
- Runtime styling is equivalent in visual direction and state mapping, but it is emitted as standalone generated helper code rather than sharing the editor header.
- Manual DPI, zoom, maximized-window, exported-build, autosave, and recovery checks remain pending.

## Final Result Summary

- Added a compact reusable visual-style baseline with centralized color blending, border, highlight/shadow edges, hover, pressed, recessed, focus-color, disabled, and text-contrast behavior.
- Applied raised/recessed treatments to the scoped Designer/Preview widgets while preserving text and editor overlays.
- Added a compact raised Splitter divider with hover/drag emphasis and a centered three-line drag affordance.
- Updated generated runtime controls with equivalent styling and hover bookkeeping while retaining existing pressed, focused, checked, and slider drag behavior.
- Added support for existing `enabled`, `disabled`, and `readOnly` properties in the generated runtime without adding serialized properties.
- Established the Phase 105 cleanup invariant that internal widget names are editor metadata, not runtime-visible content.
- Added one shared `shouldDrawEditorLabel()` policy plus mode-aware text helpers in Designer Canvas. Design Mode may show internal identification when an explicit runtime label is empty; Preview Mode receives only explicit runtime text.
- Removed the confirmed Slider name fallback while preserving its track and thumb and adding no automatic value caption.
- Removed generated name fallbacks from generic runtime text initialization, Frame and Group Box captions, modal dialog titles, and legacy direct draw emission. Sizer, Spacer, and Image identification placeholders are now editor-only rather than runtime content.
- Checked all registry-backed widget renderers: Form Window, Frame, Group Box, Panel, Sizer, Tab Control/Tab Page, Menu Bar, Tool Bar, Label, Button, Text Box, Combo Box, List Box, Table/Grid, Tree View, Check Box, Radio Button, Slider, Scroll Bar, Status Bar, Progress Bar, Color Picker, Image, Modal Dialog, and Spacer. Legitimate text continues to come from explicit title, text, item, value, tab, status-field, or dialog properties.

## Cleanup Pass Checklist

- [x] Locate internal-name rendering in shared Designer Canvas and generated runtime paths.
- [x] Centralize Design-only editor-label visibility.
- [x] Suppress internal names and editor placeholders in Preview Mode.
- [x] Preserve explicit runtime text and empty-text behavior.
- [x] Correct Slider without adding a runtime caption or value label.
- [x] Remove generated control-caption and modal-title fallbacks to Name.
- [x] Add a focused generated-source invariant test.
- [x] Run targeted searches and `git diff --check`.
- [ ] Build the normal Windows Debug configuration through an approved exact command or workspace pipeline.
- [ ] Run the new Catch2 invariant and existing relevant tests.
- [ ] Perform the manual Design/Preview/export checklist.

## Cleanup Pass Confirmed Sources

- `src/ui/DesignerCanvas.cpp`: Slider drew `widgetLabel(widget)` above the track when no `text` property existed. Form Window and Frame also used Name as a title fallback, while Label, Button, Check Box, Radio Button, Group Box, Sizer, Spacer, Image, and empty-data placeholders had automatic editor text that needed a mode-aware boundary.
- `src/generator/VisageCppEmitter.cpp`: generic runtime initialization copied `widgetLabel(widget)` into both `widget.text.value` and `widget.dialogTitle`; Frame and Group Box repeated title-to-Name fallbacks; modal opening fell back from title to Name/ID. The legacy direct emitter also contained automatic Sizer, Spacer, and Image labels.
- Internal names remain in generated `widget.name`, event metadata, lookup APIs, and generated identifier/comment paths. Those uses are non-visual and intentional.

## Cleanup Pass Compatibility

- No `.vfb.json` change.
- No generated identifier rename.
- No USER CODE region change.
- Editor labels do not alter bounds, layout, hit testing, selection, or saved geometry.
- Version remains `1.0.11`; no Phase 106 was created.

## Remaining TODOs

- Build the normal Windows Debug `VisiForm` target through an explicitly approved exact command.
- Run the new Catch2 generated-runtime label invariant.
- Manually inspect all six scoped controls in Design and Preview modes at several zoom levels and window sizes.
- Export and build a project containing the five project widget types, then compare runtime appearance and state feedback.
- Manually verify representative internal names never appear in Preview or exported applications and empty user-facing text does not fall back to Name.
- Manually verify save/reload, autosave, and recovery remain unchanged.
