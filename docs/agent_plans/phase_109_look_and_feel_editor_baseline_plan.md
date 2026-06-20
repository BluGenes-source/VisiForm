# Phase 109 Look and Feel Editor Baseline Plan

## Scope

- Add one project-level Look and Feel editor for the selected built-in preset.
- Store only explicit project overrides and keep `ProjectDocument::lookAndFeelId` as the base preset.
- Route Design Mode, Preview Mode, editor splitters, serialization, autosave/recovery, and generated output through the final resolved project style.
- Update authoritative version declarations from `1.0.14` to `1.0.15`.

## Requirements

- Use Phase 109 and version `1.0.15`.
- Use no subagents.
- Add no per-widget style overrides, user preset library, import/export format, font editor, gradients, animations, or second theme system.
- Preserve built-in presets, generated `MainWindow`, USER CODE regions, selection overlays, grid/guides, CMake presets, vcpkg settings, and static MSVC runtime settings.
- Do not launch `VisiForm.exe` or generated applications from an automated agent.

## Version Notes

- Previous Phase 108 version: `1.0.14`.
- Phase 109 version: `1.0.15`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, `docs/versioning.md`, and the current-version line in `README.md`.

## Architecture Decisions

- Add a compact `LookAndFeelOverrides` model with optional values; absence means inherit from the selected preset.
- Add one registry-owned project resolver that resolves the base preset, applies normalized project overrides, and then lets the existing widget resolver apply established per-widget properties.
- Serialize project overrides as an optional top-level object and omit it when empty.
- Use the existing editor-modal framework and native color picker through one Project-menu command path.
- Keep temporary editor values outside the project model until Apply or OK.
- Use `DocumentStateCommand` so each changed Apply or OK operation is one undo step; no-op commits create no history entry and do not dirty the project.
- Preserve explicit overrides when the base preset changes.

## Editable Values

### Colors

- application/form surface
- control surface
- recessed surface
- primary text
- disabled text
- border
- focus outline
- accent/selection
- highlight edge
- shadow edge

### Metrics

- border thickness: `0.0` to `20.0`
- corner radius: `0.0` to `50.0`
- control padding: `0.0` to `40.0`
- splitter highlight thickness: `0.0` to `8.0`
- splitter shadow thickness: `0.0` to `8.0`

## Dialog Semantics

- Apply commits changed sparse overrides, repaints immediately, creates one undo entry, marks dirty, and keeps the dialog open.
- OK applies changed values and closes; a no-op OK only closes.
- Cancel discards only uncommitted temporary values; prior Apply operations remain committed.
- Reset to Preset clears temporary overrides and shows base values, requiring Apply or OK to commit.
- The compact sample preview uses the temporary resolved style and never mutates the project model.

## TODO Checklist

- [x] Inspect Git state, the Phase 109 prompt, project status, Phase 108 plan, Look and Feel resolution, modal framework, serialization, generator, undo/dirty behavior, and version declarations.
- [x] Confirm Phase 109 is unused and select version `1.0.15`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.15`.
- [x] Add sparse project-level Look and Feel overrides and shared final resolution.
- [x] Add backward-compatible serialization and focused tests.
- [x] Add the shared editor command, modal controls, color/numeric editing, and live sample.
- [x] Integrate no-op-safe Apply/OK/Cancel/Reset with dirty state and undo/redo.
- [x] Confirm Design/Preview/splitter/generated output consume final resolved values.
- [x] Update focused Look and Feel documentation.
- [x] Run focused static validation.
- [x] Run the approved normal Windows Debug build once.
- [x] Record final validation, files changed, limitations, and remaining issues.

## Validation Plan

- Add focused tests for empty sparse serialization, override round-trip, legacy loading, field normalization, base-preset changes, final resolution, and generated output.
- Confirm editor command mappings and Preview Mode enablement rules statically.
- Confirm Apply/OK no-op checks compare normalized sparse overrides.
- Confirm autosave/recovery need no separate path because they use the normal serializer.
- Run `git diff --check` and targeted source searches.
- Build the normal Windows Debug configuration once through the developer-requested approved path.
- Leave application launch and generated-app launch to the developer.

## Compatibility

- Schema version remains `1`.
- Projects without `lookAndFeelOverrides` load exactly as before.
- Empty overrides are omitted from new files.
- Unknown override keys follow the existing tolerant top-level compatibility behavior.
- Invalid supported values are ignored or clamped during resolution rather than crashing.
- Existing widget-level style properties remain supported and are not expanded by this phase.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains user-owned Phase 108 session-instruction archive moves and the untracked Phase 109 prompt; they are preserved.
- Post-plan build fix: the approved workspace build reported `MainWindow.cpp(9425)`
  calling `PanelBounds::isValid()` after `PanelBounds` only exposed
  `isVisible()`. A compatibility `isValid()` wrapper was restored in
  `src/ui/MainWindow.h` so the new Look and Feel preview code compiles against
  the existing call site.
- Focused static validation: passed. `git diff --check` reported only the
  repository's existing LF-to-CRLF normalization warnings. Targeted searches
  confirmed DesignerCanvas, editor splitters, and `VisageCppEmitter` still
  consume `LookAndFeelRegistry::resolve(...)`, which now includes project
  overrides. Delimiter counts were balanced in the changed implementation
  files, and authoritative version declarations consistently report `1.0.15`.
- Focused Catch2 tests were added for sparse omission, override round-trip,
  base-preset changes, invalid-value fallback/clamping, and generated runtime
  output. They were not executed because no approved exact build/test command
  was supplied.
- Windows Debug build: passed through the approved Visual Studio workspace
  build pipeline after restoring `PanelBounds::isValid()` as a compatibility
  wrapper for the `MainWindow.cpp` preview call site.
- Build regression fix: `src/model/LookAndFeelRegistry.cpp` incorrectly reused
  the widget-property `applyColorOverride(const char*, std::string&)` helper for
  `WidgetLookAndFeelOverrides` optional string fields. The helper usage was
  split so optional appearance overrides now apply through a matching
  `std::optional<std::string>` path.
- Post-fix Windows Debug build: passed again through the approved Visual Studio
  workspace build pipeline after correcting the `WidgetLookAndFeelOverrides`
  color-override helper path in `src/model/LookAndFeelRegistry.cpp`.
- Manual Design/Preview/generated-runtime validation: not performed; automated agents may not launch either application.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/VISIFORM_PROJECT_SPEC.md`
- `docs/agent_plans/phase_109_look_and_feel_editor_baseline_plan.md`
- `docs/look_and_feel.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/commands/CommandIds.h`
- `src/commands/CommandRegistry.cpp`
- `src/model/LookAndFeelDefinition.h`
- `src/model/LookAndFeelRegistry.cpp`
- `src/model/LookAndFeelRegistry.h`
- `src/model/ProjectDocument.h`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `tests/test_generated_runtime_labels.cpp`
- `tests/test_project_serialization.cpp`

## Known Limitations

- The live sample uses the established shared raised/recessed helpers. Corner
  radius is updated immediately in its displayed metric, while the compact
  sample controls remain rectangular; Design/Preview/generated boxed widgets
  continue to use the existing rounded renderer.
- Existing generated runtime code still stores resolved style values in the
  established per-widget runtime style structure so legacy per-widget overrides
  remain possible; this phase did not introduce a second style table or new
  per-widget properties.
- Existing undo/redo infrastructure marks the document dirty after both undo
  and redo. The prior and new Look and Feel override states are restored
  correctly, but saved-state cleanliness is not tracked by the command stack.
- Runtime layout, DPI behavior, native color-picker interaction, and generated
  project compilation remain unverified pending developer validation.

## Final Result Summary

- Updated the Phase 109 version from `1.0.14` to `1.0.15`.
- Added sparse optional project overrides for ten shared colors and five
  metrics, omitted from JSON when empty.
- Added `resolveProjectStyle(...)` as the base-preset plus project-override
  resolver and retained `resolve(...)` as the final widget-style path.
- Added `Project > Edit Look and Feel...` through the shared command registry.
- Added native color selection, validated numeric editing, explicit-override
  markers, and a temporary representative-control sample.
- Apply and OK create one no-op-safe `DocumentStateCommand`; Cancel discards
  only uncommitted changes, and Reset to Preset remains temporary until commit.
- Preserved project overrides when the base preset changes.
- Restored `PanelBounds::isValid()` as a thin alias to `isVisible()` so the
  Look and Feel preview path matches the current `MainWindow.cpp` call site.
- Fixed a follow-up Phase 109 compile regression in `LookAndFeelRegistry` by
  applying `WidgetLookAndFeelOverrides` colors through an optional-string helper
  instead of the widget-property lookup helper that expects `const char*` keys.
- Revalidated the normal Windows Debug workspace build after the regression fix;
  the workspace build now succeeds.
- Design Mode, Preview Mode, editor splitters, autosave/recovery serialization,
  and generated output consume the final resolved project style.
- Added focused serialization, fallback, base-change, and generated-output
  tests and updated the Look and Feel documentation/specification.

## Remaining TODOs

- Run the focused Catch2 tests.
- Manually execute the Phase 109 dialog, persistence, base-change, Preview Mode,
  undo/redo, autosave/recovery, and generated-project validation checklist.
