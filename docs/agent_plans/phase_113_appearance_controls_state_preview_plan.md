# Phase 113 Appearance Controls and State Preview Plan

## Scope

- Restore slider-backed normal Appearance editing for Corner Radius and Border Thickness.
- Add a compact, editor-only preview of the selected Appearance state on the selected widget.
- Preserve Phase 111 normal overrides, Phase 112 state overrides, Preview Mode, serialization, generated output, undo/redo, and dirty-state behavior.
- Update authoritative version declarations from `1.0.18` to `1.0.19`.

## Requirements

- Use Phase 113 and version `1.0.19`.
- Use no subagents.
- Restore shared slider metadata rather than adding Button-specific rows.
- Keep Corner Radius and Border Thickness normal-only; state Appearance continues to inherit metrics.
- Preview only states supported by the selected widget.
- Keep selection outlines, resize handles, and editor overlays visible in Design Mode.
- Never persist temporary state preview or route it through project commands.
- Clear temporary preview on selection/project changes, deletion, Preview Mode entry, and inspector reset/closure paths.
- Do not launch VisiForm or generated applications from an automated agent.

## Version Notes

- Previous Phase 112 version: `1.0.18`.
- Phase 113 version: `1.0.19`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, `docs/versioning.md`, and the current-version line in `README.md`.

## Root Cause

- Phase 112 rebuilt Appearance property rows directly from override keys.
- Color keys retained their `Color` editor, but every metric key was assigned generic `Float`.
- That path dropped the existing slider editor kind and range/step metadata even though the Property Inspector slider renderer and interaction implementation remained intact.
- Phase 113 centralizes Appearance metric editor metadata and uses it when constructing normal-state rows.

## Architecture Decisions

- Represent Appearance metric editor kind, range, and step as shared metadata used by the existing Appearance row builder.
- Preserve zero as valid for both metrics, matching resolver and setter semantics.
- Keep state-specific row keys color-only so geometry metrics remain inherited from Normal.
- Store temporary preview state only in editor UI state, associated with the currently selected widget.
- Pass the editor-only state intent into Designer Canvas rendering for the selected widget while remaining in Design Mode.
- Reuse the existing shared runtime-style resolver; do not duplicate Preview Mode interaction state.

## Slider Compatibility

- Show metric rows only where `LookAndFeelRegistry::supportsWidgetOverride()` confirms the property has a visual effect.
- Corner Radius and Border Thickness use their existing normal widget override path.
- Historical property metadata used `1-25` for both sliders.
- Existing resolver semantics make zero meaningful, so both restored sliders use `0-25`.
- Slider edits remain clamped, immediately applied, and no-op safe through the existing inspector and document-command paths.

## Temporary Preview Rules

- Appearance State chooses which sparse override data is edited.
- Preview State toggles temporary rendering of that selected state.
- Changing Appearance State updates an enabled preview immediately.
- Preview state does not mutate model values, dirty state, undo history, serialized JSON, generated output, enabled state, checked values, selected indexes, slider values, or events.
- Preview clears on selection change, project replacement/change, selected-widget deletion, Preview Mode entry, and inspector reset/closure.

## TODO Checklist

- [x] Inspect Git state, recent history, Phase 113 prompt, project status, Phase 112 plan, and targeted files.
- [x] Confirm Phase 113 is unused and select version `1.0.19`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.19`.
- [x] Identify and record the missing-slider root cause.
- [x] Restore shared slider editor metadata and compatibility behavior.
- [x] Add editor-only Appearance state preview UI and state ownership.
- [x] Integrate selected-widget Design Mode rendering and cleanup rules.
- [x] Add focused tests or static checks for compatibility and non-persistence.
- [x] Update final project status and this plan.
- [ ] Run approved validation once, if an exact approved path is available.

## Validation Plan

- Confirm normal Appearance rows use sliders with visible values and existing drag behavior.
- Confirm ranges match model resolver and setter semantics and safely clamp typed values.
- Confirm state Appearance rows remain color-only.
- Confirm supported-state choices come from the central compatibility mapping.
- Confirm Design Mode preview affects only the selected widget and leaves editor decorations visible.
- Confirm Preview Mode clears editor-only state and retains its existing interaction state machine.
- Confirm no model field, serializer, generator, dirty flag, or undo command stores temporary preview state.
- Run `git diff --check` and focused source searches.
- Run the normal Windows Debug build only through an exact developer-authorized command or approved Visual Studio workspace pipeline.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains the uncommitted Phase 112 implementation and session-instruction archival changes; all are preserved.
- Focused static validation: passed. `git diff --check` reported only the
  repository's existing LF-to-CRLF normalization warnings. Targeted searches
  confirmed registry-backed `0-25` slider metadata, editor-only preview
  ownership, selected-widget Design Mode routing, cleanup calls, and the
  absence of preview-state fields in model, serialization, and generator code.
- Focused tests: added registry metadata coverage for both geometry sliders.
  Tests were not executed because no approved exact build/test command was supplied.
- Windows Debug build: not run.
- Manual runtime validation: not performed.

## Compatibility Considerations

- Existing `.vfb.json` schema remains unchanged.
- Generated output remains unchanged except for actual committed Appearance override values.
- Normal and state override inheritance and reset behavior remain unchanged.
- Temporary preview is editor-only and cannot survive a project or selection transition.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_113_appearance_controls_state_preview_plan.md`
- `docs/look_and_feel.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/model/LookAndFeelRegistry.cpp`
- `src/model/WidgetRegistry.cpp`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `src/ui/PropertyInspector.cpp`
- `src/ui/PropertyInspector.h`
- `tests/test_widget_palette_registry.cpp`

## Final Result Summary

- Updated the phase version from `1.0.18` to `1.0.19`.
- Restored Corner Radius and Border Thickness Appearance sliders by retaining
  the shared widget-registry editor kind, range, and step metadata while
  constructing normal Appearance rows.
- Preserved the historical upper bound of `25` and expanded the lower bound to
  `0` because existing rendering semantics use zero for square corners or no
  border.
- Kept state Appearance geometry inherited from Normal; non-normal state rows
  remain color-only.
- Added a compact `Preview State` Boolean control beside the state selector.
- Routed temporary preview through the existing visual-state and Look and Feel
  resolver only for the selected widget in Design Mode, leaving selection and
  resize overlays visible.
- Kept temporary preview out of the project model, serializer, generator,
  dirty flag, and undo stack.
- Cleared temporary preview on selection/multi-selection changes, project load
  or replacement, recovery restore, Appearance reset, Events-tab entry, undo,
  redo, and full Preview Mode entry.
- Grouped live Appearance slider changes into one completed no-op-safe undoable
  document command.

## Remaining TODOs

- Run the normal Windows Debug build through the approved Visual Studio
  workspace pipeline or an exact developer-authorized command.
- Run the focused Catch2 tests.
- Manually validate the Phase 113 slider, repaint, undo/redo, compatibility,
  state-preview, cleanup, dirty-state, save/reload, and Preview Mode checklist.
