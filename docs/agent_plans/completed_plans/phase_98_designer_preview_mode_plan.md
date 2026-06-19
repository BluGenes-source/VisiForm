# Phase 98 Designer Preview Mode Plan

## Scope

- Add a shared Designer Canvas mode with explicit `Design` and `Preview` states.
- Add one Preview command to the existing command registry, View menu, toolbar, and shortcut system.
- Reuse the current Designer Canvas widget rendering while hiding editor-only decorations in Preview Mode.
- Block project-changing editor input and commands while Preview Mode is active.
- Preserve selection and editor settings without persisting preview state.
- Update authoritative version declarations from `1.0.3` to `1.0.4`.

## Requirements

- Use Phase 98 and version `1.0.4`.
- Use no subagents.
- Do not change `.vfb.json`, serialization, generated C++, model ownership, or user-code preservation.
- Do not launch `VisiForm.exe` from the agent.
- Build the normal Windows Debug `VisiForm` target once after implementation only through an explicitly approved command.

## Version Notes

- Previous Phase 97 version: `1.0.3`.
- Phase 98 version: `1.0.4`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Architecture Decisions

- Canvas mode: `DesignerCanvas::Mode` is the single explicit Design/Preview state.
- Rendering: Preview Mode calls the existing recursive widget renderer with editor decorations disabled; widget rendering is not duplicated.
- Input: MainWindow keeps menus, the Preview toggle, file operations, export, validation, and workspace splitters available while blocking palette, Project Tree, Property Inspector, and Designer Canvas editing input.
- Command state: project-mutating editor and layout commands are disabled in Preview Mode. Undo and Redo retain their existing availability as required.
- Selection: the document selection is not changed when entering or leaving Preview Mode, so it naturally returns when Design Mode resumes.
- Settings: grid, snap, guide, and multi-select preferences are not modified by the mode transition.
- Preview interaction policy: widgets are visual-only in this base iteration. No generated handlers execute and no temporary model state is created.
- Temporary state: no preview-only widget state is introduced; leaving Preview Mode only restores normal input and decoration rendering.
- Project transitions: new/open/load paths return to Design Mode before replacing or editing the current document.

## TODO Checklist

- [x] Inspect branch and worktree.
- [x] Read the Phase 98 brief, project status, Phase 97 plan, and directly related files.
- [x] Confirm Phase 98 is unused and select version `1.0.4`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations to `1.0.4`.
- [x] Record Phase 98 startup in `docs/project_status.md`.
- [x] Add the shared Designer Canvas mode and preview rendering policy.
- [x] Add the Preview registry command, View menu item, toolbar button, checked state, and Escape handling.
- [x] Block editing commands and project-changing editor input while previewing.
- [x] Ensure project transitions leave Preview Mode safely.
- [x] Update README progress if its current-progress section remains applicable.
- [x] Run focused static validation.
- [ ] Run the approved Windows Debug build once, if an exact command is available.
- [x] Record final validation, changed files, known limitations, and remaining issues.

## Validation Plan

- Run focused static checks, including `git diff --check`.
- Verify command registration, checked/enabled state, and input guards with targeted source searches.
- Build the normal Windows Debug `VisiForm` target once only if an exact developer-approved command is available.
- Do not perform automated runtime validation because repository rules prohibit launching `VisiForm.exe`.
- Record the manual runtime checklist as deferred unless the developer performs it.

## Compatibility

- No `.vfb.json` schema or persistence change.
- No model, serialization, validation, generator, or command-stack format change.
- Preview state is editor-only and non-persistent.
- Existing grid, snap, smart-guide, selection, and splitter settings remain unchanged.

## Build / Test Status

- Branch: `main`.
- Most recent relevant commit before Phase 98 changes: `51b6ad7 Update to version 1.0.3 with UI and GroupBox fixes`.
- Starting worktree contained pre-existing Phase 97 session-instruction archive moves and the untracked Phase 98 instruction file; they are preserved outside implementation scope.
- Focused static validation: `git diff --check` passed with line-ending normalization warnings only. Targeted searches confirmed the Preview command is registered once and mapped through the existing menu, toolbar, checked state, shortcut, and execution paths; the Designer Canvas uses one explicit mode; editor decorations are gated together; and preview input guards precede project-changing Inspector, Project Tree, and canvas behavior.
- Windows Debug build: pending an approved exact command.
- Manual runtime validation: not performed because automated agents may not launch `VisiForm.exe`.

## Files Changed

- `docs/agent_plans/phase_98_designer_preview_mode_plan.md`
- `CMakeLists.txt`
- `src/app/Version.h`
- `docs/versioning.md`
- `docs/project_status.md`
- `README.md`
- `src/commands/CommandIds.h`
- `src/commands/CommandRegistry.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`

## Final Result Summary

- Added `DesignerCanvas::Mode` with explicit `Design` and `Preview` states.
- Added one Preview command through the existing command registry, View menu, toolbar, checked state, hint system, and configurable keyboard shortcut infrastructure. The default shortcut is F5.
- Preview Mode expands the form into the canvas workspace and suppresses the canvas header, preview frame, grid, selection outlines, resize handles, marquee, smart guides, and selected-widget label.
- The existing recursive widget renderer remains the only widget drawing path.
- Palette creation, Insert menu items, Property Inspector edits, Project Tree selection, canvas selection/drag/resize/reparenting, nudging, layout tools, grid/snap/guide controls, and project settings/resource mutations are blocked while Preview Mode is active.
- Save, load, export, validation, Copy, Undo, and Redo retain their existing command behavior. New/open/load transitions return to Design Mode.
- Entering Preview Mode cancels active inline editing and canvas interaction, clears hover state, preserves selection and user settings, and repaints cleanly.
- Escape, F5, the View menu item, or the toolbar button returns to Design Mode.
- Preview widgets are visual-only. No generated handlers execute, no temporary widget state is stored, and no preview state is serialized.
- Known runtime-parity limitations: current designer widget visuals remain approximate; hover/pressed/checked/slider simulation, advanced shadows/depth, runtime themes, animation, and exact generated-app parity are deferred.

## Remaining TODOs

- Build the normal Windows Debug `VisiForm` target through an explicitly approved exact command.
- Manually launch VisiForm and complete the Phase 98 checklist, including repeated toggles, Escape, selection/grid restoration, blocked editing, representative widgets, save/reload, undo-history stability, and project replacement.
