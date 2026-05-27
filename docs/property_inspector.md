# Property inspector

The `Property Inspector` shows editable properties for the current selection.

## Inline editing behavior

`MainWindow` uses `TextEditControl` for inline text-style property edits and `DropdownControl` for choice properties.

While inline text editing is active:

- printable characters edit the property value
- `Backspace` edits the property value
- `Delete` edits the property value
- `Left` and `Right` move the cursor
- `Home` and `End` move the cursor when supported
- `Enter` commits the edit
- `Escape` cancels the edit

This prevents global editor commands from stealing text-entry keys.
For example, pressing `Delete` while editing text updates the field instead of deleting the selected widget.

## Dropdown behavior

While a property dropdown is open:

- `Up` and `Down` move the selection
- `Enter` commits the selected item
- `Escape` closes the dropdown
- global shortcuts do not fire

## Shortcut ownership

The main editor only evaluates global shortcuts after the property inspector's inline editors decline the key event.
That keeps text-entry, dropdown navigation, and modal dialog keys local to the active editor surface.
