# Phase 36 smart guides and snap tools plan

## Goal

Add smart guides and snap-to-widget alignment during move operations while preserving current grid snap, multi-select, group move, and export behavior.

## Current editor state

- Multi Select mode works.
- Box select works.
- Group move works.
- Copy/paste works.
- Layout tools and nudge tools work.
- Save/load/export and user-code preservation work.

## Files to inspect

- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/PropertyInspector.h`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`
- `docs/layout_tools.md`
- `docs/selection_model.md`
- `docs/agent_plans/phase_36_smart_guides_snap_tools_plan.md`

## Planned smart guide behavior

1. Add a toolbar toggle for smart guides and persist the preference in app settings.
2. Snap moving widgets or moving groups to root form edges, margins, centers, and non-selected widget edges and centers.
3. Draw guide lines on top of the form preview while dragging.
4. Apply smart guides only during move, not resize, in this phase.

## Planned snap behavior

1. Compute move snapping independently on X and Y.
2. Use the moving widget bounds for single selection and the selected-group bounding box for group move.
3. Prefer the closest smart-guide match within the snap threshold.
4. If no smart guide match exists on an axis, fall back to grid snap for that axis when grid snapping is enabled.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Turn smart guides on and off with the toolbar toggle.
- Move a widget near another widget edge and confirm snapping and guide lines appear.
- Move a widget near another widget center and confirm center snapping.
- Move a widget near root form center and margins and confirm snapping.
- Move a multi-selected group and confirm snapping uses the group bounding box.
- Confirm grid snap still works when no guide match exists on an axis.
- Confirm box select, group move, copy/paste, and layout tools still work.

## Final result summary

Completed.

- Added a persisted `smartGuidesEnabled` editor setting and a `Gde` toolbar toggle.
- Added smart-guide snapping targets for root form edges, root margins, root centers, and non-selected widget edges and centers.
- Integrated axis-specific smart-guide snapping into single-widget move and group move while preserving grid snap on axes without a guide match.
- Added visible guide-line rendering on top of the form preview during move operations.
- Kept box select, multi-select, group move, copy/paste, layout tools, save/load/export, and user-code preservation working.
- Verified the main project builds successfully with `build-static-debug`.
