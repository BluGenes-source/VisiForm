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
6. Add generator rendering, event signature mapping, and callback hook support.
7. Update sample data and docs.

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

## Current limitations

- no runtime plugin loading yet
- no external JSON widget-definition files yet
- registry is read-only after initialization for now
