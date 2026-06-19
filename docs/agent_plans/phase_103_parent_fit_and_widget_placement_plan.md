# Phase 103 Parent Fit and Widget Placement Plan

## Scope

- Add visible Fit Width to Parent and Fit Height to Parent Layout commands.
- Extend the existing alignment commands to align one free-position widget within its direct parent.
- Use each selected widget's direct parent child-content bounds.
- Keep sizer-managed and dock-managed geometry protected.
- Trace and correct all active new-widget placement paths.
- Update authoritative version declarations from `1.0.8` to `1.0.9`.

## Requirements

- Use Phase 103 and version `1.0.9`.
- Use no subagents.
- Preserve model-space, parent-local widget geometry.
- Preserve selection and use one undo step per successful invocation.
- Create no undo history for rejected or no-op operations.
- Do not launch `VisiForm.exe` from an automated agent.
- Do not run a build command unless the developer explicitly authorizes that exact command.

## Version Notes

- Previous Phase 102 version: `1.0.8`.
- Phase 103 version: `1.0.9`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Architecture Decisions

- `LayoutEngine::clientBoundsForParent()` remains the shared source of direct-parent child-content bounds for the form, Panel, Frame, GroupBox, TabPage, Sizer, and other containers.
- Small model-only helpers in `WidgetPlacement.h` will clamp complete rectangles and produce width-only or height-only parent fits without UI dependencies.
- Parent-fit commands may process mixed-parent selections because each selected widget can be validated and fitted independently.
- With exactly one compatible selected widget, Align Left/Top/Right/Bottom and Center Horizontally/Vertically use that widget's direct parent child-content bounds and change position only.
- With two or more compatible sibling widgets selected, the Phase 100 primary-reference alignment behavior remains unchanged.
- Oversized single widgets align to the starting parent content edge on the affected axis, preserving size and avoiding negative overflow offsets.
- A parent-fit command is enabled only when every selected non-root widget is compatible and at least one selected widget would change.
- Single-widget alignment commands may remain enabled for no-op positions, but execution creates no undo entry.
- Direct children of Sizers and dock-managed widgets are incompatible; Phase 103 will not rewrite sizer metadata, detach widgets, or break dock layout.
- New-widget callers calculate parent-local bounds before constructing the undoable add command, and `ProjectDocument::addChildToParent()` now enforces the same direct-parent content-bound invariant at the model insertion boundary.

## Placement Trace

- Default form model bounds: `(0, 0, 900, 600)`.
- Root child-content bounds: `(0, 28, 900, 572)`.
- Default Button registry size: `260 x 56`.
- Default requested position: `(40, 40)`.
- Phase 101 helper result: `(40, 48, 260, 56)`.
- Startup and New Project both begin from `ProjectDocument::createDefault()`, but New Project replaces the root before applying its selected template.
- Palette click and Insert menu creation use `MainWindow::createDefaultWidget()` and `nextDefaultWidgetBounds()`.
- Canvas pointer paths convert through `DesignerCanvas::toFormPoint()` before parent targeting and reparenting.
- The checked-in startup and New Project default path already reaches the Phase 101 helper and stores the valid values above. Static inspection therefore does not reproduce invalid startup model coordinates.
- The confirmed permanent gap was below those call sites: `ProjectDocument::addChildToParent()` accepted unchecked free-position bounds from any caller, so fallback insertion, duplicate/undo re-add, and future creation paths could bypass safe placement. Canvas reparenting also converted form-space bounds to new-parent-local coordinates without clamping them to the new parent's content area.
- Phase 103 closes both gaps by enforcing placement in `addChildToParent()` and clamping reparented local bounds before the undoable reparent operation.

## TODO Checklist

- [x] Inspect branch, worktree, Phase 102 plan, Phase 101 placement findings, and targeted source.
- [x] Confirm Phase 103 is unused and select version `1.0.9`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations to `1.0.9`.
- [x] Record Phase 103 startup in `docs/project_status.md`.
- [x] Add shared placement and parent-fit geometry helpers with focused tests.
- [x] Add Fit Width to Parent and Fit Height to Parent registration, menu, enablement, and execution.
- [x] Integrate one-step undo/redo and selection/property synchronization.
- [x] Correct all active widget-creation placement paths and document the confirmed root cause.
- [x] Add single-widget direct-parent alignment while retaining multi-selection primary-reference behavior.
- [x] Add oversized-widget start-edge handling and focused model-helper tests.
- [x] Update alignment command enablement, hints, one-step undo/redo integration, and no-op handling.
- [x] Run focused static validation.
- [ ] Run the normal Windows Debug build once if the developer supplies or approves an exact command.
- [x] Record final validation, files changed, limitations, and remaining issues.

## Validation Plan

- Add focused Catch2 coverage for complete-bounds clamping and width-only/height-only fitting.
- Add focused Catch2 coverage for parent-edge/center alignment, axis preservation, and oversized-widget start-edge behavior.
- Inspect command registration, menu visibility, dynamic enablement, mixed-parent processing, and no-op behavior.
- Inspect single-selection parent alignment, retained multi-selection behavior, sizer restrictions, selection preservation, and Property Inspector refresh.
- Inspect startup, New Project, palette click, Insert menu, and canvas drag/reparent placement paths.
- Run `git diff --check`.
- Build the normal Windows Debug `VisiForm` target once only through an explicitly approved exact command.
- Do not perform automated runtime validation because repository rules prohibit launching `VisiForm.exe`.

## Compatibility

- No `.vfb.json` schema change.
- No generator behavior change.
- No ownership or parent-child model change.
- Existing Fit Text, matching, alignment, distribution, zoom/pan, Preview Mode, autosave, and recovery behavior remain separate.
- Existing multi-selection alignment semantics remain primary-reference based.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains user-owned session-instruction archive changes and a modified `session-instructions/notes.txt`; those changes are preserved.
- Focused static validation: `git diff --check` passed with line-ending normalization warnings only. Targeted source checks confirmed version consistency, command registration and mappings, menu visibility, Preview Mode blocking, parent-fit mixed-selection enablement, single-widget direct-parent alignment enablement, retained multi-selection compatibility checks, sizer/dock/TabPage restrictions, one `DocumentStateCommand` per successful operation, no-op handling, shared model helpers, model insertion clamping, canvas reparent clamping, and focused parent-edge/center/oversized alignment test coverage.
- Windows Debug build: pending an explicitly approved exact command.
- Manual runtime validation: not performed; automated agents may not launch `VisiForm.exe`.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_103_parent_fit_and_widget_placement_plan.md`
- `docs/layout_tools.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/commands/CommandIds.h`
- `src/commands/CommandRegistry.cpp`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetPlacement.h`
- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `tests/test_layout_engine.cpp`

## Final Result Summary

- Updated authoritative version declarations and current-progress documentation to `1.0.9` / Phase 103.
- Added visible Fit Width to Parent and Fit Height to Parent commands.
- Width fitting sets `x` and `width` from the direct parent's content bounds while preserving `y` and `height`.
- Height fitting sets `y` and `height` from the direct parent's content bounds while preserving `x` and `width`.
- Compatible mixed-parent selections fit independently in one undoable document-state command and preserve selection.
- Commands are unavailable for direct Sizer children, dock-managed widgets, layout-owned TabPages, invalid/minimum-incompatible parent areas, Preview Mode, and complete no-op selections.
- Consolidated complete-bounds clamping in `WidgetPlacement.h`, enforced it at `ProjectDocument::addChildToParent()`, and applied it after canvas coordinates are converted to a new parent's local space.
- Added focused Catch2 cases for parent fitting, complete-bounds clamping, direct-parent insertion clamping, and the existing default-project model coordinates.
- Existing alignment commands now align one compatible widget within its direct parent's usable child-content rectangle while preserving size and the unaffected position axis.
- Multi-selection alignment continues to use the primary selected widget as the unchanged reference.
- Oversized widgets use the starting content edge on the affected axis; Sizer-managed, dock-managed, layout-owned TabPage, root, empty, invalid-parent, and Preview Mode selections remain unavailable.
- Successful single-widget alignment is one undoable document-state change, preserves selection, refreshes Property Inspector bounds, and creates no history entry for a no-op.

## Known Limitations

- The reported visual startup/New Project symptom could not be runtime-confirmed because automated agents may not launch VisiForm. The current checked-in model path statically produces valid default Button coordinates.
- Fit commands intentionally do not rewrite Sizer item metadata or dock layout.
- Single-widget alignment intentionally does not resize oversized widgets or rewrite Sizer/dock layout ownership.

## Remaining TODOs

- Build the normal Windows Debug `VisiForm` target through an explicitly approved exact command.
- Manually launch VisiForm and complete the Phase 103 runtime checklist, including nested parent insets, undo/redo, mixed parents, Preview Mode, startup/New Project, zoomed and panned creation, drag/drop edges, save/reload, and autosave interaction.
- Manually verify single-widget alignment in the form, Panel, Frame, GroupBox, and tab page, including captions/padding, oversized widgets, all six undo/redo paths, sizer disablement, retained multi-selection behavior, Preview Mode, and save/reload.
