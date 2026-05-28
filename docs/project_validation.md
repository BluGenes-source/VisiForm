# Project validation

`VisiForm` validates the current in-memory project before export and can also run the same checks on demand from the toolbar `Chk` action or from `Project > Validate / Check`.

## Validation report

Validation writes a markdown report to:

- `Generated/validation_report.md`

Report summary fields:

- Errors
- Warnings
- Info

Export behavior:

- validation errors block export
- validation warnings do not block export
- a clean validation pass keeps normal export behavior

## Project-level checks

Current project-level validation includes:

- `projectName` empty or default placeholder detection
- `projectName` sanitization warnings for generated CMake names
- `executableName` empty detection
- `executableName` sanitization warnings for generated targets and executables
- `userSubclassName` C++ identifier validation
- `userSubclassName == MainWindow` rejection
- generated source file safety checks for `userSubclassName`
- project `lookAndFeelId` preset validation
- project resource id uniqueness
- project resource source-path existence checks
- project resource export-path safety checks under `assets/`
- duplicate project resource export-path conflict checks

## Dependency-setting checks

Current `AppSettings` validation includes:

- `localVisageSourceDirectory` may be empty
- non-empty `localVisageSourceDirectory` should contain `CMakeLists.txt`
- empty `visageGitRepository` is reported as a warning
- empty `visageGitTag` is reported as a warning

## Widget checks

Each widget is validated recursively.

Current widget checks include:

- empty widget `id`
- duplicate widget `id`
- unregistered widget type
- empty widget `name`
- duplicate widget `name`
- widget-name sanitization warnings
- root `parentId` must stay empty
- non-root widgets must store a valid `parentId`
- stored parent references must match the real parent-child hierarchy
- stored parent metadata must not form cycles
- non-container widgets must not own `children`
- widgets must not be attached under a parent type that cannot contain children
- negative width or height
- zero width or height warnings
- bounds extending outside the root form
- overlap with a bottom-docked root `StatusBar` when the overlap is obvious

## Property checks

Current property validation includes:

- color format validation for `backgroundColor`, `fillColor`, `textColor`, `borderColor`, `accentColor`, `panelColor`, `controlFillColor`, `controlTextColor`, `controlBorderColor`, `disabledColor`, and `ColorPicker.value`
- `ScrollBar.orientation` must be `Horizontal` or `Vertical`
- `dock` must be empty, `None`, `Bottom`, `Top`, `Left`, `Right`, or `Fill`
- `layoutMode` must be empty, `Absolute`, `Horizontal`, `Vertical`, `Grid`, or `TabPage`
- `layoutMode` on non-container widgets is reported as a warning because it is unused
- widget `lookAndFeelId` values must be empty or match a known preset
- `fontSize` range warnings outside `8..72`
- `borderThickness` range warnings outside `1..25`
- `cornerRadius` range warnings outside `1..25`
- `Slider`, `ScrollBar`, and `ProgressBar` require `max > min`
- out-of-range `value` warnings for `Slider`, `ScrollBar`, and `ProgressBar`
- `StatusBar.fields` must remain in the supported `1..4` range
- `StatusBar` warns when it is not attached to the root form
- `StatusBar` warns when it is not bottom-docked on the root form
- `Image.scaleMode` must be `Stretch`, `Fit`, `Fill`, or `Center`
- `Image.resourceId` must resolve to a managed project resource when set
- `Image.resourceId` must resolve specifically to a project resource of type `Image`
- managed image resources referenced by `Image.resourceId` still depend on the project-level resource source-path validation that blocks export when the source file is missing
- direct `Image.imagePath` fallback values must exist when used without `resourceId`

## Callback and event checks

Current callback validation includes:

- empty callback names are allowed
- non-empty callback names must be valid C++ identifiers
- callback names may be reused only when every use shares the same `handlerSignatureKind`
- incompatible callback signature reuse is reported as an error

Supported signature groups:

- `void_event`
- `bool_event`
- `float_event`
- `string_event`

## RadioButton group checks

Current radio-group validation includes:

- warning for an empty `group`
- warning when a group contains only one `RadioButton`
- warning when a group has no selected button
- error when a group has more than one selected button

## Export-compatibility checks

Current export-compatibility validation includes:

- hierarchy checks before export so invalid parent metadata, non-container parenting, and tab-container visibility problems are caught before code generation
- warning when an `Image` widget has neither `resourceId` nor `imagePath`
- error when a managed project resource source file is missing
- error when an `Image.resourceId` points to a missing or non-image project resource
- error when a managed project resource export path escapes `assets/`
- warning that `ColorPicker` export is currently static-only and runtime interaction is not generated yet

## UI behavior

The toolbar `Chk` action:

- runs the validator
- updates the status area with a summary
- rewrites `Generated/validation_report.md`
- shows a centered editor modal summary dialog with the validation result counts
- uses a dimmed overlay behind the dialog panel
- keeps the dialog within these bounds:
  - max width `min(720, windowWidth - 120)`
  - max height `min(520, windowHeight - 120)`
  - min width `420`
  - min height `240`
- limits the inline preview to a short message list instead of expanding to full screen
- includes `Full report written to Generated/validation_report.md` when the report file is written

The export action:

- runs the same validator before showing the export result
- blocks export when errors exist
- allows export when only warnings exist
- reports warning counts after a successful export with warnings
- shows a modal error summary when validation errors block export

The modal dialog remains editor-modal for this phase:

- underlying editor UI should not receive mouse clicks while the dialog is open
- `OK` closes the dialog
- `Escape` closes the dialog
- `Enter` activates `OK`
