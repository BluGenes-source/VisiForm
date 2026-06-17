# Widget registry

`VisiForm` now uses a built-in `WidgetRegistry` to centralize widget metadata used by the editor and generator.
It also uses a built-in `LookAndFeelRegistry` for shared visual presets.

## Current purpose

The registry stores built-in definitions for:

- widget type name
- palette display name
- palette visibility
- palette order
- palette group
- default name prefix
- default hint
- default and minimum size
- editable properties
- event properties

The look-and-feel registry stores built-in visual presets for:

- panel color
- control fill color
- control text color
- control border color
- accent color
- disabled color
- border thickness
- corner radius
- font size

The registry is built-in and static for now.
It does not support runtime plugin loading yet.

## Current built-in widget flow

The `Widget Palette` now uses `WidgetRegistry` as its source of truth.
Palette-visible widgets are controlled by `WidgetDefinition::paletteVisible`, ordered by `WidgetDefinition::paletteOrder`, and grouped by `WidgetDefinition::paletteGroup`.

Current palette categories:

| Category | Widgets |
| --- | --- |
| Common | Label, Button, TextBox, CheckBox, RadioButton |
| Containers | Frame, GroupBox, Panel, TabControl |
| Layout | Spacer, Sizer |
| Forms | Slider, ScrollBar, ProgressBar, ColorPicker |
| Data | ComboBox, ListBox, TreeView, TableGrid |
| Menu/Toolbar | MenuBar, ToolBar, StatusBar |
| Additional | Image, ModalDialog |

The top palette renders categories in the table order. Widgets inside each category use `paletteOrder`.

`FormWindow` and `TabPage` remain registered but are intentionally hidden from the palette.

To add a new widget type internally:

1. Add the `WidgetType` enum value.
2. Add widget type string conversion.
3. Add a `WidgetDefinition` to `WidgetRegistry`.
4. Set `paletteVisible`, `paletteOrder`, and `paletteGroup` when the widget should appear in the palette.
5. Add common style override properties if the widget should support look-and-feel overrides.
6. Add designer rendering in `DesignerCanvas`.
7. Add generator rendering.
8. Add generated runtime hit testing and interaction if the exported widget should be interactive.
9. Add or confirm callback signature mapping and dispatch behavior.
10. Update sample data and docs.

## Style override properties

Most widgets now expose common style override properties through the registry:

- `lookAndFeelId`
- `fillColor`
- `textColor`
- `borderColor`
- `accentColor`
- `borderThickness`
- `cornerRadius`
- `fontSize`

Empty override values inherit from the project-level look and feel.

`Sizer` also stores `orientation`, legacy `padding`, side padding, and `gap` to drive its BoxSizer layout.
`Spacer` stores `spacer.kind` and `spacer.size` so it can act as either a fixed or stretch layout item inside a `Sizer`.

## Event signatures and suggestions

Registry event definitions currently include a `handlerSignatureKind` value.

This is used for:

- export handler signature generation
- handler conflict detection
- signature-compatible handler reuse in the Property Inspector `Events` tab

Current signature kinds map to the generated listener API payloads:

- `void_event` -> `void handler(const WidgetEvent& event)`
- `bool_event` -> `void handler(const WidgetEvent& event, bool value)`
- `float_event` -> `void handler(const WidgetEvent& event, float value)`
- `string_event` -> `void handler(const WidgetEvent& event, const std::string& value)`

When adding a new widget event, choose the signature kind that matches the value payload the future generated emit helper should forward.
Widgets may share the same handler name when the signature kind is compatible.
If the same handler name is reused with incompatible signature kinds, export fails with a conflict error.
The inspector's `Existing` event assignment dropdown must list only handlers with the same signature kind, while `Create` may reuse an exact compatible handler name or suffix around incompatible collisions.

## Generated interactive widget responsibilities

When a widget is meant to be interactive in exported projects, the generated runtime now expects four related pieces to stay aligned:

- drawing from generated runtime state
- hit testing
- input dispatch
- callback signature kind

Generated widgets that expose useful runtime state should now also stay aligned with the protected generated `MainWindow` helper API used by exported user subclasses.

For new interactive widget types, update the generator so the exported `MainWindow` can:

- initialize runtime widget state from the exported document
- hit test the widget in form-local coordinates
- update runtime state on mouse or keyboard input
- dispatch the correct sender-aware callback signature
- expose safe helper coverage through id-or-name lookup where practical
- request a repaint after helper-driven state changes

## Generated runtime helper API guidance

The exported `MainWindow` base now provides protected helpers such as:

- `findWidgetById(...)`
- `findWidgetByName(...)`
- `setText(...)`
- `setChecked(...)`
- `setSelected(...)`
- `setValue(...)`
- `setProgressValue(...)`
- `setStatusBarField(...)`

When extending the generator for a new widget type, decide whether the widget should participate in one or more of those helpers or whether a future dedicated helper is needed.

Recommended rules for future widget additions:

1. store mutable runtime state in the generated `RuntimeWidget` model
2. allow helper lookup by exact widget `id` first, then exact widget `name`
3. use safe defaults and `false` returns instead of exceptions for unsupported helper calls
4. avoid re-firing the same generated callback automatically when a helper is called from user callback code
5. request a generated UI repaint after successful helper-driven state changes

## Current limitations

- no runtime plugin loading yet
- no external JSON widget-definition files yet
- registry is read-only after initialization for now
- the palette currently displays text labels because the registry does not yet expose widget icon metadata
