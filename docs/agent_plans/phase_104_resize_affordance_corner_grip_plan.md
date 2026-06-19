# Phase 104 Resize Affordance and Corner Grip Plan

## Scope

- Make the bottom-right resize target easier to acquire in screen space.
- Draw subtle curved grip arcs inside the selected widget's bottom-right corner.
- Keep hover cursor feedback aligned with the actual resize hit region.
- Hide direct-resize affordances where the existing editor does not permit meaningful direct resizing.
- Update authoritative version declarations from `1.0.9` to `1.0.10`.

## Requirements

- Use Phase 104 and version `1.0.10`.
- Use no subagents.
- Preserve existing resize geometry, minimum sizes, undo/redo, Property Inspector refresh, zoom/pan behavior, and Preview Mode restrictions.
- Do not launch `VisiForm.exe` from an automated agent.
- Do not run a build command unless the developer explicitly authorizes that exact command.

## Version Notes

- Previous Phase 103 version: `1.0.9`.
- Phase 104 version: `1.0.10`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Architecture Decisions

- Keep resize affordance geometry in `DesignerCanvas.cpp`, next to the existing shared handle rectangle, hit-test, and drawing helpers.
- Preserve the existing four visible square handles and existing hit zones for the other three corners.
- Improve the bottom-right corner first, as explicitly permitted by the phase prompt.
- The bottom-right hit zone extends primarily inward from the rendered corner in fixed screen pixels, with only a small allowance outside the selection boundary. This keeps the affordance usable across zoom levels without converting the pointer threshold to model units.
- The bottom-right visual handle rectangle, enlarged hit rectangle, and grip-arc placement derive from one corner geometry helper.
- One arc is drawn when the selected widget has at least 18 screen pixels available on both axes; two arcs are drawn at 28 screen pixels or larger. Below 18 pixels, the square handle remains the only visual affordance.
- Direct Sizer children retain their existing preferred-size resize behavior and therefore retain affordances.
- FormWindow, TabPage, and dock-managed widgets do not show or hit-test direct resize affordances.
- Preview Mode continues to suppress all editor decorations and interaction before resize hit testing is reached.

## Resize Hit-Area Policy

- Existing square handle: 10 screen pixels.
- Existing standard corner hit target: 16 screen pixels centered on the corner.
- Bottom-right target: 22 screen pixels, extending 18 pixels inward and 4 pixels outward from the corner on each axis.
- Only the selected primary widget receives handle hit testing.
- Hit testing remains deterministic because the bottom-right geometry maps to one explicit `BottomRightHandle` region.

## Arc Geometry

- Each arc is a rounded quadratic curve from the right selection edge to the bottom selection edge.
- The original curve control point was the selected widget's bottom-right corner. That bowed the curve toward the corner, visually reproducing an outer rounded-border corner and placing the curve beneath the square handle.
- The corrected control point is the upper-left corner of each arc's radius square. This bows the curve toward the widget interior/top-left while keeping both endpoints inset near the right and bottom selection edges.
- Visage's angle direction is not involved because this implementation uses `Canvas::quadratic()` rather than an angle-based arc API.
- Arc radii are 8 and 13 screen pixels.
- Arcs use the existing blue selection highlight and a subtle 1.5-pixel stroke.

## TODO Checklist

- [x] Inspect branch, worktree, Phase 103 plan, Phase 104 prompt, and targeted source.
- [x] Confirm Phase 104 is unused and select version `1.0.10`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations to `1.0.10`.
- [x] Record Phase 104 startup in `docs/project_status.md` and README current progress.
- [x] Add shared bottom-right grip geometry.
- [x] Enlarge the bottom-right screen-space hit target.
- [x] Draw adaptive bottom-right corner arcs.
- [x] Correct the inverted grip-arc orientation.
- [x] Align cursor feedback with resize hit testing.
- [x] Suppress misleading affordances for non-directly-resizable widgets.
- [x] Run focused static validation.
- [ ] Run the normal Windows Debug build once if the developer supplies or approves an exact command.
- [x] Record final validation, files changed, limitations, and remaining issues.

## Validation Plan

- Inspect selected-widget rendering for outline, square handles, adaptive arcs, and small-widget fallback.
- Inspect interaction hit testing at different zoom values to confirm thresholds remain screen-space constants.
- Inspect cursor mapping for all existing corner regions and the enlarged bottom-right region.
- Inspect FormWindow, TabPage, dock-managed, direct Sizer-child, and Preview Mode behavior.
- Inspect resize startup, existing geometry update, minimum-size handling, undo/redo, and Property Inspector refresh paths for regressions.
- Run `git diff --check`.
- Build the normal Windows Debug `VisiForm` target once only through an explicitly approved exact command.
- Do not perform automated runtime validation because repository rules prohibit launching `VisiForm.exe`.

## Compatibility

- No `.vfb.json` schema change.
- No model, serialization, validation, command, or generator behavior change.
- No change to resize anchors or minimum-size semantics.
- Existing selection, movement, layout commands, zoom/pan, autosave, recovery, and save/reload behavior remain separate.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains user-owned session-instruction archive changes, a modified `session-instructions/notes.txt`, and the untracked Phase 104 prompt; those changes are preserved.
- Compile-error root cause: `hitTestInteraction()` declared `selectedWidget` in the initializer of the TabPage guard `if`, so the pointer went out of scope before the following resize-handle hit-test block referenced it.
- Compile fix: the existing `document.findWidgetById(selectedWidgetId)` lookup now initializes one function-scope `selectedWidget` pointer used by both the TabPage guard and the selected widget's screen-bounds resize hit test. The hit test explicitly requires a non-null pointer, so empty or stale selection IDs remain safe without introducing duplicate selection state.
- Arc-orientation root cause: the quadratic started on the inset right edge and ended on the inset bottom edge, but used the bottom-right corner as its control point. That pulled the curve toward the corner like an outer rounded frame and into the square handle area. The corrected control point is the upper-left corner of the radius square, so the curve bows toward the widget interior and stays clear of the handle without changing endpoints, radii, thresholds, or hit testing.
- Focused static validation: `git diff --check` passed with line-ending normalization warnings only. Targeted inspection confirmed one shared resize geometry helper supplies visual and hit rectangles, the bottom-right target remains fixed in screen pixels, overlapping tiny-widget targets choose the nearest rendered corner deterministically, arc thresholds are based on rendered screen extent, arcs remain inset inside the selected boundary, dock-managed/FormWindow/TabPage affordances are suppressed, direct Sizer children retain their existing preferred-size resize path, Preview Mode remains decoration-free and read-only, and cursor mapping uses the same `hitTestInteraction()` result as resize startup.
- Post-correction static validation: `git diff --check` passed again with line-ending normalization warnings only. The drawing-only change leaves resize hit testing and cursor selection untouched; both adaptive arc counts retain their existing radii, inset, and small-widget thresholds.
- Windows Debug build: pending an explicitly approved exact command.
- Manual runtime validation: not performed; automated agents may not launch `VisiForm.exe`.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_104_resize_affordance_corner_grip_plan.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/MainWindow.cpp`

## Final Result Summary

- Updated authoritative version declarations and current-progress documentation to `1.0.10` / Phase 104.
- Replaced separate handle sizing members with file-local screen-space resize geometry constants beside the rendering and hit-test implementation.
- Preserved the existing 10-pixel square handles and 16-pixel hit targets for top-left, top-right, and bottom-left.
- Added a 22-pixel bottom-right hit target extending 18 pixels inward and 4 pixels outward from the rendered corner.
- Added deterministic nearest-corner resolution when hit targets overlap on very small or heavily zoomed-out widgets.
- Added one inset blue rounded corner arc at 18-27 rendered pixels and two arcs at 28 rendered pixels or larger.
- Corrected the arc control points so the curves bow into the widget's upper-left interior instead of resembling an outside rounded border.
- Kept the square handle as the only visual affordance below 18 rendered pixels.
- Added matching Visage diagonal resize cursors for all four corner hit regions.
- Suppressed resize handles, arcs, hit zones, and resize cursors for FormWindow, TabPage, and dock-managed widgets.
- Retained direct Sizer-child affordances because the existing interaction edits the Sizer item's preferred size.

## Known Limitations

- The phase intentionally improves the bottom-right corner first; the other three corners retain their existing square-handle hit geometry.
- Arc appearance and pointer ergonomics require manual runtime verification on the developer's display and DPI configuration.

## Remaining TODOs

- Build the normal Windows Debug `VisiForm` target through an explicitly approved exact command.
- Manually complete the Phase 104 runtime checklist at 100%, zoomed-in, and zoomed-out views, including small widgets, docked widgets, direct Sizer children, Preview Mode, undo/redo, Property Inspector refresh, and save/reload.
