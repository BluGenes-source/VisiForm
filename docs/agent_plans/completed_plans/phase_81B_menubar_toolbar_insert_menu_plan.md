# Phase 81B - MenuBar/ToolBar editor clarity and Insert menu coverage plan
# Phase 81B - MenuBar/ToolBar editor clarity and Insert menu coverage plan

## Phase title
MenuBar/ToolBar editor clarity and Insert menu coverage

## Current state
`MenuBar` and `ToolBar` currently expose both `Items` and `Action Bindings`, but both entries lead into the same modal workflow and the modal can still read like a label-only editor. `MainWindow::menus()` also hard-codes the `Insert` menu, so newer palette widgets are missing there.

## Goal
Make the shared item editor clearly communicate that `MenuBar` and `ToolBar` rows edit both labels and callback/action names, and make the `Insert` menu reflect the intended addable palette widgets through a registry-driven source.

## Files to inspect
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetItemUtils.h`
- `src/model/WidgetItemUtils.cpp`
- `docs/widget_catalog.md`
- `docs/menu_bar.md`
- `docs/project_file_format.md`
- `docs/agent_plans/phase_81B_menubar_toolbar_insert_menu_plan.md`

## Investigation notes
- `MainWindow::beginInspectorEdit()` routes both `items` and `itemActions` to the same shared item editor modal.
- `PropertyInspector` already summarizes `items`, `itemActions`, and `Selected Action`, but the shared modal still uses generic row labels such as `Item Text` and a generic combined title.
- `MainWindow::menus()` currently hard-codes `Insert` menu entries and omits several palette-visible widgets that are already exposed through `WidgetRegistry::paletteDefinitions()`.

## Implementation notes
- `src/ui/MainWindow.h` now stores the edited widget type inside the shared item-editor dialog state so the modal can present MenuBar and ToolBar specific wording consistently.
- `src/ui/MainWindow.cpp` now uses `Edit Menu Items` and `Edit Tool Items` titles, clearer `Label` and `Callback / Action` field labels, explicit action-binding status text, and preview rows that display both label and callback/action values.
- The shared item-editor keeps the existing `items`, `itemActions`, selected-index, and undoable apply flow unchanged for `ComboBox`, `ListBox`, `MenuBar`, and `ToolBar`.
- `MainWindow::menus()` now builds the `Insert` menu from `WidgetRegistry::paletteDefinitions()` and derives stable `insert-...` ids from each widget type name while explicitly skipping `FormWindow` and `TabPage`.
- Documentation was updated in `docs/widget_catalog.md`, `docs/menu_bar.md`, and `docs/project_file_format.md` to reflect the clearer item editor and registry-driven insert coverage.

## TODO checklist
- [x] Create this phase plan file before changing code.
- [x] Inspect the requested UI, model, and documentation files for the current item-editor and insert-menu behavior.
- [x] Update this plan with implementation progress as work proceeds.
- [x] Improve `MenuBar` and `ToolBar` item-editor clarity without breaking `ComboBox` or `ListBox` editing.
- [x] Keep item/action alignment, undo/redo behavior, and `items`/`itemActions` persistence intact.
- [x] Replace hard-coded `Insert` menu widget entries with registry-driven palette coverage while excluding `FormWindow` and direct `TabPage` insertion.
- [x] Update documentation for the clearer editor behavior and expanded insert coverage.
- [x] Validate edited files for compile issues introduced by this phase.
- [x] Record Visual Studio workspace build validation for the `VisiForm` target, or explicitly defer build validation to the developer if the workspace pipeline is unavailable or ambiguous.
- [x] Write the final result summary and remaining manual TODOs before finishing.

## Build validation checklist
- [x] No repository build scripts, generated build scripts, terminal build commands, PowerShell build commands, or `cmd.exe` build commands were run in this phase.
- [ ] Validation used the Visual Studio workspace build pipeline for the `VisiForm` target only.
- [x] Validation was deferred to the developer because the approved workspace build pipeline was unavailable or ambiguous.
- [x] No generated projects or executables were launched.
- [x] No files under `Generated/` were modified.

### Latest build validation
- [x] File-level diagnostics for `src/ui/MainWindow.h` and `src/ui/MainWindow.cpp` were checked after the edits and reported no errors.
- [x] `get_projects_in_solution` returned no open solution/workspace, so the approved Visual Studio workspace build pipeline for `VisiForm` was unavailable to the agent in this session.
- [x] Main app build validation is deferred to the developer through Visual Studio for the `VisiForm` target only.

## Final result summary
Phase 81B updated `src/ui/MainWindow.h`, `src/ui/MainWindow.cpp`, `docs/widget_catalog.md`, `docs/menu_bar.md`, `docs/project_file_format.md`, and this phase plan. The shared item editor now presents clearer `MenuBar` and `ToolBar` wording with `Edit Menu Items` / `Edit Tool Items` titles, preview rows that show both `Label` and `Callback / Action`, clearer field labels, explicit action-binding status text, and safer `Add Item` / `Remove Item` button text without changing the underlying `items`, `itemActions`, selected-index, or undoable apply flow. The `Insert` menu now comes from `WidgetRegistry::paletteDefinitions()` so it covers the same intended addable widgets as the palette while continuing to exclude `FormWindow` and direct `TabPage` insertion. Documentation was updated to match. No files under `Generated/` were changed, no scripts were created, no build scripts or terminal build commands were run, and main-app build validation was deferred to the developer because the approved Visual Studio workspace build pipeline was unavailable in this session.

## Remaining TODOs
- Build the `VisiForm` target in Visual Studio because the approved workspace build pipeline was unavailable to the agent.
- Manually verify the `MenuBar` and `ToolBar` editor modal wording, preview rows, add/remove/reorder behavior, and persisted save/load behavior.
- Manually verify the `Insert` menu now matches the addable palette widgets and still excludes `FormWindow` and direct `TabPage` insertion.
