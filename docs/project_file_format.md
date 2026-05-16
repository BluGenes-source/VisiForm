# VisiForm project file format

`VisiForm` stores editor documents as `.vfb.json` files.

## Purpose

A `.vfb.json` file captures the project document used by the editor, including the root form, widget hierarchy, widget properties, and current selection.

## Top-level schema

Current schema version: `1`

Top-level fields:

- `schemaVersion` - integer schema identifier
- `projectName` - project display name
- `mainFormClassName` - generated C++ class name for the main form
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

## Widget type strings

Supported `type` values:

- `FormWindow`
- `Frame`
- `Label`
- `Button`
- `TextBox`
- `CheckBox`
- `Slider`
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

## Event property keys

Current event handler name properties:

- `onClick`
- `onToggle`
- `onChanged`
- `onTextChanged`
- `onLoad`
- `onClose`

These values are stored as strings for now.
Empty strings mean no handler is assigned.

Future schema versions may move event metadata into a dedicated events object.

## Common widget help property

The editor may store a widget help hint as:

- `hint` - string help text shown by the editor UI

The `hint` property is editor-facing help text. It is preserved by save and load, and may also appear as generated code comments during export.

## Example project file

```json
{
  "schemaVersion": 1,
  "projectName": "UntitledVisiFormProject",
  "mainFormClassName": "MainWindow",
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
      "title": "MainWindow",
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
