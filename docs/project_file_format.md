# VisiForm project file format

`VisiForm` stores editor documents as `.vfb.json` files.

## Purpose

A `.vfb.json` file captures the project document used by the editor, including the root form, widget hierarchy, widget properties, and current selection.

## Top-level schema

Current schema version: `1`

Top-level fields:

- `schemaVersion` - integer schema identifier
- `projectName` - project display name
- `executableName` - generated executable target name
- `mainFormClassName` - generated C++ class name for the main form
- `generatedBaseClassName` - generated base window class name
- `userSubclassName` - generated user subclass name
- `windowTitle` - generated runtime window title
- `lookAndFeelId` - project-level look and feel preset id
- `selectedWidgetId` - currently selected widget id
- `root` - root `WidgetNode`

The `dirty` flag is runtime-only and is not stored in the file.

## WidgetNode fields

Each widget node stores:

- `id` - unique widget id
- `name` - widget name used by the editor and generated code
- `type` - widget type string
- `bounds` - rectangle object
- `properties` - object mapping property names to simple JSON values
- `children` - array of child widget nodes

For this phase, generated event metadata is also stored in `properties` as string values.
Widget help text is also stored in `properties` using the common `hint` string key.

The project-level `lookAndFeelId` is stored at the top level and defaults to `VisiFormDark` when missing.

## Widget type strings

Supported `type` values:

- `FormWindow`
- `Frame`
- `Label`
- `Button`
- `TextBox`
- `CheckBox`
- `RadioButton`
- `Slider`
- `ScrollBar`
- `StatusBar`
- `ProgressBar`
- `ModalDialog`
- `ColorPicker`
- `Image`
- `Spacer`

## Rect format

`bounds` is stored as:

- `x` - number
- `y` - number
- `width` - positive number
- `height` - positive number

Example:

```json
{
  "x": 40,
  "y": 40,
  "width": 160,
  "height": 40
}
```

## PropertyValue JSON types

Supported property JSON value types:

- `null` -> empty property
- `boolean` -> `bool`
- integer number -> `int`
- floating-point number -> `float`
- string -> `std::string`

Objects and arrays are not currently supported as property values.

## Common style override properties

Most widgets can also store optional style override properties inside `properties`:

- `lookAndFeelId`
- `fillColor`
- `textColor`
- `borderColor`
- `accentColor`
- `borderThickness`
- `cornerRadius`
- `fontSize`

Empty style override values inherit from the project-level look and feel.

## Event property keys

Current event handler name properties:

- `onClick`
- `onToggle`
- `onSelected`
- `onChanged`
- `onTextChanged`
- `onLoad`
- `onClose`
- `onAccepted`
- `onCancelled`

These values are stored as strings for now.
Empty strings mean no handler is assigned.

Callback compatibility in the editor and generator now follows sender-aware signature groups:

- `void_event`
- `bool_event`
- `float_event`
- `string_event`

Future schema versions may move event metadata into a dedicated events object.

For compatibility, older files may omit `executableName`, `generatedBaseClassName`, `userSubclassName`, or `windowTitle`.
When missing:

- `generatedBaseClassName` remains fixed to `MainWindow`
- `userSubclassName` falls back to `AppMainWindow` or the older `mainFormClassName`
- `executableName` falls back to a sanitized `projectName`
- `windowTitle` falls back to the root form `title` property or `projectName`

## Common widget help property

The editor may store a widget help hint as:

- `hint` - string help text shown by the editor UI

The `hint` property is editor-facing help text. It is preserved by save and load, and may also appear as generated code comments during export.

Current export naming rule:

- `generatedBaseClassName` is effectively fixed to `MainWindow`
- `userSubclassName` is the editable user subclass name used by export

## RadioButton properties

Common `RadioButton` properties include:

- `text`
- `selected`
- `group`
- `hint`
- `onSelected`

Widgets in the same `group` are normalized so only one selected radio button remains true.

## ScrollBar properties

Common `ScrollBar` properties include:

- `orientation` - `Horizontal` or `Vertical`
- `min`
- `max`
- `value`
- `pageSize`
- `hint`
- `onChanged`

## StatusBar properties

Common `StatusBar` properties include:

- `fields`
- `text0`
- `text1`
- `text2`
- `fieldWidths`
- `hint`

## ProgressBar properties

Common `ProgressBar` properties include:

- `min`
- `max`
- `value`
- `showText`
- `text`
- `hint`

`text` is optional custom display text.

- if `showText` is `true` and `text` is empty, the editor and generated preview display percent text
- if `showText` is `true` and `text` is non-empty, the custom `text` value is displayed
- if `showText` is `false`, no ProgressBar text is displayed

## ModalDialog properties

Common `ModalDialog` properties include:

- `title`
- `message`
- `buttons` - comma-separated button labels such as `OK` or `OK,Cancel`
- `modal`
- `visibleAtStartup`
- `hint`
- `onAccepted`
- `onCancelled`

`ModalDialog` widgets are stored in the normal widget tree and reuse the same generic property serialization path as other widgets.
The editor preview renders them as design-time placeholders, while generated projects show them through exported runtime modal helpers.

## Validation expectations

Before export, `VisiForm` validates the current in-memory document and reports warnings or errors.

Current validation expectations include:

- `projectName` should not stay empty or on the default placeholder value
- `executableName` must not be empty and may be sanitized for generated target names
- `userSubclassName` must be a valid C++ identifier and must not be `MainWindow`
- widget `id` values must be present and unique
- duplicate widget `name` values are allowed but reported as warnings because generated lookup by name becomes ambiguous
- callback property values must be empty or valid C++ identifiers
- the same callback name must not be reused across incompatible signature groups
- invalid enum-like string values such as `ScrollBar.orientation` are reported before export

## Color format validation

Color-valued properties are validated before export.

Accepted color string formats:

- empty string
- `#RRGGBB`
- `#AARRGGBB`

Color-related keys currently validated include:

- `backgroundColor`
- `fillColor`
- `textColor`
- `borderColor`
- `accentColor`
- `panelColor`
- `controlFillColor`
- `controlTextColor`
- `controlBorderColor`
- `disabledColor`
- `ColorPicker.value`

Invalid color strings are reported as export-blocking validation errors.

## Enum value validation

Current enum-like string validation includes:

- `ScrollBar.orientation` - `Horizontal` or `Vertical`
- `dock` - empty, `None`, `Bottom`, `Top`, `Left`, `Right`, or `Fill`
- `lookAndFeelId` - empty or a known preset id

## Project file workflow note

The editor stores project documents as `.vfb.json` files and now uses `Generated/Projects` as the default project-folder base when no prior `lastProjectDirectory` setting is available.

## Example project file

```json
{
  "schemaVersion": 1,
  "projectName": "VisiFormProject",
  "executableName": "VisiFormProject",
  "mainFormClassName": "AppMainWindow",
  "generatedBaseClassName": "MainWindow",
  "userSubclassName": "AppMainWindow",
  "windowTitle": "VisiFormProject",
  "selectedWidgetId": "button_hello",
  "root": {
    "id": "form_main",
    "name": "MainWindow",
    "type": "FormWindow",
    "bounds": {
      "x": 0,
      "y": 0,
      "width": 900,
      "height": 600
    },
    "properties": {
      "title": "VisiFormProject",
      "backgroundColor": "#202026"
    },
    "children": [
      {
        "id": "button_hello",
        "name": "helloButton",
        "type": "Button",
        "bounds": {
          "x": 40,
          "y": 40,
          "width": 160,
          "height": 40
        },
        "properties": {
          "text": "Click Me"
        },
        "children": []
      }
    ]
  }
}
```

## Schema migration notes

Future schema versions should keep `schemaVersion` at the top level and migrate older documents in the serialization layer rather than pushing JSON-specific logic into the model layer.
