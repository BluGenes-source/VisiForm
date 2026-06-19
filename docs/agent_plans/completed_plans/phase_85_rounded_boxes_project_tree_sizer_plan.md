# Phase 85 - Rounded boxes, Project Tree text, and Sizer widget plan
# Phase 85 - Rounded boxes, Project Tree text, and Sizer widget plan

## Goal

Implement shared rounded-corner box drawing, improve Project Tree row text formatting, and add an optional wxBoxSizer-style `Sizer` container widget.

## Checklist

- [x] Create the Phase 85 plan and Copilot prompt.
- [x] Add `WidgetType::Sizer` model, registry, ID, insertion, validation, and export plumbing.
- [x] Add horizontal/vertical Sizer layout in `LayoutEngine`.
- [x] Route designer boxed-widget drawing through shared rounded-box helpers.
- [x] Mirror rounded boxed-widget drawing in generated runtime code.
- [x] Update Project Tree rows to render name as primary text and type as secondary text.
- [x] Update widget, registry, project format, validation, and code generation docs.
- [x] Inspect the final diff.

## Implementation Notes

- `Sizer` is optional in this pass. Existing projects and new default projects do not require one.
- The first Sizer implementation is a box sizer with one `orientation` property: `Vertical` or `Horizontal`.
- Sizer layout uses `padding` around the client area and `gap` between children, distributing available main-axis space evenly across direct children.
- Rounded corner behavior should be centralized in the current switch-based drawing architecture through shared helpers, not through per-widget subclasses.

## Validation Status

- `git diff --check` completed with only CRLF normalization warnings and no whitespace errors.
- Visual Studio workspace build pipeline completed successfully for `VisiForm` after the Phase 85 follow-up fix.
- Developer-reported Debug abort traced to `WidgetRegistry` startup validation: `Sizer` reused palette order `22`, colliding with `ColorPicker` and tripping `assert(hasConsistentPaletteDefinitions(definitions_))`.
- Fixed the startup abort by assigning `Sizer` a unique palette order.
- Developer confirmed the app now runs after the follow-up fix.
- `VisiForm.exe` was not run.
- Generated apps were not launched.

## Manual Validation Checklist

- [ ] Add `GroupBox`, `Panel`, `TextBox`, `ComboBox`, `ListBox`, `TreeView`, and `ProgressBar`; set non-default `cornerRadius`; confirm rounded corners in designer.
- [ ] Export a sample project and inspect generated drawing behavior for matching rounded boxes.
- [ ] Confirm Project Tree rows show name primary and type secondary without overlap.
- [ ] Add a `Sizer`, add several child widgets, toggle Horizontal/Vertical, change padding/gap, and confirm child bounds update predictably.
- [ ] Save/reload a project containing a Sizer and confirm hierarchy, properties, and layout persist.

## Remaining TODOs

- None for Phase 85 scope.

## Final Result Summary

- Added an optional `Sizer` container widget with vertical/horizontal layout, padding, gap, designer preview, validation, persistence through existing project serialization, and generated runtime type support.
- Routed boxed-widget rendering through shared rounded-box helpers in the designer and generated runtime paths, including the `GroupBox` titled content rectangle.
- Updated Project Tree rows to draw widget name as primary text and widget type as secondary text.
- Fixed the Phase 85 Debug abort by removing the `Sizer`/`ColorPicker` palette-order collision that triggered `WidgetRegistry` startup validation.
- Phase 85 is complete for repository scope, with build validation passing and the developer confirming the application starts successfully.
