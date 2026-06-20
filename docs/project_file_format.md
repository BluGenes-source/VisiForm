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
- optional `appearanceOverrides` - sparse normal and runtime-state Appearance values
- `children` - array of child widget nodes

Current `GroupBox` hierarchy storage notes:

- a widget moved into a `GroupBox` is stored inside that `GroupBox` node's `children` array
- the child widget also stores `parentId` equal to the owning `GroupBox` id
- child `bounds.x` and `bounds.y` remain local to the `GroupBox`
- removing the child back to the root form converts those bounds back to root-form coordinates before save

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
- `TabPage`
- `Label`
- `Button`
- `TextBox`
- `ComboBox`
- `ListBox`
- `TreeView`
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

## ComboBox and ListBox item storage

`ComboBox` and `ListBox` use these widget properties:

- `items` - saved as a JSON string array in `.vfb.json`, then normalized back to newline-delimited editor text in memory
- `selectedIndex` - stored as an integer beside `items`

Normalization rules:

- blank item lines are ignored
- leading and trailing whitespace is trimmed from each item string
- if the final item list is empty, `selectedIndex` becomes `-1`
- if the final item list is non-empty, `selectedIndex` is clamped into the valid `0..count-1` range

## MenuBar and ToolBar item-action storage

`MenuBar` and `ToolBar` use these widget properties:

- `items` - saved as a JSON string array in `.vfb.json`, then normalized back to newline-delimited editor text in memory
- `itemActions` - saved as a JSON string array in `.vfb.json`, then normalized back to newline-delimited editor text in memory
- `selectedMenuIndex` - stored as the current `MenuBar` selection
- `selectedToolIndex` - stored as the current `ToolBar` selection

Normalization and compatibility rules:

- blank item lines are ignored
- leading and trailing whitespace is trimmed from each item label and each action name
- `itemActions` are aligned to the normalized `items` count by index
- missing `itemActions` entries become empty callback names
- extra stored `itemActions` entries are ignored safely during normalization and validation warns about the mismatch
- if the final item list is empty, the selected index becomes `-1`
- if the final item list is non-empty, the selected index is clamped into the valid `0..count-1` range
- older projects that only contain `items` remain loadable because `itemActions` defaults to an empty aligned list
- the editor presents `MenuBar` and `ToolBar` rows as paired `Label` and `Callback / Action` fields, but the persisted storage format remains the same aligned `items` and `itemActions` arrays

## TreeView node storage

`TreeView` uses these widget properties:

- `nodes` - stored as newline-delimited indented text in `properties`
- `selectedNodePath` - stored as a string path such as `Root/Child 1`
- `expandedNodePaths` - stored as a comma-separated string of expanded node paths
- `showRoot` - stored as a boolean
- `showLines` - stored as a boolean

Current editor and normalization rules:

- the editor uses a visual node editor, but `Apply` still writes the existing indented `nodes` text format
- two leading spaces represent one tree depth level in stored `nodes`
- blank lines are ignored during normalization
- leading and trailing whitespace around each stored node title is trimmed
- invalid indentation jumps are normalized safely during load and property normalization
- `selectedNodePath` is clamped to an existing node path or cleared when no valid node remains
- `expandedNodePaths` are filtered so only currently expandable node paths remain stored

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

## Per-widget Appearance overrides

Supported widgets may include an optional sparse `appearanceOverrides` object
beside `properties`:

```json
"appearanceOverrides": {
  "controlSurfaceColor": "#445566",
  "textColor": "#F0E0D0",
  "borderColor": "#778899",
  "accentColor": "#2D7FF9",
  "focusOutlineColor": "#80AFFF",
  "highlightEdgeColor": "#DDE7F4",
  "shadowEdgeColor": "#111820",
  "borderThickness": 2.0,
  "cornerRadius": 8.0,
  "controlPadding": 10.0
}
```

Only explicit values are written. Missing fields inherit from the resolved
project Look and Feel, and an empty object is omitted. Old projects without
`appearanceOverrides` load with no widget overrides.

Legacy style values inside `properties` remain readable for compatibility.
New Phase 111 Appearance edits use the dedicated sparse object.

State-capable widgets may also store a sparse `states` object inside
`appearanceOverrides`:

```json
"appearanceOverrides": {
  "controlSurfaceColor": "#2B313D",
  "states": {
    "hover": {
      "controlSurfaceColor": "#354052",
      "borderColor": "#7AA7E8"
    },
    "focused": {
      "focusOutlineColor": "#2D7FF9"
    }
  }
}
```

Supported state identifiers are `hover`, `pressed`, `focused`,
`checkedOrSelected`, and `disabled`. Normal values remain at the top level.
Each state stores only explicit control surface, text, border, accent, focus
outline, highlight edge, or shadow edge colors. Empty states are omitted,
unknown state identifiers are ignored for forward compatibility, and Preview
Mode interaction state is never serialized.

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

- `FormWindow`, `Frame`, `GroupBox`, `Panel`, `Sizer`, and `TabControl` can contain `children`
- `Sizer` stores `orientation`, side padding (`paddingLeft`, `paddingTop`, `paddingRight`, `paddingBottom`), legacy `padding`, and `gap` in `properties`; it is optional and existing projects do not require one
- direct children of a `Sizer` may store `sizerItem.proportion`, `sizerItem.expand`, `sizerItem.alignment`, `sizerItem.border`, `sizerItem.borderSides`, `sizerItem.preferredWidth`, `sizerItem.preferredHeight`, `sizerItem.minimumWidth`, `sizerItem.minimumHeight`, and `sizerItem.shown`
- `Spacer` widgets may store `spacer.kind` (`Fixed` or `Stretch`) and `spacer.size`
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
