# Phase 59 modal size and menu bar toolbar cleanup plan

## Phase title
Phase 59 modal size and menu bar toolbar cleanup

## Current bug
The internal `VisiForm` validation dialog opened by `Chk / Validate` appears full screen instead of as a centered modal-sized dialog, and the main toolbar is crowded enough that commands are becoming hard to find.

## Goal
1. Fix the internal validation modal so it opens as a centered modal dialog instead of appearing full screen.
2. Add a menu bar system to `VisiForm`.
3. Move major commands into menus while keeping existing commands working.
4. Reduce toolbar clutter so only high-frequency commands remain visible.
5. Reuse the same editor modal system for `Help > About VisiForm`.

## Files to inspect
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/validation/ProjectValidator.h`
- `src/validation/ProjectValidator.cpp`
- `src/model/ProjectDocument.h`
- `src/model/WidgetRegistry.h`
- `docs/project_validation.md`
- `docs/widget_catalog.md`
- `docs/code_generation.md`
- `docs/agent_plans/phase_59_modal_size_and_menu_bar_toolbar_cleanup_plan.md`

## Modal sizing diagnosis
- [x] Locate where the editor modal rectangle is calculated.
  - `MainWindow::editorModalDialogBounds()` calculates the editor modal rectangle in `src/ui/MainWindow.cpp`.
- [x] Check whether modal bounds use full window bounds by mistake.
  - The dialog width and height are derived from `width()` and `height()` of the full main window instead of explicit modal sizing rules. This does not directly assign overlay bounds to the panel, but it does make the modal depend on full-window space.
- [x] Check whether validation content forces the dialog to full-screen.
  - Validation content is already capped to a small preview and does not directly force full-screen height.
- [x] Check whether width or height are assigned from overlay bounds instead of desired dialog size.
  - The panel uses a computed size, not the overlay rectangle, but it currently uses generic full-window availability instead of fixed min/max dialog rules requested for this phase.
- [x] Check whether generated `ModalDialog` rendering was reused incorrectly.
  - The validation and message dialogs use separate editor-only code in `MainWindow`; the generated widget runtime is not reused here.
- [x] Capture current likely root cause.
  - The current overlay is drawn as a fully opaque full-window fill, so the dialog can visually read as full screen even though a centered panel is also rendered.

## TODO checklist
- [x] Inspect current modal sizing and command routing.
- [x] Create this phase plan before code changes.
- [x] Add explicit menu bar and toolbar rows to the main window layout.
- [x] Centralize shared UI command dispatch so menus and toolbar use the same handlers.
- [x] Fix editor modal sizing to use centered min/max dialog bounds with visible margins.
- [x] Limit validation preview lines so content does not force large dialog growth.
- [x] Add menu rendering, menu hit testing, and dropdown drawing above editor panels.
- [x] Add `File`, `Edit`, `View`, `Insert`, `Layout`, `Project`, `Export`, and `Help` menus.
- [x] Route insert actions through the existing add-widget flow.
- [x] Reuse the editor modal system for `Help > About VisiForm`.
- [x] Reduce toolbar buttons to high-frequency commands while preserving hints.
- [x] Update documentation for menu structure, toolbar cleanup, and validation modal behavior.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Write the final result summary and remaining TODOs.

## Build validation checklist
- [x] Build only the main `VisiForm` project.
- [x] Use the `build-static-debug` workflow.
- [x] Confirm the build completes successfully.
- [x] Fix compile errors introduced by this phase.
- [x] Record the successful build result in this plan.
- [x] Do not run `VisiForm.exe`.
- [x] Do not launch generated apps.

Build result:

- Initial plain-shell build attempt failed with x86 SDK link mismatches.
- Retried `build-static-debug` from `VsDevCmd.bat -arch=x64 -host_arch=x64`.
- Main `VisiForm` app built successfully after that retry.

## Manual test checklist
- [ ] Open the editor and trigger `Chk` from the toolbar.
- [ ] Confirm the validation dialog is centered and no longer appears full screen.
- [ ] Confirm the validation dialog keeps visible margins around all sides.
- [ ] Confirm validation messages are clipped to a reasonable preview count.
- [ ] Confirm the dialog shows `Generated/validation_report.md` guidance.
- [ ] Confirm clicking outside the dialog does not activate underlying UI.
- [ ] Confirm `OK` closes the modal.
- [ ] Confirm `Escape` closes the modal.
- [ ] Confirm `Enter` activates `OK`.
- [ ] Open each top-level menu and confirm the dropdown appears above the canvas and inspectors.
- [ ] Confirm `File` commands still work.
- [ ] Confirm `Edit` commands still work.
- [ ] Confirm `View` toggles still work.
- [ ] Confirm `Insert` menu items add the expected widget types.
- [ ] Confirm `Layout` commands still work.
- [ ] Confirm `Project > Validate` opens the same centered validation modal.
- [ ] Confirm `Help > About VisiForm` opens a centered modal.
- [ ] Confirm the trimmed toolbar still exposes high-frequency actions and hints.
- [ ] Confirm save, load, and export behavior still works.

## Final result summary
- Added a dedicated menu bar row above the toolbar in `MainWindow`.
- Added lightweight editor-local `File`, `Edit`, `View`, `Insert`, `Layout`, `Project`, `Export`, and `Help` menus.
- Centralized menu and toolbar actions through shared `MainWindow` command routing.
- Reduced the toolbar to high-frequency commands while keeping existing hint text behavior.
- Updated the editor modal sizing rules so validation and help dialogs stay centered with visible margins instead of reading as full screen.
- Kept validation preview content capped so long reports do not force the dialog to full-screen height.
- Reused the same editor modal system for `Help > About VisiForm`, keyboard shortcuts, generated code guide, and simple project-setting guidance.
- Updated documentation in `docs/menu_bar.md`, `docs/project_validation.md`, `docs/widget_catalog.md`, and `docs/code_generation.md`.
- Built the main `VisiForm` app successfully with `build-static-debug` after retrying from the x64 Visual Studio developer environment.

## Remaining TODOs
- Manually verify the new menu interactions and modal sizing inside the running editor.
- Consider richer future actions for `Open Export Folder` and similar placeholder convenience commands.
