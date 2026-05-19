# Widget catalog

## Supported widgets

Current widget types:

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
- `ColorPicker`
- `Image`
- `Spacer`

## Widget registry

Built-in widgets are currently described through an internal `WidgetRegistry`.

The registry stores:

- palette display names
- default hints
- default and minimum sizes
- editable properties
- event properties
- event signature kinds used by export callback hooks and suggestions

## Look and feel foundation

The editor now supports a project-level look and feel plus optional per-widget style overrides.

Built-in presets:

- `VisiFormDark`
- `VisiFormLight`
- `ImGuiDark`
- `FlatClassic`

Project-wide behavior:

- the root `FormWindow` exposes `lookAndFeelId`
- widgets inherit colors and numeric style values from the active project preset by default

Common widget style overrides:

- `lookAndFeelId`
- `fillColor`
- `textColor`
- `borderColor`
- `accentColor`
- `borderThickness`
- `cornerRadius`
- `fontSize`

Empty override values inherit from the active project look and feel.

## Event metadata

For this phase, event handler names are stored as normal widget string properties.

Supported event properties:

- `FormWindow`
  - `onLoad`
  - `onClose`
- `Button`
  - `onClick`
- `CheckBox`
  - `onToggle`
- `RadioButton`
  - `onSelected`
- `Slider`
  - `onChanged`
- `ScrollBar`
  - `onChanged`
- `ColorPicker`
  - `onChanged`
- `TextBox`
  - `onTextChanged`

## Handler name rules

Non-empty handler names must be valid C++ identifiers:

- first character: letter or underscore
- remaining characters: letter, digit, or underscore
- spaces are not allowed
- punctuation is not allowed

Blank event properties are allowed and mean no generated handler stub is emitted.

## Common editor help property

All widgets can also use a common string property:

- `hint`

This help text is used by the editor for status-bar hints and property inspector editing.

## Callback suggestions

When editing an event property in the Property Inspector, matching existing callbacks are shown as separate clickable suggestion items.

Compatibility now groups handlers by sender-aware signature kind:

- `void_event`
- `bool_event`
- `float_event`
- `string_event`

This allows, for example, `RadioButton.onSelected` and `CheckBox.onToggle` to reuse the same `bool_event` handler names because the generated callback now includes sender metadata.

## Current limitations

- Event metadata is edited through the property inspector only
- Event fields currently show simple existing-callback suggestions instead of a full dropdown editor
- No separate visual event editor yet
- Generated runtime interaction is intentionally lightweight and not a full retained-mode widget toolkit
- Rounded-corner drawing is stored as a style property but still uses normal rectangle drawing for now
- No theme editor UI yet

## Root FormWindow project naming fields

When the root `FormWindow` is selected, the property inspector also exposes generated-project naming fields:

- `projectName`
- `executableName`
- `userSubclassName`
- `windowTitle`

These values drive generated CMake naming, executable naming, user subclass filenames, and the generated runtime title bar.

## Generated runtime behavior

Current exported generated runtime behavior by widget type:

- `Button` - clickable, fires `onClick`
- `CheckBox` - toggles and fires `onToggle`
- `RadioButton` - enforces single selection per group and fires `onSelected` for the clicked item
- `Slider` - supports mouse dragging and fires `onChanged`
- `ScrollBar` - supports arrow clicks, track paging, thumb dragging, and fires `onChanged`
- `ColorPicker` - renders a static color swatch preview and exports a string-based `onChanged` callback signature
- `TextBox` - supports click focus, basic text entry, Backspace, and fires `onTextChanged`
- `ProgressBar` - display only, but renders current runtime value text
- `StatusBar` - display only, but renders configured runtime field text

Current generated runtime limitations:

- no text selection, clipboard, or IME support in generated `TextBox` yet
- no full retained-mode widget framework yet
- no generated layout manager yet

## Default editor sizes

Current default editor widget sizes are tuned for readable text:

- `Label` - `180 x 32`
- `Button` - `180 x 46`
- `TextBox` - `220 x 36`
- `CheckBox` - `220 x 32`
- `RadioButton` - `280 x 52`
- `Slider` - `220 x 36`
- `ScrollBar` - `240 x 36`
- `ColorPicker` - `220 x 40`
- `Frame` - `260 x 160`
- `Image` - `180 x 120`
- `Spacer` - `160 x 40`

Text-capable widgets may grow wider after text or title edits to keep text readable.
Automatic growth currently expands width only and does not shrink widgets automatically.

## RadioButton notes

Current `RadioButton` properties:

- `text`
- `selected`
- `group`
- `hint`
- `onSelected`

Current behavior:

- `selected = true` clears other `RadioButton` widgets in the same `group`
- `selected = false` does not automatically select another widget

## ScrollBar notes

Current `ScrollBar` properties:

- `orientation`
- `min`
- `max`
- `value`
- `pageSize`
- `hint`
- `onChanged`

Current editor rendering uses arrow-button regions, a track, and a thumb so it looks like a scrollbar instead of a slider.

## StatusBar notes

Current `StatusBar` properties:

- `fields`
- `text0`
- `text1`
- `text2`
- `fieldWidths`
- `hint`

Current editor rendering shows a static multi-field status bar preview.

## ProgressBar notes

Current `ProgressBar` properties:

- `min`
- `max`
- `value`
- `showText`
- `text`
- `hint`

Current `ProgressBar` text behavior:

- if `showText = true` and `text` is empty, the editor shows percent text such as `25%`
- if `showText = true` and `text` is not empty, the editor shows the custom text value
- if `showText = false`, no ProgressBar text is drawn

Current editor rendering uses a simple centered text strategy with a readable single text color:

- dark text when fill percent is below 50%
- white text when fill percent is 50% or higher
