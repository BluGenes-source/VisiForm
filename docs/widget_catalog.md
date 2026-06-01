# Widget catalog

## Supported widgets

Current widget types:

- `FormWindow`
- `Frame`
- `GroupBox`
- `Panel`
- `TabControl`

Current editor-active container workflow:

- `GroupBox` is the first container with a focused parenting workflow in the editor
- selecting a `GroupBox` inserts new widgets into that `GroupBox`
- selecting the root form inserts new widgets at the root
- other selected widgets currently fall back to root insertion instead of accepting new children directly

## GroupBox repair-pass behavior

Current `GroupBox` editor behavior:

- child widgets use coordinates relative to the `GroupBox`
- `ProjectTree` shows `GroupBox` children recursively under the `GroupBox`
- the `Property Inspector` shows a `Children` section when a `GroupBox` is selected
- the `Property Inspector` can select a current child, add an existing root-level widget into the `GroupBox`, and remove a child back to the root form
- dragging a marquee from empty content inside a selected or active `GroupBox` now requires additive multi-select intent and draws the selection rectangle in root canvas coordinates while limiting selection results to that `GroupBox`'s descendants
- selected child widgets inside a `GroupBox` keep their normal absolute-position selection outlines and handles
- adding a new `GroupBox` keeps it at the root level for this repair pass and selects it immediately after creation
- clicking a `GroupBox` border, title, or empty body selects the `GroupBox`, while clicking a child still selects the child first
- clicking or dragging a selected `GroupBox` title, border, or empty body starts the normal widget move interaction on the same press
- dragging a selected root-level `GroupBox` moves the parent widget while child widget local coordinates remain unchanged

Current `GroupBox` limitations in this repair pass:

- the child management workflow is currently specialized for `GroupBox`
- this pass does not enable equivalent general-purpose parenting UI for every other container-capable widget
- `Label`
- `Button`
- `TextBox`
- `CheckBox`
- `RadioButton`
- `Slider`
- `ScrollBar`
- `StatusBar`
- `ProgressBar`
- `ModalDialog`
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

The editor exposes these widget types through both the `Widget Palette` and the `Insert` menu.
Both entry points use the same add-widget flow in `MainWindow`.

## Hierarchy and container metadata

Common hierarchy-aware properties now used by multiple widgets:

- `dock`
- `anchor`
- `layoutMode`
- `tabIndex`

Container-capable widget types in the current registry:

- `FormWindow`
- `Frame`
- `GroupBox`
- `Panel`
- `TabControl`

`TabControl` also stores:

- `tabs`
- `selectedTab`

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

Current style editor behavior:

- `borderThickness` uses a Property Inspector slider editor
- `cornerRadius` uses a Property Inspector slider editor
- both sliders use the numeric range `1..25`
- legacy empty or `<unset>` values are treated safely as `1` in the editor and validation flow

## Event metadata

For this phase, event handler names are stored as normal widget string properties.

Supported event properties:

- `FormWindow`
  - `onLoad`
  - `onClose`
- `Button`
  - `onClick`
  - `onRelease`
  - `onDoubleClick`
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
- `ModalDialog`
  - `onAccepted`
  - `onCancelled`

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
Property rows also show property-definition hints in the main status pane when hovered.

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

- `GroupBox` - exported as a titled container and preserves parent-child hierarchy metadata
- `Panel` - exported as a generic container and preserves parent-child hierarchy metadata
- `TabControl` - exported as a tab container, keeps `tabs`, `selectedTab`, and child `tabIndex` metadata, and only draws children for the selected tab in generated runtime output
- `Button` - click completion fires `onClick`, release-over-button fires `onRelease`, double-click fires `onDoubleClick`, toggle buttons preserve a checked state, and callback code can update other generated widgets through protected `MainWindow` helpers such as `setText(...)`, `setChecked(...)`, or `setStatusBarField(...)`
- `CheckBox` - toggles and fires `onToggle`, and callback code can read or write checkbox state with `getChecked(...)` and `setChecked(...)`
- `RadioButton` - enforces single selection per group and fires `onSelected` for the clicked item; callback code can also use `setSelected(...)` to apply group-aware selection changes
- `Slider` - supports mouse dragging and fires `onChanged`; callback code can read or write numeric state with `getValue(...)` and `setValue(...)`
- `ScrollBar` - supports arrow clicks, track paging, thumb dragging, and fires `onChanged`; callback code can read or write numeric state with `getValue(...)` and `setValue(...)`
- `ColorPicker` - renders a static color swatch preview and exports a string-based `onChanged` callback signature
- `TextBox` - supports click focus, basic text entry, Backspace, and fires `onTextChanged`
- `ProgressBar` - display only, but renders current runtime value text and can be updated from callbacks with `setValue(...)` or `setProgressValue(...)`
- `StatusBar` - display only, but renders configured runtime field text and can be updated from callbacks with `setStatusBarField(...)`
- `ModalDialog` - exports reusable modal dialog definitions with generated `showModalDialog(...)`, `closeModalDialog()`, and `activeModalDialogId()` helpers; message boxes can also be shown from callbacks with `showMessageDialog(...)`, and widget-backed dialogs fire `onAccepted` or `onCancelled`

Generated callback code can find widgets by exact `id` or exact `name` through protected `MainWindow` helpers.
When a helper accepts `idOrName`, lookup order is:

1. exact `id`
2. exact `name`

Generated helper setters request a repaint after state changes and use safe return values instead of exceptions.

Current generated runtime limitations:

- no text selection, clipboard, or IME support in generated `TextBox` yet
- no full retained-mode widget framework yet
- no generated layout manager yet
- container export currently preserves hierarchy metadata and selected-tab visibility, but does not yet generate a full retained-mode layout system

## StatusBar defaults

Current editor-facing `StatusBar` defaults:

- height `50`
- `fields = 3`
- `text0 = Ready`
- `text1 = This`
- `text2 = Cool`
- `dock = Bottom`
- `fillWidth = true`

The `Property Inspector` now presents the editable field labels as `Section 1`, `Section 2`, `Section 3`, and `Section 4` instead of raw `text0`-style keys.

## Image widget notes

Current `Image` widget properties:

- `resourceId`
- `imagePath`
- `scaleMode`
- `hint`

`scaleMode` choices:

- `Stretch`
- `Fit`
- `Fill`
- `Center`

Current `Image` behavior:

- the property inspector shows `resourceId` with the display label `Resource`
- the `Resource` row uses a dropdown of available managed image resources
- dropdown entries show readable labels such as `Logo (image_1)` while the stored value remains the stable id such as `image_1`
- `resourceId` points to a managed project resource of type `Image`
- `imagePath` remains available as a direct-path fallback
- `imagePath` is used only when no managed image resource is assigned
- older `source` values are treated as legacy fallback data
- the designer renders a placeholder label such as `Image: Logo` or `Image: background.png` instead of full decoded image previews
- missing managed image resources are shown with a warning-style placeholder state
- generated export currently uses the managed relative asset path as safe placeholder text or comments

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
- `ModalDialog` - `420 x 240`
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

Generated runtime helper support:

- `setStatusBarField("statusBar_1", 0, "Ready")`
- valid field indices are `0` through `3`

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

Generated runtime helper support:

- `setValue("progressBar_1", 50.0f)`
- `setProgressValue("progressBar_1", 50.0f)`

This makes slider-to-progress and scrollbar-to-progress callback flows straightforward in exported projects.
