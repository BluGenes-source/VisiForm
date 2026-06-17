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

## Temporary Codex Runtime Constraint

Custom VisiForm subagents currently do not spawn reliably under GPT-5.5
in the Windows Codex desktop app.

For workflows using `.codex/agents/`, use GPT-5.4 until the known
child-model/service-tier validation issue is resolved.

Known error:

`spawn_agent could not resolve the child model for service tier validation`

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
- Inspect `git status --short --branch` before making changes.
- Do not discard unrelated user changes.
- Do not use destructive Git commands without explicit approval.
- Do not commit, push, tag, create releases, open pull requests, or rewrite
  history unless explicitly requested by the developer.

## Authoritative Project References

For non-trivial work, read references in this order:

1. `docs/project_status.md`
2. the active `docs/agent_plans/phase_N_<name>_plan.md`, when one exists
3. only the sections of `docs/VISIFORM_PROJECT_SPEC.md` relevant to the task
4. directly relevant source files, tests, build docs, and feature docs

Do not perform a general repository survey unless these references are
insufficient, inconsistent, or clearly stale.

Source code and tests take precedence when documentation is stale or
contradictory. Correct stale documentation within the task scope when practical.

## Scope And Stopping Rules

Stay within the requirements stated in the active prompt and phase plan.

- Do not fix unrelated defects.
- Do not refactor neighboring systems unless required for correctness.
- Do not update unrelated documentation.
- Record unrelated findings as follow-up items only.
- Prefer targeted file searches over repository-wide exploration.
- Inspect only the layers and files reasonably affected by the requested change.
- Prefer focused validation during implementation.
- Run complete required validation once after the focused changes are ready.
- Stop when the requested requirements, required documentation, and approved
  validation are complete.
- Do not continue polishing after the definition of done has been met.
- Ask the developer only when a required decision cannot be safely inferred.

## Lead-Agent Ownership

The primary Codex session is the lead agent. Only the lead agent may:

- define final task scope;
- create or select the official numbered phase;
- divide implementation assignments;
- authorize overlapping-scope work after assigning non-conflicting ownership;
- integrate results;
- update final checklist status;
- decide whether review findings are resolved;
- declare a phase complete;
- commit, push, or open a pull request when explicitly requested.

## Agent Usage Policy

Use the fewest agents needed to complete the task safely.

Default behavior:

- Use the lead agent only for focused bug fixes, UI adjustments, documentation
  changes, and changes confined to one subsystem.
- Do not spawn agents merely because a task has multiple requirements.
- Do not use a mandatory explorer, architect, validator, reviewer, and
  documenter sequence.

Use one specialist agent when:

- repository exploration is substantial;
- implementation ownership can be isolated;
- independent review is justified by risk;
- a focused task can be delegated without duplicate repository reading.

Use multiple agents only when:

- assignments are genuinely independent;
- file or subsystem ownership does not overlap;
- the task crosses major architectural layers;
- parallel work provides a clear benefit over sequential implementation.

Recommended maximum for normal phases:

1. lead agent;
2. one explorer or implementer;
3. optionally one reviewer for risky changes.

Use more agents only for unusually large, cross-layer, release-sensitive, or
high-risk work. Record why the additional agents are justified.

### Agent Responsibilities

- `visiform_explorer`: read-only, bounded repository investigation.
- `visiform_architect`: read-only design for substantial cross-layer work.
- `visiform_implementer`: bounded code changes and focused tests.
- `visiform_validator`: independent validation when risk justifies it.
- `visiform_reviewer`: read-only independent review for risky changes.
- `visiform_documenter`: documentation-only work when substantial documentation
  updates are needed.

### Parallel Work Rules

- Use explicit bounded assignments.
- Define explicit file or subsystem ownership.
- Do not allow concurrent edits to the same files.
- Do not allow concurrent edits to the official phase plan.
- Do not allow concurrent edits to `docs/VISIFORM_PROJECT_SPEC.md`.
- Run work in parallel only when assignments are genuinely independent.
- Use sequential work when shared headers, model types, schemas, central
  generator files, or other shared symbols overlap.

## Validation

Preferred validation is the Visual Studio workspace build pipeline for the main
`VisiForm` target, when available and unambiguous.

If that pipeline is unavailable, ambiguous, or appears to target a dependency
instead of `VisiForm`, stop and ask the developer to build manually.

When tests are relevant, prefer the existing CMake/Catch2 test target. Do not
invent a new test framework.

During implementation:

- use targeted checks when possible;
- avoid repeatedly running the complete build or full test suite;
- run final required validation once after the focused changes are ready;
- do not regenerate the CMake cache unless build configuration or source
  registration changed, or the developer explicitly requests it.

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

When changing behavior, first identify which layers are actually affected.
Use this list as an impact checklist, not as a requirement to inspect every
layer for every task:

- in-memory model;
- ownership and parent-child relationships;
- `.vfb.json` persistence;
- schema compatibility;
- validation;
- designer canvas;
- hierarchy and selection;
- property editing;
- undo/redo;
- generated C++;
- `USER CODE` preservation;
- CMake, presets, and scripts;
- generated-project build behavior;
- Windows behavior;
- macOS/Linux behavior only when relevant or verified.

## Generated Code Rules

- Preserve the generated base class name rule: `MainWindow`.
- Preserve `USER CODE BEGIN` / `USER CODE END` regions.
- Do not compile generated projects into the main `VisiForm` target.
- Do not delete arbitrary user files during export.
- Only overwrite known generated files.
- When changing export behavior, update the relevant docs.

## Widget Changes

When adding or changing widgets, inspect only the affected portions of:

- `WidgetRegistry`;
- `WidgetDefinition`;
- save/load behavior;
- validation;
- editor preview;
- property editing;
- code generation;
- relevant widget documentation.

Keep affected layers in sync. Do not inspect or modify unaffected layers merely
to satisfy the checklist.

## Documentation And Plans

Every multi-step VisiForm change requires a persistent phase plan in
`docs/agent_plans/`.

For small one-off fixes, a phase plan is not necessary unless the change grows
into multi-step work.

### Phase Number Selection

To determine the next phase number:

1. inspect filenames in `docs/agent_plans/`;
2. confirm the highest apparent number against `docs/project_status.md`;
3. use repository history only when those sources conflict or appear incomplete.

Do not search the full specification, changelog, and Git history solely to
determine a phase number.

Never reuse, renumber, or overwrite an existing phase document. If a requested
phase number already exists, automatically advance to the next unused number
and report the selected number before implementation.

Use the file name pattern:

`phase_N_<name>_plan.md`

### Phase Plan Contents

Include:

- scope;
- requirements;
- important architectural decisions;
- Markdown TODO checklist;
- validation plan;
- compatibility considerations when relevant;
- build/test status;
- final result summary;
- remaining TODOs.

Update the checklist as work progresses. Check off items only when there is
supporting evidence. Record validation status before finishing. Do not claim
completion while required validation is failing or unperformed.

Avoid duplicating large amounts of information already present in
`docs/project_status.md` or the project specification. Link or reference those
documents instead.

## Versioning

VisiForm versions use `Major.Minor.Build`.

- `Major`: incompatible architectural, project-format, or product-level
  changes.
- `Minor`: significant backward-compatible feature milestones.
- `Build`: completed phases containing fixes, improvements, or
  backward-compatible additions.

A phase uses the current version while it is implemented and validated. After
the phase builds successfully and is considered complete, increment `Build` by
one for the next phase, reset `Build` to `0` whenever `Major` or `Minor` is
intentionally incremented, update every authoritative version location
consistently, and record the old and new versions in the completed phase plan
and `docs/project_status.md`. Do not increment the version when required build
validation has not passed; for example, if Phase 94 completes successfully as
`1.0.0`, update the project to `1.0.1` for the next phase.

## Work Continuity

Keep the active phase plan usable as a handoff note. Before finishing
multi-step work, record:

- current branch and most recent relevant commit, if known;
- files changed or intentionally left untouched;
- completed work in plain language;
- remaining work;
- validation run or deferred;
- important manual testing notes.

When asked where development left off:

1. check `git status --short --branch`;
2. check `git log --oneline -n 8`;
3. inspect the newest applicable phase plan;
4. inspect `docs/project_status.md`;
5. distinguish confirmed repository state from inferred next steps.

Do not perform a broader repository survey unless these sources conflict or do
not answer the question.

## Session Instructions Accounting

Keep `session-instructions/` focused on active work.

- Move clearly dated and completed instruction files into
  `session-instructions/old/`.
- Do not move active, pending, or ambiguous files.
- Never move `session-instructions/notes.txt`.
- Keep `session-instructions/README.txt` in place unless explicitly asked to
  archive or rewrite it.
- Prefer the reusable `session-instruction-archiver` skill when available.
- Report moved files and any dated files intentionally left active.

Do not inspect the full session-instructions history unless the task concerns
archiving, prior instructions, or work continuity.

## Useful Agent Skills

Use relevant reusable skills when they reduce work or improve accuracy.
Skill usage must still follow this file's safety rules.

Do not scan for or load unrelated skills. Prefer only the skill directly
applicable to the current task.

## Local Files

- `CMakeUserPresets.json` is local-only and should not be committed.
- Avoid editing generated, build, cache, or IDE folders unless explicitly
  requested:
  - `build/`
  - `out/`
  - `.vs/`
  - `Generated/`

## Completion Reports

For normal phases, report:

- phase number and plan path;
- implementation summary;
- validation result;
- remaining TODOs;
- final Git status.

For major cross-layer, release-related, or high-risk phases, also report:

- agents used and assignments;
- files changed;
- exact validation commands;
- review findings and resolutions;
- specification/documentation updates;
- known limitations.

For small one-off fixes, give a concise summary of:

- what changed;
- validation status;
- any remaining issue;
- final Git status when repository changes were made.

## Before Finishing

Before reporting completion:

- confirm the intended files were changed;
- confirm required docs were updated;
- confirm validation was run through an approved path, or clearly state that it
  was deferred to the developer;
- do not claim a build passed unless it actually ran through the approved
  workflow;
- stop after the task definition of done has been met.
