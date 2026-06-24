# Phase 114 Typography and Text Rendering Foundation Plan

## Scope

- Integrate typography into the existing Look and Feel resolution path.
- Add sparse project-level and widget-level typography overrides.
- Render supported text-bearing widgets with resolved typography in Design Mode and Preview Mode.
- Persist only explicit typography overrides in `.vfb.json`.
- Emit generated runtime code that reproduces the resolved typography intent.
- Update authoritative version declarations from `1.0.19` to `1.0.20`.

## Requirements

- Use Phase 114 and version `1.0.20`.
- Use no subagents.
- Resolution order: Look and Feel typography -> project typography overrides -> widget typography overrides.
- Support font family, font size, font weight, italic, horizontal alignment, meaningful vertical alignment, text padding, and disabled-text treatment.
- Do not add embedded fonts, rich text, text effects, typography presets, or per-state typography overrides.
- Keep project and widget overrides sparse.
- Keep unsupported typography properties hidden or unavailable.
- Do not launch `VisiForm.exe` or generated applications from an automated agent.

## Version Notes

- Previous Phase 113 version: `1.0.19`.
- Phase 114 version: `1.0.20`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, `docs/versioning.md`, and the current-version line in `README.md`.

## Architecture Decisions

- Extend the existing Look and Feel structs rather than creating a separate font or theme system.
- Add a compact typography override struct reused by project and widget scope.
- Keep legacy widget `fontSize` properties readable as a compatibility fallback.
- Resolve unavailable or empty font families to the existing application/default designer font path without asserting.
- Treat text padding as the typography padding value for text layout, separate from existing appearance `controlPadding`.
- Keep state appearance color overrides independent of typography; disabled text treatment resolves through the shared disabled text color behavior.
- Font Weight uses one shared supported-value mapping: internal `400` displays as `Regular`, and internal `700` displays as `Bold`.
- Unsupported numeric font weights from loaded projects or UI commits normalize to `400` / `Regular`; display labels are not stored redundantly.

## Supported Widgets

- Button
- Label
- Text Box
- Check Box
- Radio Button
- Combo Box
- List Box
- Group Box
- Frame
- Tab Control
- Status Bar
- Menu Bar

## TODO Checklist

- [x] Inspect Git state, project status, Phase 113 plan, and Phase 114 instructions.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.20`.
- [x] Extend model and Look and Feel typography resolution.
- [x] Add sparse serialization and copy/duplicate support coverage.
- [x] Add Property Inspector Typography rows and reset actions.
- [x] Apply resolved typography to supported Design/Preview rendering paths.
- [x] Update generated runtime style and rendering output.
- [x] Add focused tests or static validation coverage.
- [x] Fix Font Weight Property Inspector display so closed rows use readable labels instead of raw numeric values.
- [x] Update final project status and this plan.
- [ ] Run approved validation once, if an exact approved path is available.

## Validation Plan

- Run focused static checks and `git diff --check`.
- Run the normal Windows Debug build only through an exact developer-authorized command or approved Visual Studio workspace pipeline.
- Manual validation remains developer-run unless explicitly performed outside automation: version display, project/widget typography edits, Preview Mode parity, save/reload, copy/paste, export, generated output, unavailable font fallback, undo/redo, and DPI checks.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains unrelated session-instruction archival changes; they are preserved.
- Focused static validation: passed. `git diff --check` reported only the repository's existing LF-to-CRLF normalization warnings.
- Focused tests: added project/widget typography serialization and resolver coverage in `tests/test_project_serialization.cpp`; added unsupported font-weight fallback coverage (`500`/`600` -> `400` / `Regular`). Tests were not executed because no approved exact build/test command was supplied.
- Font Weight display bug-fix static validation: `git diff --check -- src/model/LookAndFeelDefinition.h src/model/LookAndFeelRegistry.cpp src/serialization/JsonProjectReader.cpp src/ui/PropertyInspector.cpp src/ui/MainWindow.cpp tests/test_project_serialization.cpp docs/agent_plans/phase_114_typography_text_rendering_plan.md docs/project_status.md` passed with only existing LF-to-CRLF warnings.
- Windows Debug build: not run because the repository instructions prohibit terminal build commands without an exact developer-requested command and no unambiguous Visual Studio workspace build pipeline was available to the agent.
- Manual runtime validation: not performed.

## Compatibility Considerations

- Old projects load with preset defaults and no typography overrides.
- Legacy widget `fontSize` remains readable.
- Unsupported serialized typography weights are accepted and normalized to `Regular` (`400`) during load and style resolution.
- `.vfb.json` stores only explicit project and widget typography overrides.
- Generated projects do not depend on editor-only font resources.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_114_typography_text_rendering_plan.md`
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
- `src/utils/LookAndFeelPresetStore.cpp`
- `tests/test_project_serialization.cpp`

## Final Result Summary

- Updated the phase version from `1.0.19` to `1.0.20`.
- Added sparse project typography overrides for font family, font size, font weight, italic, text padding, and disabled-text treatment.
- Added sparse widget typography overrides for supported text-bearing widgets: font family, font size, font weight, italic, horizontal text alignment, meaningful vertical alignment, and text padding.
- Preserved the resolution order: Look and Feel typography -> project typography overrides -> widget typography overrides.
- Kept legacy widget `fontFamily`, `fontSize`, `fontBold`, and `fontItalic` readable as compatibility inputs.
- Added compact Typography rows inside the existing Property Inspector Appearance area and kept multi-selection editing disabled through the existing Appearance guard.
- Used safe editor font fallback: requested family/weight/italic are recorded in resolved style, known Windows families use the existing font-loading path, and unavailable families fall back to the current label font.
- Applied resolved typography to the supported Design/Preview text-bearing render paths where a clear text rectangle exists.
- Extended generated runtime style emission with typography fields and applied font size/text padding to generated text rendering while avoiding editor-only font resources.
- Extended custom Look and Feel preset import/export so typography survives preset save/duplicate/import/export while older preset JSON remains loadable.
- Added a shared font-weight label/value mapping and changed the Property Inspector Font Weight row to show `Regular` / `Bold` in the closed row while still committing internal numeric values.
- Normalized unsupported font weights to `Regular` (`400`) for fallback safety.

## Remaining TODOs

- Run the normal Windows Debug build through the approved Visual Studio workspace pipeline or an exact developer-authorized command.
- Run the focused Catch2 tests.
- Manually validate the Phase 114 runtime checklist, including the Font Weight row label/dropdown behavior, inherited and explicit weight display, Preview Mode parity, unavailable font fallback, undo/redo, save/reload, copy/paste, export, generated output, and DPI behavior.
