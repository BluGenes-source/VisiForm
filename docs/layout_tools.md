# Layout tools

`VisiForm` includes multi-selection layout tools that use the primary selected widget as the alignment and sizing reference.

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
- `sizerItem.preferredWidth` and `sizerItem.preferredHeight` store designer resize requests with `-1` meaning automatic
- `sizerItem.minimumWidth` and `sizerItem.minimumHeight` can override automatic minimum size with `-1` meaning automatic
- `sizerItem.shown=false` removes a child from layout participation

Sizer-managed children keep sizer-controlled final bounds, so position, size, Dock, and Anchor do not directly control the rendered result while their direct parent is a `Sizer`. Dragging a selected sizer child's resize handles edits `sizerItem.preferredWidth` and `sizerItem.preferredHeight`, which keeps repeated grow/shrink resize gestures reversible while leaving explicit minimum-size overrides intact. Resizing a parent container reruns layout for anchored, docked, and nested sizer children, and undo/redo preserves the parent resize plus the resulting child layout.

Designer movement is constrained to the parent canvas. Dragging, nudging, or editing bounds keeps normal widgets and sizers inside their parent client area, including root-level sizers inside the form.

Dragging a widget close to a `Sizer` can snap-connect it into that sizer when the pointer is within the editor's sizer drop threshold. Spacers are the intended widgets for empty space: fixed spacers reserve a fixed amount, and stretch spacers take weighted extra space.

Geometry layout commands are disabled for direct children of a `Sizer`; sizer-owned dimensions must be changed through sizer-item properties or the existing sizer resize interaction.

## Fit to parent

The Layout menu includes `Fit Width to Parent` and `Fit Height to Parent`.

- each selected widget uses its own direct parent's child-content bounds
- width fitting changes `x` and `width` while preserving `y` and `height`
- height fitting changes `y` and `height` while preserving `x` and `width`
- form title areas, Frame and GroupBox insets, TabPage bounds, Panel bounds, and Sizer padding come from the shared model layout calculation
- compatible mixed-parent selections are processed in one undo step
- direct Sizer children, dock-managed widgets, and layout-owned TabPages are protected
- commands are disabled in Preview Mode and when the complete selection is incompatible or already fitted

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

With one selected widget, aligns its left edge to its direct parent's usable content-left edge.
With multiple selected widgets, aligns compatible siblings to the primary widget's left edge.

Current behavior:

- single selection changes only x and preserves y and size
- multi-selection requires compatible siblings with the same direct parent, and the primary widget stays unchanged
- y, width, and height stay unchanged
- root, cross-parent multi-selection, sizer-managed, dock-managed, layout-owned TabPage, and Preview Mode operations are disabled

## Align Top

With one selected widget, aligns its top edge to its direct parent's usable content-top edge.
With multiple selected widgets, aligns compatible siblings to the primary widget's top edge.

Current behavior:

- single selection changes only y and preserves x and size
- multi-selection requires compatible siblings with the same direct parent, and the primary widget stays unchanged
- x, width, and height stay unchanged

## Align Right

With one selected widget, aligns its right edge to its direct parent's usable content-right edge.
With multiple selected widgets, aligns compatible siblings to the primary widget's right edge.

Current behavior:

- single selection changes only x
- multi-selection retains the primary widget as the unchanged reference
- size and y stay unchanged

## Align Bottom

With one selected widget, aligns its bottom edge to its direct parent's usable content-bottom edge.
With multiple selected widgets, aligns compatible siblings to the primary widget's bottom edge.

Current behavior:

- single selection changes only y
- multi-selection retains the primary widget as the unchanged reference
- size and x stay unchanged

## Center Horizontally

With one selected widget, centers it horizontally in its direct parent's usable content area.
With multiple selected widgets, aligns compatible sibling centers to the primary widget horizontally.

Current behavior:

- single selection changes only x
- multi-selection retains the primary widget as the unchanged reference
- size and y stay unchanged

## Center Vertically

With one selected widget, centers it vertically in its direct parent's usable content area.
With multiple selected widgets, aligns compatible sibling centers to the primary widget vertically.

Current behavior:

- single selection changes only y
- multi-selection retains the primary widget as the unchanged reference
- size and x stay unchanged

For all single-widget alignment commands, parent content bounds include existing border, padding, caption, tab-page, and container insets.
If the widget is larger than the parent content area on the affected axis, VisiForm places it at the starting content edge without resizing it.
Successful alignment creates one undo step; an already-aligned no-op creates no undo history.

## Same Width

The Layout menu exposes:

- `Same Width: Match Primary`
- `Same Width: Match Smallest`
- `Same Width: Match Largest`

Current behavior:

- requires at least two compatible selected widgets
- calculates the reference from model-space widths
- Primary uses the current primary selected widget
- Smallest and Largest use the corresponding selected width
- raises the shared target when necessary to satisfy every selected widget's minimum width
- positions stay unchanged
- heights stay unchanged
- direct Sizer children, dock-managed widgets, and cross-parent selections are protected

## Same Height

The Layout menu exposes:

- `Same Height: Match Primary`
- `Same Height: Match Smallest`
- `Same Height: Match Largest`

Current behavior:

- requires at least two compatible selected widgets
- calculates the reference from model-space heights
- Primary uses the current primary selected widget
- Smallest and Largest use the corresponding selected height
- raises the shared target when necessary to satisfy every selected widget's minimum height
- positions stay unchanged
- widths stay unchanged
- direct Sizer children, dock-managed widgets, and cross-parent selections are protected

## Distribute Horizontally

Distributes at least three selected widgets across their current horizontal range.

Current behavior:

- requires at least three selected non-root widgets
- keeps the leftmost and rightmost widgets fixed
- creates equal gaps between widget bounds
- negative available space produces deterministic equal overlap

## Distribute Vertically

Distributes at least three selected widgets across their current vertical range.

Current behavior:

- requires at least three selected non-root widgets
- keeps the topmost and bottommost widgets fixed
- creates equal gaps between widget bounds
- negative available space produces deterministic equal overlap

## Ordering, Fit Text, and undo

- `Bring Forward` and `Send Backward` move the primary widget by one position within its direct parent's sibling order.
- Ordering commands are disabled at the frontmost or backmost boundary and preserve the complete selection.
- `Fit Text` is available only for supported text-bearing widgets whose geometry is not parent-layout managed, and uses the widget's rendered font metrics.
- Every successful layout action creates one undo step; failed and no-op actions do not mark the project dirty or add undo history.

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
