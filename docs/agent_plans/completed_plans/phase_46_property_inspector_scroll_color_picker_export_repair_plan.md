# Phase 46 property inspector scroll color picker export repair plan

## Goal

Repair and stabilize the Property Inspector, add color editing and `ColorPicker` widget support, fix generated C++ syntax/output regressions, and update documentation without changing protected build/export conventions.

## Current problems

- `PropertyInspector` truncates rows once the visible panel height is exhausted.
- Additional Look and Feel rows are generated but become inaccessible.
- The inspector has no internal scroll state or scrollbar chrome.
- Color properties are edited as plain text only.
- `ColorPicker` is not a supported widget type.
- Generated C++ output from the current emitter contains compile-breaking issues.
- Exported generated projects need to build again in Visual Studio Debug and Release.

## Files inspected

- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/ui/WidgetMetrics.h`
- `src/ui/WidgetMetrics.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/ProjectDocument.h`
- `src/model/LookAndFeelDefinition.h`
- `src/model/LookAndFeelRegistry.h`
- `src/generator/CodeGenerator.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CMakeEmitter.cpp`
- `src/utils/NativeFileDialogs.h`
- `src/utils/NativeFileDialogs.cpp`
- `Generated/ExportedVisageProject/src/MainWindow.cpp`
- `Generated/VisForm-Test/build/vs2022-x64-static-debug/_deps/visage-src/visage_ui/scroll_bar.h`
- `Generated/VisForm-Test/build/vs2022-x64-static-debug/_deps/visage-src/visage_graphics/canvas.h`

## Step-by-step TODO list with checkboxes

- [x] Create persistent phase plan file before code changes
- [x] Inspect inspector layout, scrollbar reuse points, and generator regressions
- [x] Add inspector scroll state and metrics helpers
- [x] Add internal Property Inspector vertical scrollbar behavior
- [x] Keep inspector hit testing and editing stable while scrolled
- [x] Add color swatch and picker workflow for color properties
- [x] Add `ColorPicker` widget type through model, palette, designer, save/load, and export paths
- [x] Repair generated C++ emitter syntax and safety issues
- [x] Update widget, project format, code generation, and look-and-feel docs
- [x] Build main `VisiForm` with `build-static-debug`
- [ ] Verify generated project Debug and Release build expectations
- [ ] Write final result summary

## Investigation notes

### Property Inspector layout diagnosis

- Rows are generated in `PropertyInspector::buildRows()` from fixed base rows, widget definition properties, event rows, and fallback raw properties.
- Row height is a fixed constant: `kRowHeight = 30.0f`.
- Visible row bounds are computed by `buildRowLayouts(top, height, rows)`, where `top` is `y_ + kHeaderHeight + 8.0f` and the bottom limit is `height - 8.0f` using an absolute y value.
- `buildRowLayouts()` stops adding rows once `rowTop + kRowHeight > height - 8.0f`, so rows beyond the visible area are not reachable and are not laid out for hit testing or editor placement.
- The inspector currently does not maintain content height, visible height, or scroll offset state.
- `PropertyInspector::draw()` draws only the truncated layout list, so lower rows are effectively inaccessible instead of scrollable.
- `PropertyInspector::hitTestRow()` and `activeEditorBounds()` rely on the same truncated row layout logic, so editing cannot reach rows below the visible panel.
- Callback suggestion placement also relies on the truncated layout list and will need scroll-aware coordinates.

### Input and scrollbar reuse diagnosis

- `MainWindow` currently handles `mouseDown`, `mouseMove`, `mouseDrag`, and `mouseUp` but does not yet override `mouseWheel`.
- Visage does provide `bool mouseWheel(const MouseEvent& e)` and wheel deltas (`precise_wheel_delta_y`) in the dependency headers, so wheel scrolling can be added in this phase.
- Existing scrollbar behavior is available in Visage `ScrollBar` / `ScrollableFrame`; the built-in scrollbar logic provides range, position, drag handling, and wheel-driven behavior that can be mirrored or reused for the inspector chrome.
- The designer already has static scrollbar rendering logic for the `ScrollBar` widget in `DesignerCanvas.cpp`, which is a practical visual reference for the editor-side inspector scrollbar.

### Clipping diagnosis

- `PropertyInspector` currently does not clip its row drawing; it draws rows directly after filling the panel.
- Visage `Canvas` supports clamp bounds via `saveState()`, `setClampBounds(...)`, and `restoreState()`, so inspector row clipping can be added safely in the UI layer.

### Generated C++ regression diagnosis

- The current `VisageCppEmitter` emits generated headers that reference `RuntimeWidget`, `RuntimeRect`, and `std::vector` without emitting the corresponding type definitions or required include in the header.
- The current horizontal scrollbar export path in `emitWidgetDraw()` emits `width` instead of the generated width expression, producing invalid generated code such as `std::max(0.0f, width - arrowSize * 2.0f)` outside a valid local declaration context.
- Existing exported files also show older invalid handler text patterns, but the current emitter regression is primarily structural: missing generated runtime type declarations and malformed emitted scrollbar code.

## Build validation checklist

- [ ] Build the main `VisiForm` project with `build-static-debug`
- [ ] Fix any compile errors introduced by this phase
- [x] Do not run `VisiForm.exe`
- [x] Do not launch the generated app

## Manual test checklist

- [ ] Property Inspector scrolls to the last style/look-and-feel properties
- [ ] Inspector hit testing still matches visible rows after scrolling
- [ ] Text, number, bool, callback, and hint editing still work after scrolling
- [ ] Color rows show swatches and allow manual `#RRGGBB` editing
- [ ] Color picker workflow updates widget color properties
- [ ] `ColorPicker` appears in the palette and can be added to the form
- [ ] `ColorPicker` saves, loads, and exports correctly
- [ ] Exported generated project builds in Visual Studio Debug and Release
- [ ] USER CODE regions remain preserved

## Current progress notes

- Phase plan file created and populated with the initial diagnosis.
- Confirmed that the inspector currently truncates row layout instead of scrolling.
- Confirmed that Visage supports mouse-wheel handling and canvas clamp bounds.
- Identified generated-code regressions in the current emitter before edits.
- Added scroll metrics, row offset handling, content clipping, and a vertical inspector scrollbar with click, drag, and wheel support.
- Updated `MainWindow` input routing so inspector scrolling keeps editor bounds and callback suggestion placement synchronized while scrolled.
- Added a dedicated inspector color edit kind, color swatch rendering, swatch hit testing, and a native `ChooseColorW` workflow that writes back `#RRGGBB` values.
- Expanded color validation to accept empty, `#RRGGBB`, and `#AARRGGBB` manual input for supported color properties and registry-defined color editors.
- Repaired the intermediate `PropertyInspector.cpp` merge errors that were causing local-function parse failures; `build-static-debug` now succeeds again.
- Added `ColorPicker` to the widget model/registry, designer preview, default naming, and editor text-fit behavior so it now appears as a first-class widget.
- Updated generated Visage export handling so `ColorPicker` renders in exported previews and `onChanged` signatures now follow registry metadata, including `string_event` widgets.
- Updated widget catalog and code generation docs for `ColorPicker` support and its string-based `onChanged` event.
- Re-ran the main `VisiForm` build successfully after the widget and generator changes.

## Final result summary

Implemented inspector scrolling and color editing, added editor/export support for `ColorPicker`, repaired the transient `PropertyInspector` compile break, updated generator event-signature routing for registry-defined handlers, and refreshed the related documentation. The main `VisiForm` `build-static-debug` build is passing.

Remaining TODOs:

- Manually export a sample project and build the generated project in Debug and Release if that validation is still required.
