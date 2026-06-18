# Phase 99 Canvas Zoom and Pan Plan

## Scope

- Add editor-only zoom and pan to the Designer Canvas in Design and Preview modes.
- Replace implicit fit-only canvas scaling with one shared model/view viewport transform.
- Add shared Zoom In, Zoom Out, Reset to 100%, and Fit Form to Canvas commands.
- Preserve model coordinates, serialization, validation, and generated output.
- Update authoritative version declarations from `1.0.4` to `1.0.5`.

## Requirements

- Use Phase 99 and version `1.0.5`.
- Use no subagents.
- Support 25% through 400% zoom with practical preset increments.
- Anchor mouse-wheel zoom at the pointer and command zoom at the visible canvas center.
- Support middle-button drag and Space + left-button drag panning.
- Keep zoom and pan unchanged across Design/Preview transitions.
- Do not persist viewport state in `.vfb.json`.
- Do not launch `VisiForm.exe` from an automated agent.
- Build only through an explicitly approved exact command.

## Version Notes

- Previous Phase 98 version: `1.0.4`.
- Phase 99 version: `1.0.5`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Architecture Decisions

- Viewport state belongs to `DesignerCanvas` and consists of an explicit zoom scale plus a view-space pan offset.
- Model-space widget bounds remain unchanged. All drawing and pointer input pass through shared model-to-view and view-to-model helpers.
- The existing form-preview layout remains responsible for the available viewport rectangle; the viewport transform positions the model-space form within it.
- Selection handles remain approximately constant in screen pixels while widget geometry, grid spacing, and guides scale with zoom.
- Grid snapping and smart-guide calculations remain in model space.
- MainWindow owns command dispatch and pan gesture state; `DesignerCanvas` owns transform math and viewport limits.
- Zoom and pan are active-document session state only. New/open project transitions reset to a sensible fitted or centered view; Design/Preview transitions preserve state.

## Zoom Policy

- Supported presets: 25%, 33%, 50%, 67%, 75%, 100%, 125%, 150%, 200%, 300%, and 400%.
- Minimum zoom: 25%.
- Maximum zoom: 400%.
- Mouse-wheel zoom anchors the model-space point under the pointer.
- Toolbar/menu commands anchor at the visible canvas center.
- Reset selects 100% and recenters the form when needed.
- Fit computes a margin-aware aspect-preserving scale and centers the form.

## Pan Policy

- Middle-button drag pans immediately.
- Holding Space and dragging with the left button pans without selecting, creating, moving, or resizing widgets.
- Releasing the initiating button, losing the active gesture, or leaving the canvas ends panning safely.
- Space alone does not modify the document.

## TODO Checklist

- [x] Inspect branch and worktree.
- [x] Read the Phase 99 brief, project status, Phase 98 plan, and targeted version declarations.
- [x] Confirm Phase 99 is unused and select version `1.0.5`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations to `1.0.5`.
- [x] Record Phase 99 startup in `docs/project_status.md`.
- [x] Add shared viewport transform and conversion helpers.
- [x] Apply the transform to widget rendering, grid, guides, selection, handles, and hit testing.
- [x] Add zoom commands, shortcuts, toolbar/menu controls, and percentage display.
- [x] Add pointer-centered wheel zoom and center-anchored command zoom.
- [x] Add middle-button and Space + left-drag panning.
- [x] Preserve model-space drag, resize, drop, reparenting, snapping, and marquee behavior.
- [x] Preserve viewport state across Design/Preview transitions and reset it for project replacement.
- [x] Run focused static validation.
- [ ] Run the approved Windows Debug build once, if an exact command is available.
- [x] Record final validation, changed files, known limitations, and remaining issues.

## Validation Plan

- Run focused source checks for transform usage, command registration, and input routing.
- Run `git diff --check`.
- Build the normal Windows Debug `VisiForm` target once only if an exact developer-approved command is available.
- Do not perform automated runtime validation because repository rules prohibit launching `VisiForm.exe`.
- Record the developer manual checklist as deferred unless the developer performs it.

## Compatibility

- No `.vfb.json` schema or persistence change.
- No model, serialization, validation, generator, or undo-stack format change.
- Viewport state is editor-only and non-persistent.
- Preview Mode shares the same viewport state as Design Mode.

## Build / Test Status

- Branch: `main`.
- Most recent relevant commit before Phase 99 changes: `104e28e Add Designer Preview Mode and version bump to 1.0.4`.
- Starting worktree contains the Phase 98 session-instruction archive move and the untracked Phase 99 instruction file; they are preserved outside implementation scope.
- Focused static validation: `git diff --check` passed with line-ending normalization warnings only. Targeted searches confirmed all four zoom commands are registered and routed through shared command dispatch; every canvas layout calculation uses the shared zoom/pan state; Ctrl-wheel zoom is pointer anchored; command zoom uses the visible viewport center; pan input preempts editing; project replacement resets only viewport state; and no zoom/pan fields were added to model, serialization, validation, or generator layers.
- Windows Debug build: pending an approved exact command.
- Manual runtime validation: not performed; automated agents may not launch `VisiForm.exe`.

## Files Changed

- `docs/agent_plans/phase_99_canvas_zoom_pan_plan.md`
- `CMakeLists.txt`
- `README.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/commands/CommandIds.h`
- `src/commands/CommandRegistry.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`

## Final Result Summary

- Added one Designer Canvas viewport transform with 25%-400% zoom, screen-pixel pan offsets, model/view point and rectangle conversion helpers, and viewport clipping.
- Added Zoom In, Zoom Out, Reset to 100%, and Fit Form to Canvas through the command registry, View menu, toolbar, configurable shortcut path, and status-bar percentage display.
- Added pointer-centered `Ctrl + mouse wheel` zoom and visible-center anchoring for menu, toolbar, and shortcut commands.
- Added middle-button drag and Space + left-drag panning. Pan gestures preempt selection, creation, movement, resizing, and Preview Mode read-only handling.
- Added clean pan cancellation on button/Space release, canvas exit, mouse exit, and keyboard-focus loss.
- Reused the viewport transform for rendering, hit testing, tab headers, selection handles, marquee, guides, grid scaling, drag movement, resize movement, and container targeting.
- Kept drag, resize, snapping, guides, and undo/redo calculations in model space. Zoom and pan do not mutate widget bounds or project data.
- Preserved zoom and pan across Design/Preview transitions. New and loaded projects reset to a centered 100% view.
- Added viewport clipping so zoomed or panned form content cannot paint into adjacent editor panels.
- Extended shortcut parsing/formatting for `-` and `=` keys to support the default `Ctrl+-` and `Ctrl+=` zoom shortcuts.
- Updated the current-progress README entry and authoritative version declarations to `1.0.5`.
- Known limitations: viewport state is per active document only for the current session and is not persisted; there is no mini-map, ruler, touch gesture, animation, device preset, or multi-document viewport support; runtime usability at extreme zoom levels requires developer manual validation.

## Remaining TODOs

- Build the normal Windows Debug `VisiForm` target through an explicitly approved exact command.
- Manually launch VisiForm and complete the Phase 99 runtime checklist, including zoom anchoring, both pan gestures, selection/resize/drop at several zoom levels, nested containers and sizers, snapping/guides, Preview Mode preservation, splitter/window resizing, min/max zoom, and save/reload/export model-integrity checks.
