# Phase 100 Multi-Selection Layout Command Refinement Plan

## Scope

- Refine the existing Layout menu commands for predictable multi-selection behavior.
- Make command enablement match the operations that can safely execute.
- Preserve parent coordinate spaces and sizer ownership.
- Keep every successful operation as one undoable document command.
- Update authoritative version declarations from `1.0.5` to `1.0.6`.

## Requirements

- Use Phase 100 and version `1.0.6`.
- Use no subagents.
- Refine existing commands without adding competing layout modes.
- Do not launch `VisiForm.exe` from an automated agent.
- Build only through an explicitly approved exact command.

## Version Notes

- Previous Phase 99 version: `1.0.5`.
- Phase 100 version: `1.0.6`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Reference-Widget Rule

- `ProjectDocument::selectedWidgetId` is the primary/reference widget.
- The existing selection model makes the most recently added selected widget primary.
- Alignment and same-size commands keep the primary widget unchanged and update the other compatible selected widgets relative to it.
- Distribution is the full-selection exception: geometry determines the two fixed outer widgets, independent of selection order.

## Command Enablement Rules

- Alignment and same-size commands require at least two selected non-root widgets that share one direct parent and are not direct children of a Sizer.
- Distribution requires at least three widgets satisfying the same compatibility rule.
- Bring Forward and Send Backward require a non-root primary widget with a sibling position in the requested direction.
- Fit Text requires one supported primary text-bearing widget that is not controlled by a parent Sizer.
- Preview Mode disables every layout-editing command.
- Enablement is derived from the current document selection whenever menus and toolbar controls are rebuilt.

## Parent and Sizer Rules

- Geometry commands operate only on widgets with the same direct parent, so all bounds use one model-space coordinate system.
- Cross-parent selections disable geometry commands.
- Direct children of a Sizer are excluded by disabling the command for the whole ambiguous selection.
- Commands never reparent widgets or directly mutate sizer-owned geometry.

## Distribution Algorithm

- Sort horizontally by left edge and vertically by top edge, with stable ID tie-breaking.
- Keep the two geometry-outermost widgets fixed.
- Compute one equal gap from the span between the fixed outer bounds minus the sum of all widget sizes.
- Place interior widgets from the first fixed outer bound using the computed gap and original sizes.
- Do not snap distributed positions to the grid, avoiding cumulative rounding drift.
- Negative gaps are allowed deterministically, producing equal overlap between adjacent widget bounds.

## Undo / Redo Integration

- Reuse `applyUndoableDocumentChange` and `DocumentStateCommand`.
- One menu action creates one document-state command.
- Each operation compares proposed and current geometry or ordering before reporting a change.
- No-op or failed operations create no undo entry and do not mark the document dirty.
- Document snapshots preserve the complete multi-selection across undo and redo.

## TODO Checklist

- [x] Inspect branch and worktree.
- [x] Read the Phase 100 brief, project status, Phase 99 plan, and targeted version declarations.
- [x] Confirm Phase 100 is unused and select version `1.0.6`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations to `1.0.6`.
- [x] Record Phase 100 startup in `docs/project_status.md`.
- [x] Add shared compatibility and command-eligibility helpers.
- [x] Align sibling widgets to the primary widget in model space.
- [x] Match width and height to the primary while respecting minimum sizes.
- [x] Distribute by equal gaps while fixing the outer widgets.
- [x] Protect sizer-managed and cross-parent selections.
- [x] Refine z-order and Fit Text enablement.
- [x] Ensure no-op actions create no undo entry.
- [x] Run focused static validation.
- [ ] Run the approved Windows Debug build once, if an exact command is available.
- [x] Record final validation, changed files, known limitations, and remaining issues.

## Validation Plan

- Run focused source checks for compatibility, enablement, primary reference use, equal-gap distribution, and no-op detection.
- Run `git diff --check`.
- Build the normal Windows Debug `VisiForm` target once only if an exact developer-approved command is available.
- Do not perform automated runtime validation because repository rules prohibit launching `VisiForm.exe`.
- Record the developer manual checklist as deferred unless the developer performs it.

## Compatibility

- No `.vfb.json` schema or persistence change.
- No generator, validation, or serialization behavior change.
- Existing selection ordering and primary-selection semantics remain unchanged.
- Existing commands and command IDs remain unchanged.

## Build / Test Status

- Branch: `main`.
- Most recent relevant commit before Phase 100 changes: `638eb04 Add Canvas Zoom/Pan and Preview Mode Enhancements`.
- Starting worktree contains the Phase 99 session-instruction archive move and the untracked Phase 100 instruction file; they are preserved outside implementation scope.
- Focused static validation: `git diff --check` passed with line-ending normalization warnings only. Targeted source checks confirmed version `1.0.6` across authoritative declarations; alignment and same-size operations reference `selectedWidgetId`; compatibility requires one shared direct parent and rejects sizer-managed or docked widgets; distribution requires three widgets and computes equal edge gaps while leaving the outer widgets untouched; sibling-order commands swap one adjacent child; Fit Text uses the same resolved widget font path as Designer Canvas rendering; and operation lambdas return false when geometry is unchanged.
- Unit test source added for adjacent sibling ordering and multi-selection preservation; tests were not executed because no exact approved build/test command was supplied.
- Windows Debug build: pending an approved exact command.
- Manual runtime validation: not performed; automated agents may not launch `VisiForm.exe`.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_100_multi_selection_layout_refinement_plan.md`
- `docs/layout_tools.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/commands/CommandRegistry.cpp`
- `src/model/ProjectDocument.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.cpp`
- `tests/test_project_serialization.cpp`

## Final Result Summary

- Updated authoritative VisiForm version declarations and current-progress documentation to `1.0.6` / Phase 100.
- Preserved the existing selection model: the most recently added selected widget remains `selectedWidgetId` and is the primary/reference widget.
- Alignment commands now move only secondary selected siblings to the primary widget's corresponding edge or center.
- Same Width and Same Height now require compatible multi-selection, preserve positions, keep the primary unchanged, and enforce each target widget's minimum size.
- Horizontal and vertical distribution now require at least three compatible siblings, keep the outer widgets fixed, preserve sizes, and produce equal gaps between bounds without grid rounding.
- Cross-parent, direct-sizer-child, and dock-managed geometry selections are disabled consistently in menus, toolbar controls, shortcuts, and execution guards.
- Bring Forward and Send Backward now move one sibling position and retain the complete selection through execution, undo, and redo.
- Fit Text enablement is restricted to supported text-bearing widgets outside parent-managed layouts, and width measurement uses the font resolved by Designer Canvas rendering.
- Every successful operation remains one `DocumentStateCommand`; no-op operations return before creating undo history or marking the document dirty.

## Remaining TODOs

- Build the normal Windows Debug `VisiForm` target through an explicitly approved exact command.
- Manually launch VisiForm and complete the Phase 100 runtime checklist, including all commands, primary-reference behavior, nested containers, sizer and cross-parent protection, undo/redo, no-op history, zoom levels, Preview Mode, and save/reload persistence.
- Runtime-check Fit Text padding and clipping across custom fonts and DPI scales.
