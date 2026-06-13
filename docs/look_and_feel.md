# Look and feel system

`VisiForm` now has a simple built-in look-and-feel foundation for editor previews and exported static preview drawing.

## Built-in presets

Current built-in preset ids:

- `VisiFormDark`
- `VisiFormLight`
- `ImGuiDark`
- `FlatClassic`

Each preset defines:

- `panelColor`
- `controlFillColor`
- `controlTextColor`
- `controlBorderColor`
- `accentColor`
- `disabledColor`
- `borderThickness`
- `cornerRadius`
- `fontSize`

## Project-level style

The root project stores a top-level `lookAndFeelId`.

Behavior:

- missing `lookAndFeelId` defaults to `VisiFormDark`
- the selected project look and feel is used by widget rendering unless overridden per widget

## Per-widget style overrides

Most widgets support optional override properties:

- `lookAndFeelId`
- `fillColor`
- `textColor`
- `borderColor`
- `accentColor`
- `borderThickness`
- `cornerRadius`
- `fontSize`

Override rules:

- empty override values inherit from the project-level look and feel
- non-empty color overrides replace the preset colors for that widget
- numeric overrides replace the preset numeric values for that widget

## Current rendering behavior

The designer and generated preview currently use the resolved style where practical for:

- fill color
- text color
- border color
- accent color
- border thickness

`cornerRadius` and `fontSize` are stored and resolved for designer and generated runtime rendering.
Rounded-corner drawing is implemented for boxed widgets through shared rounded-rectangle helpers.

## Current limitations

- no theme editor UI yet
- no color picker yet
- no font picker yet
- no runtime theme switching in the generated app yet
- no external theme import/export yet
- no CSS-like selector or inheritance tree beyond project preset plus widget override

## Future direction

Planned future expansion may include:

- editable theme preset UI
- richer rounded-corner rendering
- external theme files
- runtime theme switching for generated apps
- theme-aware custom widgets
