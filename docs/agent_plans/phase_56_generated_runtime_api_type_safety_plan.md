# Phase 56 - Generated runtime API type safety plan

## Phase title
Generated runtime API type safety

## Current problem
The exported generated runtime files `src/MainWindow.h` and `src/MainWindow.cpp` expose several weakly typed APIs and repeated event-dispatch helpers. Direct edits to exported files would be overwritten on the next export, so the improvements must be implemented in the VisiForm generator and supporting documentation instead.

## Files to inspect
- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `src/generator/VisageCppEmitter.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CodeGenerator.h`
- `src/generator/CodeGenerator.cpp`
- `src/generator/CMakeEmitter.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetNode.h`
- `src/model/ProjectDocument.h`
- `docs/code_generation.md`
- `docs/widget_registry.md`
- `docs/widget_catalog.md`
- `Generated/ExportedVisageProject/src/MainWindow.h` if present
- `Generated/ExportedVisageProject/src/MainWindow.cpp` if present

## TODO checklist
- [x] Create this phase plan file with required sections.
- [ ] Inspect generator, model, documentation, and exported sample files that define or describe generated runtime APIs.
- [ ] Confirm whether generated `showWindow()` overrides a virtual base method before applying `override`.
- [ ] Update the generator to emit stronger runtime types for `WidgetEvent`, `RuntimeWidgetType`, `RuntimeOrientation`, and runtime colors.
- [ ] Reduce generated `RuntimeWidget` field sprawl by grouping related runtime state where practical.
- [ ] Update generated getters to use `std::optional` for missing or unsupported widget lookups.
- [ ] Mark generated status-returning setters `[[nodiscard]]` where appropriate.
- [ ] Replace or reduce repeated per-widget event emitter generation with shared signature-based dispatch helpers.
- [ ] Preserve USER CODE regions and generated base/subclass export behavior.
- [ ] Update generation documentation and generated README guidance to reflect the new runtime API behavior.
- [ ] Validate generated sample output for `MainWindow.h` and `MainWindow.cpp` if an exported sample is present.
- [ ] Build the main `VisiForm` project with `build-static-debug`.
- [ ] Fix any compile errors introduced by this phase.
- [ ] Update this plan with build validation results, remaining TODOs, and the final result summary before finishing.

## Build validation checklist
- [ ] Build the main `VisiForm` project with `build-static-debug`.
- [ ] Confirm the main app build completed successfully.
- [ ] Confirm no generated-project executables or `VisiForm.exe` were launched.

## Manual export/build checklist
- [ ] Export a Visage project and inspect the generated `src/MainWindow.h` and `src/MainWindow.cpp` output.
- [ ] Confirm generated `WidgetEvent` uses `std::string_view` safely for synchronous dispatch.
- [ ] Confirm generated `RuntimeWidgetType` and `RuntimeOrientation` use `std::uint8_t` underlying enums.
- [ ] Confirm generated runtime colors use a named `RuntimeColor` type rather than raw `int` fields.
- [ ] Confirm generated getters return `std::optional` and setters return `[[nodiscard]] bool`.
- [ ] Confirm repeated per-widget emitters were reduced in favor of shared dispatch helpers.
- [ ] Build the exported project in Debug.
- [ ] Build the exported project in Release.
- [ ] Verify generated widgets still behave interactively when you run the exported app manually.
- [ ] Confirm USER CODE regions are preserved across re-export.

## Final result summary
Pending.

## Remaining TODOs
- Complete implementation, validation, and documentation updates for generated runtime API type-safety improvements.
- Perform manual export, generated-project build, and runtime interaction checks after the generator changes are merged.
