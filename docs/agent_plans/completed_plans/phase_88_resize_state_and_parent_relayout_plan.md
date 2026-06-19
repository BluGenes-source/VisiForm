# Phase 88 - Resize State And Parent Relayout Repair

## Scope

Repair the confirmed editor resize regressions affecting repeated resizing of
widgets inside a `Sizer` and child relayout after resizing a parent container.
Also add a permanent shared rule for phase-number selection.

## Requirements

- Add a permanent shared agent rule that phase numbers must be discovered from
  repository evidence instead of assumed from prompts.
- Keep this work on the next unused phase number after inspecting existing
  phase records.
- Fix repeated resize interactions for direct children of a `Sizer`.
- Preserve intentional `sizerItem.minimumWidth` and
  `sizerItem.minimumHeight` behavior.
- Make parent resize operations relayout affected children, including nested
  sizers.
- Keep serialization, validation, generator behavior, and documentation
  consistent with the repaired model.
- Avoid unrelated refactoring.

## Existing Behavior Findings

- Highest existing numbered phase evidence before this work was `87`, so this
  task uses Phase `88`.
- `AGENTS.md` already required inspecting existing plans, but it did not forbid
  trusting a prompt-provided phase number or require advancing when a requested
  number was already taken.
- `MainWindow::applySizerItemResizePreview(...)` currently persists direct
  sizer-child resize gestures by writing `sizerItem.minimumWidth` and
  `sizerItem.minimumHeight`.
- That behavior turns an interactive resize into a minimum-size ratchet, so a
  completed drag can prevent later smaller resize operations.
- Live relayout during mouse resize is currently special-cased for resizing a
  `Sizer` itself, but not for other resized parent containers whose children
  depend on dock, anchor, or nested sizer layout.
- Property edits already use `ProjectDocument::applyLayoutFromPrevious(...)`,
  which shows the model already has the correct relayout primitive.

## Reproduction Results

- Bug 1 reproduced from source behavior: direct sizer-child resize preview wrote
  the drag result into `sizerItem.minimumWidth` and
  `sizerItem.minimumHeight`, which permanently tightened later resize attempts
  instead of treating each drag as a fresh preferred-size request.
- Bug 2 reproduced from source behavior: mouse-driven parent resize only ran
  `applyLayoutFromPrevious(...)` for widgets whose type was `Sizer`, leaving
  other resized parent containers without a descendant relayout pass.

## Root Cause

- Bug 1: direct sizer-child resize preview stores temporary drag results in the
  permanent minimum-size properties instead of a reversible preferred-size
  request, so repeated drags become constrained by the previous drag result.
- Bug 2: the mouse-driven resize path only reapplies layout for resized
  `Sizer`s, so other parent containers can change bounds without invalidating
  anchored, docked, or nested child layout.
- Shared cause: the editor resize workflow does not consistently distinguish
  persistent preferred geometry from minimum constraints, and it does not
  consistently relayout from a before/after document snapshot when parent
  bounds change.

## Design Decision

- Store direct sizer-child interactive resize intent as preferred
  width/height overrides distinct from minimum-size overrides.
- Continue treating minimum-size overrides as explicit floors.
- Reuse `ProjectDocument::applyLayoutFromPrevious(...)` for live preview and
  undoable completion of parent resize operations that affect descendants.
- Keep the fix architecture-consistent by updating model layout helpers,
  editor interaction code, validation, generated runtime metadata, tests, and
  documentation together.

## Files Expected To Change

- `AGENTS.md`
- `src/model/BoxSizerLayout.h`
- `src/model/BoxSizerLayout.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.cpp`
- `src/validation/ProjectValidator.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `tests/CMakeLists.txt`
- `tests/test_box_sizer_layout.cpp`
- `tests/test_layout_engine.cpp`
- `docs/layout_tools.md`
- `docs/project_file_format.md`
- `docs/VISIFORM_PROJECT_SPEC.md`
- this phase plan

## TODO Checklist

- [x] Select the next unused phase number from repository evidence.
- [x] Add the permanent shared phase-number rule.
- [x] Confirm the source-level root cause for both resize bugs.
- [x] Implement preferred-size persistence for direct sizer-child resize.
- [x] Implement parent-resize relayout propagation for interactive resize.
- [x] Add focused regression tests for repeated sizer-child resize behavior.
- [x] Add focused regression tests for parent-resize relayout propagation.
- [x] Update docs and specification text to match verified behavior.
- [x] Run allowed static validation and record results.
- [x] Add final result summary and remaining TODOs before completion.

## Validation Plan

- Preferred allowed static check: `git diff --check`
- Build and automated test execution: defer to the developer unless the
  approved Visual Studio workspace pipeline is explicitly provided.
- Manual UI verification required from the developer because agents must not
  launch `VisiForm.exe`.

## Compatibility Considerations

- Keep `.vfb.json` schema version at `1`.
- Additive sizer-item preferred-size properties should round-trip through the
  existing property map without requiring a schema bump.
- Existing projects without the new preferred-size properties must continue to
  load with current behavior.
- Existing explicit minimum-size overrides must keep their current meaning.

## Build And Test Status

- `git diff --check` passed.
- Automated build and test execution not run because AGENTS.md prohibits
  terminal build/test commands unless the developer explicitly asks for the
  exact command, and no approved Visual Studio workspace build pipeline was
  available through the tool set.

## Work Continuity

- Branch: `main`
- Relevant recent commit before this phase: `9156797 feat: add multi-agent workflow and BoxSizer updates`
- Unrelated worktree changes already exist in `session-instructions/` and must
  remain untouched by this phase except for the new Phase 88 prompt file.
- Files changed in this phase: `AGENTS.md`,
  `docs/VISIFORM_PROJECT_SPEC.md`, `docs/layout_tools.md`,
  `docs/project_file_format.md`, `docs/widget_catalog.md`,
  `src/generator/VisageCppEmitter.cpp`, `src/model/BoxSizerLayout.cpp`,
  `src/model/BoxSizerLayout.h`, `src/ui/MainWindow.cpp`,
  `src/ui/MainWindow.h`, `src/ui/PropertyInspector.cpp`,
  `src/validation/ProjectValidator.cpp`, `tests/CMakeLists.txt`,
  `tests/test_box_sizer_layout.cpp`, `tests/test_layout_engine.cpp`,
  `tests/test_project_validation.cpp`, and this phase plan.
- Intentionally left untouched: unrelated archived `session-instructions/*`
  moves already present in the worktree, generated folders, build folders, and
  export scripts.

## Remaining TODOs

- Developer should build the main `VisiForm` target through the approved Visual
  Studio workflow.
- Developer should manually verify repeated direct sizer-child resize gestures,
  parent-container resize relayout, nested sizer behavior, and save/reopen
  behavior in the live editor because the agent must not launch
  `VisiForm.exe`.

## Final Result Summary

- Added a stronger shared phase-number selection rule to `AGENTS.md` so future
  VisiForm phases must be chosen from repository evidence instead of trusting a
  prompt.
- Repaired direct sizer-child resize persistence by storing drag results as
  preferred-size overrides instead of rewriting minimum-size constraints.
- Repaired interactive parent resize relayout by applying the document
  before/after relayout path to resized parent containers, not only to
  `Sizer`s.
- Added focused regression tests for repeated sizer-child preferred-size
  behavior, JSON round-trip of preferred sizes, invalid preferred-size
  validation, anchored child relayout, and nested sizer relayout from parent
  resize.
