# Phase 101 Size Matching and Safe Placement Plan

## Scope

- Expand Same Width and Same Height into visible primary, smallest, and largest reference modes.
- Reuse the existing Phase 100 compatible-selection and undoable document-change path.
- Keep initial and palette-created widgets fully inside the intended parent client bounds using model-space geometry.
- Update authoritative version declarations from `1.0.6` to `1.0.7`.

## Requirements

- Use Phase 101 and version `1.0.7`.
- Use no subagents.
- Preserve Phase 100 alignment, distribution, selection, and undo behavior.
- Do not launch `VisiForm.exe` from an automated agent.
- Do not run a build command unless the developer explicitly authorizes that exact command.

## Version Notes

- Previous Phase 100 version: `1.0.6`.
- Phase 101 version: `1.0.7`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Sizing Command Structure

- The current custom menu system has flat menu items and no submenu interaction model.
- Use six clearly named visible Layout commands:
  - Same Width: Match Primary
  - Same Width: Match Smallest
  - Same Width: Match Largest
  - Same Height: Match Primary
  - Same Height: Match Smallest
  - Same Height: Match Largest
- Keep the existing primary command IDs for backward-compatible settings and add distinct IDs for smallest and largest modes.

## Size Reference Rules

- Primary uses `ProjectDocument::selectedWidgetId`.
- Smallest uses the minimum selected model-space width or height.
- Largest uses the maximum selected model-space width or height.
- Commands require at least two selected non-root widgets with one direct parent.
- Direct Sizer children and dock-managed widgets remain protected.
- The unaffected dimension and widget positions remain unchanged.
- Per-widget minimum dimensions remain enforced.
- A successful action creates one `DocumentStateCommand`; a no-op creates no history entry and does not dirty the project.

## Safe Placement Design

- Add one small model-space helper accepting parent client bounds, desired size, preferred position, and edge inset.
- Clamp the complete widget bounds inside the parent where possible.
- If the widget is larger than the inset-adjusted available area, preserve its valid default/minimum size and place it at the inset rather than creating negative coordinates.
- Use the helper for the default project Button, wizard-template widgets, and palette click-to-create paths.
- Continue using `LayoutEngine::clientBoundsForParent()` so Frame, GroupBox, TabPage, Panel, and root-form content origins remain correct.
- Palette click creation remains model-only and therefore independent of Designer Canvas zoom and pan.
- Existing canvas drag/reparent behavior continues to convert pointer positions through `DesignerCanvas::toFormPoint()` and retains its current explicit-user-position clamping path.

## TODO Checklist

- [x] Inspect branch and worktree.
- [x] Read the Phase 101 brief, project status, Phase 100 plan, and targeted source files.
- [x] Confirm Phase 101 is unused and select version `1.0.7`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations to `1.0.7`.
- [x] Record Phase 101 startup in `docs/project_status.md`.
- [x] Add visible primary, smallest, and largest width/height commands.
- [x] Reuse one sizing implementation with compatible-selection, minimum-size, no-op, undo, and redo behavior.
- [x] Add a shared model-space safe-placement helper.
- [x] Apply safe placement to default-project, wizard-template, and palette creation paths.
- [x] Add focused tests for reference sizing or placement where practical.
- [x] Run focused static validation after the menu and root-client repair.
- [ ] Run the normal Windows Debug build once if the developer supplies or approves an exact command.
- [x] Record final validation, root cause, changed files, limitations, and remaining issues.

## Validation Plan

- Run targeted source checks for all six command registrations, mappings, enablement, execution paths, and reference calculations.
- Add or update focused unit tests for the shared placement calculation if it can live outside the Visage UI layer.
- Run `git diff --check`.
- Build the normal Windows Debug `VisiForm` target once only through an explicitly approved exact command.
- Do not perform automated runtime validation because repository rules prohibit launching `VisiForm.exe`.
- Record the Phase 101 manual checklist as deferred unless the developer performs it.

## Compatibility

- No `.vfb.json` schema change.
- No generator or serialization behavior change.
- Existing primary selection semantics remain unchanged.
- Existing primary Same Width and Same Height command IDs remain available.
- Viewport zoom and pan remain editor-only and are never written into widget model geometry.

## Build / Test Status

- Branch: `main`.
- Most recent relevant commit before Phase 101 changes: `bdde0dd Refine multi-selection layout commands`.
- Starting worktree contains the Phase 100 session-instruction archive move and the untracked Phase 101 instruction file; both are preserved.
- Focused static validation: `git diff --check` passed after the repair with line-ending normalization warnings only. Targeted source checks confirmed the old character-count menu estimate and fixed `dropdownWidth - 110` label rectangle are gone; width and drawing now share measured label/shortcut columns. The model test source now asserts a root client area of `(0, 28, 900, 572)` and a startup Button of `(40, 48, 260, 56)`.
- Windows Debug build: pending an explicitly approved exact command.
- Manual runtime validation: not performed; automated agents may not launch `VisiForm.exe`.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_101_size_matching_safe_placement_plan.md`
- `docs/layout_tools.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/commands/CommandIds.h`
- `src/commands/CommandRegistry.cpp`
- `src/model/ProjectDocument.cpp`
- `src/model/LayoutEngine.cpp`
- `src/model/WidgetPlacement.h`
- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `tests/test_layout_engine.cpp`

## Final Result Summary

- Updated authoritative version declarations and current-progress documentation to `1.0.7` / Phase 101.
- Added visible Match Primary, Match Smallest, and Match Largest commands for both width and height without introducing hidden modifier-only behavior.
- Kept the Phase 100 compatible sibling rules: cross-parent, direct-Sizer-child, and dock-managed selections are disabled.
- Unified each dimension's three reference modes in one undoable document-change path. Reference values come from model-space dimensions, the shared target is raised to satisfy all selected widget minimums, positions and the unaffected dimension remain unchanged, and no-op actions create no undo entry.
- The menu clipping root cause was character-count width estimation combined with a fixed `dropdownWidth - 110` label rectangle. The repair measures the longest label and shortcut independently with `visage::Font::stringWidth()`, then adds the checkmark area, inter-column gap, and edge padding. Drawing uses the same measured shortcut width to reserve a separate right-aligned column, so command and shortcut text do not overlap.
- The startup and New Project placement root cause was in `LayoutEngine::clientBoundsForParent()`: `FormWindow` returned the entire outer form bounds even though the designer and generated runtime both reserve a 28-pixel title-bar strip. Safe placement was therefore validating against outer bounds rather than the actual form content area.
- `FormWindow` client bounds now begin at model-space `y = 28` and have `height = form height - 28`. Startup placement uses the registry-created Button's actual default `260 x 56` size instead of the stale `160 x 40` override, then clamps it through `safeWidgetPlacement()`.
- For the default `900 x 600` startup form, the resulting model values are form `(0, 0, 900, 600)`, content client `(0, 28, 900, 572)`, and Button `(40, 48, 260, 56)`. Wizard templates use the same corrected root client bounds; widgets requesting `y = 40` clamp to `y = 48`, while already-safe lower positions remain unchanged.
- Palette click placement is entirely model-space and therefore independent of zoom/pan. Existing explicit canvas drag/drop already uses `DesignerCanvas::toFormPoint()` and `clampBoundsToParentClient()`, so that user-positioning path remains intact.

## Remaining TODOs

- Build the normal Windows Debug `VisiForm` target through an explicitly approved exact command.
- Manually launch VisiForm and complete the Phase 101 runtime checklist for all six size modes, undo/redo, no-op history, incompatible selections, startup/new-project placement, palette creation at multiple viewport states, nested containers, edge drops, Project Tree parenting, and save/reload persistence.
- Confirm the flat six-command Layout menu remains readable at the developer's runtime DPI and window size.
