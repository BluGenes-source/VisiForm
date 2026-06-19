# Phase 107 Runtime State Styling and Export Parity Plan

## Scope

- Refine the shared runtime visual-state policy used by Design/Preview rendering and generated output.
- Add preview-only interaction state without modifying the project model.
- Correct confirmed state-style and generated-output parity gaps for the Phase 105-106 widget set.
- Update authoritative version declarations from `1.0.12` to `1.0.13`.

## Requirements

- Use Phase 107 and version `1.0.13`.
- Use no subagents.
- Preserve widget behavior, saved geometry, project schema, generated USER CODE regions, and internal-name policy.
- Do not add themes, custom style properties, animation, gradients, glow, or a skinning system.
- Do not launch `VisiForm.exe` or generated applications from an automated agent.
- Build only through an exact command explicitly requested by the developer.

## Version Notes

- Previous Phase 106 version: `1.0.12`.
- Phase 107 version: `1.0.13`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Architecture Decisions

- Use one compact state description with enabled, hovered, pressed, focused, checked/selected, active, and read-only flags.
- Resolve the base visual state in this order: disabled, pressed, checked/selected/active, hovered, normal.
- Render focus as an additional outline so it does not replace or obscure the resolved base state.
- Keep Preview Mode state in editor-only transient storage and discard it when Preview Mode ends.
- Emit an equivalent compact resolver into generated projects rather than duplicating independent priority rules per widget.

## TODO Checklist

- [x] Inspect Git state, Phase 107 prompt, project status, Phase 106 plan, shared styling helpers, targeted renderers, and version declarations.
- [x] Confirm Phase 107 is unused and select version `1.0.13`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.13`.
- [x] Implement the shared state description and priority resolver.
- [x] Add preview-only hover, press, focus, checked/selected, active, and read-only rendering where existing behavior supports it.
- [x] Align generated runtime styling with the shared priority and focus-overlay policy.
- [x] Correct confirmed widget-specific parity gaps without changing behavior.
- [x] Run focused static validation.
- [ ] Run the normal Windows Debug build once if the developer supplies or approves an exact command.
- [x] Record final validation, files changed, parity gaps, DPI/zoom findings, and remaining issues.

## Validation Plan

- Confirm state priority is centralized and consistent in editor and generated rendering.
- Confirm Preview Mode suppresses editor overlays and uses transient state without model mutation.
- Confirm leaving Preview Mode clears preview-only state.
- Inspect scoped widgets for readable text, checked/selected distinction, focus visibility, and correct raised/recessed direction.
- Confirm generated output retains readable standalone helpers and USER CODE preservation.
- Run `git diff --check` and targeted source searches.
- Build the normal Windows Debug `VisiForm` target once only through an explicitly approved exact command.
- Leave application launch, generated-app launch, DPI, and manual interaction validation to the developer.

## Compatibility

- No `.vfb.json` schema change.
- No generated identifier or USER CODE region change.
- No CMake preset, vcpkg triplet, or runtime-library change.
- No saved geometry, parenting, selection, command, zoom, pan, or autosave change.

## Build / Test Status

- Branch: `main`.
- Starting worktree contains a user-owned Phase 106 session-instruction archive move and the untracked Phase 107 prompt; they are preserved.
- Focused static validation: `git diff --check` passed with line-ending normalization warnings only. Targeted brace and parenthesis counts matched in `DesignerCanvas.cpp`, `VisageCppEmitter.cpp`, and `VisualStyleBaseline.h`. Searches confirmed preview interaction code does not call project mutation APIs and internal names remain editor-only labels/metadata rather than runtime captions.
- Compile-error root cause: `endPreviewInteraction()` passed the nullable `WidgetNode*` returned by `findWidgetById()` to `previewSelectedIndex(const WidgetNode&, int)`. The existing null/disabled early return proves the pointer valid before the switch, so the call now dereferences the guarded pointer and the reference-based API remains unchanged.
- Windows Debug build: pending an explicitly approved exact command.
- Manual runtime and generated-output validation: not performed; automated agents may not launch either application. The no-selection, deletion, project-replacement, repeated Preview Mode, and model-independence checks remain for developer validation.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_107_runtime_state_export_parity_plan.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/MainWindow.cpp`
- `src/ui/Splitter.cpp`
- `src/ui/VisualStyleBaseline.h`

## Known Parity Gaps

- Preview Combo Box and List Box clicks advance temporary selection rather than opening a full dropdown or selecting a pointer-specific row; generated event handlers are intentionally not executed.
- Preview Tab Control updates temporary active-header styling, but page-child visibility still follows the saved selected tab until runtime behavior is implemented in Preview.
- Menu Bar and List Box still have control-level hover only because neither path currently stores a per-item hovered index.
- Modal Dialog preview buttons do not have individual hover/pressed state.
- Exact pixel, DPI, zoom, generated-build, and live runtime parity remain unverified without manual application testing.

## Final Result Summary

- Updated Phase 107 version metadata from `1.0.12` to `1.0.13`.
- Added a compact shared state description and deterministic base-state resolver: disabled, pressed, checked/selected/active, hovered, normal; focus is an additional outline and read-only modifies recessed fields.
- Added editor-owned Preview Mode hover, pressed, focus, toggle, radio selection, item selection, and active-tab state that does not modify `ProjectDocument` and is cleared on mode changes.
- Applied shared state rendering to Button, Text Box, Combo Box, List Box, Check Box, Radio Button, Slider, Scroll Bar, Color Picker, Tab Control, Menu Bar, Frame, and the reusable shell Splitter where meaningful.
- Expanded Splitter hover feedback to the actual drag hit area.
- Emitted an equivalent generated-runtime resolver and consistent focus overlays while preserving generated code structure and USER CODE regions.

## Remaining TODOs

- Build the normal Windows Debug `VisiForm` target through an explicitly approved exact command.
- Confirm the reported C2664 is resolved and check for follow-on compiler errors.
- Manually confirm version `1.0.13`.
- Run the full Design/Preview interaction checklist at several zoom levels.
- Export and build a representative generated project through the approved workflow.
- Compare Preview and generated runtime at normal and available scaled DPI.
- Confirm save/reload and autosave remain unchanged.
