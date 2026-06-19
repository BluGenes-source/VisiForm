# Fix actual label and checkbox bounds path plan

## Goal

Find and replace the actual runtime source of `Label` and `CheckBox` default and minimum sizing so newly created and normalized widgets use the intended editor bounds consistently.

## Current issue

- Property Inspector still shows a newly created `Label` as `200 x 34`.
- Newly created `CheckBox` is still shorter than intended.
- This proves the active runtime sizing path is still using older values.

## Likely causes

- The actual widget creation path may still use `MainWindow::nextDefaultWidgetBounds` or another older helper.
- Size logic may be duplicated across creation, resize, auto-size, and normalization paths.
- Loaded or sample documents may not be normalized through the same minimum-size logic.

## Files to inspect

- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `src/ui/WidgetPalette.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/PropertyInspector.h`
- `src/model/ProjectDocument.cpp`
- `src/model/ProjectDocument.h`
- `src/model/WidgetNode.cpp`
- `src/model/WidgetNode.h`
- `src/utils/IdGenerator.cpp`
- `src/utils/IdGenerator.h`
- `docs/agent_plans/fix_actual_label_checkbox_bounds_path_plan.md`

## Planned changes

1. Search the active runtime paths and identify the exact hard-coded `Label 200 x 34` source.
2. Add one UI-layer widget metrics source of truth for default and minimum sizes.
3. Use that helper for widget creation, manual resize, inspector bounds edits, fit and auto-size, and normalization.
4. Normalize loaded and newly created widget bounds where needed.
5. Adjust label and checkbox drawing to match the larger active bounds.
6. Keep save/load/export and user-code preservation unchanged.

## Build validation

- Build the main project with `build-static-debug`.
- Fix compile errors if any appear.
- Do not run `VisiForm.exe`.

## Manual test checklist

- Add a new `Label` and confirm status reports `260x64`.
- Add a new `CheckBox` and confirm status reports `300x68`.
- Confirm Property Inspector shows the new `Label` and `CheckBox` bounds immediately.
- Try too-small height edits in Property Inspector and confirm clamping.
- Manually resize `Label` and `CheckBox` smaller and confirm minimums hold.
- Use `Fit` and confirm height minimums are preserved.
- Load an older or sample project and confirm too-small `Label` and `CheckBox` widgets normalize.
- Confirm export and user-code preservation still work.

## Hard-coded sizes found

- `src/ui/MainWindow.cpp` in `MainWindow::nextDefaultWidgetBounds`
  - `Label` was still `200 x 34`
  - `CheckBox` was still `240 x 34`
- Older minimum-size logic also lived in `src/ui/MainWindow.cpp` before centralization.

## Actual creation path found

- `MainWindow::addWidgetFromPalette`
- `MainWindow::createDefaultWidget`
- `MainWindow::nextDefaultWidgetBounds`

This was the active runtime path still producing `Label` as `200 x 34`.

## Changes made

- Added shared UI-layer metrics in `src/ui/WidgetMetrics.h` and `src/ui/WidgetMetrics.cpp`.
- Replaced active default and minimum size logic in `MainWindow` with shared metrics.
- Normalized widget bounds recursively after new, load, sample-open, and palette-add flows.
- Enforced shared minimums during manual resize, inspector bounds edits, and Fit/auto-size.
- Updated `DesignerCanvas` label and checkbox vertical placement to use shared font-size-aware helpers.
- Added widget size diagnostics to add-widget status messages.

## Final result summary

Completed.

- The real runtime source of the stale `Label 200 x 34` values was `MainWindow::nextDefaultWidgetBounds`.
- New `Label` widgets now use `260 x 64` defaults with a `140 x 58` minimum.
- New `CheckBox` widgets now use `300 x 68` defaults with a `200 x 62` minimum.
- Shared metrics now drive creation, resizing, inspector bounds edits, Fit/auto-size, normalization, and drawing helpers.
- The main project built successfully with `build-static-debug`.
