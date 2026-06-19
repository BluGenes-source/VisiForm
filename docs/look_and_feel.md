# Look and feel system

`VisiForm` now has a simple built-in look-and-feel foundation for editor previews and exported static preview drawing.

## Built-in presets

Current built-in preset ids:

- `VisiFormDark`
- `VisiFormLight`
- `ImGuiDark`
- `FlatClassic`

Each preset defines:

- application, control, recessed, and raised surfaces
- primary, secondary, and disabled text
- border, focus, accent, selected, hover, pressed, and checked colors
- highlight and shadow edges
- `borderThickness`
- `cornerRadius`
- `fontSize`
- control padding and splitter edge metrics

`LookAndFeelRegistry::resolve(...)` is the shared source for Design Mode,
Preview Mode, editor splitter styling, and generated runtime style values.
Individual rendering paths convert the portable resolved color strings into
their native color types.

## Project-level style

The root project stores a top-level `lookAndFeelId`.

Behavior:

- missing `lookAndFeelId` defaults to `VisiFormDark`
- unknown identifiers resolve safely to `VisiFormDark`
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

The designer, Preview Mode, editor splitters, and generated runtime currently use the resolved style for:

- raised and recessed control surfaces
- normal, hover, pressed, checked/selected, focused, and disabled states
- primary and disabled text
- borders, highlight edges, and shadow edges
- border thickness and corner radius

`cornerRadius` and `fontSize` are stored and resolved for designer and generated runtime rendering.
Rounded-corner drawing is implemented for boxed widgets through shared rounded-rectangle helpers.

The `.vfb.json` format still stores only the existing `lookAndFeelId` and
optional per-widget overrides. It does not copy the resolved style table into
the project file.

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
