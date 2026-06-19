# Phase 108 Look and Feel Styling Integration Plan

## Scope

- Integrate the Phase 105-107 visual baseline with the existing project Look and Feel registry.
- Keep `ProjectDocument::lookAndFeelId` as the project-level source of truth.
- Centralize preset, fallback, and widget-override resolution for Design Mode, Preview Mode, editor splitters, and generated runtime output.
- Update authoritative version declarations from `1.0.13` to `1.0.14`.

## Requirements

- Use Phase 108 and version `1.0.14`.
- Use no subagents.
- Preserve all existing Look and Feel identifiers, per-widget overrides, project serialization, generated USER CODE regions, and editor-only overlay colors.
- Do not add a theme editor, user themes, per-state project fields, imported theme files, runtime switching UI, or a parallel appearance registry.
- Do not launch `VisiForm.exe` or generated applications from an automated agent.
- Build only through an exact command explicitly requested by the developer.

## Version Notes

- Previous Phase 107 version: `1.0.13`.
- Phase 108 version: `1.0.14`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Confirmed Existing Look and Feel Architecture

- Source of truth: `ProjectDocument::lookAndFeelId`, defaulting to `VisiFormDark`.
- Representation: string identifier resolved through the singleton `LookAndFeelRegistry`.
- Existing identifiers: `VisiFormDark`, `VisiFormLight`, `ImGuiDark`, and `FlatClassic`.
- Property Inspector: the root form exposes a registry-backed choice; widgets expose an optional registry-backed override with `<inherit>`.
- Project dialogs: New Project and Project Settings choices are populated from the same registry.
- Serialization: `JsonProjectWriter` writes the existing top-level `lookAndFeelId`; `JsonProjectReader` leaves the model default in place when the field is absent.
- Validation: unknown project identifiers produce a recoverable export-fallback warning; unknown widget overrides produce a validation error.
- Design and Preview: both use `DesignerCanvas::resolveWidgetStyle`; Preview changes interaction state only and shares the same widget drawing path.
- Generated output: `VisageCppEmitter` resolves each widget from its widget override or the project identifier, falling back to the registry default.
- Current integration gap: DesignerCanvas and the generator duplicate partial style resolution, while the Phase 105-107 hover/pressed/recessed/highlight/shadow values are derived from a hard-coded dark baseline.

## Architecture Decisions

- Extend `LookAndFeelDefinition` with the compact state colors and metrics required by the existing Phase 105-107 renderers.
- Add one registry-owned resolved-style structure and resolution function that applies fallback values and existing widget overrides.
- Keep colors as portable string values in the model layer; UI and generated-code paths convert them to their native color representations.
- Keep editor overlays independent from the selected Look and Feel.
- Store only the Look and Feel identifier in `.vfb.json`; do not serialize the resolved table.

## TODO Checklist

- [x] Inspect Git state, Phase 108 prompt, project status, Phase 107 plan, existing Look and Feel path, shared styling helpers, preview rendering, export path, serialization, and version declarations.
- [x] Confirm Phase 108 is unused and select version `1.0.14`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.14`.
- [x] Add registry-owned resolved styling and safe field-level fallback.
- [x] Migrate Design/Preview styling and editor splitters to resolved Look and Feel values.
- [x] Migrate generated runtime state styling to equivalent resolved values.
- [x] Add focused serialization, fallback, preset-difference, and generated-output checks.
- [x] Run focused static validation.
- [ ] Run the normal Windows Debug build once if the developer supplies or approves an exact command.
- [x] Record final validation, files changed, parity gaps, and remaining issues.

## Validation Plan

- Confirm all four existing presets resolve to complete, visually distinct styles.
- Confirm missing and unknown project identifiers resolve to `VisiFormDark`.
- Confirm existing widget Look and Feel and color/metric overrides still take precedence.
- Confirm missing `lookAndFeelId` loads with the established default and round-trips without schema changes.
- Confirm Design and Preview use the same resolved style path.
- Confirm generated runtime initialization contains the selected preset's equivalent state values.
- Confirm editor overlays remain independent constants.
- Run `git diff --check` and targeted source/tests where allowed.
- Build the normal Windows Debug `VisiForm` target once only through an explicitly approved exact command.
- Leave application launch and generated-app launch to the developer.

## Compatibility

- No `.vfb.json` schema change.
- No Look and Feel identifier rename or removal.
- No generated identifier or USER CODE region change.
- No CMake preset, vcpkg triplet, runtime-library, geometry, interaction, selection, autosave, or recovery change.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains user-owned Phase 107 session-instruction archive moves and the untracked Phase 108 prompt; they are preserved.
- Focused static validation: `git diff --check` passed with line-ending normalization warnings only. Targeted searches confirmed direct preset-field reads are confined to `LookAndFeelRegistry::resolve(...)`; DesignerCanvas and `VisageCppEmitter` use that resolver; generated state rendering consumes emitted recessed, hover, pressed, checked/selected, focus, disabled-text, highlight, and shadow values. Brace and parenthesis counts match in the changed implementation files.
- Focused tests added for legacy missing-field defaulting, unknown-id fallback, preset differences, widget overrides, and selected-preset generated runtime palette emission. Tests were not executed because no approved build/test command was supplied.
- Windows Debug build: pending an explicitly approved exact command.
- Manual Design/Preview/generated-runtime validation: not performed; automated agents may not launch either application.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/VISIFORM_PROJECT_SPEC.md`
- `docs/agent_plans/phase_108_look_and_feel_styling_integration_plan.md`
- `docs/look_and_feel.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/model/LookAndFeelDefinition.h`
- `src/model/LookAndFeelRegistry.cpp`
- `src/model/LookAndFeelRegistry.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.cpp`
- `src/ui/Splitter.cpp`
- `src/ui/Splitter.h`
- `src/ui/VisualStyleBaseline.h`
- `tests/test_generated_runtime_labels.cpp`
- `tests/test_project_serialization.cpp`

## Known Parity Gaps

- Existing Preview interaction limitations from Phase 107 remain: Combo/List selection is simplified, Tab page-child visibility follows the saved selection, and per-item hover is not modeled.
- Generated runtime and DesignerCanvas remain separate drawing implementations; they now share resolved visual intent and state values but are not claimed pixel-identical.
- Control padding is resolved and emitted for future geometry consumers, while existing widget-specific spacing remains unchanged to avoid geometry/interaction regressions in this phase.
- Live DPI, zoom, generated-project build, and runtime appearance remain unverified pending developer validation.

## Final Result Summary

- Updated Phase 108 version metadata from `1.0.13` to `1.0.14`.
- Preserved `ProjectDocument::lookAndFeelId` and the four existing registry identifiers as the only project-level appearance selection.
- Expanded each preset with application/control/recessed/raised surfaces, text roles, state colors, bevel edges, and compact metrics.
- Added `LookAndFeelRegistry::resolve(...)` as the shared preset, field-fallback, and widget-override path.
- Migrated DesignerCanvas, Preview Mode, the reusable editor splitters, and generated runtime state initialization to the shared resolved values.
- Preserved `.vfb.json` compatibility: projects still serialize only `lookAndFeelId` plus existing widget overrides; missing fields use `VisiFormDark`, and unknown identifiers render/export with the same safe fallback.
- Preserved registry-backed Inspector and project-dialog choices; root Inspector changes continue to mark dirty and repaint immediately through the existing path.
- Added focused tests and updated the Look and Feel documentation/specification.

## Remaining TODOs

- Run the normal Windows Debug `VisiForm` build through an explicitly approved exact command.
- Run the focused Catch2 tests through an approved build/test path.
- Manually confirm version `1.0.14`.
- Exercise all four presets in Design and Preview, including normal, hover, pressed, focused, checked/selected, and disabled states.
- Confirm editor overlays remain readable for each preset.
- Save/reload and recovery-test the selected Look and Feel.
- Export and build representative generated projects for at least the dark and light presets.
