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

Current container-capable widget types:

- `FormWindow`
- `Frame`
- `GroupBox`
- `Panel`
- `TabControl`

These types can own child widgets in `children` and use `layoutMode` metadata for future layout behavior.

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

- generated runtime layout is still a lightweight flat runtime list instead of a full retained-mode layout tree
- docking metadata is preserved and validated, but generated layout behavior is still intentionally minimal
- tab interaction is limited to basic selected-tab handling in generated output
