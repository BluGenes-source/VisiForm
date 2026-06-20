# Phase 112 Widget State Appearance Overrides Plan

## Scope

- Add sparse per-widget appearance overrides for meaningful runtime states.
- Preserve the resolution order `preset -> project overrides -> normal widget overrides -> widget state overrides`.
- Extend the existing Property Inspector Appearance section with one state selector.
- Preserve Preview Mode transient state, Design Mode editor overlays, serialization, copy/duplicate, undo/redo, autosave/recovery, and generated-output behavior.
- Update authoritative version declarations from `1.0.17` to `1.0.18`.

## Requirements

- Use Phase 112 and version `1.0.18`.
- Use no subagents.
- Extend the existing Look and Feel system; do not add selectors, style classes, animations, transitions, fonts, gradients, image skins, or state-specific geometry.
- Support only control surface, text, border, accent, highlight edge, shadow edge, and focus outline colors.
- Support Button, Text Box, Check Box, Radio Button, Combo Box, List Box, Slider, Scroll Bar, Progress Bar, Color Picker, and Tab Control.
- Hide or disable states that are not meaningful for the selected widget.
- Preserve generated `MainWindow`, USER CODE regions, CMake presets, vcpkg settings, and static MSVC runtime settings.
- Do not launch `VisiForm.exe` or generated applications from an automated agent.

## Version Notes

- Previous Phase 111 version: `1.0.17`.
- Phase 112 version: `1.0.18`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, `docs/versioning.md`, and the current-version line in `README.md`.

## Architecture Decisions

- Add one shared appearance-state identifier model rather than widget-specific state concepts.
- Store sparse per-state color overrides on `WidgetNode`, keyed by the shared state identifier.
- Keep normal widget overrides in the existing Phase 111 structure.
- Extend `LookAndFeelRegistry` as the only merge path for preset, project, normal widget, and state-specific overrides.
- Retain Phase 107 state priority: disabled first, then pressed, then checked/selected/active, then hover, then normal; focus remains an additional overlay where appropriate.
- Define one central compatibility mapping for widget types, supported states, and properties.
- Rely on `WidgetNode` value copying and document serialization so duplicate, clipboard, undo/redo, autosave, and recovery preserve state overrides without special transfer code.

## Supported States

- Normal
- Hover
- Pressed
- Focused
- Checked or Selected
- Disabled

## Supported State Override Properties

- control surface color
- text color
- border color
- accent color
- highlight edge color
- shadow edge color
- focus outline color

Metrics and geometry continue to inherit from the resolved normal widget style.

## Supported Widget Types

- Button
- Text Box
- Check Box
- Radio Button
- Combo Box
- List Box
- Slider
- Scroll Bar
- Progress Bar
- Color Picker
- Tab Control

## Property Inspector Behavior

- Add an `Appearance State` selector to the existing Appearance section.
- `Normal` edits the existing Phase 111 widget overrides.
- Other entries edit sparse overrides for the selected runtime state.
- Show inherited resolved values when an explicit override is absent.
- Hide or disable unsupported states and properties.
- Keep multi-selection Appearance editing disabled.

## Reset And Undo Behavior

- Reset Property removes only the selected normal or state property override.
- Reset State removes all overrides for the selected non-normal state.
- Reset All Appearance removes normal and state overrides for the selected widget.
- No-op edits and resets create no undo entry.
- Each committed edit or reset uses one existing `DocumentStateCommand`.
- Selection remains stable through undo and redo.

## Serialization Format

- Extend the optional per-widget `appearanceOverrides` object with a sparse state map.
- Omit empty state maps and empty state objects.
- Store only explicit supported color values.
- Existing projects without state overrides load with empty state data.
- Missing and unknown states inherit safely; temporary Preview Mode state is never serialized.

## Design, Preview, And Export Integration

- Design Mode renders normal appearance and keeps editor selection/focus overlays separate.
- Preview Mode supplies transient hover, pressed, focus, checked/selected, and disabled state to the shared resolver without modifying the model.
- Leaving Preview Mode continues to clear transient interaction state.
- Generated output emits explicit state overrides and resolves them with the same priority and inheritance intent without accessing the local preset library.

## Compatibility Considerations

- Old `.vfb.json` files remain valid.
- Widgets without state overrides serialize and generate as before.
- Unsupported widget states are not exposed as misleading editor choices.
- State-specific metrics, geometry, fonts, animation, transitions, style classes, selectors, and bulk multi-widget editing remain out of scope.

## TODO Checklist

- [x] Inspect Git state, the Phase 112 prompt, project status, Phase 111 plan, targeted appearance/state paths, and version declarations.
- [x] Confirm Phase 112 is unused and select version `1.0.18`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.18`.
- [x] Add the shared appearance-state model and compatibility mapping.
- [x] Extend the shared resolver with sparse state-last precedence.
- [x] Add sparse state serialization and backward-compatible loading.
- [x] Extend the existing Appearance inspector with state selection and resets.
- [x] Confirm duplicate/copy/paste, undo/redo, autosave, and recovery preserve state overrides.
- [x] Integrate Preview Mode and generated runtime state resolution.
- [x] Add focused model, serialization, resolution, and generated-output tests.
- [x] Update relevant Look and Feel/file-format/code-generation documentation and final project status.
- [x] Run focused static validation.
- [x] Run the approved normal Windows Debug build once.
- [x] Record final validation, files changed, limitations, and remaining issues.

## Validation Plan

- Add focused tests for sparse round-trip, empty omission, state-last precedence, invalid-value fallback, value-copy preservation, and generated runtime values.
- Confirm unsupported state choices are not exposed by the compatibility mapping.
- Confirm reset operations are no-op safe and use one undoable document state.
- Confirm Design Mode remains normal-only and Preview Mode transient state is not serialized.
- Confirm generated output preserves state priority and explicit sparse values.
- Run `git diff --check` and targeted source searches.
- Build the normal Windows Debug configuration once through an approved path.
- Leave application launch and generated-app launch to the developer.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains the user-owned Phase 111 session-instruction archival move and the untracked Phase 112 prompt; they are preserved.
- Focused static validation: passed. `git diff --check` reported only the
  repository's existing LF-to-CRLF normalization warnings. Targeted searches
  confirmed one shared compatibility map and resolver, sparse state JSON,
  Property Inspector state/reset routing, Preview transient-state routing,
  generated sparse assignments, and balanced implementation braces and
  parentheses.
- Focused tests: added for sparse state serialization, round-trip,
  state-after-normal resolution, focus overlay, value-copy preservation,
  compatibility filtering, and generated sparse state output. They were not
  executed because no approved exact build or test command was supplied.
- Windows Debug build: passed through the approved Visual Studio workspace
  build pipeline after fixing the `std::string_view` to `std::string`
  initialization in `src/ui/MainWindow.cpp`.
- Manual runtime validation: not performed; automated agents may not launch either application.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_112_widget_state_appearance_overrides_plan.md`
- `docs/code_generation.md`
- `docs/look_and_feel.md`
- `docs/project_file_format.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/model/LookAndFeelDefinition.h`
- `src/model/LookAndFeelRegistry.cpp`
- `src/model/LookAndFeelRegistry.h`
- `src/model/WidgetNode.h`
- `src/serialization/JsonProjectReader.cpp`
- `src/serialization/JsonProjectWriter.cpp`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.cpp`
- `src/ui/PropertyInspector.cpp`
- `src/ui/PropertyInspector.h`
- `tests/test_generated_runtime_labels.cpp`
- `tests/test_project_serialization.cpp`

The pre-existing Phase 111 session-instruction archive move and the Phase 112
instruction file were preserved without modification.

## Known Limitations

- State-specific metrics, geometry, fonts, animations, transitions, selectors, style classes, gradients, image skins, and bulk multi-widget editing are excluded.
- Manual runtime verification is required for visual state parity, repaint timing, and generated application behavior.

## Final Result Summary

- Updated the phase version from `1.0.17` to `1.0.18`.
- Added one shared state identifier model and sparse map of seven-color
  overrides owned by each widget.
- Added central compatibility for Button, Text Box, Check Box, Radio Button,
  Combo Box, List Box, Slider, Scroll Bar, Progress Bar, Color Picker, and Tab
  Control. Unsupported states are absent from the inspector.
- Extended the shared resolver so state values apply after preset, project, and
  normal widget values. Disabled remains highest priority, followed by pressed,
  checked/selected, hover, and normal; focus is an additional supported overlay.
- Added an Appearance State selector to the existing Appearance section.
  Normal retains Phase 111 editing, while other states expose color-only sparse
  values with inherited normal values visible.
- Added Reset Property, Reset State, and Reset All Appearance through the
  existing no-op-safe document command path.
- Added optional `appearanceOverrides.states` JSON. Empty maps and states are
  omitted, old projects load normally, unknown states are ignored, and
  Preview-only interaction is not serialized.
- Preserved state data automatically across WidgetNode value copies used by
  duplicate, clipboard, undo/redo, autosave, and recovery.
- Design Mode resolves state Appearance as Normal. Preview Mode resolves
  transient hover, press, focus, checked/selected, active-tab, and disabled
  state without modifying the model and clears it on mode exit.
- Generated runtime output emits only explicit state color assignments and
  applies them through one generated state helper without local preset-library
  access.

## Remaining TODOs

- Run the focused Catch2 tests.
- Manually execute the Phase 112 Property Inspector, Preview Mode, Design Mode,
  reset, undo/redo, save/reload, duplicate/copy-paste, base-style change,
  autosave/recovery, and generated-project validation checklist.
