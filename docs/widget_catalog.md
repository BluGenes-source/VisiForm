# Widget catalog

## Supported widgets

Current widget types:

- `FormWindow`
- `Frame`
- `Label`
- `Button`
- `TextBox`
- `CheckBox`
- `Slider`
- `Image`
- `Spacer`

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
- `Slider`
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

## Current limitations

- Event metadata is edited through the property inspector only
- No separate visual event editor yet
- Generated projects currently emit handler stubs and TODO comments
- Full generated interactive event dispatch is not implemented yet
