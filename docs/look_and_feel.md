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

`LookAndFeelRegistry::resolveProjectStyle(...)` resolves the selected preset plus
project overrides. `LookAndFeelRegistry::resolve(...)` then applies sparse
widget overrides. The resolution order is:

`built-in or custom preset -> project overrides -> widget overrides -> final widget style`

These are the shared sources for Design Mode, Preview Mode, editor splitter
styling, and generated runtime style values.
Individual rendering paths convert the portable resolved color strings into
their native color types.

## Custom presets

The same registry also exposes reusable custom presets loaded from one
user-owned library:

- Windows: `%APPDATA%/VisiForm/look_and_feel_presets.json`
- macOS: `~/Library/Application Support/VisiForm/look_and_feel_presets.json`
- Linux: `$XDG_CONFIG_HOME/VisiForm/look_and_feel_presets.json`, falling back
  to `~/.config/VisiForm/look_and_feel_presets.json`
- Last-resort environment fallback: the system temporary directory under
  `VisiForm/look_and_feel_presets.json`

The library uses JSON format version `1`. Each custom entry stores a stable
opaque identifier, display name, optional source preset identifier, and a
complete normalized style. It is separate from `.vfb.json`; projects continue
to store only the stable selection identifier and sparse project overrides.

Library writes use a temporary file followed by replacement. A missing library
is treated as empty, and malformed individual entries are skipped while valid
entries remain available.

The Look and Feel editor provides Save New, Duplicate, Rename, Delete, Import,
and Export operations. Built-ins are read-only but can be duplicated or
exported. Custom presets can be renamed without changing their stable
identifier. Portable files use `.vflnf.json`; importing generates a new custom
identifier and resolves name collisions predictably.

## Project-level style

The root project stores:

- top-level `lookAndFeelId`
- optional sparse `lookAndFeelOverrides`

Behavior:

- missing `lookAndFeelId` defaults to `VisiFormDark`
- unknown identifiers resolve safely to `VisiFormDark`
- unavailable custom identifiers remain stored in the project while rendering
  falls back to `VisiFormDark` and the editor reports a concise warning
- absent or invalid override values inherit safely from the selected preset
- changing the base preset preserves explicit project overrides
- the selected project look and feel is used by widget rendering unless overridden per widget

The Project menu's `Edit Look and Feel...` command edits a focused set of shared
colors and metrics. Apply and OK commit one undoable project change when values
actually differ. Cancel discards only unapplied temporary edits, while Reset to
Preset clears the temporary overrides and requires Apply or OK to commit.

The dialog's live sample uses the temporary resolved style without modifying the
project model. Color fields use the existing native color picker, and numeric
fields clamp to the conservative ranges recorded in the Phase 109 plan.

## Per-widget Appearance overrides

Supported widgets own one sparse `WidgetLookAndFeelOverrides` value. The
Property Inspector exposes it through an `Appearance` section for a supported
single selection.

Supported color overrides:

- control surface
- text
- border
- accent
- focus outline
- highlight edge
- shadow edge

Supported metric overrides:

- border thickness
- corner radius
- control padding

Supported widget types:

- Button, Text Box, Check Box, Radio Button
- Combo Box, List Box, Slider, Scroll Bar
- Progress Bar, Color Picker
- Frame, Group Box, Panel, Tab Control

An absent value inherits the resolved project value. Editing a field creates an
explicit override. `Reset Property` removes one optional value, and `Reset All
Overrides` clears only the selected widget's Appearance values. Both operations
are no-op safe and use the existing undo/redo command path.

Appearance editing is disabled for multi-selection. A small compatibility map
hides properties that do not render meaningfully for a particular supported
widget.

Legacy `lookAndFeelId`, `fillColor`, `textColor`, `borderColor`, `accentColor`,
`borderThickness`, and `cornerRadius` widget properties remain readable for old
projects, but new inspector edits use the dedicated sparse Appearance object.

## Current rendering behavior

The designer, Preview Mode, editor splitters, and generated runtime currently use the resolved style for:

- raised and recessed control surfaces
- normal, hover, pressed, checked/selected, focused, and disabled states
- primary and disabled text
- borders, highlight edges, and shadow edges
- border thickness and corner radius

`cornerRadius` and `fontSize` are stored and resolved for designer and generated runtime rendering.
Rounded-corner drawing is implemented for boxed widgets through shared rounded-rectangle helpers.

The `.vfb.json` format stores `lookAndFeelId`, optional sparse
`lookAndFeelOverrides`, and optional sparse per-widget `appearanceOverrides`.
Empty project and widget override objects are omitted, and resolved style tables
are never copied into the project file. Generated output receives the fully
resolved style and does not depend on the developer's local custom-preset
library.

## Current limitations

- no font picker yet
- no runtime theme switching in the generated app yet
- no CSS-like selector or inheritance tree beyond project preset plus widget override
- no state-specific widget overrides, custom fonts, gradients, images, or animations
- no mixed-value or bulk Appearance editing
- no online marketplace, cloud synchronization, thumbnails, tags, or preset
  inheritance chains

## Future direction

Planned future expansion may include:

- richer project-level style editing
- richer rounded-corner rendering
- runtime theme switching for generated apps
- theme-aware custom widgets
