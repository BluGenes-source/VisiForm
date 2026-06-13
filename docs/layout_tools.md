# Layout tools

`VisiForm` now includes basic single-selection layout tools for the currently selected widget.

`VisiForm` also supports a basic multi-select foundation for layout operations.

## BoxSizer layout

`Sizer` is a nonvisual layout manager. It can arrange direct children vertically or horizontally and can contain normal widgets, nested sizers, fixed spacers, and stretch spacers.

Current BoxSizer behavior:

- `orientation` chooses the main axis: `Vertical` top-to-bottom or `Horizontal` left-to-right
- `paddingLeft`, `paddingTop`, `paddingRight`, and `paddingBottom` inset the complete child collection
- legacy uniform `padding` still loads and is migrated to side padding when side values are absent
- `gap` adds space between participating children
- direct children use `sizerItem.proportion` for weighted main-axis growth
- direct children use `sizerItem.expand` and `sizerItem.alignment` for cross-axis placement
- `sizerItem.border` and `sizerItem.borderSides` add per-child margin
- `sizerItem.minimumWidth` and `sizerItem.minimumHeight` can override automatic minimum size with `-1` meaning automatic
- `sizerItem.shown=false` removes a child from layout participation

Sizer-managed children keep their dormant absolute bounds, but position, size, Dock, and Anchor do not control their final bounds while their direct parent is a `Sizer`.

## Box select

You can drag a marquee rectangle across the designer canvas to select widgets.

Current behavior:

- marquee selection starts from empty form space, including the root form background
- widgets are selected when their bounds intersect the marquee rectangle
- root `FormWindow` is ignored by marquee selection
- marquee selection currently replaces the existing selection

## Group move

When multiple widgets are selected, dragging the primary selected widget body moves the selected non-root set together.

Current behavior:

- all selected widgets move by the same delta
- snap-to-grid is still respected when enabled
- group resize is not implemented yet

## Copy and paste

The editor now supports an internal widget clipboard.

Available actions:

- toolbar `Copy`
- toolbar `Paste`
- `Ctrl+C`
- `Ctrl+V`

Current behavior:

- copies selected non-root widgets
- pastes deep copies with new unique ids
- offsets each paste by an additional `20,20`
- selects pasted widgets and makes the last pasted widget primary
- does not use the Windows clipboard yet
- copies event handler properties as-is for now

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

Current limitation notes:

- box select currently replaces selection instead of add/toggle marquee modes
- no Windows clipboard integration yet
- `Front` and `Back` still operate on the primary selection only

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
- `Align R`
- `Align B`
- `Center H`
- `Center V`
- `Same W`
- `Same H`
- `Dist H`
- `Dist V`
- `Front`
- `Back`

The current toolbar uses compact labels to fit the existing editor layout.

Toolbar and palette entries now surface short status-bar hints on hover to make the compact labels easier to understand.

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

## Align Right

Aligns the selected widget set to a shared right edge.

Current behavior:

- single selection aligns to the root form right margin of `20`
- multi-selection aligns all selected right edges to the current maximum selected right edge
- snap-to-grid is applied when enabled

## Align Bottom

Aligns the selected widget set to a shared bottom edge.

Current behavior:

- single selection aligns to the root form bottom margin of `20`
- multi-selection aligns all selected bottom edges to the current maximum selected bottom edge
- snap-to-grid is applied when enabled

## Center Horizontally

Centers the selected widget set horizontally.

Current behavior:

- single selection centers inside the root form
- multi-selection aligns each selected widget center to the horizontal center of the selected bounding box
- snap-to-grid is applied when enabled

## Center Vertically

Centers the selected widget set vertically.

Current behavior:

- single selection centers inside the root form
- multi-selection aligns each selected widget center to the vertical center of the selected bounding box
- snap-to-grid is applied when enabled

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

## Distribute Horizontally

Distributes at least three selected widgets across their current horizontal range.

Current behavior:

- requires at least three selected non-root widgets
- keeps the leftmost and rightmost widgets fixed
- spaces middle widgets evenly by left x position

## Distribute Vertically

Distributes at least three selected widgets across their current vertical range.

Current behavior:

- requires at least three selected non-root widgets
- keeps the topmost and bottommost widgets fixed
- spaces middle widgets evenly by top y position

## Nudge

Selected non-root widgets can be nudged with the keyboard.

Current behavior:

- `Left`, `Right`, `Up`, `Down` move by `1` pixel
- `Shift` + arrow moves by the current grid size
- nudging is disabled while the property inspector is editing text

## Smart guides

The editor can show smart guides while moving a widget or a selected group.

Current behavior:

- toolbar `Gde` toggles smart guides on and off
- smart guides target root form edges, root centers, and 20-pixel root margins
- smart guides also target non-selected widget edges and centers
- single-widget move uses the widget bounds as the moving box
- group move uses the selected group bounding box as the moving box
- vertical and horizontal snapping are resolved independently
- the closest eligible target within the snap threshold is used on each axis
- guide lines are drawn on top of the form preview while dragging

### Grid snap interaction

- smart guide snapping is checked first on each axis
- if no smart guide match is found on an axis, grid snap is used on that axis when grid snapping is enabled

### Current limitations

- smart guides apply only during move and group move
- resize snapping is not implemented yet
- no guide labels or distance measurements yet

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
- no smart guides yet
- no marquee selection yet
- no full layout manager support yet
- layout actions currently use direct edits instead of dedicated undo commands
- no toolbar nudge buttons yet due toolbar space

## Future work

Planned future improvements may include:

- multi-select alignment
- horizontal and vertical distribution
- guide lines and smart alignment hints
- dedicated undo commands for layout operations
