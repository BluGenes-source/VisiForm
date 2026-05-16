# Final text widget height pass plan

## Goal

Make one final pass on `Label` and `CheckBox` sizing and readability so they are tall enough by default, respect better minimum heights, and stay visually centered and readable on `DesignerCanvas`.

## Current issue

- `Label` text is still slightly vertically cramped.
- `CheckBox` text is still slightly vertically cramped.
- `Label` default and minimum heights need one more increase.
- `CheckBox` default and minimum heights need one more increase.
- The sizing logic should be prepared for future font-size support.

## Files to inspect

- `src/ui/WidgetPalette.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/DesignerCanvas.h`
- `src/model/WidgetNode.h`
- `src/model/PropertyValue.h`
- `docs/agent_plans/final_text_widget_height_pass_plan.md`

## Planned changes

1. Increase `Label` default and minimum height.
2. Increase `CheckBox` default and minimum height.
3. Add simple font-size-aware sizing helper constants/functions for future use.
4. Make auto-size and `Fit` raise `Label` and `CheckBox` height using estimated line height.
5. Improve vertical centering for `Label` and `CheckBox` drawing in `DesignerCanvas`.
6. Ensure manual resize still respects improved minimum heights.
7. Keep save/load/export and user-code preservation unchanged.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Add a new `Label` and confirm it is visibly tall enough by default.
- Add a new `CheckBox` and confirm it is visibly tall enough by default.
- Use `Fit` on a `Label` and confirm height is raised if needed.
- Use `Fit` on a `CheckBox` and confirm height is raised if needed.
- Confirm `Label` text is vertically readable and not clipped.
- Confirm `CheckBox` square and text are vertically aligned.
- Manually resize `Label` and `CheckBox` and confirm minimum heights are enforced.
- Confirm save, export, and user-code preservation still work.

## Final result summary

Completed.

- Increased `Label` default height to `48` and minimum height to `44`.
- Increased `CheckBox` default height to `52` and minimum height to `48`.
- Added simple future-facing font-size-aware helper functions for estimated character width and line height.
- Updated auto-size and `Fit` so they raise `Label` and `CheckBox` height floors without shrinking widgets.
- Improved `DesignerCanvas` vertical centering for `Label` text and `CheckBox` square/text alignment.
- Verified the changes with a successful `build-static-debug` build.
