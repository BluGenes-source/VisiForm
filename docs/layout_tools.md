# Layout tools

`VisiForm` now includes basic single-selection layout tools for the currently selected widget.

`VisiForm` also supports a basic multi-select foundation for layout operations.

## Multi-select

Current selection options:

- normal click selects one widget
- toolbar `Multi` mode lets click toggle widgets into and out of the selection
- primary selection is the last selected widget

Current notes:

- primary selection shows resize handles
- secondary selections show red outlines
- Property Inspector continues editing the primary widget only
- `Front` and `Back` still operate on the primary selection only
- `Duplicate` still duplicates the primary selection only
- group move and group resize are not implemented yet

The current input path does not use direct Visage mouse modifier helpers in this project.
The editor uses the `Multi` toolbar toggle as the reliable fallback and also checks the Windows key state for `Ctrl` and `Shift` during clicks.

## Multi-select visuals

- single selection keeps the normal blue outline and resize handles
- primary multi-selection keeps the normal blue outline and resize handles
- secondary multi-selection uses a red outline
- only the primary selection shows resize handles

## Toolbar tools

Available toolbar actions:

- `Align L`
- `Align T`
- `Same W`
- `Same H`
- `Front`
- `Back`

The current toolbar uses compact labels to fit the existing editor layout.

## Align Left

Moves the selected non-root widget to the left editor margin.

Current behavior:

- target x position is `20`
- snap-to-grid is applied when enabled
- y, width, and height stay unchanged

## Align Top

Moves the selected non-root widget to the top editor margin.

Current behavior:

- target y position is `20`
- snap-to-grid is applied when enabled
- x, width, and height stay unchanged

## Same Width

Sets the selected widget width using a simple reference strategy.

Current behavior:

- if a previous sibling exists, use that sibling width
- otherwise use root form width minus `40`
- widget-specific minimum width is enforced

## Same Height

Sets the selected widget height using a simple reference strategy.

Current behavior:

- if a previous sibling exists, use that sibling height
- otherwise use the widget default height from shared widget metrics
- widget-specific minimum height is enforced

## Z-order convention

Current child ordering rules:

- `children[0]` is backmost
- `children.back()` is frontmost
- `DesignerCanvas` draws children from first to last
- `DesignerCanvas` hit testing checks children from last to first

## Front

Moves the selected widget to the front of its parent child order.

This affects draw order because later children are drawn on top of earlier children.

## Back

Moves the selected widget to the back of its parent child order.

This also affects draw order.

## Current limitations

- single-selection only
- no distribution tools yet
- no marquee selection yet
- no full layout manager support yet
- layout actions currently use direct edits instead of dedicated undo commands

## Future work

Planned future improvements may include:

- multi-select alignment
- horizontal and vertical distribution
- guide lines and smart alignment hints
- dedicated undo commands for layout operations
