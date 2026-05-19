# Widget registry

`VisiForm` now uses a built-in `WidgetRegistry` to centralize widget metadata used by the editor and generator.
It also uses a built-in `LookAndFeelRegistry` for shared visual presets.

## Current purpose

The registry stores built-in definitions for:

- widget type name
- palette display name
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

To add a new widget type internally:

1. Add the `WidgetType` enum value.
2. Add widget type string conversion.
3. Add a `WidgetDefinition` to `WidgetRegistry`.
4. Add common style override properties if the widget should support look-and-feel overrides.
5. Add designer rendering in `DesignerCanvas`.
6. Add generator rendering.
7. Add generated runtime hit testing and interaction if the exported widget should be interactive.
8. Add or confirm callback signature mapping and dispatch behavior.
9. Update sample data and docs.

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

## Event signatures and suggestions

Registry event definitions currently include a `handlerSignatureKind` value.

This is used for:

- export handler signature generation
- handler conflict detection
- simple same-signature callback suggestions in the property inspector

Current signature kinds map to the generated listener API payloads:

- `void_event` -> `void handler(const WidgetEvent& event)`
- `bool_event` -> `void handler(const WidgetEvent& event, bool value)`
- `float_event` -> `void handler(const WidgetEvent& event, float value)`
- `string_event` -> `void handler(const WidgetEvent& event, const std::string& value)`

When adding a new widget event, choose the signature kind that matches the value payload the future generated emit helper should forward.
Widgets may share the same handler name when the signature kind is compatible.
If the same handler name is reused with incompatible signature kinds, export fails with a conflict error.

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
