# Phase 102 Autosave and Crash Recovery Plan

## Scope

- Add one five-minute application-owned autosave timer.
- Save dirty, valid projects to separate recovery data without changing the normal `.vfb.json` format.
- Detect relevant recovery data at startup and offer Restore, Discard, or Later.
- Remove associated recovery data after a successful normal Save or Save As.
- Update authoritative version declarations from `1.0.7` to `1.0.8`.

## Requirements

- Use Phase 102 and version `1.0.8`.
- Use no subagents.
- Preserve the existing serializer and normal project-file schema.
- Do not overwrite, rename, or automatically replace the user's main project file.
- Do not launch `VisiForm.exe` from an automated agent.
- Do not run a build command unless the developer explicitly authorizes that exact command.

## Version Notes

- Previous Phase 101 version: `1.0.7`.
- Phase 102 version: `1.0.8`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Architecture Decisions

- `MainWindow` owns the single autosave timer and all recovery workflow state.
- The default autosave interval is five minutes (`300000` milliseconds) and is documented rather than added to settings in this base phase.
- Recovery data lives under the existing application-data root in a `recovery` directory so startup discovery does not depend on scanning project folders.
- Saved projects use a deterministic path-derived identifier; unsaved projects use one stable document identifier for the current editor session.
- Recovery project data remains normal serializer output in a separate `*.recovery.vfb.json` file.
- A companion `*.recovery.meta.json` file records the original project path, display name, update time, VisiForm version, unsaved state, and document identifier.
- Recovery files are written to temporary siblings, flushed and closed, then replaced atomically where supported.
- Autosave eligibility requires a dirty document, no save/load/autosave operation, no editor modal, and no project validation errors.

## Startup Decision Flow

- Enumerate recovery metadata from the application recovery directory.
- Ignore incomplete entries and saved-project recovery data that is not newer than the original project.
- Offer the newest relevant entry.
- Restore loads recovery content, preserves the original project path when known, keeps the document dirty, and retains recovery data until a successful manual save.
- Discard removes the recovery project and metadata files.
- Later closes the prompt and leaves recovery data unchanged.

## Cleanup Rules

- Successful Save or Save As removes the active document's previous recovery association and resets the timer. If deletion fails, the association is retained so cleanup can be retried instead of orphaning discoverable recovery data.
- Failed manual saves keep recovery data and dirty state.
- New/opened projects reset the active recovery association and timer after the existing unsaved-changes workflow succeeds.
- Recovery data is not deleted merely because startup restore fails, the user chooses Later, or the application exits unexpectedly.

## TODO Checklist

- [x] Inspect branch and worktree.
- [x] Read the Phase 102 brief, project status, Phase 101 plan, and targeted source files.
- [x] Confirm Phase 102 is unused and select version `1.0.8`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations to `1.0.8`.
- [x] Record Phase 102 startup in `docs/project_status.md`.
- [x] Add recovery storage, metadata, safe replacement, discovery, load, and cleanup helpers.
- [x] Add the application-owned five-minute autosave timer and eligibility checks.
- [x] Add startup Restore, Discard, and Later flow.
- [x] Integrate successful Save / Save As cleanup and new/open association reset.
- [x] Run focused static validation.
- [ ] Run the normal Windows Debug build once if the developer supplies or approves an exact command.
- [x] Record final validation, files changed, limitations, and remaining issues.

## Validation Plan

- Inspect recovery naming, metadata, safe-write, discovery, restore, discard, and cleanup paths.
- Verify autosave never clears dirty state and normal save/load serializer behavior is unchanged.
- Run `git diff --check`.
- Build the normal Windows Debug `VisiForm` target once only through an explicitly approved exact command.
- Do not perform automated runtime validation because repository rules prohibit launching `VisiForm.exe`.
- Record the manual recovery checklist as deferred unless the developer performs it.

## Compatibility

- No normal `.vfb.json` schema change.
- No generator behavior change.
- Recovery files use the existing project serializer and reader.
- Canvas zoom, pan, selection, and Preview Mode remain outside recovery data except for selection already serialized by the normal project format.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains the Phase 101 session-instruction archive move and the untracked Phase 102 instruction file; those user changes are preserved.
- Focused static validation: `git diff --check` passed with line-ending normalization warnings only. Targeted source checks confirmed the five-minute timer, dirty/validation/operation/modal guards, Windows write-through replacement, startup Restore/Discard/Later actions, restored dirty state, successful manual-save cleanup, retained cleanup association after deletion failure, and consistent `1.0.8` declarations. Normal serializer and schema source files were not modified.
- Windows Debug build: pending an explicitly approved exact command.
- Manual runtime validation: not performed; automated agents may not launch `VisiForm.exe`.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_102_autosave_crash_recovery_plan.md`
- `docs/file_workflow.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `src/utils/NativeFileDialogs.cpp`
- `src/utils/NativeFileDialogs.h`
- `src/utils/ProjectRecovery.cpp`
- `src/utils/ProjectRecovery.h`

## Final Result Summary

- Updated all authoritative application version declarations and current-progress documentation to `1.0.8` / Phase 102.
- Added one `MainWindow`-owned five-minute autosave timer with safe destruction and timer reset after project replacement or successful manual save.
- Autosave runs only for dirty documents with no validation errors while no save, load, export, editor modal, native dialog, unsaved-changes prompt, or prior autosave is active.
- Added application recovery storage under `%APPDATA%/VisiForm/recovery/` on Windows, with platform configuration-directory and `Generated/recovery/` fallbacks.
- Recovery project data uses unchanged `JsonProjectWriter` output in `*.recovery.vfb.json`; companion `*.recovery.meta.json` files record original path, display name, UTC update time, VisiForm version, unsaved state, and document identifier.
- Each recovery component is written to a sibling temporary file, flushed, closed, and replaced with `MoveFileExW(..., MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)` on Windows.
- Startup discovery ignores incomplete, invalid, and saved-project recovery entries that are not newer than their original file, then offers the newest relevant entry.
- Restore loads the recovery content, preserves the known original path, retains the recovery association, and marks the document dirty. Discard removes recovery project and metadata files. Later leaves them intact.
- Successful Save and Save As remove the active recovery files, clear dirty state, update the saved-path association, and restart the autosave interval. If recovery deletion fails, VisiForm reports it and retains the old association for a later cleanup attempt. Failed saves retain dirty state and recovery data.

## Known Limitations

- This base phase offers the newest relevant recovery entry at startup rather than a multi-entry recovery manager.
- The autosave interval is a documented five-minute default and is not user-configurable.
- Recovery uses one generation per document; there is no history browser or journaling.

## Remaining TODOs

- Build the normal Windows Debug `VisiForm` target through an explicitly approved exact command.
- Manually launch VisiForm and complete the Phase 102 runtime checklist: version display, saved/unsaved autosave, unchanged main file, repeated safe replacement, Save and Save As cleanup, startup Restore/Discard/Later, invalid data handling, failed autosave dirty state, project replacement, shutdown behavior, and timer teardown.
