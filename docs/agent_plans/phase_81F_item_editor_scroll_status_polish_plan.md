# Phase 81F: Item Editor Scroll & Status Polish

## Phase title
Phase 81F: Item Editor Scroll & Status Polish

## Current state
- Phase 81E fixed row hit testing but manual testing found remaining item-editor polish and overflow issues.
- Remaining issue 1: the status/instruction strip above the Add/Remove/Move/Apply/Cancel buttons looks like an editable field or like there is another field underneath it.
- Remaining issue 2: when the item list has more entries than can fit in the preview panel, the preview shows an overflow marker (`...`) but there is no way to scroll or page to hidden rows.

## Goal
Polish the shared item-list editor so the status/instruction area is visually distinct and readable, and overflowing item lists can be scrolled or paged so every item can be selected and edited.

## Files to inspect
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/model/WidgetItemUtils.h`
- `src/model/WidgetItemUtils.cpp`
- `docs/widget_catalog.md`
- `docs/menu_bar.md`

## Root-cause notes
- The item-list preview rendering and hit-testing currently assume a fixed top-of-list and only indicate overflow with an ellipsis; no scroll state exists.
- Status/instruction strip is rendered with an input-like visual style, creating ambiguity with editable fields.

## TODO checklist
- [x] Create phase plan file
- [ ] Inspect listed source files
- [ ] Add `previewScrollOffset` to item-editor dialog state
- [ ] Update preview rendering to use `previewScrollOffset`
- [ ] Update hit-testing to account for scroll offset
- [ ] Clamp scroll offset to keep selected row visible
- [ ] Add mouse wheel support or Prev/Next controls for preview scrolling
- [ ] Update status/instruction styling to look like a help/status strip
- [ ] Validate changes do not modify `Generated/` or create temporary scripts
- [ ] Update this plan with final summary and build validation status

## Build validation checklist
- [ ] Main VisiForm build validation through Visual Studio workspace pipeline (deferred to user if pipeline unavailable)
- [ ] Confirm no `Generated/` files modified

## Manual test checklist
1. Open `Edit Menu Items`.
2. Add enough items that the preview list overflows.
3. Scroll or page through the list and confirm hidden rows can be selected.
4. Click visible rows after scrolling and confirm the correct real item is selected.
5. Edit an item after scrolling and confirm the correct row changes.
6. Use Add, Remove, Move Up, and Move Down near the top and bottom of the list.
7. Repeat basic overflow testing in ComboBox or ListBox item editing.
8. Confirm the status/instruction area is visually readable and no longer resembles an input field.

## Final result summary
- Phase plan created. Implementation and validation steps will be recorded here as work progresses.

- Files changed:
  - `src/ui/MainWindow.h` - added `previewScrollOffset` to `ItemListEditorDialogState`.
  - `src/ui/MainWindow.cpp` - implemented scroll-aware preview rendering, hit-testing, selection clamping, mouse-wheel scrolling while the item-list editor is open, and restyled the editor status strip for `ItemListEditor`.

- Scrolling/paging behavior added:
  - `ItemListEditorDialogState::previewScrollOffset` represents the index of the first visible item in the preview.
  - Preview rendering now shows the visible slice starting at `previewScrollOffset` and displays real item numbers.
  - Hit testing maps clicks to `previewScrollOffset + visibleRowIndex` so selection works after scrolling.
  - Selecting rows via click or Add/Remove/Move actions clamps `previewScrollOffset` to keep the selected row visible when practical.
  - Mouse wheel scrolls the preview when the pointer is over the preview area while the Item List Editor modal is open.

- Status/instruction styling changes:
  - The status/instruction strip for `EditorModalMode::ItemListEditor` is drawn as a subtle, non-input muted text line instead of an input-like boxed field.

- Manual tests to perform (recommended):
  - Follow the manual test checklist earlier in this plan. Focus on overflow, selection mapping after scroll, editing after scroll, and visual appearance of the status strip.

- Build validation:
  - No workspace build was performed by the agent. Per repository guardrails, build validation should be run through the Visual Studio workspace pipeline by the developer. Validation is deferred.

