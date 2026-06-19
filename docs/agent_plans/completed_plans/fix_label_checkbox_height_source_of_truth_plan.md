# Fix label and checkbox height source of truth plan

## Goal

Make `Label` and `CheckBox` height behavior reliable by introducing one source of truth for widget default and minimum sizes, then using it consistently for creation, resizing, fitting, bounds edits, and editor normalization.

## Current issue

- Newly added `Label` is still visibly too short.
- Newly added `CheckBox` is still visibly too short.
- Text appears vertically clipped or crowded.
- Previous size changes were likely applied in some places but not all runtime paths.

## Likely causes

- The actual widget creation path may still use a different helper than the one previously updated.
- Default sizes and minimum sizes may be duplicated across multiple code paths.
- Resize and inspector bounds edits may still use generic minimums.
- Older or sample-loaded projects may not be normalized for editor readability.
- `DesignerCanvas` vertical text placement may still be too aggressive for `Label` and `CheckBox`.

## Files to inspect

- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `src/ui/WidgetPalette.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/PropertyInspector.h`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `docs/agent_plans/fix_label_checkbox_height_source_of_truth_plan.md`

## Planned changes

1. Identify the actual runtime widget creation path used by the Widget Palette.
2. Add a centralized UI-layer widget size metrics helper.
3. Use shared metrics for default size creation and minimum size enforcement.
4. Apply shared minimums during manual resize, inspector bounds edits, and `Fit`/auto-size.
5. Normalize loaded and sample project widget bounds for editor readability.
6. Refine `Label` and `CheckBox` vertical drawing using simple font-size-aware estimate helpers.
7. Preserve all currently working save/load/export and user-code preservation behavior.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Add a new `Label` and confirm the status message reports the larger default size.
- Add a new `CheckBox` and confirm the status message reports the larger default size.
- Confirm new `Label` height is visibly around `60`.
- Confirm new `CheckBox` height is visibly around `64`.
- Resize `Label` and confirm height cannot go below `56`.
- Resize `CheckBox` and confirm height cannot go below `58`.
- Edit bounds in the inspector and confirm too-small heights clamp to the correct minimums.
- Use `Fit` and confirm larger heights are preserved or raised as needed.
- Load an older or sample project and confirm too-small `Label` and `CheckBox` bounds are normalized.
- Confirm export and user-code preservation still work.

## Final result summary

Pending.
