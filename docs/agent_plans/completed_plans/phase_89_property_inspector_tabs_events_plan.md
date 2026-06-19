# Phase 89 Property Inspector Tabs And Events Plan

## Objective

Add fixed `Properties` and `Events` tabs to the Property Inspector so normal widget properties remain on the Properties tab and widget event handler rows move to a dedicated Events tab.

## Scope

- Update the Property Inspector UI to render a fixed tab strip below the title area.
- Split the current inspector row model into property rows and event rows without duplicating the editor implementation.
- Preserve existing event editing, selection refresh, undo/redo, serialization, and reload behavior.
- Keep inspector scroll ownership inside the active tab content area so the tab strip remains visible.
- Update documentation and equivalent project history for the new inspector behavior.

## Requirements

- Remove the current `Events` section and event rows from the Properties tab.
- Show supported widget events on the Events tab using widget metadata, not visible labels.
- Preserve existing row editor behavior, dropdown suggestions, editing, and command integration.
- Preserve empty-selection and multi-selection safety behavior.
- Avoid duplicate rows and empty orphaned section headers.
- Keep the tab strip fixed while the active tab content scrolls.

## Investigation Report

### 1. Current Property Inspector implementation files

- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/MainWindow.cpp`

`PropertyInspector` owns row generation, hit testing, scrolling, active editor placement, and drawing. `MainWindow` owns applying edits, opening dropdowns, and syncing editor control bounds.

### 2. Current event-row implementation files

- `src/ui/PropertyInspector.cpp`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetDefinition.h`
- `src/ui/MainWindow.cpp`

`PropertyInspector::buildRows()` appends widget event rows from `WidgetRegistry` metadata and currently inserts a `"__section_events"` row before them. `MainWindow::setSelectedWidgetPropertyFromString()` still validates event handler names via a hard-coded event-key list.

### 3. How events are currently identified in the inspector model

Events are already identified explicitly through `WidgetDefinition::events` / `WidgetEventDefinition`, not by label text. `PropertyInspector::buildRows()` reads `definition->events` and `callbackChoices(document, event)` to create editable rows and compatible handler suggestions.

### 4. Existing tab control or tabbed-panel implementation to reuse

There is no reusable editor-side tab-strip widget in the current UI shell. Existing `TabControl` logic in `DesignerCanvas`, `WidgetRegistry`, and `ProjectDocument` is for designed widgets inside the form model, not chrome for a docked tool panel. The smallest safe solution is an inspector-local tab strip while reusing the existing row/editor system underneath it.

### 5. Proposed UI hierarchy

- Property Inspector panel
- Header title area
- Fixed tab strip
- Active tab content viewport
- Shared row list and shared scrollbar for the active tab only

### 6. Proposed property/event filtering method

Keep one shared row-generation pipeline, but split the built rows into:

- properties rows: all non-event rows except the legacy `__section_events` section row
- event rows: rows whose keys match the selected widget's `WidgetEventDefinition::key`

Event identification should come from widget definition metadata. Fallback raw properties remain on the Properties tab to avoid hiding unknown persisted fields.

### 7. Scrollbar ownership

`PropertyInspector` should continue to own scrolling, but only for the active tab content viewport. The tab strip must live above `contentBounds()` so it never scrolls away. A single scroll offset for the active tab is sufficient for this change.

### 8. Selection-refresh behavior

- Rebuild filtered rows from the current selected widget on demand, as the inspector already does.
- Preserve the currently selected inspector tab across widget selection changes where practical.
- If no events exist for the selected widget, keep the Events tab visible but empty/non-editable.
- Clear active editing when switching tabs to avoid stale editor anchors and stale widget references.

### 9. Undo/redo and serialization impact

Event rows already edit normal widget properties through `MainWindow::setSelectedWidgetProperty*()` and `DocumentStateCommand`, so moving them to another tab should not change undo/redo or persistence. The only behavior update needed outside `PropertyInspector` is replacing the hard-coded event validation list in `MainWindow` with metadata-aware validation.

### 10. Planned tests

- Add focused automated coverage for widget event metadata and event-property JSON persistence.
- Defer build/manual UI verification to the developer because repository rules do not allow me to run the build pipeline or launch `VisiForm.exe` without explicit instruction.

## Architecture Decision

Implement a local inspector tab strip and shared filtered row pipeline inside `PropertyInspector`. Do not add a second event editor implementation. Do not reuse model-level `TabControl` code for the inspector shell.

## Files Expected To Change

- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/MainWindow.cpp`
- `tests/CMakeLists.txt`
- `tests/test_widget_registry_events.cpp`
- `docs/agent_plans/phase_89_property_inspector_tabs_events_plan.md`
- `docs/VISIFORM_PROJECT_SPEC.md`

## TODO Checklist

- [x] Inspect current inspector, event metadata, scrolling, and edit application paths.
- [x] Record the Phase 89 investigation report and implementation plan.
- [x] Add fixed inspector tabs and route active content through filtered row lists.
- [x] Remove event rows from the Properties tab.
- [x] Show supported widget events on the Events tab using widget metadata.
- [x] Preserve active editor placement, dropdowns, and scroll behavior after tab changes.
- [x] Replace hard-coded event-key validation with metadata-aware validation.
- [x] Add focused automated coverage where practical.
- [x] Update documentation / equivalent project history for the new inspector behavior.
- [x] Record validation status, final result summary, and remaining TODOs.

## Validation Plan

- Automated: run relevant existing tests if an approved path is provided later; otherwise add test coverage and report that execution was deferred.
- Manual:
  - verify Properties and Events tabs appear
  - verify the Properties tab no longer shows the Events section
  - verify Events tab rows change with selection
  - verify event edits persist across tab switches and save/reload
  - verify inspector tab strip remains fixed while content scrolls
  - verify undo/redo still works for event edits

## Compatibility Considerations

- No schema change is planned.
- Event keys remain stored as normal widget properties.
- Unknown persisted properties remain on the Properties tab.
- Multi-selection behavior remains unchanged because `PropertyInspector::buildRows()` still returns empty rows when there is no single selected widget.

## Build / Test Status

- Build not run. Repository instructions require the developer to explicitly request the exact build command/path.
- UI manual verification not run. Repository instructions forbid launching `VisiForm.exe`.
- Automated tests added but not run in this pass.

## Final Result Summary

Implemented a tabbed Property Inspector with fixed `Properties` and `Events` tabs. Event rows now come from existing widget event metadata and render only on the Events tab, while non-event properties remain on the Properties tab. Inspector scrolling stays inside the active tab content area so the tab strip remains visible. `MainWindow` now validates event handler edits through widget metadata rather than a hard-coded event-key list. Focused tests were added for widget event metadata/defaults and JSON round-trip persistence of event handler properties. Phase 89 is not yet ready to be marked complete because the required build and manual UI verification steps have not been run in this pass.

## Remaining TODOs

- Run the approved Windows build/test workflow if the developer wants command-based validation.
- Manually verify the UI checklist in `VisiForm.exe`, especially tab switching, focus behavior, resizing, and scrollbar feel.
