# Phase 115 Text Layout and Wrapping Foundation Plan

## Scope

- Add one shared text-layout foundation for current and future text-bearing widgets.
- Support single-line and multiline layout, word wrapping, clipping, ellipsis, alignment, and padding-aware content bounds.
- Apply compatible behavior to Design Mode, Preview Mode, serialization, Property Inspector editing, undo/redo, and generated runtime output.
- Preserve existing geometry unless an explicit existing command changes size.
- Update authoritative version declarations from `1.0.20` to `1.0.21`.

## Requirements

- Use Phase 115 and version `1.0.21`.
- Use no subagents.
- Keep the pass focused on text layout and wrapping; do not build a full text-editor widget.
- Support `Multiline`, `Word Wrap`, `Overflow Mode`, horizontal alignment, vertical alignment, and existing text padding where meaningful.
- Supported widget scope: Label, Button, Text Box, Check Box, Radio Button, Group Box, Frame, tab labels, and Status Bar.
- Store only explicit text-layout properties.
- Do not launch `VisiForm.exe` or generated applications from an automated agent.

## Version Notes

- Previous Phase 114 version: `1.0.20`.
- Phase 115 version: `1.0.21`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, `docs/versioning.md`, and the current-version line in `README.md`.

## Architecture Decisions

- Reuse Phase 114 typography resolution for font size, alignment defaults, and text padding.
- Add sparse widget appearance override fields for layout-specific intent rather than creating a separate serialized cache.
- Prefer one deterministic shared layout helper for editor rendering and tests; generated output receives equivalent helper code in the existing runtime emitter.
- Keep Text Box editing single-line for this phase while rendering multiline stored text where configured.
- Treat multiline final-line ellipsis as optional; safely clipped multiline output is acceptable if recorded.

## Supported Properties

- `Multiline`: explicit widget override where multiline rendering is meaningful.
- `Word Wrap`: explicit widget override where multiline wrapping is meaningful.
- `Overflow Mode`: `Clip` or `Ellipsis`.
- Horizontal alignment: `Left`, `Center`, `Right`, backed by existing typography alignment fields.
- Vertical alignment: `Top`, `Center`, `Bottom`, backed by existing typography alignment fields.
- Text padding: reused from Phase 114 typography.

## TODO Checklist

- [x] Inspect Git state, project status, Phase 114 plan, and Phase 115 instructions.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.21`.
- [x] Inspect targeted model, rendering, inspector, serialization, generator, and test files.
- [x] Add shared text-layout model/helper behavior.
- [x] Add sparse serialization and compatibility fallback coverage.
- [x] Add compatible Property Inspector controls and undo/redo behavior.
- [x] Apply shared text layout to supported Design/Preview render paths.
- [x] Update generated runtime output with equivalent layout intent.
- [x] Add focused tests or static validation coverage.
- [x] Update final project status and this plan.
- [ ] Run approved validation once, if an exact approved path is available.

## Validation Plan

- Run focused static checks and `git diff --check`.
- Run focused tests if an approved exact test command is available.
- Run the normal Windows Debug build only through an exact developer-authorized command or approved Visual Studio workspace pipeline.
- Manual runtime validation remains developer-run unless explicitly performed outside automation: version display, Label/Button/Text Box/Check Box/Radio Button/Group Box/Frame/Tab/Status Bar layout behavior, Preview Mode parity, undo/redo, save/reload, export, generated output, and autosave/recovery preservation.

## Build / Test Status

- Branch: `main`.
- Starting commit: `2f2a41e Phase 114: Add typography and rendering updates`.
- Starting worktree contains unrelated session-instruction archival changes; they are preserved.
- Focused static validation: `git diff --check` passed with only existing LF-to-CRLF normalization warnings.
- Focused tests: serialization/resolver coverage was added in `tests/test_project_serialization.cpp`; tests were not executed because no exact approved test command was supplied.
- Windows Debug build: not run because repository instructions prohibit terminal build commands without an exact developer-requested command and no unambiguous Visual Studio workspace build pipeline was available to the agent.
- Manual runtime validation: not performed.

## Compatibility Considerations

- Old projects load with current single-line, no-wrap, `Clip` defaults.
- Missing layout properties preserve previous behavior.
- Invalid serialized overflow values fall back to `Clip`; alignment continues through the existing safe normalization path.
- `.vfb.json` stores only explicit layout overrides.
- No editor-only layout cache is serialized.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_115_text_layout_wrapping_plan.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/model/LookAndFeelDefinition.h`
- `src/model/LookAndFeelRegistry.cpp`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.cpp`
- `src/ui/TextLayout.cpp`
- `src/ui/TextLayout.h`
- `tests/test_project_serialization.cpp`

## Final Result Summary

- Updated the phase version from `1.0.20` to `1.0.21`.
- Added sparse widget text-layout overrides for `multiline`, `wordWrap`, and `overflowMode`.
- Added a reusable editor text-layout helper that measures text with `visage::Font`, preserves explicit newlines, wraps at word boundaries where possible, splits oversized words safely, computes line positions, clips to content bounds, and applies single-line ellipsis with font metrics.
- Routed Design/Preview rendering for Label, Button, Text Box, Check Box, Radio Button, Group Box, Frame, tab labels, and Status Bar through the shared helper.
- Added Property Inspector Text Layout rows with bool controls for Multiline and Word Wrap and a dropdown for Overflow Mode; Word Wrap hides while Multiline is effectively off unless it has an explicit override to reset.
- Added sparse save/load behavior and safe invalid-overflow fallback.
- Updated generated runtime output with equivalent layout fields and helper behavior for the supported widgets.
- Left Text Box editing behavior single-line for this phase; multiline stored text can render, but full multiline editing/caret navigation remains future work.
- Multiline final-line ellipsis is not implemented in this base pass; multiline overflow clips safely within content bounds.

## Remaining TODOs

- Run the normal Windows Debug build through the approved Visual Studio workspace pipeline or an exact developer-authorized command.
- Run the focused Catch2 tests.
- Manually validate the Phase 115 runtime checklist, including version display, Label/Button/Text Box/Check Box/Radio Button/Group Box/Frame/Tab/Status Bar layout behavior, Preview Mode parity, undo/redo, save/reload, export, generated output, and autosave/recovery preservation.
