# Phase 110 Look and Feel Preset Management Plan

## Scope

- Extend the existing Look and Feel registry with reusable user-owned custom presets.
- Add save, duplicate, rename, delete, import, and export operations to the existing Look and Feel editor.
- Preserve the resolution order `built-in or custom preset -> project overrides -> resolved Look and Feel`.
- Update authoritative version declarations from `1.0.15` to `1.0.16`.

## Requirements

- Use Phase 110 and version `1.0.16`.
- Use no subagents.
- Preserve built-in identifiers and definitions as immutable shipped presets.
- Store custom presets outside projects and the repository.
- Preserve Phase 109 project overrides, generated `MainWindow`, USER CODE regions, CMake presets, vcpkg settings, and static MSVC runtime settings.
- Add no per-widget overrides, marketplace, cloud synchronization, thumbnails, tags, inheritance chains, font bundles, gradients, animations, or VisiForm IDE theme switching.
- Do not launch `VisiForm.exe` or generated applications from an automated agent.

## Version Notes

- Previous Phase 109 version: `1.0.15`.
- Phase 110 version: `1.0.16`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, `docs/versioning.md`, and the current-version line in `README.md`.

## Architecture Decisions

- Add one shared custom-preset library service that owns persistence, validation, stable-ID creation, collision handling, and import/export.
- Keep `LookAndFeelRegistry` as the single resolver and option source; it exposes immutable built-ins plus a replaceable custom-definition collection loaded by the application.
- Store complete normalized styles in custom presets rather than project override semantics.
- Keep project files unchanged: `lookAndFeelId` remains a stable identifier and `lookAndFeelOverrides` remains sparse.
- Resolve missing custom IDs through the existing `VisiFormDark` fallback while retaining the unresolved stored identifier until the user selects another preset or a deletion of the active preset explicitly applies fallback.

## Custom-Preset Storage

- Windows: `%APPDATA%/VisiForm/look_and_feel_presets.json`.
- macOS: `~/Library/Application Support/VisiForm/look_and_feel_presets.json`.
- Linux: `$XDG_CONFIG_HOME/VisiForm/look_and_feel_presets.json`, or `~/.config/VisiForm/look_and_feel_presets.json`.
- Last-resort environment fallback: the system temporary directory under
  `VisiForm/look_and_feel_presets.json`, never the repository.
- The store is readable JSON with top-level `formatVersion: 1` and a `presets` array.
- Each entry contains `id`, `displayName`, `formatVersion`, optional `sourcePresetId`, and one complete style object.
- Writes use a sibling temporary file followed by replacement.
- Missing stores are treated as an empty library. Malformed individual entries are skipped while valid entries are retained where practical.

## Stable Identifier Policy

- Built-in identifiers remain unchanged and reserved.
- New custom identifiers use the `custom.` prefix plus a generated opaque suffix and do not depend on display names.
- Rename preserves the identifier.
- Duplicate and import collisions generate a fresh custom identifier.
- Display names are trimmed, non-empty, case-insensitively unique, and receive deterministic `Copy`, numbered, or `Imported` suffixes where automatic collision resolution is required.

## Built-In Versus Custom Rules

- Built-ins are selectable, duplicable, and exportable.
- Built-ins cannot be renamed, deleted, or overwritten.
- Custom presets are selectable, duplicable, renamable, deletable, importable, and exportable.
- Editing project overrides never mutates either preset type.

## Management UI

- Extend the existing Look and Feel editor with a selected-preset field and compact management buttons.
- Save as New Preset captures the editor's current fully resolved temporary style.
- Duplicate captures the selected preset's complete resolved style.
- Rename and Delete operate only on custom presets.
- Import and Export use the existing native file-picker pattern and `.vflnf.json`.
- Library operations refresh every Look and Feel choice source immediately.

## Import / Export And Collision Handling

- Import validates file and entry format version, required complete style values, color syntax, and finite numeric values before changing the library.
- Safe numeric ranges are normalized to the established Look and Feel limits.
- Imported built-in or existing identifiers receive a new custom identifier.
- Imported display-name collisions receive a deterministic unique suffix.
- Export writes one portable complete preset and no project data, project overrides, editor state, or local paths.

## Missing-Preset Fallback

- Loading a project with an unavailable preset preserves its stored `lookAndFeelId`, resolves visually through `VisiFormDark`, preserves project overrides, and shows one concise warning.
- Choosing another preset updates the stored identifier normally.
- Deleting the active custom preset requires confirmation, switches the active project to `VisiFormDark`, preserves overrides, creates one project change, marks the document dirty, and repaints.

## Project Overrides And Generated Output

- Custom presets remain below sparse project overrides in the existing resolver.
- Reset to Preset clears temporary project overrides and reveals the selected custom or built-in preset.
- Export resolves the custom preset and project overrides inside VisiForm and emits the existing portable runtime style values; generated applications never read the local preset store.

## TODO Checklist

- [x] Inspect Git state, the Phase 110 prompt, project status, Phase 109 plan, Look and Feel registry/editor, settings storage, serialization helpers, generated styling, and version declarations.
- [x] Confirm Phase 110 is unused and select version `1.0.16`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.16`.
- [x] Add the shared custom-preset library, safe persistence, validation, stable IDs, and collision handling.
- [x] Integrate custom definitions into registry lookup, choices, fallback, validation, Design/Preview, and generation.
- [x] Add save, duplicate, rename, delete, import, and export UI operations.
- [x] Add focused preset-library, fallback, override, and generated-output tests.
- [x] Update Look and Feel documentation and final project status.
- [x] Run focused static validation.
- [ ] Run the approved normal Windows Debug build once.
- [x] Record final validation, files changed, limitations, and remaining issues.

## Validation Plan

- Add focused tests for library round-trip, malformed-entry tolerance, complete-style normalization, stable rename, built-in protection, duplicate/import collisions, active/missing fallback, project override layering, and generated output.
- Confirm all Look and Feel choice sources expose custom presets with stable IDs.
- Confirm generated output contains resolved values and no preset-store path.
- Run `git diff --check` and targeted source searches.
- Build the normal Windows Debug configuration once through an approved path.
- Leave application launch and generated-app launch to the developer.

## Compatibility

- `.vfb.json` schema version remains `1`.
- Existing built-in identifiers and projects remain compatible.
- Projects with unavailable custom IDs preserve the ID in memory and on save until the user changes the selection.
- Project overrides, autosave, recovery, and generated output continue through existing project serialization and resolution paths.
- The custom-preset format is independently versioned at `1`.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains the user-owned Phase 109 session-instruction archive move and the untracked Phase 110 prompt; they are preserved.
- Focused static validation: passed. `git diff --check` reported only the
  repository's existing LF-to-CRLF normalization warnings. Targeted searches
  confirmed every new store, registry, dialog, and native-file-dialog method
  has a matching declaration/definition, all new sources are registered in the
  application/test targets, authoritative version declarations report
  `1.0.16`, and changed implementation files have balanced brace/parenthesis
  counts.
- Focused tests: added for complete-style persistence, stable-ID rename,
  built-in duplication/protection, deterministic collision handling, invalid
  import atomicity, malformed-entry recovery, custom resolution, and portable
  generated output. They were not executed because no approved exact build or
  test command was supplied.
- Windows Debug build: not run. No Visual Studio workspace-build tool is
  available in this session, and repository safety rules prohibit inferring a
  terminal build command without exact developer authorization.
- Manual runtime validation: not performed; automated agents may not launch either application.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/VISIFORM_PROJECT_SPEC.md`
- `docs/agent_plans/phase_110_look_and_feel_preset_management_plan.md`
- `docs/look_and_feel.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/model/LookAndFeelDefinition.h`
- `src/model/LookAndFeelRegistry.cpp`
- `src/model/LookAndFeelRegistry.h`
- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `src/ui/PropertyInspector.cpp`
- `src/utils/LookAndFeelPresetStore.cpp`
- `src/utils/LookAndFeelPresetStore.h`
- `src/utils/NativeFileDialogs.cpp`
- `src/utils/NativeFileDialogs.h`
- `tests/CMakeLists.txt`
- `tests/test_generated_runtime_labels.cpp`
- `tests/test_look_and_feel_preset_store.cpp`

The pre-existing Phase 109 session-instruction archive move and the Phase 110
instruction file were preserved without modification.

## Known Limitations

- Online sharing, cloud synchronization, thumbnails, categories, tags, inheritance chains, automatic updates, font bundles, gradients, animations, per-widget preset management, and IDE theme switching are outside Phase 110.
- The compact editor uses a two-row command area; visual fit at the minimum
  supported window size requires manual runtime verification.
- Library load warnings use the existing operation-status surface rather than
  a blocking startup dialog.

## Final Result Summary

- Updated the phase version from `1.0.15` to `1.0.16`.
- Added one complete-style custom-preset library in the established VisiForm
  user-data location with format version `1`, per-entry recovery, stable
  `custom.*` identifiers, and temporary-file replacement.
- Extended the shared registry so built-in and custom presets use the same
  lookup, choice, validation, Design/Preview, and generated-output paths.
- Added Save New, Duplicate, Rename, Delete, Import, and Export operations to
  the existing Look and Feel editor, with built-in protections and two-step
  delete confirmation.
- Added `.vflnf.json` portable files, full validation/range normalization,
  fresh imported identifiers, and deterministic display-name collision
  handling.
- Preserved sparse project overrides over custom presets. Reset to Preset still
  clears only temporary project overrides, and library edits do not dirty the
  project.
- Missing custom presets retain the stored project identifier, preserve
  overrides, render through `VisiFormDark`, and report a concise warning.
- Deleting the active custom preset switches the project to `VisiFormDark`,
  preserves overrides, marks the project dirty, and repaints.
- Generated output resolves custom presets inside VisiForm and contains no
  preset-store path or custom identifier dependency.

## Remaining TODOs

- Run the normal Windows Debug build through the approved Visual Studio
  workspace pipeline or an exact developer-authorized command.
- Run the focused Catch2 tests.
- Manually execute the Phase 110 persistence, management, collision, fallback,
  Design Mode, Preview Mode, restart, project reload, autosave/recovery, and
  generated-project validation checklist.
