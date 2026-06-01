# Component hierarchy

`VisiForm` now treats the form and widgets as an explicit component hierarchy instead of relying only on a mostly flat editor workflow.

## Current model

Each widget is stored as a `WidgetNode` with:

- `id`
- `name`
- `type`
- parent-relative `bounds`
- explicit `parentId`
- sibling `zOrder`
- `properties`
- nested `children`

The root `FormWindow` remains the single document root.

## Container-capable widgets

Current container-capable widget types in the model metadata:

- `FormWindow`
- `Frame`
- `GroupBox`
- `Panel`
- `TabControl`

These types can own child widgets in `children` and use `layoutMode` metadata for future layout behavior.

Current editor-active parenting workflow in this repair pass:

- selecting a `GroupBox` makes that `GroupBox` the insertion parent for newly added widgets
- selecting the root `FormWindow` returns insertion to the root form
- selecting any non-`GroupBox` widget still falls back to inserting at the root form
- drag/drop reparenting in the editor is intentionally limited to `GroupBox` and the root form for now
- box-selection marquee coordinates are stored in root-form coordinates and drawn in root canvas space
- box-selection intersection tests convert child widget bounds to absolute root-form bounds before checking the marquee rectangle
- starting box selection from empty content inside a selected or active `GroupBox` now requires additive multi-select intent and scopes selection to that `GroupBox`'s descendants
- newly created `GroupBox` widgets remain root-level in this repair pass instead of inheriting the currently selected `GroupBox` as their parent
- clicking a `GroupBox` title, border, or empty body selects the `GroupBox`, while clicking a child still selects that child first
- clicking or dragging a `GroupBox` title, border, or empty body now creates the same-press move interaction after selection instead of requiring a second click
- moving a root-level `GroupBox` updates only the `GroupBox` root coordinates; child widgets keep their parent-relative local bounds and move visually with the parent

## Hierarchy metadata

Common hierarchy-related metadata now used by the editor, persistence, validation, and export paths:

- `parentId`
- `zOrder`
- `dock`
- `anchor`
- `layoutMode`
- `tabIndex`

`TabControl` also stores:

- `tabs`
- `selectedTab`

## Persistence behavior

Project save/load now preserves hierarchy through both:

- recursive `children` arrays
- explicit `parentId` and `zOrder` metadata

Compatibility behavior:

- legacy flat projects with a top-level `widgets` array are migrated by attaching those widgets to the root form during load
- missing `children` arrays are treated as empty
- hierarchy metadata is normalized after load so runtime tree traversal stays consistent

For the current `GroupBox` workflow, child widget `bounds.x` and `bounds.y` remain relative to the owning `GroupBox`, not absolute root-form coordinates.
Selection outlines and marquee intersection tests therefore use absolute bounds computed from the parent-relative hierarchy.

## Validation behavior

Current hierarchy validation checks include:

- duplicate widget ids detected recursively
- valid `parentId` references for non-root widgets
- root form `parentId` must stay empty
- stored parent metadata must not form cycles
- non-container widgets cannot contain children
- widgets cannot be parented under non-container widgets
- `dock` values must stay in the supported set
- `layoutMode` values must stay in the supported set
- `StatusBar` docking is validated with root-bottom expectations

## Export behavior

Generated code keeps hierarchy-related metadata in the exported runtime widget model.

Current export behavior:

- runtime widgets preserve `parentId`
- `GroupBox`, `Panel`, and `TabControl` export as concrete runtime widget types
- `TabControl` preserves `tabs`, `selectedTab`, and child `tabIndex`
- generated drawing and hit testing hide children on non-selected tab pages
- exported `USER CODE` regions remain preserved during re-export

## Current limitations

- `GroupBox` is the first editor-facing container with an add/remove child workflow in the property inspector
- other container-capable types may still preserve hierarchy metadata, but this repair pass does not enable full general parenting workflows for every container type yet
- generated runtime layout is still a lightweight flat runtime list instead of a full retained-mode layout tree
- docking metadata is preserved and validated, but generated layout behavior is still intentionally minimal
- tab interaction is limited to basic selected-tab handling in generated output
