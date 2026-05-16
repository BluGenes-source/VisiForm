# Fix text widget heights plan

## Goal

Fix default and minimum heights for text-capable widgets so new widgets are readable by default, `Fit` also enforces sane heights, and `DesignerCanvas` aligns text vertically with the updated widget bounds.

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
- `docs/agent_plans/fix_text_widget_heights_plan.md`

## Planned changes

1. Raise default sizes for newly added widgets, especially text-capable widgets.
2. Enforce widget-specific minimum heights during auto-size and manual resize.
3. Ensure `Fit` raises height as well as width where needed.
4. Improve `DesignerCanvas` vertical alignment for label, button, textbox, checkbox, and slider drawing.
5. Keep save/load, export, and user-code preservation unchanged.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Add new `Button`, `Label`, `TextBox`, and `CheckBox` widgets and confirm they are tall enough by default.
- Use `Fit` on text-capable widgets and confirm height is raised if needed.
- Edit longer text and confirm widgets remain tall enough.
- Confirm checkbox square and text are vertically aligned.
- Manually resize widgets and confirm minimum readable heights are enforced.
- Confirm save, export, and user-code preservation still work.

## Final result summary

Completed.

- Raised default sizes for newly added text-capable widgets and other commonly used widgets.
- Enforced widget-specific minimum heights during auto-size, Fit, inspector bounds edits, and manual resize updates.
- Kept width auto-size and Fit behavior intact while ensuring widgets do not remain too short for readable text.
- Improved `DesignerCanvas` vertical placement for label, textbox, and checkbox text so taller widgets look visually balanced.
- Verified the changes with a successful `build-static-debug` build.
