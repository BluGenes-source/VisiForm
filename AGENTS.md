# AGENTS.md

## Project Overview

VisiForm is a C++20 / Visage UI form builder. It saves `.vfb.json` project
files and exports generated Visage-based C++ projects.

Primary development environment:

- Windows 10/11
- Visual Studio 2022
- CMake presets with Ninja
- vcpkg
- Static MSVC runtime
- Main executable target: `VisiForm`

## Safety Rules

- Do not rename the `VisiForm` target or the produced `VisiForm.exe`.
- Do not run `VisiForm.exe` from an automated agent.
- Do not launch generated apps from an automated agent.
- Do not use `Start-Process`, keyboard automation, or OS interaction to run
  the app.
- Do not run repository build scripts, generated build scripts, terminal build
  commands, PowerShell build commands, or `cmd.exe` build commands unless the
  developer explicitly asks for that exact command.
- Do not change CMake presets, vcpkg triplets, or
  `CMAKE_MSVC_RUNTIME_LIBRARY` unless explicitly requested.
- Never build or validate against `freetype.vcxproj`; the intended application
  target is `VisiForm`.

## Validation

Preferred validation is the Visual Studio workspace build pipeline for the main
`VisiForm` target, when available and unambiguous.

If that pipeline is unavailable, ambiguous, or appears to target a dependency
instead of `VisiForm`, stop and ask the developer to build manually.

When tests are relevant, prefer the existing CMake/Catch2 test target. Do not
invent a new test framework.

## Architecture Boundaries

Keep these layers separated:

- `src/ui/`: Visage UI code, editor interaction, canvas, palette, inspector,
  tree, resource preview.
- `src/model/`: project data model, widget definitions, layout data,
  registries.
- `src/serialization/`: `.vfb.json` read/write behavior.
- `src/generator/`: generated C++ project output.
- `src/validation/`: project validation rules.
- `src/commands/`: undo/redo and command system.
- `src/utils/`: shared non-UI utilities.

Model, serialization, validation, and generator code should not depend on
Visage UI headers.

## Generated Code Rules

- Preserve the generated base class name rule: `MainWindow`.
- Preserve `USER CODE BEGIN` / `USER CODE END` regions.
- Do not compile generated projects into the main `VisiForm` target.
- Do not delete arbitrary user files during export.
- Only overwrite known generated files.
- When changing export behavior, update the relevant docs.

## Widget Changes

When adding or changing widgets:

- Register widget types through `WidgetRegistry`.
- Update `WidgetDefinition` data as needed.
- Keep save/load, validation, editor preview, property editing, and code
  generation in sync.
- Update docs such as `docs/widget_catalog.md`, `docs/widget_registry.md`, or
  feature-specific docs when behavior changes.

## Documentation And Plans

For multi-step agent work:

- Create or update a persistent phase plan in `docs/agent_plans/`.
- Use the file name pattern `phase_N_<name>_plan.md`.
- Include a markdown checklist.
- Update the checklist as work progresses.
- Record validation status before finishing.
- Add a final result summary before claiming completion.
- Summarize remaining TODOs in the plan file.

For small one-off fixes, a phase plan is not necessary unless the change grows
into multi-step work.

## Work Continuity

Help the developer resume work after time away.

When doing multi-step work, keep the active phase plan in
`docs/agent_plans/` usable as a handoff note. Before finishing, make sure it
records:

- Current branch and most recent relevant commit, if known.
- Files changed or intentionally left untouched.
- What was completed in plain language.
- What still needs to be done next.
- Validation that was run, or validation that was deferred to the developer.
- Any important manual testing notes, especially UI behavior that agents must
  not validate by launching `VisiForm.exe`.

When asked where the developer left off, inspect the repository before
answering:

- Check `git status --short --branch`.
- Check the most recent commits with `git log --oneline -n 8`.
- Review the newest files in `docs/agent_plans/`, especially unchecked
  checklist items, "Remaining TODOs", "Validation Status", and "Final Result
  Summary" sections.
- Mention any untracked or modified files separately from committed work.
- Clearly distinguish confirmed repository state from inferred next steps.

For small one-off fixes that do not need a phase plan, leave enough context in
the final response for the developer to resume from the git status and commit
history alone.

## Session Instructions Accounting

Keep `session-instructions/` focused on active work.

- Move dated and completed session instruction files into
  `session-instructions/old/`.
- A session instruction file counts as dated when its name or contents clearly
  identify a dated phase, prompt, or completed work period.
- A session instruction file counts as completed when the matching phase plan
  or repository history shows the work has a final result summary, completed
  implementation notes, or an explicit developer/agent decision that no further
  active work remains for that prompt.
- Do not move active, pending, or ambiguous instruction files. Leave them in
  `session-instructions/` until completion is clear.
- Never move `session-instructions/notes.txt`; it is the reusable debugging
  notes file.
- Keep `session-instructions/README.txt` in place unless the developer
  explicitly asks to archive or rewrite it.
- When archiving instructions, mention which files moved and which dated files
  were intentionally left active.
- Prefer the reusable `session-instruction-archiver` skill when available.

## Useful Agent Skills

When an agent environment supports reusable skills, prefer skills that match
the work being done instead of relying only on general code editing.

Useful skill categories for this repository:

- C++ / CMake project navigation and build-system reasoning.
- CMake preset and vcpkg dependency troubleshooting.
- Visual Studio workspace diagnostics for the `VisiForm` target.
- GitHub pull request review, issue triage, and CI log analysis.
- Documentation maintenance for feature docs and phase plans.
- JSON schema or structured-data editing for `.vfb.json` project files.
- UI implementation review for Visage editor behavior.

Skill usage must still follow this file's safety rules. In particular, skills
must not launch `VisiForm.exe`, run generated apps, run build scripts, or change
CMake/vcpkg/static-runtime settings unless the developer explicitly requests it.

## Local Files

- `CMakeUserPresets.json` is local-only and should not be committed.
- Avoid editing generated, build, cache, or IDE folders unless explicitly
  requested:
  - `build/`
  - `out/`
  - `.vs/`
  - `Generated/`

## Before Finishing

Before reporting completion:

- Confirm the intended files were changed.
- Confirm docs were updated when behavior changed.
- Confirm validation was run through an approved path, or clearly say it was
  deferred to the developer.
- Do not claim a build passed unless it was actually run through the approved
  workflow.
