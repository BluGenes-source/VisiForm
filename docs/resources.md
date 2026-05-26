`VisiForm` project resources let a project track external asset files and export them into generated projects under safe relative `assets/` paths.

## Resource model

Current managed resource types:

- `Image`
- `Font`
- `Icon`
- `Theme`
- `Other`

Each project resource stores:

- `id` - stable project resource id such as `image_1` or `font_1`
- `type` - resource type
- `displayName` - editor-facing label
- `sourcePath` - original source file path on disk
- `exportRelativePath` - generated-project relative asset path

Resources are stored at the project-document level in `ProjectDocument.resources`.

## Resource Manager

The editor exposes project assets from:

- `Project > Resources`

Current `Resource Manager` behavior:

- shows the current project resource list
- shows readable resource entries including display name, id, type, and source filename
- allows cycling the selected resource inside the existing modal dialog system
- shows the selected resource type, display name, source path, and export path
- shows a scaled preview for image resources when the source file can be loaded
- supports `Add Image`
- supports `Add Font`
- supports `Remove`
- add and remove actions now participate in editor undo and redo through the shared command stack

Current add flows:

- image resources use the native file dialog with `png`, `jpg`, `jpeg`, `bmp`, and `webp` filters
- font resources use the native file dialog with `ttf` and `otf` filters
- adding an image resource reports a status message such as `Added image resource: Logo (image_1)`

Current platform note:

- Windows currently has the native dialog implementation used by these flows
- non-Windows builds currently compile with safe dialog fallbacks that return no selection until native macOS or Linux dialog support is implemented

Default export folders:

- images -> `assets/images/`
- fonts -> `assets/fonts/`
- icons -> `assets/icons/`
- themes -> `assets/themes/`
- other -> `assets/resources/`

Default export filenames are sanitized and made unique when needed.

## Image widget integration

The `Image` widget now uses these properties:

- `resourceId` - managed image resource id
- `imagePath` - direct-path fallback for legacy or manual image usage
- `scaleMode` - `Stretch`, `Fit`, `Fill`, or `Center`
- `hint`

Current editor behavior:

- the property inspector shows `resourceId` as `Resource`
- the `Resource` row uses a dropdown of project image resources with readable labels such as `Logo (image_1)`
- the dropdown stores the stable resource id such as `image_1`, not the display name
- the designer draws the selected managed image resource from cached encoded bytes when available
- selected image widgets use a lightweight preview while moving or resizing to keep drag interaction responsive
- the canvas resets image tint before drawing so image widgets display at normal brightness
- missing `resourceId` references are shown as missing-resource placeholders
- direct `imagePath` fallback values are still accepted
- older `source` values are treated as a legacy fallback path
- hovering `Resource`, `Image Path`, and `Scale Mode` in the property inspector shows help text in the main status bar

`resourceId` versus `imagePath`:

- `resourceId` is the preferred managed reference to a project resource added through `Project > Resources`
- `imagePath` is a direct file-path fallback used when no managed image resource is assigned
- when both are present, the managed `resourceId` takes priority in the editor and export flow

## Export behavior

During export, `VisiForm` copies managed project resources into the generated project using each resource's `exportRelativePath`.

Current generated asset folders may include:

- `assets/images/`
- `assets/fonts/`
- `assets/icons/`
- `assets/themes/`
- `assets/resources/`

The exporter:

- creates needed asset directories
- keeps copies inside the generated project folder
- overwrites known copied asset files when exporting again
- does not require assets to be compiled by CMake
- uses `exportRelativePath` when a managed image `resourceId` is assigned
- uses direct `imagePath` only as a fallback when no managed image resource is selected
- keeps generated image placeholders safe by using the exported relative path as text or a generated comment

## Validation behavior

Current validation checks include:

- resource id must be unique
- `sourcePath` must not be empty
- `sourcePath` must exist before export
- `exportRelativePath` must not be empty
- `exportRelativePath` must stay under `assets/`
- duplicate export paths are allowed only when they map to the same source file
- `Image.resourceId` must resolve to an `Image` project resource when set
- `Image` widgets warn when both `resourceId` and `imagePath` are empty
- direct `imagePath` fallback values must exist when used without `resourceId`

Validation errors block export.
Warnings do not block export.
