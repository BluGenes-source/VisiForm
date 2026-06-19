# Phase 47 editor bugfix stabilization plan

## Phase title

Phase 47 editor bugfix stabilization

## Bug list

1. `ProjectTree` needs a vertical scrollbar.
2. `Label` needs font settings: `fontFamily`, `fontSize`, `fontBold`, `fontItalic`.
3. `fontSize` appears not to work on any widget.
4. `Label` text moves too high on the canvas when `fontSize` changes.
5. `RadioButton` event row/dropdown can be hidden when the `PropertyInspector` is short.
6. `ScrollBar.orientation` should be a dropdown with `Horizontal` / `Vertical`.
7. Need a `ScrollBar` widget/control that can attach to a window/component later.
8. `StatusBar` should automatically attach to the root window bottom and fill width.
9. Changing the main window background color breaks subtle major/minor grid contrast.
10. Adding `CheckBox` and then `RadioButton` places `RadioButton` on top of `CheckBox`.
11. `RadioButton` selected state must always be mutually exclusive inside a group.
12. `RadioButton` visual indicator is square but should be round.
13. Copilot instructions should require a new phase plan and build validation for each phase.

## Files to inspect

- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetDefinition.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetNode.h`
- `src/model/ProjectDocument.h`
- `src/model/LookAndFeelDefinition.h`
- `src/model/LookAndFeelRegistry.h`
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `docs/widget_catalog.md`
- `docs/widget_registry.md`
- `docs/look_and_feel.md`
- `docs/project_file_format.md`
- `docs/selection_model.md`

## Step-by-step TODO list with markdown checkboxes

- [x] Create the persistent phase plan file before code changes
- [x] Inspect the requested editor, model, generator, settings, docs, and instruction files
- [x] Strengthen Copilot instruction files for per-phase plans and required build validation
- [ ] Add vertical scrolling and scrollbar interaction to `ProjectTree`
- [ ] Keep `ProjectTree` hit testing and multi-selection accurate while scrolled
- [ ] Add common font properties to text-capable widget definitions
- [ ] Make `fontSize` affect editor drawing and layout where supported
- [ ] Fix label vertical text placement for font-size changes
- [ ] Make fit-text and auto-size account for widget font size
- [ ] Keep `PropertyInspector` event suggestions visible in smaller inspector heights
- [ ] Add dropdown-style property support in the registry and inspector
- [ ] Use the dropdown for `ScrollBar.orientation`
- [ ] Add simple root-bottom attach behavior for `StatusBar`
- [ ] Fix new-widget placement to avoid overlap and respect bottom-docked status bars
- [ ] Enforce `RadioButton` mutual exclusivity through edits, duplication, and normalization flows
- [ ] Render `RadioButton` with a round indicator in the designer and generated output where practical
- [ ] Restore distinct major/minor grid colors after background color changes
- [ ] Update docs for scrolling, font properties, dropdowns, docking, radio groups, and grid behavior
- [ ] Build the main `VisiForm` app successfully with `build-static-debug`
- [ ] Write the final result summary and remaining manual test notes

## Current progress notes

- Phase plan created before code changes.
- Repository instruction files were read first.
- Initial inspection started for `ProjectTree`, `PropertyInspector`, `DesignerCanvas`, `MainWindow`, `WidgetRegistry`, and `ProjectDocument`.
- Confirmed `ProjectTree` currently truncates rows instead of scrolling and has no internal scroll state.
- Confirmed `PropertyInspector` already has scroll state, but callback suggestions are clipped to remaining visible space below the active row.
- Confirmed common style metadata already stores `fontSize`, but drawing still uses one shared editor font and auto-size width estimates still assume the default font size.
- Confirmed `ScrollBar.orientation` is currently plain text metadata in `WidgetRegistry`.
- Confirmed `StatusBar` insertion and general widget placement currently use the same root-child flow and can overlap later widgets.
- Confirmed `ProjectDocument` already has radio-group normalization helpers that can be reused and strengthened.
- Confirmed `docs/selection_model.md` does not currently exist in the workspace.
- Main application execution remains prohibited in Agent Mode.
- Updated both repository instruction files to require per-phase plan creation, checklist maintenance, build validation, and final summary updates before completion.
- `DesignerCanvas.cpp` had phase-introduced references to missing helper functions for grid colors, widget fonts, and circle drawing; those helpers were added.
- Current full-build output is masked by MSVC toolchain header lookup failures (`<string>`, `<memory>`), so source-level validation is being done incrementally while that broader build issue remains present.

## Build validation checklist

- [ ] Build only the main `VisiForm` project
- [ ] Use the `build-static-debug` workflow
- [ ] Fix compile errors introduced by this phase
- [ ] Do not run `VisiForm.exe`
- [ ] Do not use `Start-Process`
- [ ] Do not launch generated applications
- [ ] Record successful build completion in this phase plan

## Manual test checklist

- [ ] `ProjectTree` scrolls vertically with mouse wheel and scrollbar interactions
- [ ] `ProjectTree` selection and multi-selection still work after scrolling
- [ ] `Label`, `Button`, `TextBox`, `CheckBox`, `RadioButton`, `StatusBar`, `ProgressBar`, `ColorPicker`, and `Frame` expose the new font properties where applicable
- [ ] `fontSize` changes affect drawing or at minimum layout placement without regressing text alignment
- [ ] Label text stays vertically centered instead of shifting too high
- [ ] Fit Text / auto-size expands appropriately for larger font sizes
- [ ] `RadioButton` event suggestions remain accessible in a short `PropertyInspector`
- [ ] `ScrollBar.orientation` uses a dropdown and updates the preview immediately
- [ ] Adding a `StatusBar` docks it to the bottom of the root form and fills width
- [ ] Adding `CheckBox` then `RadioButton` places the second widget below the first without overlap
- [ ] `RadioButton` groups keep only one selected item after edits, duplication, and load normalization
- [ ] `RadioButton` indicator renders round or the best available circular approximation
- [ ] Changing root background color keeps major and minor grid lines visually distinct
- [ ] Save/load/export still behave correctly after the changes

## Final result summary

In progress.

Remaining TODOs:

- Complete implementation, documentation, and validation items above.
- Perform the manual editor verification steps in Visual Studio after the main build passes.
