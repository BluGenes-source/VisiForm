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

## Current limitations

- Event metadata is edited through the property inspector only
- Event fields currently show simple existing-callback suggestions instead of a full dropdown editor
- No separate visual event editor yet
- Generated projects currently emit handler stubs and TODO comments
- Full generated interactive event dispatch is not implemented yet

## Default editor sizes

Current default editor widget sizes are tuned for readable text:

- `Label` - `180 x 32`
- `Button` - `180 x 46`
- `TextBox` - `220 x 36`
- `CheckBox` - `220 x 32`
- `RadioButton` - `280 x 52`
- `Slider` - `220 x 36`
- `ScrollBar` - `240 x 36`
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
