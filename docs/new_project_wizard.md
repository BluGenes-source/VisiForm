# New Project Wizard

`VisiForm` now opens a centered modal `New Project Wizard` from `File > New`.

## Purpose

The wizard creates a clean in-memory `ProjectDocument` before the user starts placing widgets.
It also provides a matching `Project Settings` dialog for editing the same project values later.

## Wizard fields

Current wizard fields:

- `Project Name`
- `Executable Name`
- `Window Title`
- `User Subclass Name`
- `Form Width`
- `Form Height`
- `Look And Feel`
- `Template`

## Built-in templates

Current built-in templates:

- `blank` - empty form
- `basic_app` - label, button, and status bar
- `form_with_status` - label, button, progress bar, and status bar
- `control_panel` - common controls plus a status bar
- `dialog_test` - button, modal dialog, and status bar

The wizard keeps the templates intentionally simple for this phase.

## Validation rules

Before creating a project, the wizard currently checks:

- `projectName` must not be empty
- `executableName` must already be safe for CMake and the generated executable name
- `userSubclassName` must be a valid C++ identifier
- `userSubclassName` must not be `MainWindow`
- form width and height must stay within a reasonable range
- `lookAndFeelId` and `templateId` must match supported built-in choices

Invalid input keeps the wizard open and shows an in-editor status message in the modal.

## Project Settings dialog

`Project > Settings` opens a centered modal dialog for editing:

- `projectName`
- `executableName`
- `windowTitle`
- `userSubclassName`
- `lookAndFeelId`
- `localVisageSourceDirectory`
- `visageGitRepository`
- `visageGitTag`

## What is saved to `.vfb.json`

The project file stores document-level values:

- `projectName`
- `executableName`
- `userSubclassName`
- `windowTitle`
- `lookAndFeelId`
- root `FormWindow` bounds, including the default form size

## What stays in `AppSettings`

Machine-specific export dependency values stay out of `.vfb.json`:

- `localVisageSourceDirectory`
- `visageGitRepository`
- `visageGitTag`

These values are applied during export and remain specific to the current developer machine.

## Export naming relationship

The wizard and project settings dialog feed the same export behavior:

- `projectName` controls the generated CMake `project(...)` name after sanitization
- `executableName` controls the generated executable target and `.exe` name
- `userSubclassName` controls the generated user subclass filenames and class name
- `windowTitle` controls the generated runtime window caption
- `localVisageSourceDirectory` stays in `AppSettings` and controls local `Visage` source use during export
