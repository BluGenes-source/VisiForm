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

When the `Property Inspector` scrolls:

- the shared property dropdown popup closes immediately on mouse-wheel scrolling
- the shared property dropdown popup closes immediately when the inspector scrollbar thumb is captured or dragged
- the shared property dropdown popup closes immediately on scrollbar arrow clicks or track/page scrolling
- this applies to enum, resource, callback, look-and-feel, dock, anchor, and `TreeView` `Selected Node` dropdowns
- scrolling should not leave a floating dropdown behind after the property rows move

## Shortcut ownership

The main editor only evaluates global shortcuts after the property inspector's inline editors decline the key event.
That keeps text-entry, dropdown navigation, and modal dialog keys local to the active editor surface.
