# Phase 92 Reusable Splitter Widget Plan

## Objective

Implement a genuine reusable VisiForm splitter widget in `src/ui/` and use it between the Designer Canvas and Property Inspector so the inspector width can be adjusted interactively without changing the main window size.

## Scope

- Add a reusable two-pane splitter control for the editor UI layer.
- Support vertical and horizontal splitter orientations in the reusable control.
- Integrate one vertical splitter instance into the main editor shell between the Designer Canvas and Property Inspector.
- Preserve splitter position during the current session and across restarts using the existing `AppSettings` store if practical.
- Document the reusable-widget-first editor rule in persistent documentation.
- Defer full designer/palette/model/serialization/generator exposure of Splitter as a form-builder widget unless later investigation proves it is small and low-risk.

## Requirements

- The reusable splitter must be editor-independent and must not depend on `MainWindow`, `DesignerCanvas`, `PropertyInspector`, `WidgetPalette`, or `ProjectTree`.
- It must manage two panes, a draggable separator, live resizing, minimum pane sizes, clamped split positions, and independent state per instance.
- It must provide a visible divider plus a larger hit area.
- It must support mouse capture semantics as far as the current Visage window event model allows.
- It must keep the separator usable at minimum and maximum positions.
- It must preserve a valid layout when the main window is resized.
- It must integrate without regressing the Widget Palette, Project Tree, Designer Canvas, or Property Inspector.

## Architectural Findings

### Current editor-shell layout

- `MainWindow::calculateLayout(...)` currently hard-codes a three-column shell:
  - fixed-width left column for `WidgetPalette` and optional `ProjectTree`
  - flexible middle column for `DesignerCanvas`
  - fixed-width right column for `PropertyInspector`
- The current inspector width constants are `kRightPanelWidth = 430.0f` and `kRightPanelMinimumWidth = 386.0f`.
- `MainWindow::applyLayout(...)` pushes the calculated rectangles into `WidgetPalette`, `DesignerCanvas`, `PropertyInspector`, and `ProjectTree`.
- There is no reusable splitter, sash, divider, or two-pane layout control in the repository today.

### Reusable UI patterns already present

- `WidgetPalette`, `ProjectTree`, and `PropertyInspector` are self-contained reusable UI-region classes that own:
  - bounds
  - hit testing
  - scroll or drag state
  - drawing
  - mouse down/drag/up logic
- `PropertyInspector` already demonstrates reusable interaction patterns relevant to a splitter:
  - per-instance transient interaction state
  - clamped geometry calculations
  - row-local hit testing
  - drag lifecycle with `mouseDown`, `mouseDrag`, and `mouseUp`
- `MainWindow` centrally dispatches mouse events to child UI regions and is the right place to compose a new reusable splitter with existing panels.

### Cursor and capture constraints

- Repository search found no existing generalized resize-cursor API usage such as `setCursor`, `setMouseCursor`, or similar calls.
- Repository search also found no explicit platform-level pointer-capture helper outside the normal Visage window drag event flow.
- Current UI controls rely on `mouseDown` -> `mouseDrag` -> `mouseUp` event routing through `MainWindow`.
- Phase 92 should therefore implement correct drag-state ownership and cleanup within the current Visage event model, and treat any missing platform cursor/capture hook as an integration detail to verify manually.

### Settings and persistence findings

- `utils::AppSettings` already persists editor-level state in `%APPDATA%/VisiForm/settings.json`.
- Existing persisted editor settings include recent files, export dependency values, grid visibility, snap, smart guides, grid sizes, and keyboard shortcuts.
- Adding a persisted splitter width to `AppSettings` is aligned with current architecture and is preferable to inventing a new settings subsystem.

### Designer integration findings

- Full designer-facing Splitter exposure would require touching all major layers:
  - `WidgetType` / `WidgetRegistry`
  - property metadata
  - designer preview
  - validation
  - JSON round-trip expectations
  - generated C++ output
  - tests
  - widget documentation
- Existing widget work in this repository shows that adding a new designer widget is cross-layer and broad by default.
- Phase 92's immediate problem is editor-shell resizing, not end-user form design.
- Conclusion: full palette/model/serialization/generator integration is too broad for Phase 92 and should be deferred to a follow-up phase after the editor-shell widget is proven.

## Editor UI Audit

| Area | Current classification | Notes |
| --- | --- | --- |
| Widget Palette | Reusable VisiForm widget | `src/ui/WidgetPalette.*` is a standalone reusable panel. |
| Project Tree | Reusable VisiForm widget | `src/ui/ProjectTree.*` is a standalone reusable panel. |
| Designer Canvas | Reusable VisiForm widget with editor-specific behavior | `src/ui/DesignerCanvas.*` is reusable within the editor but deeply designer-specific. |
| Property Inspector | Reusable VisiForm widget with editor-specific behavior | `src/ui/PropertyInspector.*` is a reusable panel class specialized for project/widget editing. |
| Properties / Events tabs | Reusable widget behavior inside `PropertyInspector` | Implemented as reusable tabbed inspector behavior, not shell-specific one-off code. |
| Toolbar | Custom editor-only component | Drawn directly by `MainWindow`. |
| Menus | Custom editor-only component | Drawn and hit-tested directly by `MainWindow`. |
| Status bar | Custom editor-only component | Drawn directly by `MainWindow`. |
| Designer Canvas / Property Inspector boundary | Manual one-off layout region | Currently a fixed right column in `MainWindow::calculateLayout(...)`; best Phase 92 insertion point for reusable splitter dogfooding. |

## Approved Architecture

### Recommended class

- `visiform::ui::Splitter`

This matches current repository naming like `WidgetPalette`, `ProjectTree`, and `PropertyInspector` while staying concise.

### Ownership model

- `Splitter` lives in `src/ui/`.
- `MainWindow` owns the editor-shell splitter instance.
- `Splitter` does not own child widgets or panes.
- Instead, `Splitter` computes pane rectangles for two externally owned panes.

### Public API

Proposed API shape:

- `setBounds(float x, float y, float width, float height)`
- `setOrientation(Orientation orientation)`
- `setMinimumFirstPaneSize(float size)`
- `setMinimumSecondPaneSize(float size)`
- `setDividerThickness(float size)`
- `setHitThickness(float size)`
- `setSplitPosition(float size)`
- `splitPosition() const`
- `setDraggingEnabled(bool enabled)` if needed
- `firstPaneBounds() const`
- `secondPaneBounds() const`
- `dividerBounds() const`
- `hitBounds() const`
- `contains(float x, float y) const`
- `isDragging() const`
- `mouseDown(float x, float y)`
- `mouseDrag(float x, float y)`
- `mouseUp()`
- `draw(visage::Canvas&) const`

### Orientation handling

- Support `Orientation::Vertical` and `Orientation::Horizontal`.
- For Phase 92 editor integration, use `Vertical` only:
  - first pane = Designer Canvas
  - second pane = Property Inspector
  - divider = vertical bar on the inspector's left edge

### Split-position representation

- Represent split position as first-pane size in pixels from the leading edge.
- Clamp against:
  - minimum first-pane size
  - minimum second-pane size
  - current splitter bounds
- For editor persistence, store the derived Property Inspector width in `AppSettings`, then convert back to a clamped first-pane position during layout.

### Child-pane ownership and layout

- `Splitter` returns computed pane rectangles; it does not call `setBounds(...)` on children itself.
- `MainWindow::applyLayout(...)` remains the composition point:
  - apply splitter bounds
  - fetch splitter first/second pane bounds
  - assign those bounds to `DesignerCanvas` and `PropertyInspector`

### Divider thickness and hit area

- Keep a visually slim divider line.
- Use a wider invisible hit area around it for reliable dragging.
- Minimum target is to make the draggable region easier to acquire than the visible stroke alone.

### Dragging behavior

- Clicking the divider hit area begins dragging.
- Dragging updates split position live.
- `mouseUp()` ends dragging cleanly.
- If layout changes invalidate the drag bounds, clamp safely and clear drag state when needed.
- State is instance-local so multiple splitter instances can remain independent.

### Resize cursor behavior

- The reusable splitter should expose whether the pointer is over a draggable divider so `MainWindow` can set the appropriate cursor if a Visage cursor API is available.
- If an API cannot be confirmed from the repository, Phase 92 may need to defer actual cursor switching while still implementing the hover detection and documenting the gap.

### Persistence integration

- Extend `AppSettings` with a persisted editor-shell inspector width field.
- Load it during app startup along with existing settings.
- Use the stored width as the initial second-pane width when constructing the main layout.
- Clamp restored values against current window size and configured pane minimums.
- Save updated width after splitter moves.

## Exact Phase 92 Implementation Scope

Included:

- Add `src/ui/Splitter.h` and `src/ui/Splitter.cpp`.
- Integrate one vertical splitter into `MainWindow`.
- Replace fixed right-column layout math with splitter-driven center/right pane sizing.
- Persist inspector width through `AppSettings`.
- Add focused automated tests for splitter layout/clamping math where practical.
- Update persistent documentation for:
  - project spec
  - settings persistence, if new settings are added
  - phase plan

Deferred:

- Widget Palette exposure for Splitter
- `WidgetType` / `WidgetRegistry` registration
- designer-canvas authored Splitter nodes
- `.vfb.json` serialization for Splitter widgets
- validation rules for Splitter widgets
- generated C++ runtime emission for Splitter widgets
- widget-catalog documentation as a designable widget

## Expected Files To Change

Core implementation:

- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/Splitter.h`
- `src/ui/Splitter.cpp`
- `src/utils/AppSettings.h`
- `src/utils/AppSettings.cpp`

Tests:

- `tests/test_splitter.cpp`
- `tests/CMakeLists.txt`

Documentation:

- `docs/agent_plans/phase_92_reusable_splitter_widget_plan.md`
- `docs/VISIFORM_PROJECT_SPEC.md`
- `docs/settings.md`

Intentionally not changed in Phase 92:

- `src/model/*`
- `src/serialization/*`
- `src/validation/*`
- `src/generator/*`
- `src/ui/DesignerCanvas.*`
- `src/ui/PropertyInspector.*`
- `src/ui/WidgetPalette.*`
- `src/ui/ProjectTree.*`

## Delegated Assignments

- Intended multi-agent sequence from the Phase 92 session instruction could not be executed in this session.
- Repository note: custom VisiForm subagents currently do not spawn reliably under GPT-5.5 in the Windows Codex desktop app.
- This phase is therefore being executed by the lead agent directly, with investigation, architecture, implementation, documentation, and review consolidated in one session.

## Validation Plan

- Static/source validation:
  - inspect splitter bounds and clamp math
  - inspect `MainWindow` event routing and layout integration
  - inspect `AppSettings` save/load wiring
- Automated validation:
  - add focused unit tests for splitter geometry and clamping if practical without launching the app
- Manual validation by developer:
  - launch `VisiForm.exe` manually
  - verify divider hover, dragging, min widths, live resize, main-window resize handling, inspector readability, and no regressions in Widget Palette / Project Tree / Events tab
- Build validation:
  - must use the supported Visual Studio 2022 `VisiForm` target path
  - agent will not run build commands unless the developer explicitly asks for the exact command

## Compatibility Considerations

- No `.vfb.json` schema change in Phase 92.
- No generated C++ behavior change in Phase 92.
- Existing editor-shell layout remains visually similar except for the new resizable divider.
- Persisted splitter width must tolerate older settings files by falling back to the current default inspector width.

## TODO Checklist

- [x] Inspect repository status before changes.
- [x] Read `docs/VISIFORM_PROJECT_SPEC.md`.
- [x] Read the active Phase 91 plan for current editor-shell context.
- [x] Inspect repository history and confirm Phase 92 is the next unused phase number.
- [x] Investigate existing reusable UI widgets, resizers, divider patterns, and mouse interaction patterns.
- [x] Investigate editor-shell layout and current Designer Canvas / Property Inspector sizing logic.
- [x] Investigate existing settings persistence for possible splitter-state storage.
- [x] Decide whether full designer-facing Splitter integration is safe in Phase 92.
- [x] Record architectural findings and approved splitter architecture in this plan before implementation.
- [x] Record the reusable-widget-first editor rule in this plan.
- [x] Implement `src/ui/Splitter.*`.
- [x] Integrate the splitter into `MainWindow`.
- [x] Persist inspector width through `AppSettings`.
- [x] Add or update focused automated tests.
- [x] Update persistent docs for the editor reusable-widget rule and settings persistence.
- [x] Perform source review of the integrated change set.
- [x] Record files changed.
- [ ] Record build-validation status.
- [ ] Record manual-test status.
- [x] Add final implementation summary.
- [x] Summarize deferred work and remaining TODOs.

## Editor UI Architecture Rule

When modifying a VisiForm editor UI region, prefer using or improving a reusable VisiForm widget rather than adding new one-off editor-only layout code.

Phase 92 applies this rule specifically by replacing the hard-coded Designer Canvas / Property Inspector boundary with a reusable splitter widget instead of adding editor-only drag math directly inside `MainWindow`.

## Build / Test Status

- Current branch: `main`
- Most recent relevant commit before Phase 92 work: `70dfd6c Add compact event rows to \`Events\` tab workflow`
- Build not run in this session because repository rules prohibit build commands unless the developer explicitly asks for the exact command.
- Automated tests not run yet for the same reason, even though `tests/test_splitter.cpp` and `tests/CMakeLists.txt` were updated for focused splitter coverage.
- Manual app validation not run because agents must not launch `VisiForm.exe`.
- Static validation performed:
  - reviewed the integrated diff for `MainWindow`, `Splitter`, `AppSettings`, test wiring, and docs
  - ran `git diff --check` on touched files with no whitespace errors; Git reported only expected CRLF normalization warnings

## Files Changed

- Phase plan created:
  - `docs/agent_plans/phase_92_reusable_splitter_widget_plan.md`
- Core implementation:
  - `src/ui/Splitter.h`
  - `src/ui/Splitter.cpp`
  - `src/ui/MainWindow.h`
  - `src/ui/MainWindow.cpp`
  - `src/utils/AppSettings.h`
  - `src/utils/AppSettings.cpp`
  - `CMakeLists.txt`
- Tests:
  - `tests/test_splitter.cpp`
  - `tests/CMakeLists.txt`
- Documentation:
  - `docs/VISIFORM_PROJECT_SPEC.md`
  - `docs/settings.md`

## Final Implementation Summary

Implemented a reusable `src/ui/Splitter.*` control that supports vertical and horizontal orientation, two computed pane regions, a visible divider with a wider hit area, live dragging, minimum-pane clamping, and per-instance drag state. Integrated one vertical splitter into `MainWindow` so the Designer Canvas and Property Inspector now size through the reusable splitter instead of a hard-coded fixed-width right column. Added editor-session persistence for the inspector width through `AppSettings`, updated test wiring with focused splitter geometry/drag tests, and updated persistent documentation to record both the reusable-widget-first editor rule and the new stored layout state.

## Deferred Work And Remaining TODOs

- Full designer-facing Splitter widget registration and generation are intentionally deferred out of Phase 92.
- Build, automated test execution, and manual UI validation remain pending until an approved/manual path is available.
- Manual validation still needs to confirm:
  - hover cursor changes over the divider
  - live drag behavior
  - minimum-width clamping at both extremes
  - main-window resize behavior
  - Widget Palette / Project Tree / Events-tab regression safety
