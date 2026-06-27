# Phase 118 Callback Corner Menu and Status Geometry Plan

## Scope

- Add a dedicated status-bar geometry segment for the single selected visual widget in Design Mode.
- Reuse the selected widget's upper-right resize handle as a click affordance for an assigned-callback popup while preserving upper-right resizing after drag threshold movement.
- Navigate popup callback actions to the existing Property Inspector Events tab and event row.
- Update authoritative version declarations from `1.0.23` to `1.0.24`.
- Leave Phase 117 Preview Mode widget interaction tabled and avoid Preview Mode interaction changes.

## Requirements

- Use Phase 118 and version `1.0.24`.
- Use no subagents.
- Do not execute callbacks, launch generated code, create handlers automatically, or add a new event-assignment system.
- Do not modify Preview Mode interaction behavior beyond hiding this editor-only affordance while Preview Mode is active.
- Do not serialize status or popup state and do not mark the project dirty for popup use.

## Version Notes

- Previous Phase 117 version: `1.0.23`.
- Phase 118 version: `1.0.24`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, `docs/versioning.md`, and the current-version line in `README.md`.

## Architecture Decisions

- Status geometry remains `MainWindow` presentation state derived directly from the current selected widget bounds.
- Callback discovery uses `WidgetRegistry` event metadata and the selected widget's existing event properties.
- The callback popup is editor-owned `MainWindow` UI state keyed by widget ID, not retained widget pointers.
- Upper-right handle presses enter a pending callback-click state. Movement beyond the existing `kMarqueeDragThreshold` converts the gesture into the normal resize path.
- The Events tab navigation is a focused `PropertyInspector` helper that switches tabs, scrolls the target row into view, and highlights the existing handler selector for that event.

## TODO Checklist

- [x] Inspect Git state, project status, Phase 117 plan, and Phase 118 instructions.
- [x] Create this persistent Phase 118 plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.24`.
- [x] Inspect targeted selection handle, resize hit-testing, event metadata, Events tab, popup/dropdown, status bar, and version paths.
- [x] Add status-bar geometry segment.
- [x] Add callback marker to the selected upper-right handle.
- [x] Add callback popup ownership, drawing, dismissal, and stable-ID cleanup.
- [x] Add click-versus-drag handoff for upper-right handle.
- [x] Add Events-tab row navigation from popup actions.
- [x] Run focused static validation.
- [x] Record build/manual validation status.

## Validation Plan

- Run focused static checks such as `git diff --check`.
- Do not run terminal build commands unless the developer supplies the exact approved command.
- Do not launch `VisiForm.exe` from automation.
- Manual runtime validation remains developer-run unless an approved non-automated workflow is available: version display, geometry status updates, upper-right click popup, callback row navigation, upper-right drag resize, empty state, dismissal, multi-selection, Preview Mode hiding, zoom/pan, and dirty-state preservation.

## Compatibility Considerations

- `.vfb.json` schema is unchanged.
- Popup and status geometry state are editor-only and not serialized, generated, exported, or stored in undo history.
- Existing Events tab editing remains the canonical callback assignment UI.

## Build / Test Status

- Branch: `main`.
- Starting worktree contained Phase 117 implementation changes and session-instruction archival changes; they are preserved.
- Focused static validation: `git diff --check` passed; output contained only existing LF-to-CRLF working-copy normalization warnings.
- Windows Debug build: not run because repository instructions prohibit terminal build commands unless the developer asks for that exact command, and no unambiguous Visual Studio workspace pipeline was available to the agent.
- Manual runtime validation: not performed; automated agents must not launch `VisiForm.exe`.

## Files Inspected

- `AGENTS.md`
- `docs/project_status.md`
- `docs/agent_plans/phase_117_preview_mode_widget_interaction_plan.md`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.h`
- `src/ui/PropertyInspector.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetRegistry.cpp`
- `src/model/WidgetNode.h`
- `src/model/WidgetNode.cpp`
- `CMakeLists.txt`
- `src/app/Version.h`
- `docs/versioning.md`

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_118_callback_corner_menu_plan.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/PropertyInspector.h`

## Final Result Summary

- Updated the phase version from `1.0.23` to `1.0.24`.
- Added a dedicated status-bar geometry field that shows `X`, `Y`, `W`, and `H` from the current selected widget's model-space bounds only when exactly one non-root visual widget is selected in Design Mode. The value is derived at draw time, so drag, resize, inspector edits, undo/redo, layout commands, docking/layout updates, and selection changes do not store or serialize any status data.
- Added a small upper-right handle indicator. It uses a subdued dot when no callbacks are assigned and a brighter accent dot when existing widget event metadata has one or more assigned handler names.
- Added an editor-only callback popup owned by `MainWindow` and keyed by stable widget ID. The popup lists only assigned callbacks as `eventKey -> handlerName`, shows `No callbacks assigned` when empty, and includes `Open Events Inspector`.
- Callback discovery uses `WidgetRegistry::find(widget.type)->events` and each event's existing widget property value; no widget-type hardcoding or callback execution was added.
- Clicking an assigned callback closes the popup, keeps the widget selected, switches the Property Inspector to Events, scrolls/reveals the matching event row, and highlights the existing-handler selector where the current inspector API supports it.
- Upper-right handle mouse input now starts in a pending callback-click state. Releasing on the same upper-right handle below the `kMarqueeDragThreshold` opens the popup; dragging beyond that threshold converts to the normal upper-right resize path, including sizer-item and layout resize snapshots.
- The popup closes on outside click, Escape, selection changes, command dispatch, undo/redo, Preview Mode entry, zoom, pan, and document-changing helper paths. Popup use does not mark the document dirty or create undo history.

## Remaining TODOs

- Run the normal Windows Debug build through the approved Visual Studio workspace pipeline or an exact developer-authorized command.
- Manually validate the Phase 118 checklist from the session instructions, including version display, geometry updates, callback popup contents, Events-tab navigation, click-versus-resize behavior, empty state, dismissal, zoom/pan, multi-selection, Preview Mode hiding, dirty-state preservation, and unchanged callback assignments.
- Future work: richer callback actions such as source navigation, handler creation, or generated-code integration remain out of scope for this phase.
