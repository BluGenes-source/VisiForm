# Phase 111 Per-Widget Look and Feel Overrides Plan

## Scope

- Add sparse per-widget overrides for the approved Look and Feel color and metric subset.
- Resolve styles in the order `built-in or custom preset -> project overrides -> widget overrides -> final widget style`.
- Add a focused Property Inspector Appearance section for supported single-widget selections.
- Preserve Design Mode, Preview Mode, serialization, copy/duplicate, undo/redo, autosave/recovery, and generated-output behavior.
- Update authoritative version declarations from `1.0.16` to `1.0.17`.

## Requirements

- Use Phase 111 and version `1.0.17`.
- Use no subagents.
- Extend the existing Look and Feel system; do not create selectors, style classes, or a second styling system.
- Support only control surface, text, border, accent, focus outline, highlight edge, shadow edge, border thickness, corner radius, and control padding.
- Support Button, Text Box, Check Box, Radio Button, Combo Box, List Box, Slider, Scroll Bar, Progress Bar, Color Picker, Frame, Group Box, Panel, and Tab Control.
- Disable Appearance editing for multi-selection.
- Preserve generated `MainWindow`, USER CODE regions, CMake presets, vcpkg settings, and static MSVC runtime settings.
- Do not launch `VisiForm.exe` or generated applications from an automated agent.

## Version Notes

- Previous Phase 110 version: `1.0.16`.
- Phase 111 version: `1.0.17`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, `docs/versioning.md`, and the current-version line in `README.md`.

## Architecture Decisions

- Add one reusable `WidgetLookAndFeelOverrides` value owned by `WidgetNode`.
- Keep every field optional; absence means inherit.
- Keep `LookAndFeelRegistry::resolve(...)` as the only preset/project/widget merge path.
- Retain read compatibility for the older scattered widget style properties while new inspector edits and serialization use the sparse override object.
- Define one central compatibility mapping by widget type and property rather than per-widget editor implementations.
- Treat copy, duplicate, paste, undo, redo, autosave, and recovery as value-copy/serialization behavior of `WidgetNode`, avoiding special-case transfer code.

## Supported Widget Override Properties

### Colors

- control surface
- text
- border
- accent
- focus outline
- highlight edge
- shadow edge

### Metrics

- border thickness
- corner radius
- control padding

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
- Frame
- Group Box
- Panel
- Tab Control

## Property Inspector Behavior

- Add an `Appearance` section for a supported single selected widget.
- Show the resolved inherited value when no override exists and mark it as inherited.
- Editing a field creates or updates one explicit sparse override.
- Provide a reset-one-property selector for explicit overrides.
- Provide one Reset All Widget Appearance Overrides action.
- Hide legacy duplicate style rows for supported widgets.
- Show a concise read-only limitation when Appearance editing is unavailable for multi-selection.

## Reset And Undo Behavior

- Resetting one property removes only that optional value.
- Reset All clears only the selected widget's appearance override object.
- No-op resets do not create an undo entry or dirty the document.
- Each edit or reset uses one existing `DocumentStateCommand`.
- Selection remains part of the before/after document state and therefore remains stable.

## Serialization Format

- Add optional per-widget `appearanceOverrides` objects beside the existing `properties` object.
- Omit `appearanceOverrides` when empty.
- Store only explicitly set supported fields.
- Existing projects load with an empty widget override object.
- Existing legacy widget style properties remain readable and continue to resolve.
- Unknown fields follow the reader's existing forward-compatibility behavior and are ignored.

## Design, Preview, And Export Integration

- Designer and Preview already consume `LookAndFeelRegistry::resolve(...)`; extending that resolver supplies final widget styles to both modes without changing editor overlays.
- Generated output already consumes the same resolver while emitting portable resolved runtime values; no local preset library access is added.
- Runtime interaction state styling remains layered over the final widget style.
- No override changes saved widget width or height.

## Compatibility Considerations

- Old `.vfb.json` files without `appearanceOverrides` remain valid.
- Older scattered widget style keys remain effective for backward compatibility.
- New files serialize the dedicated sparse override object and do not copy preset or project defaults into widgets.
- Unsupported widgets receive no Appearance editor and no forced overrides.
- Property compatibility is intentionally conservative; fields that do not render meaningfully for a widget are hidden.

## TODO Checklist

- [x] Inspect Git state, the Phase 111 prompt, project status, Phase 110 plan, shared resolver, widget model, Property Inspector, serializer, generator, command system, and version declarations.
- [x] Confirm Phase 111 is unused and select version `1.0.17`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.17`.
- [x] Add the sparse widget override model and compatibility mapping.
- [x] Extend the shared resolver with correct widget-last precedence.
- [x] Add sparse widget serialization and backward-compatible loading.
- [x] Add the supported single-selection Appearance inspector and reset actions.
- [x] Confirm duplicate/copy/paste and undo/redo preserve sparse overrides.
- [x] Add focused model, serialization, resolution, and generated-output tests.
- [x] Update Look and Feel/file-format/code-generation documentation and final project status.
- [x] Run focused static validation.
- [ ] Run the approved normal Windows Debug build once.
- [x] Record final validation, files changed, limitations, and remaining issues.

## Validation Plan

- Add focused tests for sparse round-trip, omission when empty, legacy compatibility, project-to-widget precedence, invalid-value fallback/clamping, copy/duplicate preservation, and generated resolved values.
- Confirm the Appearance section is limited to supported single selections and uses the compatibility mapping.
- Confirm reset operations are no-op safe and use one undoable document state.
- Confirm Designer, Preview, and generated output all still call the shared resolver.
- Run `git diff --check` and targeted source searches.
- Build the normal Windows Debug configuration once through an approved path.
- Leave application launch and generated-app launch to the developer.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains the user-owned Phase 110 session-instruction archival move and the untracked Phase 111 prompt; they are preserved.
- Focused static validation: passed. `git diff --check` reported only the
  repository's existing LF-to-CRLF normalization warnings. Targeted searches
  confirmed Designer Canvas, Preview rendering, generated static/runtime
  output, and the Property Inspector all converge on the shared resolver;
  widget JSON omits empty override objects; and changed implementation files
  have balanced brace/parenthesis counts apart from brace text inside the
  serialization test's raw JSON fixture.
- Focused tests: added for sparse serialization, empty omission, widget-last
  precedence, invalid-value fallback/clamping, value-copy preservation, and
  generated runtime values. They were not executed because no approved exact
  build or test command was supplied.
- Windows Debug build: not run. No Visual Studio workspace-build tool is
  available in this session, and repository safety rules prohibit inferring a
  terminal build command without exact developer authorization.
- Manual runtime validation: not performed; automated agents may not launch either application.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_111_per_widget_look_and_feel_overrides_plan.md`
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
- `tests/test_generated_runtime_labels.cpp`
- `tests/test_project_serialization.cpp`

The pre-existing Phase 110 session-instruction archive move and the Phase 111
instruction file were preserved without modification.

## Known Limitations

- Multi-selection Appearance editing is disabled in this phase.
- Style classes, selectors, state-specific overrides, fonts, images, gradients, animation, bulk editing, and per-child selector styling remain future work.
- Manual runtime verification is required for visual fit, repaint timing, Preview interaction parity, and generated application behavior.

## Final Result Summary

- Updated the phase version from `1.0.16` to `1.0.17`.
- Added `WidgetLookAndFeelOverrides` with seven optional colors and three
  optional metrics; empty values inherit and empty objects are not serialized.
- Added a central supported-widget and property compatibility map for Button,
  Text Box, Check Box, Radio Button, Combo Box, List Box, Slider, Scroll Bar,
  Progress Bar, Color Picker, Frame, Group Box, Panel, and Tab Control.
- Extended the shared resolver so dedicated widget overrides apply after the
  selected built-in/custom preset and sparse project overrides. Legacy widget
  style properties remain readable for older project files.
- Added an Appearance inspector section showing inherited or explicit values,
  with editing disabled for multi-selection and irrelevant fields hidden.
- Added reset-one and reset-all actions. Each committed edit/reset is one
  no-op-safe `DocumentStateCommand`, preserving selection and project defaults.
- Added optional per-widget `appearanceOverrides` JSON with backward-compatible
  loading. Widget value copies automatically preserve overrides across the
  existing duplicate, copy/paste, undo/redo, autosave, and recovery paths.
- Preserved Design/Preview overlays and runtime interaction states while
  feeding both renderers the final resolved widget style. Control padding now
  affects Button, Text Box, and Combo Box text content bounds.
- Generated projects receive final portable per-widget values through the
  existing runtime style helper and do not access the custom-preset library.

## Remaining TODOs

- Run the normal Windows Debug build through the approved Visual Studio
  workspace pipeline or an exact developer-authorized command.
- Run the focused Catch2 tests.
- Manually execute the Phase 111 Design Mode, Preview Mode, reset, undo/redo,
  save/reload, duplicate/copy-paste, base-style change, autosave/recovery, and
  generated-project validation checklist.
