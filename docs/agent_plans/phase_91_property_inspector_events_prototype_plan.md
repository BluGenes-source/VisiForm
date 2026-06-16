# Phase 91 Property Inspector Events Prototype Plan

## Objective

Produce a reviewable interactive prototype and design review package for the Phase 89 Property Inspector `Properties` and `Events` tab workflow, then implement the approved Phase 91 Events-tab workflow in the VisiForm C++ application after explicit approval.

## Scope

- Build a frontend-only prototype that preserves the current VisiForm desktop visual style.
- Cover empty selection, button selection, assigned/unassigned events, handler creation, existing-handler selection, removal, and validation/error states.
- Summarize the intended user flow and implementation recommendations.
- Keep prototype artifacts isolated from production source and generated output.
- Document the current C++ architecture and exact expected implementation files.
- Implement the approved workflow while preserving existing Properties-tab behavior.
- Keep this plan updated as the implementation handoff record.

## Requirements

- Show fixed `Properties` and `Events` tabs.
- Show `No widget selected`.
- Show a selected `Button` with no assigned events.
- Show a selected `Button` with an assigned click handler.
- Demonstrate creating a new handler.
- Demonstrate selecting an existing compatible handler from a signature-filtered dropdown.
- Demonstrate removing an event assignment without confirmation.
- Demonstrate row-local invalid handler and duplicate/conflicting-handler messaging.
- Preserve the compact event-row layout.
- Preserve existing Properties-tab behavior.
- Persist event assignments in the existing project model and `.vfb.json` string properties.
- Preserve generated C++ event output behavior and `USER CODE` preservation.
- Do not run `VisiForm.exe` or generated apps.

## Approved Prototype Behavior

- Event handler assignment rows use explicit compact `Create`, `Existing`, and `Clear` controls.
- Direct handler-name text entry is removed from the main Events-tab row.
- `Existing` shows only handlers whose `handlerSignatureKind` matches the current event.
- `Create` proposes a default handler name from selected widget name plus event name.
- Validation errors appear directly beneath the affected event row.
- `Clear` removes an assignment immediately without confirmation.
- The `Events` tab remains visible with a clear empty state when no widget is selected.
- Event rows keep the current compact inspector row rhythm.

## Architectural Decisions

- Use a standalone static HTML prototype in `docs/prototypes/phase_91_property_inspector_events_review/`.
- Mirror the colors, row heights, tab treatment, and panel proportions from `src/ui/PropertyInspector.cpp`.
- Treat the repository source and docs as the visual target; no Product Design saved context was available.
- Use mock state transitions in JavaScript rather than wiring to the real model.
- Remove direct handler-name text entry from event rows in favor of explicit `Create`, `Existing`, and `Clear` controls.
- Let `Create` assign a proposed default handler based on widget name plus event name.
- Limit `Existing` dropdown options to handlers with the matching event signature kind.

## Current Repository Architecture

### Existing Property Inspector architecture

- `src/ui/PropertyInspector.h` defines the inspector-local `InspectorTab`, row model, edit kinds, choices, pending edits, bounds helpers, active editing state, active tab, scroll metrics, and drawing/hit-test API.
- `src/ui/PropertyInspector.cpp` owns row creation, tab filtering, tab strip drawing, row rendering, scrollbar behavior, slider behavior, color swatch hit testing, and active editor bounds.
- `PropertyInspector::buildRows(...)` constructs one full row list from the selected widget, project settings, `WidgetRegistry` metadata, and fallback unknown widget properties.
- `PropertyInspector::rowsForActiveTab(...)` filters that full list into either `Properties` rows or event rows. Event rows are detected with `PropertyInspector::isEventRow(...)`.
- `PropertyInspector::draw(...)` renders the dock panel, fixed header, fixed `Properties` / `Events` tabs, empty states, rows, value cells, section rows, action buttons, sliders, swatches, and scrollbar.
- `src/ui/MainWindow.cpp` owns application-level mouse dispatch, beginning/committing inspector edits, text editor placement, dropdown opening/selection, event/property validation, undo command creation, dirty-state updates, status text, and redraws.
- Existing dropdown behavior is centralized in `DropdownControl`, opened from `MainWindow::openInspectorDropdown(...)`, and completed through `MainWindow::handleDropdownSelection(...)` / `applyInspectorDropdownSelection(...)`.

### Existing Properties tab implementation

- `PropertyInspector::buildRows(...)` currently appends base rows (`id`, `type`, `name`, geometry), root project/export settings, special rows for `Sizer`, `GroupBox`, `TabControl`, `TabPage`, `TreeView`, and `TableGrid`, then registry-defined normal properties.
- Registry-defined normal properties are converted to `PropertyRow` using `editKindForDefinition(...)`, definition choices, defaults, hints, style overrides, and special handling for image resources, docking, anchors, tree nodes, table/grid data, and sliders.
- Events are created in `buildRows(...)` as read-only event rows with compatible handler choices, signature metadata, and row-local validation text. Phase 89 filtering prevents them from showing on the Properties tab.
- Unknown scalar widget properties not already drawn are appended at the end of the Properties tab as fallback rows. This preserves unknown persisted fields.

### Widget selection flow

- Canvas and tree clicks route through `MainWindow::mouseDown(...)`.
- Project Tree hit testing uses `ProjectTree::hitTestWidgetId(...)`; canvas hit testing uses `DesignerCanvas` hit helpers.
- Normal single selection flows through `MainWindow::handleWidgetClicked(...)`, which calls `ProjectDocument::setSelection(...)`, updates property editor bounds, sets operation status, and redraws.
- Additive selection toggles `ProjectDocument::toggleSelection(...)`; the inspector title receives the selection count from `propertyInspector_.draw(...)`.
- `ProjectDocument::selectedWidget()` is the source of truth for inspector row generation. With no selected widget, `PropertyInspector::buildRows(...)` returns no rows and `draw(...)` displays empty-state copy.

### Event metadata currently available

Registry events are defined by `model::WidgetEventDefinition` with:

- `key`
- `label`
- `handlerSignatureKind`
- `hint`

Current registry-defined events:

| Widget type | Events |
| --- | --- |
| `FormWindow` | `onLoad` (`void_event`), `onClose` (`void_event`) |
| `TableGrid` | `onSelectionChanged` (`void_event`), `onCellDoubleClick` (`void_event`) |
| `TreeView` | `onChanged` (`void_event`), `onDoubleClick` (`void_event`) |
| `Button` | `onClick` (`void_event`), `onRelease` (`void_event`), `onDoubleClick` (`void_event`) |
| `TextBox` | `onTextChanged` (`string_event`) |
| `ComboBox` | `onChanged` (`void_event`) |
| `ListBox` | `onChanged` (`void_event`), `onDoubleClick` (`void_event`) |
| `CheckBox` | `onToggle` (`bool_event`) |
| `RadioButton` | `onSelected` (`bool_event`) |
| `Slider` | `onChanged` (`float_event`) |
| `ScrollBar` | `onChanged` (`float_event`) |
| `ColorPicker` | `onChanged` (`string_event`) |
| `ModalDialog` | `onAccepted` (`void_event`), `onCancelled` (`void_event`) |

`WidgetRegistry::createDefaultWidget(...)` initializes every registry-defined event key as an empty string property when a widget is created.

## Implementation Specification

### Project-model changes required for event assignments

- No new persistent model field is required.
- Keep event assignments as existing string entries in `WidgetNode::properties`, keyed by event key such as `onClick`.
- Keep blank strings as the representation for "no assignment".
- Add UI/helper-only functions if useful, but avoid changing `ProjectDocument`, `WidgetNode`, or `.vfb.json` schema unless implementation reveals a blocking need.
- Continue relying on `WidgetRegistry::createDefaultWidget(...)` to seed missing event properties for new widgets.

### Handler-name validation rules

- Preserve the current rule: blank event handler names are allowed.
- Non-empty handler names must pass `utils::isValidCppIdentifier(...)`.
- Current accepted shape: first character is a letter or underscore; remaining characters are letters, digits, or underscores.
- Invalid handler names must produce row-local feedback under the affected event row.
- `MainWindow::setSelectedWidgetPropertyFromString(...)` currently returns `false` and status text `"Invalid event handler name"` when an event property is invalid; implementation should expose a way for the inspector to retain and render row-local error text.

### Compatible-handler signature rules

- Compatible existing handlers are grouped by `WidgetEventDefinition::handlerSignatureKind`.
- Existing compatible choices must be collected across the document tree, matching the current `callbackChoices(...)` / `collectMatchingHandlers(...)` behavior.
- Handler names with the same signature kind may be reused across multiple events.
- Handler names reused with different signature kinds are conflicts. `ProjectValidator` reports `CALLBACK_SIGNATURE_CONFLICT`; `VisageCppEmitter::collectHandlers(...)` also fails with `"Event handler name conflict: <name> has multiple signatures"`.
- The `Existing` dropdown should exclude incompatible handlers by default. Incompatible assignments may still appear from older files or direct JSON edits and must render row-local conflict feedback when detected.

### UI components required for Create, Existing, and Clear

Required `PropertyInspector` model updates:

- Extend `PropertyRow` or add a specialized event-row/action model so event rows can represent:
  - current assigned handler display text
  - event key
  - event label/hint
  - handler signature kind
  - compatible existing handler choices
  - row-local validation/error text
  - compact action hit targets for `Create`, `Existing`, and `Clear`
- Add an inspector interaction result type distinct from text/slider `PendingEdit`, for example `PendingEventAction`.
- Add hit testing for individual event-row controls instead of treating the entire event row as a text-edit target.
- Draw compact controls inside the current value cell without increasing default row height unless row-local error text is present.
- Draw row-local error text directly below the affected event row. This may require variable row layout heights for error rows or synthetic error rows after the event row.
- Preserve existing tab strip, content scrolling, and empty-state behavior.

Required `MainWindow` updates:

- Handle `Create` by generating a proposed handler name from selected widget name plus event name, then assigning it via the existing property mutation/undo path.
- Handle `Existing` by opening the existing `DropdownControl` anchored to the event row/action, populated only with signature-compatible handlers.
- Handle `Clear` by assigning an empty string through the existing property mutation/undo path without confirmation.
- Convert invalid assignment failures into row-local inspector errors while still updating status text for accessibility/diagnostics.
- Decide whether event actions use existing `setSelectedWidgetPropertyFromString(...)` directly or a new event-specific helper such as `setSelectedWidgetEventHandler(...)`.

Default handler name proposal:

- Suggested algorithm: `handle` + sanitized PascalCase widget name + PascalCase event name without a leading `on`.
- Example: widget name `helloButton`, event `onClick` -> `handleHelloButtonClick`.
- If widget name is empty or sanitizes to empty, fall back to widget id.
- If the proposed name already exists with a compatible signature, append a numeric suffix or ask for approval before choosing a collision policy.
- If the proposed name exists with an incompatible signature, generate a unique suffix and show row-local warning/status text.

### Serialization changes required in `.vfb.json`

- No schema change is required.
- `JsonProjectWriter` already serializes all widget scalar properties, including event assignment strings.
- `JsonProjectReader` already accepts string properties and preserves unknown scalar properties.
- Existing `.vfb.json` files with event assignments remain compatible.
- Existing `.vfb.json` files with invalid or conflicting handler names should continue loading; validation/export should report errors rather than reader rejection.
- Add/extend tests proving event assignments created through the new UI shape still round-trip as widget properties.

### Generated C++ code changes

- No generator behavior change is expected for the approved UI workflow because generated handlers already derive from event property strings.
- `VisageCppEmitter::collectEventBindings(...)` already skips blank handler names, rejects invalid C++ identifiers, maps event keys to registry-defined signature kinds, deduplicates compatible handler names, and rejects incompatible signature reuse.
- `emitUserSubclassHeader(...)` and `emitUserSubclassCpp(...)` already emit handler declarations/definitions and preserve `USER CODE` blocks by handler name.
- Generated C++ tests may be useful if the existing test target is later expanded to include generator source files, but implementation of the approved inspector controls should not require generator file edits.

### Backward compatibility requirements

- Do not change `.vfb.json` schema version.
- Preserve existing event property keys and values.
- Preserve blank string behavior for unassigned handlers.
- Preserve existing compatible-handler reuse semantics.
- Preserve validation/export behavior for invalid or incompatible handler values already present in project files.
- Preserve existing Properties tab behavior and fallback unknown-property display.
- Preserve Phase 89 tab behavior: event rows appear only on the Events tab.
- Preserve `USER CODE BEGIN` / `USER CODE END` preservation by handler name.
- Preserve current `MainWindow` generated base class rule.

### Automated tests

Focused model/validation/serialization tests:

- Extend `tests/test_widget_registry_events.cpp` to cover all registry event metadata, not just Button, including signature kinds.
- Add validation tests in `tests/test_project_validation.cpp` for invalid handler names and incompatible signature reuse.
- Add serialization coverage confirming blank and assigned event strings round-trip unchanged.

UI logic tests:

- There is currently no UI test harness for `PropertyInspector` because it depends on Visage drawing and app event flow.
- If feasible without launching `VisiForm.exe`, add small pure helper tests only for default handler-name generation and compatible-handler filtering by extracting helpers into non-Visage utility code.
- Do not invent a new UI framework.

Generated code tests:

- If generator source files are added to `VisiFormTests` in a later implementation, add source-inspection tests for:
  - blank handlers emit no user stub
  - `Create`-style assigned handler names emit user subclass declarations/definitions
  - compatible reuse emits a single handler
  - incompatible reuse fails before output
- Otherwise document generator verification as manual/source-inspection pending.

### Manual validation steps

- Developer launches `VisiForm.exe` manually; automated agents must not launch it.
- Select no widget and confirm the Events tab remains visible with clear empty state.
- Select a Button with no assigned events and confirm compact event rows show `<unset>`, `Create`, `Existing`, and `Clear`.
- Use `Create` on `onClick` and confirm the proposed handler name follows the widget-name/event-name rule.
- Use `Existing` on `onRelease` and confirm only compatible `void_event` handlers are offered.
- Confirm incompatible handlers are not listed in the `Existing` dropdown.
- Use `Clear` and confirm assignment disappears immediately without confirmation.
- Verify invalid or conflicting project data shows row-local errors under the affected row.
- Switch between Properties and Events tabs and confirm edits persist and no stale editor/dropdown remains.
- Save and reload a `.vfb.json` project and confirm event assignments persist.
- Run validation and confirm invalid/conflicting callbacks produce expected errors.
- Export generated C++ and inspect that handler declarations, definitions, and `USER CODE` preservation markers remain correct.

### Exact files changed in this implementation

Core UI:

- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/editors/DropdownControl.cpp`

Tests:

- `tests/test_widget_registry_events.cpp`
- `tests/test_project_validation.cpp`

Documentation:

- `docs/agent_plans/phase_91_property_inspector_events_prototype_plan.md`
- `docs/VISIFORM_PROJECT_SPEC.md`
- `docs/widget_catalog.md`
- `docs/widget_registry.md`
- Existing untracked prototype artifacts remain under `docs/prototypes/phase_91_property_inspector_events_review/`.

Intentionally not changed:

- `src/model/ProjectDocument.*`
- `src/model/WidgetNode.*`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.cpp`
- `src/serialization/JsonProjectReader.*`
- `src/serialization/JsonProjectWriter.*`
- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CodeGenerator.*`
- `tests/CMakeLists.txt`
- CMake presets, vcpkg triplets, and MSVC runtime settings.

## Architectural Decisions Requiring Approval

1. Create behavior: resolved by assigning the proposed default immediately, matching the approved prototype.
2. Existing-handler UI: resolved for this implementation by reusing `DropdownControl` anchored to the event row/action for consistency and clipping behavior.
3. Row-local error state ownership: resolved for this implementation by deriving row-local errors during `PropertyInspector::buildRows(...)` from persisted assignment values and document-wide signature conflicts.
4. Default-name collision policy: resolved for this implementation by reusing an exact compatible handler name and suffixing around incompatible collisions.
5. Clear disabled state: resolved for this implementation as disabled visual state with harmless no-op click handling when unset.

## TODO Checklist

- [x] Inspect repository status before changes.
- [x] Review Phase 89 plan and current Property Inspector implementation.
- [x] Confirm Button event metadata and handler validation rules.
- [x] Create an isolated interactive prototype.
- [x] Revise the Events tab interaction model from direct text entry to explicit compact controls.
- [x] Inspect current C++ Property Inspector architecture.
- [x] Inspect existing Properties-tab and Events-tab row filtering.
- [x] Inspect widget selection flow.
- [x] Inventory registry-defined widget event metadata and signature kinds.
- [x] Inspect project-model, serialization, validation, and generator behavior for event assignments.
- [x] Document implementation specification for the approved prototype.
- [x] Document exact expected implementation files.
- [x] Identify architectural decisions requiring approval before C++ work starts.
- [x] Add user-flow summary and implementation recommendations.
- [x] Record validation status and final result summary.

## Future Implementation TODO Checklist

- [x] Confirm architectural decisions before editing C++.
- [x] Add event action state/types to `PropertyInspector`.
- [x] Add event-row control hit testing for `Create`, `Existing`, and `Clear`.
- [x] Add compact event-row drawing for assignment text plus controls.
- [x] Add row-local error rendering beneath affected event rows.
- [x] Preserve Events-tab empty state for no selection and no supported events.
- [x] Add default handler-name proposal helper.
- [x] Add compatible-handler filtering helper using `handlerSignatureKind`.
- [x] Wire `Create` action through undoable property assignment.
- [x] Wire `Existing` action through filtered dropdown selection.
- [x] Wire `Clear` action through undoable blank assignment with no confirmation.
- [x] Preserve existing tab switching, scrolling, dropdown closing, and editor cancellation behavior.
- [x] Preserve existing property editing for non-event rows.
- [x] Add automated tests for registry event metadata/signatures.
- [x] Add automated tests for invalid handler names and incompatible signature reuse.
- [x] Add serialization tests for blank and assigned event strings.
- [x] Update `docs/VISIFORM_PROJECT_SPEC.md`.
- [x] Update `docs/widget_catalog.md`.
- [x] Update `docs/widget_registry.md`.
- [ ] Record build/test status after developer-approved validation command or manual build.
- [ ] Record manual validation outcomes or clearly defer them to the developer.
- [ ] User approval for native `DropdownControl` overlay as the closest equivalent to the prototype's inline HTML `<select>`.
- [ ] User approval for 430px native inspector width as the closest readable equivalent to the 386px HTML mock within Visage.

## Prototype Conformance Matrix

| Prototype behavior | Native implementation behavior | Match status | Required correction | Validation result |
| --- | --- | --- | --- | --- |
| Fixed `Properties` and `Events` tabs remain visible at the top of the inspector. | Native inspector draws the fixed tab strip and preserves active tab state. | Match | None. | Source review: `PropertyInspector::draw(...)` still renders both tabs before row content. |
| Properties tab contains widget attributes only; event rows are not duplicated there. | Event rows and Events support rows are filtered out of the Properties tab. | Match | None. | Source review: `rowsForActiveTab(...)` skips event/support rows for `Properties`. |
| Events tab remains visible when no widget is selected. | Native tab strip remains visible and Events can be selected with no widget. | Match | None. | Source review. |
| No-selection Events state shows clear empty copy: no widget selected and select a widget to assign handlers. | Native Events empty state now shows `No widget selected.` and a second instructional line. | Match | Resolved: replaced previous one-line `No selection for Events` copy. | Source review. Manual visual validation pending. |
| Unsupported-widget Events state is distinct from no selection. | Native Events tab now shows `No supported events.` plus guidance to use Properties when selected widget has no event metadata. | Match | Resolved: updated empty copy for selected widgets with no Events rows. | Source review. Manual visual validation pending. |
| Event rows are grouped under widget-specific section title such as `Button Events`. | Native Events tab now inserts `<WidgetType> Events`, e.g. `Button Events`. | Match | Resolved: changed generic/missing event section to widget-specific event section. | Source review. Manual visual validation pending. |
| Event rows preserve compact row layout with label, assignment value, `Create`, `Existing`, and `Clear` controls. | Native event rows draw a dedicated handler selector field plus compact buttons in the value cell. Inspector width is now 430px as the closest native equivalent that avoids clipped controls in Visage. | Close native equivalent; needs approval | Resolved: widened right panel from 300px to 430px, narrowed the Events label column, and retained compact same-row controls. | Source review. Manual visual validation pending. |
| Existing-handler dropdown is a clean popup associated with the handler-selection area, not a cramped or overlapping control. | Native now draws a distinct handler selector field, opens Existing from the field or button, anchors the popup to the selector field, and lets `DropdownControl` use a readable minimum popup width clamped to the inspector viewport. | Match | Resolved: stopped using the narrow `Existing` button rectangle as the popup anchor; widened/cleaned event row spacing and popup sizing. | Source review. Manual visual validation pending. |
| Assigned handler text truncates in the value cell when needed. | Native text is clipped by canvas clamp bounds and available assignment width before action buttons. | Close match | No correction beyond panel-width fix. | Source review; exact ellipsis behavior depends on Visage text clipping. |
| `Create` assigns a proposed default handler name based on widget name and event name. | Native `Create` assigns `handle` + sanitized PascalCase widget name + event name without leading `on`. | Match | None after implementation. | Source review: `proposedEventHandlerName(...)`. |
| `Create` status says the proposed handler was assigned. | Native status now says `Proposed and assigned <handler> for <event>.` | Match | Resolved: updated status copy. | Source review. |
| `Existing` exposes only compatible handlers for the current event signature. | Native `Existing` opens `DropdownControl` with only valid C++ identifiers matching `handlerSignatureKind`; incompatible handlers are excluded. | Match | Resolved: filtered invalid identifiers out of compatible choices. | Source review. Automated tests cover signature metadata and conflict validation; UI filtering manual validation pending. |
| `Existing` uses an inline HTML `<select>` with a `Select...` option in the prototype. | Native uses existing `DropdownControl` as an overlay anchored to the row action, without an inert placeholder item. | Close native equivalent; needs approval | Technical/design limitation: native app has a shared dropdown component rather than inline select. Keep closest native equivalent unless user requires a custom inline select control. | Source review. User approval pending. |
| Compatible suggestions summary appears below event rows. | Native Events tab now adds `Compatible Suggestions` and one row per signature kind used by the selected widget's events. | Match | Resolved: added suggestion support rows. | Source review. Manual visual validation pending. |
| `Clear` is visible as a danger action and removes assignment immediately without confirmation. | Native `Clear` now draws as a danger button and clears through existing undoable property assignment when non-empty. Blank clear is harmless and does not create a model edit. | Match | Resolved: changed Clear visual from disabled gray to danger styling and no-confirm behavior. | Source review. Manual validation pending. |
| Invalid C++ identifier error appears directly beneath the affected event row and names the invalid value. | Native row-local error now includes the invalid handler name plus identifier guidance. | Match | Resolved: improved row-local validation copy. | Source review. Validation tests cover invalid callback identifiers. |
| Duplicate/incompatible handler error appears directly beneath the affected event row and names existing/required signatures. | Native row-local error now includes conflicting handler name, existing signature, and required signature. | Match | Resolved: added signature-specific conflict copy. | Source review. Validation tests cover incompatible signature reuse. |
| Compatible duplicate reuse is allowed and presented as normal reuse. | Native compatible reuse is allowed by validation and `Create` may reuse an exact compatible existing name. | Match | None after implementation. | Automated test covers compatible event callback reuse. |
| Disabled/enabled states are visible for unavailable actions. | Native `Existing` shows disabled styling when no compatible handlers are available; `Clear` stays visible as danger action to match prototype and harmlessly no-ops when blank. | Match with prototype-priority adjustment | Resolved: changed Clear from disabled visual to prototype danger action. | Source review. Manual validation pending. |
| Validation messages are placed beneath the affected row, not in a distant global message. | Native error band is drawn below the row within the value column; status bar may still mirror operation results. | Match | None after implementation. | Source review. |
| Changing widget selection clears stale event dropdown/edit state and shows the new selection's rows or empty state. | Native `handleWidgetClicked(...)` now clears inspector editing, text edit, and dropdown state before changing selection. | Match | Resolved: added selection-change cleanup. | Source review. Manual validation pending. |
| Existing projects with blank event properties remain valid and emit no handler stub. | Native preserves blank strings in the model and JSON; generator behavior unchanged. | Match | None. | Automated blank event JSON round-trip test added; build/test execution pending. |
| Generated C++ output consumes assigned event handler strings and preserves user code by handler name. | Native generator already supports assigned event strings; no generator change required. | Match | None. | Source review only; generator build/export validation pending. |

## Follow-up Bug Audit: Event Row State and Dropdown Layout

### Confirmed Causes

- Bug 1 cause: event rows did not own any event-specific transient interaction state. `PropertyInspector::draw(...)` could only use generic row enabled/read-only styling and `activeKey_`, which is for text/property editing and is never set for event actions. As a result, the UI had no reliable way to style only the specific event/action associated with the currently open Existing popup or latest row interaction.
- Bug 1 contributing risk: transient popup state lived only in `DropdownControl` as one global active dropdown key. Although the key includes the event name, `PropertyInspector` did not receive or mirror that key, so row rendering could not distinguish active event identity.
- Bug 2 cause: the Existing-handler popup was opened from event-row action geometry that could collapse to a narrow button-sized anchor, and the shared dropdown was sized primarily from the anchor. This produced compressed/clipped menu appearance when handler names were longer than the button/field.
- Bug 2 contributing risk: the popup viewport is intentionally constrained to the property inspector, so an undersized anchor made the menu look like it was participating in row layout rather than rendering as a clean overlay.
- Layout cause: Events rows were using the generic property label/value layout with limited value-cell width, and the main-window right panel could shrink below the width needed by the handler selector plus `Create`, `Existing`, and `Clear` buttons. At those widths the row math allowed controls to overlap even after popup anchoring was corrected.

### Planned Corrections

- Add event-specific transient active state to `PropertyInspector`, keyed by event key and action.
- Set the active state only for the event/action being interacted with and clear it when popup/edit state closes or widget selection changes.
- Keep event assignments keyed by event property name; do not introduce shared widget-type-level state.
- Anchor Existing-handler popups to a dedicated handler selector field, not to the whole row or a narrow button.
- Ensure dropdown popup width has a readable minimum and is clamped to the inspector viewport without distorting row layout.
- Preserve the approved Create/Existing/Clear workflow and dark VisiForm styling.

### Implemented Corrections

- Added `PropertyInspector::setActiveEventControl(...)` / `clearActiveEventControl(...)` and state keyed by event key plus `EventAction`.
- Existing-handler popup opening now marks only the specific event row/action as active; closing, selecting, scrolling, cancelling, or changing widget selection clears the transient state.
- `Create` and `Clear` remain immediate actions and do not leave persistent row highlight behind.
- Event selector fields and action buttons are calculated independently so the selector no longer overlaps `Create`, `Existing`, or `Clear`.
- The property inspector right panel now uses the intended native Events width in normal layouts and has a 386px minimum, matching the approved mock's width neighborhood instead of shrinking to a malformed 24% width.
- The event label column is Events-tab-specific and reserves the actual event-action button width before assigning space to labels or handler text.
- Existing dropdown popup still renders as the shared native overlay, but with selector anchoring and readable minimum popup width.

## Validation Plan

- Static review: inspect prototype source for required states and interactions.
- Browser/manual review: open `docs/prototypes/phase_91_property_inspector_events_review/index.html` and click through scenario controls and inspector actions.
- Specification review: inspect current repository architecture and document implementation constraints before C++ changes.
- Source review: inspect `PropertyInspector` and `MainWindow` changes for Properties-tab preservation, Events-tab action routing, row-local errors, and dropdown compatibility filtering.
- Automated tests: run the existing Catch2/CMake test target after the developer provides or approves the exact supported Visual Studio build/test command.
- Build validation: run the supported Visual Studio 2022 configuration after the developer provides or approves the exact command required by repository safety rules.

## Compatibility Considerations

- No `.vfb.json` schema changes.
- Event handler names remain modeled as string widget properties.
- Compatible duplicate handler reuse is valid; incompatible signature reuse is the conflict case.
- Existing `.vfb.json` schema and generated handler behavior are preserved.
- Existing projects with missing, blank, invalid, or conflicting event strings continue to load; validation/export surfaces problems instead of rejecting the file at read time.

## Build / Test Status

- Current branch: `main`.
- Most recent commit before this work: `cfc79e5 Update documentation for phases and project status`.
- Build not run. The user requested the supported Visual Studio 2022 configuration, but repository instructions prohibit running build commands unless the developer explicitly asks for the exact command. Waiting on the exact command or manual developer build result.
- `VisiForm.exe` not launched, per repository safety rules.
- Prototype JavaScript syntax checked with `node -e`.
- Browser click-through verification not run because `agent-browser` is not available on PATH and no in-app browser navigation tool was exposed in this session.
- C++ source review performed by inspection. Automated C++ tests not run yet for the same exact-command validation constraint.
- `git diff --check` completed with no whitespace errors; Git reported expected CRLF normalization warnings for touched text files.
- Prototype conformance matrix completed for every requested visible state and interaction; native-equivalent design items still need user approval.
- Follow-up layout conformance fix: corrected malformed Existing-handler dropdown anchoring/width and stabilized event-row selector/button spacing by adding a handler selector field, anchoring the popup to that field, widening the inspector panel to 430px with a 386px native minimum, reserving event-action width in the Events label/value split, and giving shared dropdown popups a 180px minimum width clamped to viewport bounds.

## Final Result Summary

Created a standalone interactive prototype and review notes for the Phase 89 Property Inspector Events tab workflow. After prototype approval, inspected the current C++ architecture and documented the implementation specification, expected files, validation plan, compatibility constraints, and approval decisions. The approved C++ workflow is now implemented in the inspector and main-window event routing: event rows use compact `Create`, `Existing`, and `Clear` controls, `Existing` is signature-filtered, default created names are proposed from widget/event names, row-local validation errors render beneath affected rows, event assignments continue to persist as widget string properties, and generated C++ behavior remains schema-compatible. Automated tests were added/updated for registry event metadata, validation failures, compatible reuse, and event JSON round-trip behavior. The prototype conformance review corrected native discrepancies around inspector width, event section headings, compatible suggestions, Clear danger styling, row-local validation copy, and stale dropdown cleanup on selection changes. A follow-up presentation pass corrected shared visual interaction state by adding per-event/action active state, and corrected malformed Existing-handler dropdown formatting by adding a proper handler selector field, anchoring the popup to that field, widening/clamping dropdown popup width, reserving event-action width during row layout, and increasing native inspector width for readable row spacing. Build/test execution, manual app validation, and approval of the `DropdownControl` overlay as the native equivalent of the prototype inline `<select>` remain pending.

## Remaining TODOs

- Run the supported Visual Studio 2022 build/test path once the exact command is provided or a manual developer build result is available.
- Manually validate the Events-tab workflow in `VisiForm.exe`; automated agents must not launch the app.
- Approve the native `DropdownControl` overlay as the closest equivalent to the prototype inline `<select>`, or request a custom inline native select-style control.
- Approve the 430px native inspector width as the closest readable equivalent to the 386px HTML mock, or request a stricter-width layout with more aggressive text clipping.
- If build, test, or manual validation finds issues, update this plan and resolve them before claiming Phase 91 complete.
