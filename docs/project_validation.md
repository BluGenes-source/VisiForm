# Project validation

`VisiForm` validates the current in-memory project before export and can also run the same checks on demand from the toolbar `Chk` action.

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
- negative width or height
- zero width or height warnings
- bounds extending outside the root form
- overlap with a bottom-docked root `StatusBar` when the overlap is obvious

## Property checks

Current property validation includes:

- color format validation for `backgroundColor`, `fillColor`, `textColor`, `borderColor`, `accentColor`, `panelColor`, `controlFillColor`, `controlTextColor`, `controlBorderColor`, `disabledColor`, and `ColorPicker.value`
- `ScrollBar.orientation` must be `Horizontal` or `Vertical`
- `dock` must be empty, `None`, `Bottom`, `Top`, `Left`, `Right`, or `Fill`
- widget `lookAndFeelId` values must be empty or match a known preset
- `fontSize` range warnings outside `8..72`
- `borderThickness` range warnings outside `0..20`
- `cornerRadius` range warnings outside `0..50`
- `Slider`, `ScrollBar`, and `ProgressBar` require `max > min`
- out-of-range `value` warnings for `Slider`, `ScrollBar`, and `ProgressBar`
- `StatusBar.fields` must remain in the supported `1..4` range

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

- warning when an `Image` widget has an empty `source` path
- warning that `ColorPicker` export is currently static-only and runtime interaction is not generated yet

## UI behavior

The toolbar `Chk` action:

- runs the validator
- updates the status area with a summary
- rewrites `Generated/validation_report.md`
- shows a modal summary dialog with the validation result counts

The export action:

- runs the same validator before showing the export result
- blocks export when errors exist
- allows export when only warnings exist
- reports warning counts after a successful export with warnings
- shows a modal error summary when validation errors block export
