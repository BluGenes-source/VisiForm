# VisiForm project file format

`VisiForm` stores editor documents as `.vfb.json` files.

## Purpose

A `.vfb.json` file captures the project document used by the editor, including the root form, widget hierarchy, hierarchy metadata, widget properties, and current selection.

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
- `resources` - array of managed project resources
- `root` - root `WidgetNode`

Compatibility note:

- older flat documents may also include a top-level `widgets` array
- when present, those legacy widgets are attached to the root form during load

The `dirty` flag is runtime-only and is not stored in the file.

## Project settings persistence

The new modal `New Project Wizard` and `Project Settings` dialog edit two kinds of values:

- project document values stored in `.vfb.json`
- machine-specific export values stored in `AppSettings`

Stored in `.vfb.json`:

- `projectName`
- `executableName`
- `mainFormClassName`
- `generatedBaseClassName`
- `userSubclassName`
- `windowTitle`
- `lookAndFeelId`
- `resources`
- root form `bounds.width` and `bounds.height` for the default form size

Stored in `AppSettings`, not in `.vfb.json`:

- `localVisageSourceDirectory`
- `visageGitRepository`
- `visageGitTag`

## WidgetNode fields

Each widget node stores:

- `id` - unique widget id
- `name` - widget name used by the editor and generated code
- `type` - widget type string
- `bounds` - rectangle object
- `parentId` - stored parent widget id, empty only for the root form
- `zOrder` - stored sibling order index inside the parent
- `properties` - object mapping property names to simple JSON values
- `children` - array of child widget nodes

For this phase, generated event metadata is also stored in `properties` as string values.
Widget help text is also stored in `properties` using the common `hint` string key.

The project-level `lookAndFeelId` is stored at the top level and defaults to `VisiFormDark` when missing.

## Widget type strings

Supported `type` values:

- `FormWindow`
- `Frame`
- `GroupBox`
- `Panel`
- `TabControl`
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

## Project resource format

Managed project resources are stored in a top-level `resources` array.

Each resource entry stores:

- `id`
- `type`
- `displayName`
- `sourcePath`
- `exportRelativePath`

Example:

```json
"resources": [
  {
    "id": "image_1",
    "type": "Image",
    "displayName": "Logo",
    "sourcePath": "J:/Assets/logo.png",
    "exportRelativePath": "assets/images/logo.png"
  }
]
```

Compatibility rules:

- older `.vfb.json` files may omit `resources`
- missing `resources` defaults to an empty list
- managed resource export paths must stay under `assets/`

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

The wizard-created form size is preserved through the root `FormWindow` bounds that are already serialized in `root.bounds`.

## Common widget help property

The editor may store a widget help hint as:

- `hint` - string help text shown by the editor UI

The `hint` property is editor-facing help text. It is preserved by save and load, and may also appear as generated code comments during export.

## Hierarchy and container metadata

Common hierarchy-related widget properties now include:

- `dock` - `None`, `Top`, `Bottom`, `Left`, `Right`, or `Fill`
- `anchor` - stored anchor metadata for future resize behavior
- `layoutMode` - `Absolute`, `Horizontal`, `Vertical`, `Grid`, or `TabPage`
- `tabIndex` - tab page index used for widgets inside a `TabControl`

Container-specific notes:

- `FormWindow`, `Frame`, `GroupBox`, `Panel`, and `TabControl` can contain `children`
- `TabControl` also stores `tabs` and `selectedTab` in `properties`
- hierarchy is persisted through both the nested `children` arrays and the explicit `parentId` / `zOrder` metadata

Current export naming rule:

- `generatedBaseClassName` is effectively fixed to `MainWindow`
- `userSubclassName` is the editable user subclass name used by export

## Image widget properties

Current `Image` widget properties include:

- `resourceId`
- `imagePath`
- `scaleMode`
- `hint`

Legacy compatibility:

- older projects may still contain `source`
- the editor treats `source` as a fallback image path when `imagePath` is empty

Managed image-reference behavior:

- `resourceId` stores the stable managed project resource id such as `image_1`
- editor dropdowns may show readable labels such as `Logo (image_1)`, but the saved value remains the resource id
- `imagePath` remains a direct-path fallback for legacy or unmanaged image usage

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

## Button properties

Common `Button` properties include:

- `text`
- `normalText`
- `pressedText`
- `toggleMode`
- `checked`
- `normalFillColor`
- `pressedFillColor`
- `hint`
- `onClick`
- `onRelease`
- `onDoubleClick`

`text` remains the basic button label. `normalText` and `normalFillColor` optionally override the exported default state, while `pressedText` and `pressedFillColor` optionally override the pressed or toggle-on state.

`toggleMode` keeps the button in a checked state after activation. When `toggleMode` is enabled, `checked` stores the initial exported toggle state and is also used by the editor preview.

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
- `normalFillColor`
- `pressedFillColor`
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
