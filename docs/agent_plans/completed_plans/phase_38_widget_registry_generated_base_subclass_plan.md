# Phase 38 widget registry and generated base/subclass plan

## Goal

Add a built-in widget registry to centralize widget metadata and defaults, add `RadioButton` and `ScrollBar`, and refactor export to generate a base class plus user subclass structure with preserved user code.

## Current widget architecture

- Widget type handling is currently spread across enum conversion, palette rows, default widget creation, property inspector switch logic, designer rendering, widget metrics, and generator mappings.
- Save/load uses `WidgetNode` with generic property serialization.
- Export currently emits a single generated `MainWindow` pair with preserved user-code regions.

## Files to inspect

- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/PropertyValue.h`
- `src/ui/WidgetPalette.h`
- `src/ui/WidgetPalette.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/WidgetMetrics.h`
- `src/ui/WidgetMetrics.cpp`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/generator/CodeGenerator.h`
- `src/generator/CodeGenerator.cpp`
- `src/generator/VisageCppEmitter.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CMakeEmitter.cpp`
- `templates/cpp/`
- `docs/widget_catalog.md`
- `docs/project_file_format.md`
- `docs/code_generation.md`
- `docs/agent_plans/phase_38_widget_registry_generated_base_subclass_plan.md`

## Step-by-step TODO list

- [x] Create persistent phase TODO file
- [x] Inspect widget and export paths
- [x] Add widget definition types
- [x] Add built-in widget registry
- [x] Add `RadioButton` and `ScrollBar` enum and type-string support
- [x] Use registry defaults for widget creation where practical
- [x] Use registry definitions for palette display where practical
- [x] Use registry definitions for property inspector rows where practical
- [x] Add designer rendering for `RadioButton`
- [x] Add designer rendering for `ScrollBar`
- [x] Add project class naming fields for generated base and user subclass
- [x] Update export to emit generated base and user subclass files
- [x] Preserve user code in generated user subclass file
- [x] Add export support for `RadioButton` and `ScrollBar`
- [x] Update sample project data
- [x] Update widget registry and export docs
- [x] Run `build-static-debug`
- [x] Record final result summary

## Current progress notes

- Phase TODO file created before code changes.
- Added `WidgetDefinition` and `WidgetRegistry` as a built-in registry foundation in the model layer.
- Added `RadioButton` and `ScrollBar` across enum conversion, palette population, property inspector rows, designer rendering, sample data, and export drawing.
- Export now emits a generated base class plus user subclass structure and preserves user code in the subclass `.cpp` file.
- Root form selection now exposes `generatedBaseClassName` and `userSubclassName` in the property inspector.

## Build validation checklist

- [x] Build with `build-static-debug`
- [x] Fix compile errors if any appear
- [x] Do not run `VisiForm.exe`
- [x] Do not launch generated app

## Manual test checklist

- [ ] Add `RadioButton` from the palette and verify it renders and edits correctly
- [ ] Add `ScrollBar` from the palette and verify it renders and edits correctly
- [ ] Save and reload projects containing `RadioButton` and `ScrollBar`
- [ ] Verify root form class name fields are editable and validated
- [ ] Export a project and confirm generated base and user subclass files are created
- [ ] Re-export and confirm user code is preserved in the user subclass file
- [ ] Re-check selection, layout tools, box select, group move, copy/paste, smart guides, and save/load/export

## Final result summary

Completed.

- Added a built-in `WidgetRegistry` with centralized widget metadata for built-in widgets.
- Added `RadioButton` and `ScrollBar` widget support throughout the editor and export paths.
- Switched palette population, default widget creation, widget metrics, and most property inspector rows to registry-driven metadata.
- Added generated base and user subclass export output with user-code preservation in the user subclass implementation.
- Added root-form class naming fields for the generated base class and user subclass.
- Added documentation for the widget registry, new widget types, project format fields, and export structure.
- Verified the main VisiForm project builds successfully with `build-static-debug`.
