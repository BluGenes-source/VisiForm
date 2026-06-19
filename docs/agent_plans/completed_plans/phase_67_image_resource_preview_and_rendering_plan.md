# Phase 67 - Image resource preview and rendering plan

## Phase title

Implement managed image preview and rendering support for the editor and generated projects where the local `Visage` image API supports it.

## Current state

- `VisiForm` is version `1.0.0`.
- `Resource Manager` can add image resources.
- `Image` widgets can reference managed image resources by `resourceId`.
- The `Image` widget resource dropdown works.
- Property hints already work.
- Managed image resources are copied to exported `assets/images/`.
- `Image` widgets currently render placeholder text rather than decoded image previews.
- `Resource Manager` currently reads raw image bytes only to report a placeholder preview state.
- Generated projects currently keep `Image` widgets safe by emitting placeholder text/comments instead of runtime image drawing.

## Goal

Use real local `Visage` image and drawing APIs where they exist to:

- add an editor-side image loading/cache layer in the `ui/` layer
- display scaled image previews in `Resource Manager`
- render `Image` widget contents on `DesignerCanvas`
- support `Stretch`, `Fit`, `Fill`, and `Center` scale modes with safe fallback behavior
- preserve save/load/export/validation behavior
- update generated output only when the local `Visage` runtime API can support the generated code safely

## Files to inspect

- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectDocument.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetDefinition.h`
- `src/utils/FileUtils.h`
- `src/utils/FileUtils.cpp`
- `src/validation/ProjectValidator.h`
- `src/validation/ProjectValidator.cpp`
- `src/generator/VisageCppEmitter.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CodeGenerator.cpp`
- `src/generator/CMakeEmitter.cpp`
- `src/utils/NativeFileDialogs.cpp`
- `docs/resources.md`
- `docs/widget_catalog.md`
- `docs/code_generation.md`
- `docs/project_validation.md`
- `docs/agent_plans/phase_67_image_resource_preview_and_rendering_plan.md`
- local `Visage` graphics sources under `out/build/x64-debug/_deps/visage-src/visage_graphics/`

## Image API diagnosis notes

### Confirmed local `Visage` image support

- `Visage` has a real image type: `visage::Image` in `visage_graphics/image.h`.
- `DesignerCanvas` and the rest of the editor already draw in the same `visage::Canvas` context that supports image drawing.
- `visage::Canvas` exposes `image(...)` overloads in `visage_graphics/canvas.h`.
- The relevant overload for editor/runtime work is `canvas.image(const unsigned char* image_data, int image_size, x, y, width, height)`.
- `visage::Canvas` also supports `canvas.image(const visage::Image& image, x, y)` and `canvas.image(const EmbeddedFile&, x, y, width, height)`.

### How images are loaded

- The local `Visage` graphics layer does not expose a direct high-level `loadImageFromFile(...)` helper in the inspected headers.
- Internally, `visage_graphics/image.cpp` decodes image bytes with `bimg::imageParse(...)`.
- `ImageAtlas::addImage(...)` infers width and height from encoded bytes when `visage::Image.width == 0`.
- This means editor/runtime integration should read files into memory first, then pass encoded bytes to `Visage` drawing APIs.

### How images are drawn

- `canvas.image(...)` adds an `ImageWrapper` shape that uses the same canvas/render pipeline as the existing editor drawing code.
- `ImageWrapper` stores the decoded image in an `ImageAtlas`, so the draw path is already integrated with the renderer used by `DesignerCanvas`.
- Current visible API supports scaled drawing into a destination rectangle.
- The inspected public API does not show a crop-source-rectangle overload, so `Fill` and clipped `Center` may need a safe fit-style fallback if clipping is not practical in the existing draw path.

### Formats and support evidence

- The current Windows image resource picker explicitly allows `png`, `jpg`, `jpeg`, `bmp`, and `webp`.
- The local `Visage` decode path goes through `bimg::imageParse(...)`, but the inspected repository code does not expose a project-local explicit supported-format list from `Visage` itself.
- For this phase, the safe documented baseline is the repository-supported editor extensions already exposed by the file dialog: `.png`, `.jpg`, `.jpeg`, `.bmp`, and `.webp`.
- `webp` should remain conditional in validation/documentation if runtime support cannot be confirmed safely from the local decode path.

### Loading model and runtime suitability

- Image decode appears synchronous from the inspected API usage because it happens during `ImageAtlas::addImage(...)` / `updateImage(...)` from already available bytes.
- No renderer-specific file dialog or asynchronous device-context callback is required to prepare encoded bytes.
- The same `visage::Canvas` image API is available to generated projects through `#include <visage/graphics.h>`.
- Generated projects can use the same byte-backed draw approach if the generator emits safe helper code to read files and preserve fallbacks.

### Current repository gaps confirmed before implementation

- `MainWindow::refreshResourceManagerPreview()` currently only reads raw file bytes into `previewImageBytes` and sets text status.
- `DesignerCanvas` currently renders `Image` widgets as placeholders using `imageWidgetDisplayText(...)`.
- `VisageCppEmitter` currently emits placeholder comments/text for generated `Image` widgets instead of runtime image drawing.
- `ProjectValidator` currently validates `Image.scaleMode`, `resourceId`, and fallback `imagePath`, but does not yet validate supported image resource extensions.

## TODO checklist

- [x] Create the new phase plan before changing code.
- [x] Inspect the requested editor, model, validation, generator, documentation, and local `Visage` image files.
- [x] Confirm whether local `Visage` has usable image loading and drawing APIs.
- [ ] Add a UI-layer `ImageResourceCache` abstraction for encoded image bytes, metadata, and load status.
- [ ] Integrate cache lifetime and invalidation into the editor flow without leaking `Visage` types into model, serialization, validation, or generator public model types.
- [ ] Add scaled image preview rendering to `Resource Manager`.
- [ ] Add real `Image` widget rendering to `DesignerCanvas` with safe placeholder states.
- [ ] Implement or reuse scale-mode rectangle calculations for `Stretch`, `Fit`, `Fill`, and `Center`.
- [ ] Preserve selection, dragging, resizing, and existing editor interactions after image drawing changes.
- [ ] Verify `Image` widget property labels and `scaleMode` dropdown behavior remain correct and refresh immediately.
- [ ] Update generated image widget output to use safe runtime image rendering when supported, or keep safe placeholders when unsupported.
- [ ] Update generated README text for exported image assets and runtime image behavior.
- [ ] Extend validation for image resource file existence, supported extensions, resource typing, and scale mode values.
- [ ] Update `docs/resources.md`.
- [ ] Update `docs/widget_catalog.md`.
- [ ] Update `docs/code_generation.md`.
- [ ] Update `docs/project_validation.md`.
- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Fix any compile errors introduced by this phase.
- [ ] Confirm the main `VisiForm` app built successfully.
- [ ] Confirm `VisiForm.exe` was not run.
- [ ] Confirm no generated apps were launched.
- [ ] Update this phase plan with the final result summary and remaining TODOs.

## Build validation checklist

- [ ] Configure using the existing main static debug preset flow.
- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Confirm the main `VisiForm` target built successfully.
- [ ] Confirm no new compile errors remain from this phase.
- [ ] Confirm `VisiForm.exe` was not run.
- [ ] Confirm no generated apps were launched.

## Manual test checklist

- [ ] Open `Project > Resources`, select an image resource, and verify the preview panel shows a scaled image or a clear failure placeholder.
- [ ] Verify the `Resource Manager` still shows resource id, name, type, source path, and export path details.
- [ ] Add or edit an `Image` widget with a valid `resourceId` and verify the designer renders the actual image when supported.
- [ ] Add or edit an `Image` widget with only `imagePath` set and verify the fallback path renders when valid.
- [ ] Verify missing `resourceId` shows `Missing image resource`.
- [ ] Verify missing source files show `Missing image file`.
- [ ] Verify decode failures show `Image load failed`.
- [ ] Change `Scale Mode` through `Stretch`, `Fit`, `Fill`, and `Center` and verify the designer updates immediately.
- [ ] Save and reload a project containing image resources and `Image` widgets and verify `resourceId` and `scaleMode` persist.
- [ ] Export a project with image resources and verify files still copy to `assets/images/`.
- [ ] Build generated Debug and Release output manually and verify generated image behavior matches the documented runtime status.
- [ ] Verify no `USER CODE` preservation markers were broken by export updates.

## Final result summary

- In progress.

## Remaining TODOs

- Complete the image cache, editor preview, designer rendering, generator, validation, and documentation work.
- Build `VisiForm` with `build-static-debug` after the implementation changes.
- Update this phase plan with validation results and the final summary before finishing.
