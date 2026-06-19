# Phase 45 look and feel style system plan

## Goal

Add a simple, extensible Look and Feel system for VisiForm that supports built-in presets, a project-level look-and-feel selection, per-widget style overrides, style-aware editor rendering, style-aware generated preview rendering, and documentation for future theme expansion.

## Current problems

- There is no dedicated look-and-feel registry or preset system.
- The root form does not expose a global `lookAndFeelId`.
- Widgets do not have a shared style override foundation.
- Designer rendering is hard-coded instead of style-driven.
- Generated preview drawing is not style-aware.
- Documentation does not describe theme presets, inheritance, or override behavior.

## Files to inspect

- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/model/ProjectDocument.h`
- `src/model/WidgetNode.h`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `docs/widget_registry.md`
- `docs/widget_catalog.md`
- `docs/project_file_format.md`
- `docs/code_generation.md`
- `docs/look_and_feel.md`
- `docs/agent_plans/phase_45_look_and_feel_style_system_plan.md`

## Step-by-step TODO list with checkboxes

- [x] Create persistent phase plan file before code changes
- [x] Inspect project, registry, inspector, designer, and generator style integration points
- [x] Add model-neutral look-and-feel definition and registry files
- [x] Add built-in presets: `VisiFormDark`, `VisiFormLight`, `ImGuiDark`, `FlatClassic`
- [x] Add project-level `lookAndFeelId`
- [x] Add per-widget style override properties
- [x] Add UI-layer style resolution helpers
- [x] Update DesignerCanvas rendering to use resolved styles
- [x] Expose style properties in PropertyInspector
- [x] Validate style color and numeric property inputs
- [x] Persist look-and-feel data through save/load
- [x] Export style-aware generated preview drawing
- [x] Update docs including new `docs/look_and_feel.md`
- [x] Build with `build-static-debug`
- [x] Write final result summary

## Current progress notes

- Phase plan file created before edits.
- Added model-neutral `LookAndFeelDefinition` and `LookAndFeelRegistry` files with built-in presets `VisiFormDark`, `VisiFormLight`, `ImGuiDark`, and `FlatClassic`.
- Added project-level `lookAndFeelId` to `ProjectDocument` and persisted it in JSON save/load with a backward-compatible `VisiFormDark` default.
- Added common widget style override properties in `WidgetRegistry`: `lookAndFeelId`, `fillColor`, `textColor`, `borderColor`, `accentColor`, `borderThickness`, `cornerRadius`, and `fontSize`.
- Root `FormWindow` now exposes the project `lookAndFeelId` in `PropertyInspector`, and style override rows are grouped under a `Style` section.
- `MainWindow` now validates look-and-feel ids, style color strings, and clamped numeric style values.
- `DesignerCanvas` now resolves project presets plus widget overrides and applies style colors and border thickness across built-in widget rendering paths.
- Generated preview rendering in `VisageCppEmitter` now resolves the same project and widget style data during export and emits style-aware static drawing.
- Added documentation updates in `docs/widget_registry.md`, `docs/widget_catalog.md`, `docs/project_file_format.md`, `docs/code_generation.md`, and created `docs/look_and_feel.md`.

## Build validation checklist

- [x] Build the main `VisiForm` project with `build-static-debug`
- [x] Fix any compile errors introduced by this phase
- [x] Do not run `VisiForm.exe`
- [x] Do not launch the generated app

## Manual test checklist

- [ ] Root `FormWindow` shows editable `lookAndFeelId`
- [ ] Built-in presets are available and documented
- [ ] Changing project `lookAndFeelId` changes widget rendering in the designer
- [ ] Widget style overrides (`fillColor`, `textColor`, `borderColor`, `accentColor`, `borderThickness`, `cornerRadius`, `fontSize`) affect rendering
- [ ] Empty widget style overrides inherit from the project look and feel
- [ ] Save/load preserves project `lookAndFeelId` and widget style overrides
- [ ] Exported preview rendering reflects the selected look and feel and overrides
- [ ] Existing widget editing, export, and generated project build still work

## Final result summary

Completed.

- Added a model-neutral look-and-feel registry and four built-in presets.
- Added a project-level `lookAndFeelId` plus per-widget style override properties with inheritance when override values are empty.
- Updated the Property Inspector, designer rendering, serialization, and generated preview export to use the new style system foundation.
- Added `docs/look_and_feel.md` and updated related documentation for presets, overrides, inheritance, and current limitations.

Remaining TODOs:

- Manual Visual Studio verification is still needed for preset switching, widget override rendering, save/load persistence, and exported project preview appearance.
