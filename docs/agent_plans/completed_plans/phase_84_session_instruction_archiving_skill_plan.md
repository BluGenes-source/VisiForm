# Phase 84 - Session Instruction Archiving Skill

## Goal

Add repository rules and a reusable Codex skill for archiving dated, completed
session instruction files from `session-instructions/` into
`session-instructions/old/`, while preserving `notes.txt` as the active
debugging notes file.

## Checklist

- [x] Create this phase plan.
- [x] Inspect current `session-instructions/` files and completion signals.
- [x] Update `AGENTS.md` with the new archiving rules.
- [x] Create a reusable skill for the session-instruction archiving process.
- [x] Move dated and completed instruction files into `session-instructions/old/`.
- [x] Record validation status and final summary.

## Validation Status

- Created and validated the reusable Codex skill at
  `C:\Users\Herb\.codex\skills\session-instruction-archiver` with
  `quick_validate.py`; validation reported `Skill is valid!`.
- Confirmed `session-instructions/notes.txt` remains in place.
- Confirmed pending or ambiguous prompt files remain active:
  `phase_81C_palette_button_tree_tests_prompt.txt` and
  `phase_81_export_build_and_ui_repair_prompt.txt`.
- Confirmed completed dated prompt files were moved into
  `session-instructions/old/`.
- Ran `git status --short` to confirm the repository sees the README/rule
  edits, phase plan, and moved prompt files.
- No build, app launch, generated app launch, build script, generated script,
  terminal build command, PowerShell build command, or `cmd.exe` build command
  was run.

## Remaining TODOs

- Commit or otherwise preserve this housekeeping change when ready.
- Continue to leave `notes.txt` active for reusable debugging notes.
- Archive `phase_81C_palette_button_tree_tests_prompt.txt` only after Phase 81C
  is completed.
- Archive `phase_81_export_build_and_ui_repair_prompt.txt` only if the broad
  Phase 81 plan is completed or superseded.

## Final Result Summary

- Added session-instruction accounting rules to `AGENTS.md`.
- Created the reusable `session-instruction-archiver` Codex skill.
- Updated `session-instructions/README.txt` so the active folder documents
  active prompts, the protected `notes.txt` file, and the `old/` archive.
- Moved completed dated prompts for phases 81A, 81B, 81E, and 81F into
  `session-instructions/old/`.
- Left `notes.txt`, `README.txt`, the pending Phase 81C prompt, and the broad
  pending Phase 81 prompt in `session-instructions/`.
