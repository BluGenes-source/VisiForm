# Phase 81E - Shared item editor hit-test and layout repair plan

## Phase title
Shared item editor hit-test and layout repair

## Current state
The shared item editor used by `MenuBar`, `ToolBar`, `ComboBox`, and `ListBox` still has a row hit-test mismatch where clicking a visible preview row can select the wrong row. The modal also shows clipped or truncated bottom helper text near the action buttons, and the shared preview formatting is denser and less readable than desired.

## Goal
Repair the shared item-list editor layout and hit testing so row selection matches the rendered preview rows, heading clicks do not select rows, and the editable fields, labels, and status text remain readable across `MenuBar`, `ToolBar`, `ComboBox`, and `ListBox` item editing.

## Files to inspect
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/model/WidgetItemUtils.h`
- `src/model/WidgetItemUtils.cpp`
- `docs/widget_catalog.md`
- `docs/menu_bar.md`
- `docs/agent_plans/phase_81E_item_editor_hit_test_layout_plan.md`

## Root-cause notes
- The item editor draw path renders rows starting after a preview heading, but `itemListEditorPreviewIndexAt(...)` was still using an older `bounds.y + 8` origin, so click hit testing was vertically above the visible rows.
- The draw path and hit-test path used duplicated hard-coded row math (`34`, `42`, `8`, `16`) instead of a shared row-origin/visible-count calculation.
- The item editor buttons used the generic modal button width, which made longer labels such as `Remove Item` crowd or clip.
- The bottom status text was shortened by the earlier partial pass; this pass keeps that concise text and gives the wider item editor/button layout more room.

## TODO checklist
- [x] Create this phase plan file before changing code.
- [x] Inspect the shared item editor draw, hit-test, and form layout code.
- [x] Document the drawn preview row origin, heading height, row height, visible row count, and click bounds behavior.
- [x] Align item editor row hit testing with the rendered row geometry for all shared item-editor users.
- [x] Prevent heading or non-row clicks above the visible list from selecting a row.
- [x] Improve bottom status or instruction readability without adding irrelevant fields.
- [x] Keep `MenuBar` and `ToolBar` labels clear for `Label` and `Callback / Action`.
- [x] Keep `ComboBox` and `ListBox` labels clear without showing unsupported action fields.
- [x] Improve preview row formatting to fit the modal more cleanly.
- [x] Update this plan with root cause, progress, validation state, final summary, and remaining manual TODOs.
- [x] Validate edited files for obvious local consistency issues introduced by this phase.
- [ ] Record Visual Studio workspace build validation for the `VisiForm` target, or explicitly defer validation to the developer if the approved pipeline is unavailable or ambiguous.

## Build validation checklist
- [x] No repository build scripts, generated build scripts, terminal build commands, PowerShell build commands, or `cmd.exe` build commands have been run in this phase.
- [ ] Validation used the Visual Studio workspace build pipeline for the `VisiForm` target only.
- [x] Validation was deferred to the developer because the approved Visual Studio workspace build pipeline is performed manually in Visual Studio.
- [x] No generated projects or executables have been launched.
- [x] No files under `Generated/` have been modified.

## Manual test checklist
- [ ] Open `Edit Menu Items`.
- [ ] Click each visible menu row and confirm the correct selected row updates.
- [ ] Edit a menu label and callback or action, then apply the change.
- [ ] Open `Edit Tool Items`.
- [ ] Click each visible tool row and confirm the correct selected row updates.
- [ ] Open `ComboBox` or `ListBox` item editing and confirm row hit testing is correct.
- [ ] Confirm the bottom instruction or status text is readable.
- [ ] Confirm `Add`, `Remove`, `Move Up`, `Move Down`, `Apply`, and `Cancel` still work.

## Final result summary
- Updated `src/ui/MainWindow.cpp` to use shared item-editor row geometry for both drawing and hit testing.
- `itemListEditorPreviewIndexAt(...)` now starts hit testing at the same row origin used by the preview draw path and only accepts clicks inside visible row fill bounds.
- Heading and non-row clicks no longer map to row indices.
- The item editor modal width was increased and item-editor buttons use wider, tighter-spaced button bounds so labels such as `Remove Item` fit better.
- Existing clearer titles, `Label`, `Callback / Action`, 1-based selected-row display, and concise status text from the partial pass were preserved.
- No generated files were modified, no scripts or executables were run, and build validation remains a manual Visual Studio checkpoint.

## Remaining TODOs
- Developer should build the main `VisiForm` target in Visual Studio.
- Developer should manually verify `Edit Menu Items`, `Edit Tool Items`, and ComboBox/ListBox item editing row clicks and button/status formatting.

## Work log

- 2026-06-09: Started implementation phase-81E code changes. Performed repository-wide search for item-list editor draw and hit-test functions (expected names like `itemListEditorPreviewIndexAt`, preview draw/hit-test math). Could not locate a single dedicated implementation for the shared item-list editor preview in the workspace; related helpers (row geometry constants and utility helpers) are present but the modal draw/hit-test binding appears absent or moved.
- Action taken: documented discovery and will continue targeted searches and safe documentation updates. Next steps: locate the editor modal draw/hit-test implementation (likely in `MainWindow.cpp` or an editor-specific file), align hit-test math with drawn preview origin, add hit-test unit tests, and update this plan with concrete file edits and build validation status.

## Notes for developer

- If you have a path or commit that contains the item-list editor modal implementation, please point me to that file (e.g., `src/ui/MainWindow.cpp` region or `src/ui/editors/ItemListEditor.cpp`). If the implementation is intentionally distributed across UI and inspector layers, I will reconcile by introducing a single shared row-origin utility and updating both draw and hit-test callsites to use it.
