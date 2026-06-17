# Phase 95 Top Tabbed Widget Palette Plan

## Scope

- Replace the old left-side vertical `WidgetPalette` with a compact top tabbed palette below the menu/toolbar.
- Reuse `WidgetRegistry::paletteDefinitions()` and `WidgetDefinition::paletteGroup` / `paletteOrder` metadata as the palette source of truth.
- Preserve the existing click-to-create widget path through `MainWindow::addWidgetFromPalette`.
- Reclaim left-side workspace width for the Project Tree and Designer Canvas.
- Update authoritative version declarations from `1.0.0` to `1.0.1` for Phase 95.

## Requirements

- Use Phase 95 and version `1.0.1`.
- Do not use subagents.
- Do not launch `VisiForm.exe` from the agent.
- Do not rename the `VisiForm` target or modify CMake presets, vcpkg triplets, or MSVC runtime settings.
- Keep model, serialization, validation, and generator behavior unchanged unless required by palette UI changes.

## Version Notes

- Previous version discovered in `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`: `1.0.0`.
- Phase 95 version required by `session-instructions/phase 95.txt`: `1.0.1`.
- `docs/project_status.md` currently contains stale prompt text rather than a maintained status snapshot; Phase 95 will add a concise status entry without performing a broad project-status rewrite.

## Widget-To-Category Mapping

- Initial approach: derive tabs from existing `WidgetDefinition::paletteGroup` values.
- Display names may be normalized for readability while retaining the registry group as the category key.
- Entries inside each category use existing `paletteOrder` metadata, with display name as a tie-breaker.

## Layout Decisions

- Place `WidgetPalette` below the main toolbar and above the primary workspace.
- Remove the old left palette region from layout calculations.
- Keep the Project Tree on the left when vertical space allows.
- Keep `canvasInspectorSplitter_` as the Designer Canvas / Property Inspector splitter.
- Use a compact bounded palette height so the canvas remains usable at narrow heights.

## TODO Checklist

- [x] Inspect current branch and worktree.
- [x] Read Phase 95 brief, `AGENTS.md`, `docs/project_status.md`, and Phase 94 plan.
- [x] Inspect directly relevant palette, registry, main-window layout, version, and widget documentation files.
- [x] Update authoritative version declarations to `1.0.1`.
- [x] Implement top tabbed palette grouping from registry metadata.
- [x] Move palette to the top main-window region and remove the old left palette area.
- [x] Preserve palette click-to-create behavior through the existing creation path.
- [x] Update focused docs/status notes.
- [x] Run focused static validation.
- [ ] Run approved final build validation if available.
- [x] Record runtime validation status and remaining issues.

## Validation Plan

- Run focused static checks after implementation, including `git diff --check`.
- Run available unit tests only if an approved non-build path is available and relevant.
- Build the normal Windows Debug `VisiForm` target once only if the developer approves or an approved Visual Studio workspace path is available.
- Manual runtime validation remains developer-owned unless the developer performs it, because repository rules prohibit automated launch of `VisiForm.exe`.

## Build / Validation Status

- Current branch at start: `main` ahead of `origin/main` by 2 commits.
- Starting worktree included unrelated user changes in `session-instructions/` and `session-instructions/notes.txt`; these are intentionally left untouched.
- Static validation: `git diff --check` passed with LF-to-CRLF normalization warnings only. Targeted registry inspection confirmed all 23 palette-visible widget definitions use one of the seven final category names.
- Automated tests: not run because the existing Catch2 tests require a build through the approved build pipeline.
- Build validation: deferred. The approved Visual Studio workspace pipeline was not available, and repository rules prohibit substituting a terminal build command without an exact developer request.
- Manual runtime validation: not performed. Repository rules prohibit automated launch of `VisiForm.exe`; the developer must verify tabs, widget creation, Project Tree/inspector updates, undo/redo, narrow/maximized/restored layouts, save/reload, and export.

## Final Category List

- Common: Label, Button, TextBox, CheckBox, RadioButton
- Containers: Frame, GroupBox, Panel, TabControl
- Layout: Spacer, Sizer
- Forms: Slider, ScrollBar, ProgressBar, ColorPicker
- Data: ComboBox, ListBox, TreeView, TableGrid
- Menu/Toolbar: MenuBar, ToolBar, StatusBar
- Additional: Image, ModalDialog

The category name remains registry metadata. `WidgetPalette` only defines the display order for known categories and falls back to alphabetical ordering for any future unknown category.

## Layout Changes

- `WidgetPalette` is a compact 82-pixel top region below the main toolbar.
- The first row contains category tabs; the second contains the selected category's widget buttons.
- Both rows provide horizontal mouse-wheel scrolling and arrow controls when their content exceeds the available width.
- The former vertical palette region was removed.
- The Project Tree now owns the left workspace column at normal widths and is hidden below 760 pixels so narrow windows return that width to the Designer Canvas.
- The Project Tree preferred width was reduced from 220 to 190 pixels, giving the Designer Canvas additional horizontal space while preserving the existing canvas/property splitter.

## Files Changed

- `CMakeLists.txt`
- `src/app/Version.h`
- `src/model/WidgetRegistry.cpp`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/ui/MainWindow.cpp`
- `tests/test_widget_palette_registry.cpp`
- `docs/versioning.md`
- `docs/widget_registry.md`
- `docs/widget_catalog.md`
- `docs/project_status.md`
- `docs/agent_plans/phase_95_top_tabbed_widget_palette_plan.md`

## Final Summary

- The palette is now registry-backed, categorized, horizontal, and positioned above the primary workspace.
- Existing `paletteGroup`, `paletteVisible`, and `paletteOrder` metadata remains the source of truth.
- Widget activation still routes through `MainWindow::addWidgetFromPalette`, preserving command-based creation, selection, Project Tree updates, and undo/redo behavior.
- No model persistence, validation, serialization, or generated-code behavior was changed.
- The registry currently has no widget icon metadata, so Phase 95 uses readable widget labels and status-bar hints rather than inventing a duplicate icon map.

## Remaining Issues

- Build validation requires an explicitly approved command under repository safety rules.
- Manual runtime validation remains required because automated agents may not launch `VisiForm.exe`.
- Registry-backed widget icons remain a possible future enhancement if icon metadata is added to `WidgetDefinition`.
