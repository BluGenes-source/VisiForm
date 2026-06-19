# Phase 66 - Layout command regression, StatusBar text rendering, and version 1.0.0 plan

## Phase title

Fix layout command availability, repair StatusBar section text rendering, and set VisiForm version/build metadata to `1.0.0`.

## Current bugs

1. Layout functions are not available for selected widgets.
2. StatusBar sections are not displaying their section text.
3. The application needs a real version number.
4. This version/build must be `1.0.0`.

## Versioning goal

Set repository, app, UI, and related documentation metadata to a consistent `VisiForm 1.0.0` baseline without changing the `VisiForm` target name, `VisiForm.exe`, triplets, runtime settings, generator rules, or USER CODE preservation.

## Files to inspect

- `CMakeLists.txt`
- `CMakePresets.json`
- `README.md`
- `src/app/App.h`
- `src/app/App.cpp`
- `src/app/main.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/WidgetMetrics.h`
- `src/ui/WidgetMetrics.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetDefinition.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetNode.h`
- `src/model/ProjectDocument.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CodeGenerator.cpp`
- `docs/widget_catalog.md`
- `docs/menu_bar.md`
- `docs/code_generation.md`
- `docs/agent_plans/phase_66_layout_statusbar_version_1_0_0_plan.md`

## Diagnosis notes

### Layout command availability

- `MainWindow::isCommandEnabled(...)` currently controls menu enabled state for layout commands.
- The `Layout` menu still contains `Align`, `Center`, `Same Size`, `Distribute`, `Bring Forward`, `Send Backward`, and `Fit Text` entries.
- The toolbar currently exposes only high-frequency commands and does not include layout commands after the menu-bar cleanup, which matches the current toolbar design documented in `docs/menu_bar.md`.
- Menu and toolbar command routing still converge through `MainWindow::executeCommand(...)`; layout commands are not routed through a second duplicate path.
- Single-selection detection currently depends on `document_.selectedWidget()` plus `!document_.isRootWidgetId(...)`.
- Multi-selection detection currently depends on `document_.selectedWidgetIds().size() >= 2`.
- Selection state is stored in `ProjectDocument` and synchronized through `selectedWidgetId` plus `selectedWidgetIds_`.
- `handleWidgetClicked(...)` is shared by the designer canvas and `ProjectTree`, and it calls `redraw()` after selection changes, so selection changes should refresh menu enabled state and related panels.
- Current inspection indicates the menu-bar refactor did not disconnect layout command routing, and the commands still exist and remain visible in the `Layout` menu.
- Current inspection also indicates the toolbar reduction was intentional rather than a routing regression, so this phase should keep layout commands menu-first instead of re-adding toolbar buttons.
- The remaining layout issue is that command availability rules and invalid-selection status feedback are spread across `isCommandEnabled(...)` and many individual handlers, which makes the selection requirements harder to keep consistent.
- Layout handlers currently use mixed fallback status text such as `No widget selected` and `Cannot layout root form`; this phase should normalize those messages to the requested guidance for missing single- and multi-selection cases.

### StatusBar section text rendering

- `WidgetRegistry` currently defines `StatusBar` properties as `fields`, `text0`, `text1`, `text2`, `fieldWidths`, `dock`, `fillWidth`, and `hint`.
- Current default `StatusBar` height is already `50`, dock is `Bottom`, and `fillWidth` defaults to `true`.
- Current defaults only populate `text0 = Ready`; `text1` and `text2` are empty and `fields` defaults to `1`, which explains why newly inserted StatusBars do not show the requested three visible section labels.
- `DesignerCanvas` currently renders StatusBar text from `text0`, `text1`, `text2`, and `text3` keys based on the `fields` count.
- Designer rendering currently uses `Font::kTopLeft` plus `centeredTextTop(...)`; this mixed alignment approach is a likely cause of clipped or top-biased field text.
- Designer rendering currently divides the control into equal-width sections and draws divider lines, so the bug is likely text content/defaults/alignment rather than missing section geometry.
- Generated runtime currently exports StatusBar field strings from `text0...textN` into `widget.items`, and `setStatusBarField(...)` updates those runtime items.
- Generated runtime drawing also uses `Font::kTopLeft` with a fixed top inset, so the exported app is likely to share the same vertical-alignment issue as the designer.
- The Property Inspector currently uses raw registry labels such as `text0`, `text1`, and `text2`, so it needs user-friendly `Section 1`, `Section 2`, and `Section 3` labels.

### Version metadata

- Root `CMakeLists.txt` currently declares `project(VisiForm VERSION 0.1.0 LANGUAGES CXX)`.
- `MainWindow` currently uses a hard-coded window title string `VisiForm - Visage Form Builder`.
- Need to inspect existing About dialog text flow and centralize version metadata so UI strings and generated comments can reuse the same `1.0.0` version source.
- `README.md` currently does not mention the current version.
- `VisageCppEmitter` currently uses a generated file header that says `Generated by VisiForm - Visage Form Builder.` and can be updated to include the released version if implemented safely.

## TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect the requested source, generator, CMake, and documentation files.
- [x] Confirm the exact layout command regression cause and record whether it affects menu enablement, toolbar behavior, or both.
- [x] Confirm whether selection changes refresh layout command availability and related editor panels correctly.
- [ ] Repair shared layout command availability and invalid-selection safety without duplicating command logic.
- [x] Confirm the exact StatusBar text-rendering cause in the designer and generated runtime path.
- [ ] Repair StatusBar defaults, readable inspector labels, designer rendering, and generated runtime field rendering.
- [ ] Add a centralized version definition for `1.0.0` if no equivalent source already exists.
- [x] Update CMake project version, main window title, About dialog text, and any safe generated-code version comments to `1.0.0`.
- [ ] Update `README.md` with the current version only, without reworking unrelated sections.
- [ ] Update or create `docs/versioning.md` with the `1.0.0` versioning guidance.
- [x] Build the main `VisiForm` app with the `build-static-debug` preset.
- [x] Fix any compile errors introduced by this phase.
- [x] Update this phase plan with build validation, final result summary, and remaining TODOs.

## Build validation checklist

- [x] Configure with the main Windows preset used by `build-static-debug`.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Confirm the main `VisiForm` app built successfully.
- [x] Confirm `VisiForm.exe` was not run.
- [x] Confirm no generated apps were launched.

## Manual test checklist

- [ ] Select one widget and verify `Fit Text`, `Bring Forward`, `Send Backward`, and any single-selection layout commands enable in the `Layout` menu.
- [ ] Select two or more widgets and verify multi-selection layout commands enable in the `Layout` menu.
- [ ] Trigger a layout command with no valid selection and verify the editor shows a helpful status message instead of crashing.
- [ ] Edit StatusBar section text in the Property Inspector and verify the designer preview updates with readable vertically centered text.
- [ ] Save and reload a project containing a StatusBar and verify section text persists.
- [ ] Export a project containing a StatusBar and verify generated runtime StatusBar text is visible.
- [ ] Verify the main window title shows `VisiForm 1.0.0 - Visage Form Builder` while preserving unsaved-marker behavior.
- [ ] Open `Help > About VisiForm` and verify it reports version `1.0.0`.

## Final result summary

- Repaired the active `src/ui/MainWindow.cpp` build regression by replacing stale `kWindowTitle` references with the centralized version helpers from `src/app/Version.h`.
- `MainWindow::MainWindow()` now uses `makeWindowTitle(false)` and `MainWindow::updateWindowTitle()` now uses `makeWindowTitle(document_.dirty)` so title text stays aligned with the `1.0.0` version metadata and unsaved-marker behavior.
- Validation was run with `cmake --build --preset build-static-debug --target VisiForm`.
- The `MainWindow.cpp` compile error is resolved; the current build is now blocked later at link time by a pre-existing x86/x64 library mismatch in the local Windows SDK / MSVC environment.

## Remaining TODOs

- Complete the remaining phase 66 layout and StatusBar work.
- Resolve the local build environment linker mismatch so `build-static-debug` can complete successfully.
- Re-run `cmake --build --preset build-static-debug --target VisiForm` after the environment issue is fixed.
