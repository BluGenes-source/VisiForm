# Phase 61 resource manager and export assets plan

## Phase title
Phase 61 resource manager and export assets

## Goal
Add a `Resource Manager` system so `VisiForm` projects can track assets and export them into generated projects cleanly.

## Current state
- `VisiForm` builds and runs.
- New Project Wizard works.
- Project Settings dialog works.
- Menu bar works.
- Validation modal works.
- `ModalDialog` widget exists.
- Generated projects build and run.
- Generated widgets are interactive.
- Local `Visage` dependency support works.
- Generated project naming works.
- Look and Feel/style properties exist.
- `Image` widget exists but needs better asset handling.
- Future widgets and themes will need image/font/icon/resource support.

## Files to inspect
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetDefinition.h`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/WidgetPalette.h`
- `src/utils/FileUtils.h`
- `src/utils/FileUtils.cpp`
- `src/utils/NativeFileDialogs.h`
- `src/utils/NativeFileDialogs.cpp`
- `src/generator/CodeGenerator.h`
- `src/generator/CodeGenerator.cpp`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CMakeEmitter.cpp`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/validation/ProjectValidator.h`
- `src/validation/ProjectValidator.cpp`
- `docs/project_file_format.md`
- `docs/code_generation.md`
- `docs/widget_catalog.md`
- `docs/project_validation.md`
- `docs/resources.md`
- `docs/agent_plans/phase_61_resource_manager_and_export_assets_plan.md`

## TODO checklist
- [x] Inspect current project model, widget properties, modal UI flow, export flow, file dialog support, and validation behavior.
- [x] Define project resource types, resource ids, default export paths, and image widget property updates.
- [x] Add project-level resource storage and helper APIs in the model layer.
- [x] Save and load project resources in `.vfb.json` while keeping compatibility with older files.
- [x] Add resource-aware file and path helpers needed for safe export copying.
- [x] Add `Project > Resources` command and a `Resource Manager` dialog using the existing editor modal system.
- [x] Support adding image resources from native file dialogs with default managed export paths.
- [x] Support adding font resources from native file dialogs with default managed export paths.
- [x] Support removing resources with widget reference checks and confirmation behavior.
- [x] Update the `Image` widget definition and property editing flow for `resourceId`, fallback `imagePath`, `scaleMode`, and `hint`.
- [x] Update the designer preview for managed image resources without blocking on full image decoding.
- [x] Copy referenced resources into generated project `assets/` folders during export.
- [x] Generate safe relative managed asset paths for exported image resources.
- [x] Keep generated code safe when using managed image resources.
- [x] Validate project resources, missing files, export paths, and image widget references before export.
- [x] Add an easy `Manage Resources` entry point from project settings if feasible.
- [x] Add `docs/resources.md`.
- [x] Update `docs/project_file_format.md`.
- [x] Update `docs/code_generation.md`.
- [x] Update `docs/widget_catalog.md`.
- [x] Update `docs/project_validation.md`.
- [ ] Build the main `VisiForm` app with the `build-static-debug` workflow.
- [ ] Fix compile errors introduced by this phase.
- [ ] Write the final result summary and remaining TODOs.

## Build validation checklist
- [ ] Build only the main `VisiForm` project.
- [ ] Use the `build-static-debug` workflow.
- [ ] Confirm the build completes successfully.
- [ ] Fix compile errors introduced by this phase.
- [ ] Record the successful build result in this plan.
- [ ] Do not run `VisiForm.exe`.
- [ ] Do not launch generated apps.

Build result:

- `build-static-debug` now reaches the link stage after fixing the `src/generator/VisageCppEmitter.cpp` `widgetLabel` compile error and adding the missing `src/model/ProjectDocument.cpp` definitions for `widgetIdsReferencingResource`, `isResourceReferenced`, and `removeResourceById`.
- Current blocker: the x64 `VisiForm` target is resolving multiple Windows SDK and MSVC libraries from x86 paths in the active build environment, which causes `LNK4272` machine-type conflicts and many unresolved externals.

## Manual test checklist
- [ ] Open `Project > Resources` and confirm the `Resource Manager` dialog opens.
- [ ] Add an image resource and confirm its default export path is under `assets/images/`.
- [ ] Add a font resource and confirm its default export path is under `assets/fonts/`.
- [ ] Remove an unused resource.
- [ ] Attempt to remove a resource used by an `Image` widget and confirm the warning or confirmation behavior.
- [ ] Save and reload a project with resources and confirm resources persist.
- [ ] Select an `Image` widget and edit `resourceId`, `imagePath`, and `scaleMode`.
- [ ] Confirm the designer shows a managed image placeholder or missing-resource warning state.
- [ ] Export a project with managed image and font resources.
- [ ] Confirm exported assets are copied into generated `assets/images/` and `assets/fonts/` folders.
- [ ] Confirm generated code uses or safely documents managed image resource paths.
- [ ] Confirm validation reports missing resource files and invalid image widget references.
- [ ] Confirm export blocks or warns appropriately when referenced resource files are missing.
- [ ] Confirm existing save, load, export, validation, menu, wizard, project settings, and local `Visage` dependency flows still work.
- [ ] Confirm generated projects still build Debug and Release manually after export.

## Final result summary
- Pending.

## Remaining TODOs
- Resolve the active x64 versus x86 linker environment mismatch for the required `build-static-debug` workflow.
- Re-run `build-static-debug` and confirm the main `VisiForm` target links successfully.
- Record the final successful build result and phase summary after the environment issue is cleared.
