# Phase 97 Project Tree / Designer Canvas Splitter Plan

## Scope

- Add a draggable vertical splitter between the Project Tree and Designer Canvas.
- Reuse the existing `ui::Splitter` control and preserve the independent Designer Canvas / Property Inspector splitter.
- Persist the Project Tree width through the existing application settings store.
- Preserve Phase 96 Project Tree hierarchy, selection, scrolling, hover hints, and update behavior.
- Update authoritative version declarations from `1.0.2` to `1.0.3`.
- Update current-progress documentation in `README.md` and `docs/project_status.md`.

## Requirements

- Use Phase 97 and version `1.0.3`.
- Use no subagents.
- Keep workspace order: Project Tree, new splitter, Designer Canvas, existing splitter, Property Inspector.
- Enforce practical minimum widths for the Project Tree and Designer Canvas.
- Keep the existing Property Inspector width and splitter behavior independent.
- Do not launch `VisiForm.exe`.
- Do not run a build command without an exact developer request.

## Version Notes

- Previous Phase 96 version: `1.0.2`.
- Phase 97 version: `1.0.3`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Architecture Decisions

- Splitter implementation: reuse `visiform::ui::Splitter`; do not add custom mouse-drag math.
- Composition: the new outer splitter divides Project Tree from the existing canvas/inspector region. The existing inner splitter continues to divide Designer Canvas from Property Inspector.
- Minimum-width policy: Project Tree minimum is 180 px; the outer splitter reserves the existing 320 px Designer Canvas minimum plus the 386 px Property Inspector minimum and the inner 6 px divider.
- Startup sizing: when no valid persisted width exists, the Project Tree starts at its content-derived preferred width rather than a fixed screenshot-derived width.
- Persistence: store Project Tree width in `AppSettings`, matching the existing Property Inspector width persistence.
- Narrow-window policy: hide the Project Tree when the complete two-splitter workspace cannot satisfy practical minimum widths; the canvas/inspector layout remains usable.
- Drag routing: while either splitter owns a drag, mouse movement remains captured by that splitter even when the divider is clamped at an endpoint.
- Width measurement formula: `outer padding + hierarchy indentation + expander/control slot + control/label gap + optional icon slot + measured full "name : type" text + right padding + vertical-scrollbar allowance + readability buffer`. The current Project Tree renders no row icons, so the optional icon contribution is currently zero.
- Minimum-width policy: use 180 px or the measured `Project Tree` title plus normal header padding, whichever is larger.
- Readability buffer: add 20 px after the complete rendered row content.
- Maximum-width policy: cap drag width at preferred width plus a 16 px drag margin, then apply the workspace safety cap that preserves the 320 px Designer Canvas minimum, 386 px Property Inspector minimum, and inner splitter.
- Visible-row policy: measure the project root and widget rows exposed by current expansion state; collapsed descendants do not contribute.
- User-width policy: preserve a valid saved or user-selected width while it remains inside recalculated limits. Longer visible rows increase the available maximum without automatically widening the panel; shorter content clamps only widths now outside the maximum.
- Recalculation policy: update limits for branch expansion/collapse, document load/new project, project rename, widget add/delete/duplicate/paste/rename/reparent, undo/redo, font-backed initial layout, and workspace resize. Width measurement is not performed on every paint.

## TODO Checklist

- [x] Inspect branch and worktree.
- [x] Read the Phase 97 brief, project status, Phase 96 plan, README, and directly related files.
- [x] Confirm Phase 97 is unused and select version `1.0.3`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations to `1.0.3`.
- [x] Record Phase 97 startup in `docs/project_status.md`.
- [x] Add persisted Project Tree width to `AppSettings`.
- [x] Integrate the new Project Tree / canvas splitter into layout, drawing, cursor, and mouse routing.
- [x] Preserve the independent canvas / Property Inspector splitter behavior.
- [x] Update README progress.
- [x] Run focused static validation.
- [x] Record build/manual validation status, changed files, final summary, and remaining issues.
- [x] Measure expanded Project Tree rows with the active font metrics.
- [x] Replace the broad right-drag range with content-derived preferred and maximum widths.
- [x] Preserve valid persisted/user-selected widths without automatic expansion.
- [x] Recalculate width limits on relevant hierarchy, label, load, and workspace events.
- [x] Re-run focused static validation for the adjustment pass.

## Validation Plan

- Run focused static checks, including `git diff --check`.
- Review layout and event routing for both splitter instances.
- Defer the Windows Debug build until the developer supplies or approves an exact command.
- Do not perform automated runtime validation because repository rules prohibit launching `VisiForm.exe`.

## Compatibility

- No `.vfb.json` schema change.
- No model, serialization, validation, generator, or command-system change.
- Older settings files fall back to the new default Project Tree width.
- Property Inspector width persistence remains unchanged.

## Build / Test Status

- Branch: `main`.
- Most recent relevant commit before Phase 97 changes: `1ed3c49 Improve Project Tree layout, hover hints, and hierarchy`.
- Starting worktree contained pre-existing session-instruction archive changes and the Phase 97 prompt; they were preserved and not modified as part of implementation.
- Static validation: `git diff --check` passed with line-ending normalization warnings only.
- Targeted searches confirmed all authoritative version declarations use `1.0.3`, the old fixed Project Tree width constant is removed, both splitter instances have independent event routing, `projectTreeWidth` is wired through settings load/save, and Project Tree limits use actual font measurements plus the documented buffer and safety cap.
- Automated tests: not run because running the test target requires an unapproved build command.
- Windows Debug build: deferred because the adjustment prompt requested the normal Debug build but did not supply the exact command required by `AGENTS.md`.
- Manual runtime validation: not performed because automated agents may not launch `VisiForm.exe`.

## Files Changed

- `docs/agent_plans/phase_97_project_tree_canvas_splitter_plan.md`
- `CMakeLists.txt`
- `src/app/Version.h`
- `docs/versioning.md`
- `docs/project_status.md`
- `README.md`
- `docs/settings.md`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`

## Final Result Summary

- Added a second reusable vertical `Splitter` instance between the Project Tree and the existing canvas/inspector workspace.
- The workspace order is Project Tree, new divider, Designer Canvas, existing divider, and Property Inspector.
- Dragging the new divider updates the Project Tree width live; the Designer Canvas absorbs the neighboring width change while the Property Inspector retains its independent persisted width.
- The Project Tree cannot shrink below 180 px, and the outer workspace reserve prevents the Designer Canvas or Property Inspector from collapsing below their existing minimums.
- With no saved width, the Project Tree defaults to the measured longest expanded row plus a 20 px readability buffer; valid saved widths are restored from `settings.json`.
- The rightward drag limit is the content-derived preferred width plus a 16 px margin, capped again to preserve the Designer Canvas and Property Inspector minimum workspace.
- Expanded rows are measured with the active Project Tree font, including hierarchy indentation, expander spacing, full `name : type` label text, panel padding, and scrollbar allowance. Collapsed descendants do not affect the limit.
- A longer row raises the drag limit without forcing the panel wider. A shorter hierarchy preserves the user width unless it exceeds the new valid maximum.
- The horizontal-resize cursor is shown for either splitter, and splitter drags retain event ownership at clamped endpoints.
- Narrow windows hide the Project Tree when all practical pane minimums cannot fit, avoiding collapsed or overlapping panels.
- README now reports version `1.0.3`, Phase 97, and the recent palette/tree/splitter UI progress.

## Remaining TODOs

- Build the normal Windows Debug `VisiForm` target through an explicitly approved command.
- Manually launch VisiForm and complete the Phase 97 runtime checklist, including maximize/restore, both splitters, tree interactions, and visual overlap/clipping checks.
