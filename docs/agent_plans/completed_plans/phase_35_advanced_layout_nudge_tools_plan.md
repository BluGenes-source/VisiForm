# Phase 35 advanced layout and nudge tools plan

## Goal

Add advanced alignment, distribution, and nudge tools for selected widgets while preserving the existing multi-select, layout, and save/export workflows.

## Current editor state

- Multi Select mode works.
- Box select works.
- Group move works.
- Copy/paste works.
- Basic layout tools already exist.
- Save/load/export and user-code preservation work.

## Files to inspect

- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/PropertyInspector.h`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `docs/layout_tools.md`
- `docs/selection_model.md`
- `docs/agent_plans/phase_35_advanced_layout_nudge_tools_plan.md`

## Planned advanced layout tools

1. Add `Align Right` and `Align Bottom`.
2. Add `Center Horizontally` and `Center Vertically`.
3. Add `Distribute Horizontally` and `Distribute Vertically`.
4. Keep root form excluded from these layout actions.
5. Preserve multi-select behavior and primary-selection rules for width and height reference tools.

## Planned nudge behavior

1. Add selected-widget nudge support for all selected non-root widgets.
2. Support arrow-key nudging when property editing is inactive.
3. Support larger shift-arrow nudges using the current grid size.
4. Add compact toolbar nudge controls if they fit cleanly.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Test each new alignment tool with one widget and multiple selected widgets.
- Test horizontal and vertical distribution with at least three selected widgets.
- Test keyboard nudging and shift-arrow grid nudging.
- Test any added toolbar nudge controls.
- Confirm Property Inspector updates after layout and nudge actions.
- Confirm save/load/export preserve changed widget bounds.
- Confirm existing multi-select, box-select, group move, copy/paste, and z-order tools still work.

## Final result summary

Completed.

- Added `Align Right`, `Align Bottom`, `Center Horizontally`, `Center Vertically`, `Distribute Horizontally`, and `Distribute Vertically`.
- Added keyboard nudging for selected non-root widgets with arrow keys and shift-arrow grid nudges.
- Kept the toolbar compact with new advanced layout buttons and left toolbar nudge buttons out for now due space.
- Preserved existing multi-select, box-select, group move, copy/paste, save/load/export, and user-code preservation behavior.
- Verified the main project builds successfully with `build-static-debug`.
