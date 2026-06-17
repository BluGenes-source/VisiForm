# Phase 93 Property Inspector and Layout Polish Plan

## Objective

Complete the Phase 93 UI polish work around the Widget Palette, Property Inspector event dropdown presentation, startup window sizing, and top-level splitter resize behavior without regressing the Phase 92 reusable splitter workflow.

## Scope

- Ensure `Slider` is exposed reliably through the Widget Palette path and covered by regression checks.
- Improve event assignment row presentation so handler names remain readable and do not collide with the dropdown affordance or adjacent controls.
- Increase the default startup window width from `1200 x 800` to a more practical non-maximized size for the current three-panel editor.
- Refine the top-level canvas/inspector splitter resize policy so the Property Inspector does not absorb nearly all extra width on large windows.
- Preserve existing Phase 92 Properties and Events tab behavior, including splitter dragging and compatible-handler suggestions.

## Explorer Findings

### Slider registration and palette path

- `src/model/WidgetRegistry.cpp` already defines `makeSliderDefinition()` with `setPaletteMetadata(definition, true, 16, "Value/Feedback")`.
- `WidgetRegistry::paletteDefinitions()` already includes palette-visible entries and sorts them by `paletteOrder`.
- `src/ui/WidgetPalette.cpp` renders the palette directly from `WidgetRegistry::instance().paletteDefinitions()`.
- `src/ui/MainWindow.cpp` already supports slider insertion through templates and the Insert menu.
- Current likely root cause is not missing registry metadata; Phase 93 should harden the palette path with explicit regression coverage and confirm no UI-side omission remains.

### Event dropdown presentation

- `src/ui/PropertyInspector.cpp` draws event rows directly.
- The assigned-handler text and dropdown arrow currently share tight manual bounds inside `selectorBounds`.
- The arrow is drawn as a text glyph `"v"` at a fixed inset, while assignment text uses the remaining rectangle with only minimal reserved space.
- This layout is the most likely source of cramped text and inconsistent readability at narrow inspector widths.

### Startup sizing

- `src/ui/MainWindow.cpp` currently opens the app at `1200 x 800` in `MainWindow::showWindow()`.
- There is currently no persisted outer-window bounds logic in `AppSettings`; only editor state such as grid settings, keyboard shortcuts, and `propertyInspectorWidth` is stored.
- Because startup size is a single hard-coded default, Phase 93 can safely widen it without conflicting with restore logic.

### Splitter resize policy

- Phase 92 stores only `settings_.propertyInspectorWidth`.
- `MainWindow::applyLayout(...)` reconstructs the split from the stored right-pane width each time using `layout_.canvasInspectorRegion.width - divider - desiredInspectorWidth`.
- On wider windows, this policy keeps the inspector at least its stored width and pushes nearly all additional space decisions through fixed minimums instead of proportional growth.
- The current layout also uses a fixed left palette width of `220`.
- Phase 93 should adjust the splitter policy toward a bounded preferred inspector width and intentional center-panel growth, instead of preserving the inspector as the dominant resizing target.

## Affected Files And Systems

- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h` if new helper state or layout helpers are needed
- `src/ui/PropertyInspector.cpp`
- `src/ui/PropertyInspector.h` if new helper methods or constants are needed
- `src/model/WidgetRegistry.cpp` only if explorer findings later prove a palette metadata correction is still needed
- `tests/*` for focused regression coverage around widget registry/palette exposure or layout policy helpers
- `docs/project_status.md`
- `docs/agent_plans/phase_93_property_inspector_layout_polish_plan.md`

## Implementation Approach

1. Confirm the slider palette path end to end and add a focused automated regression so `Slider` remains palette-visible and ordered with the rest of the registry-backed palette.
2. Refactor the event selector drawing bounds in `PropertyInspector` so the assigned handler text, arrow region, and buttons each have stable reserved space and clean clipping behavior.
3. Increase the startup default from `1200 x 800` to a wider practical size while keeping the same non-maximized launch behavior.
4. Replace the pure persisted-width splitter reconstruction with a more balanced policy that favors center-canvas growth while keeping the inspector useful and draggable across normal and maximized widths.
5. Update documentation with:
   - old and new startup dimensions
   - final splitter sizing policy
   - validation limits in this session

## Architectural Decisions

- Keep the Phase 92 reusable `Splitter` widget as the mechanism for panel division; Phase 93 should change policy, not replace the control.
- Prefer reusable bounds/layout helpers in `PropertyInspector` instead of one-off magic numbers in the draw call body.
- Avoid schema, generator, or serialization changes unless static inspection proves they are required for slider palette exposure, which currently appears unlikely.
- Because custom VisiForm subagents are currently unreliable in this Codex environment, this phase is being executed by the lead agent directly with exploration, implementation, review, and documentation consolidated in one session.

## Risks

- Narrow-width inspector layouts could regress if the event-row spacing changes are not clamped carefully.
- Splitter-policy changes could make restored inspector widths feel inconsistent if the saved width is not combined carefully with new growth limits.
- Changing startup width without manual runtime verification leaves some uncertainty around small-display fit and perceived balance.
- Any mistaken change to slider palette behavior could affect insertion order or registry consistency assertions.

## Validation Plan

- Static/source validation:
  - inspect slider registry/palette flow
  - inspect event-row bounds and clipping calculations
  - inspect splitter policy and startup sizing changes
- Automated validation where practical:
  - add or update focused unit tests for palette visibility or layout helper behavior
- Manual developer validation still required:
  - build the normal Windows debug `VisiForm` target through the approved Visual Studio workflow
  - launch `VisiForm.exe` manually
  - verify slider palette presence, event dropdown readability, startup size, splitter behavior, save/reload, and export workflow

## Compatibility Considerations

- No `.vfb.json` schema change is planned.
- No generated-code format change is planned.
- Slider export and event handler persistence should remain unchanged.
- Existing saved `propertyInspectorWidth` settings should remain readable; any new layout policy must tolerate old settings values.

## TODO Checklist

- [x] Inspect repository state and confirm current branch/worktree.
- [x] Read repository instructions, project spec, `docs/project_status.md`, and Phase 92 plan.
- [x] Confirm Phase 93 is the next unused phase number and create this plan.
- [x] Summarize explorer findings before implementation.
- [x] Implement slider palette exposure/regression fix if needed.
- [x] Improve event dropdown text layout and clipping.
- [x] Increase default startup width and document old/new dimensions.
- [x] Refine splitter resize policy for wide windows.
- [x] Add or update focused automated tests where practical.
- [ ] Update `docs/project_status.md` for verified Phase 93 behavior.
- [x] Perform source review of touched files.
- [x] Record final validation status.
- [x] Add final result summary.
- [x] Summarize remaining TODOs.

## Build / Validation Status

- Current branch: `main`
- Most recent relevant commit before Phase 93 work: `cc0914a Add reusable Splitter widget for editor layout resizing`
- Build not run in this session because repository rules prohibit build commands unless the developer explicitly asks for the exact command.
- Automated tests not run in this session for the same reason, although a focused palette-regression test file was added.
- Manual application validation not run because agents must not launch `VisiForm.exe`.
- Static validation performed:
  - reviewed the focused diff for `MainWindow`, `PropertyInspector`, `DropdownControl`, the new palette regression test, and this plan
  - ran `git diff --check` on touched files; Git reported only CRLF normalization warnings and no whitespace errors

## Final Result Summary

- Slider palette root-cause findings: the registry was already exposing `Slider` through `WidgetRegistry::paletteDefinitions()`, so Phase 93 preserved the implementation and added `tests/test_widget_palette_registry.cpp` to lock in that exposure path and ordering.
- Event dropdown formatting: the assigned-handler selector in `PropertyInspector` now reserves a dedicated arrow area, uses stronger text padding, and separates text from the dropdown affordance; popup dropdown entries also now open with a wider minimum width and slightly larger horizontal padding.
- Startup sizing: `MainWindow::showWindow()` now starts at `1400 x 820` instead of the previous `1200 x 800`.
- Splitter resize policy: the top-level canvas/inspector layout now clamps the restored inspector width through a balanced-width policy that preserves minimum canvas space and caps inspector growth on wide windows, while still using the reusable Phase 92 splitter and persisted width setting.
- Documentation note: both `docs/project_status.md` and `docs/PROJECT_STATUS.md` currently contain stale instruction-prompt content rather than a maintained status snapshot, so the Phase 93 record remains authoritative in this plan file for now.

## Remaining TODOs

- Developer-side validation still needed:
  - build the normal Windows debug `VisiForm` target through the approved Visual Studio workflow
  - manually verify slider palette insertion, selection, save/reload, and export
  - manually verify narrow/medium/wide/maximized event-row readability and splitter behavior
- `docs/project_status.md` remains stale and was not updated as part of this focused UI phase because the file currently contains prompt text rather than a maintained project-status document.
