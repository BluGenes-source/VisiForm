# Phase 106 Runtime Styling Expansion Plan

## Scope

- Extend the Phase 105 shared runtime visual baseline to the requested additional widgets.
- Keep Design and Preview on the shared `DesignerCanvas` path and update generated runtime rendering where supported.
- Preserve project data, layout geometry, editor overlays, interaction behavior, and generated user-code regions.
- Update authoritative version declarations from `1.0.11` to `1.0.12`.

## Requirements

- Use Phase 106 and version `1.0.12`.
- Use no subagents.
- Reuse and minimally extend the Phase 105 shared visual primitives.
- Do not add themes, gradients, animation, serialized style properties, or duplicate state machines.
- Internal widget names remain editor metadata and must not become runtime captions.
- Do not launch `VisiForm.exe` or generated applications from an automated agent.
- Build only through an exact command explicitly requested by the developer.

## Version Notes

- Previous Phase 105 version: `1.0.11`.
- Phase 106 version: `1.0.12`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Architecture Decisions

- Extend `ui/VisualStyleBaseline.h` only when a generally reusable primitive is needed.
- Apply styling in `DesignerCanvas` without changing saved bounds or model properties.
- Emit equivalent standalone generated-runtime drawing because exported projects cannot include editor source.
- Reuse existing enabled, checked, selected, focused, hover, pressed, active-tab, progress, and scroll state where available.
- Keep Sizer and other layout-only visuals editor-only unless they already have a runtime-visible surface.

## TODO Checklist

- [x] Inspect branch, worktree, Phase 105 plan, Phase 106 prompt, and targeted rendering/version files.
- [x] Confirm Phase 106 is unused and select version `1.0.12`.
- [x] Create this persistent phase plan.
- [x] Update authoritative version declarations and current-progress documentation to `1.0.12`.
- [x] Reuse the Phase 105 shared visual primitives without adding a parallel styling system.
- [x] Update Designer/Preview rendering for in-scope widgets.
- [x] Update generated runtime rendering and existing interaction-state mapping.
- [x] Confirm runtime-text policy and editor-overlay ordering.
- [x] Run focused static validation.
- [ ] Run the normal Windows Debug build once if the developer supplies or approves an exact command.
- [x] Record final validation, files changed, parity gaps, and remaining issues.

## Validation Plan

- Inspect each scoped renderer for shared palette use and correct normal/disabled/static state mapping.
- Confirm Design and Preview share widget styling while Preview suppresses editor labels and overlays.
- Confirm generated runtime uses equivalent styling and existing transient interaction state.
- Confirm internal names are not used as runtime text.
- Confirm no model, serialization, validation, command, geometry, or project-schema changes.
- Run `git diff --check` and targeted source searches.
- Build the normal Windows Debug `VisiForm` target once only through an explicitly approved exact command.
- Leave application and generated-app launch/manual interaction validation to the developer.

## Compatibility

- No `.vfb.json` schema change.
- No generated identifier or USER CODE region change.
- No CMake preset, vcpkg triplet, or runtime-library change.
- Saved geometry, parenting, sizer behavior, selection, hit testing, zoom, and pan remain unchanged.

## Build / Test Status

- Branch: `main`.
- Starting commit: `688ba32 Update to version 1.0.11 with Phase 105 improvements`.
- Starting worktree contains user-owned Phase 105 session-instruction archive moves and the untracked Phase 106 prompt; they are preserved.
- Focused static validation: `git diff --check` passed with line-ending normalization warnings only. Targeted brace-count checks reported matching opening and closing brace counts for both modified C++ translation units. Targeted searches confirmed internal names remain limited to editor metadata, lookup/event metadata, and generated identifiers rather than runtime captions.
- Windows Debug build: pending an explicitly approved exact command.
- Manual runtime validation: not performed; automated agents may not launch `VisiForm.exe` or generated applications.

## Files Changed

- `CMakeLists.txt`
- `README.md`
- `docs/agent_plans/phase_106_runtime_styling_expansion_plan.md`
- `docs/project_status.md`
- `docs/versioning.md`
- `src/app/Version.h`
- `src/generator/VisageCppEmitter.cpp`
- `src/ui/DesignerCanvas.cpp`

## Designer / Preview / Export Consistency

- Design and Preview share the updated `DesignerCanvas` rendering for Radio Button, List Box, Progress Bar, Scroll Bar, Color Picker, Frame, Group Box, Panel, Tab Control/pages, Status Bar, Menu Bar, Modal Dialog, and Design-only Sizer indicators.
- Generated runtime output applies equivalent raised/recessed styling to the runtime-visible widget set and reuses existing hover, pressed, selected, focused, disabled, active-tab, progress, and scroll interaction state.
- Generated Sizers are now explicitly invisible because they are layout-only objects. The Designer retains its editor-only Sizer indicator.
- Selection outlines, resize handles, corner grip arcs, guides, and other editor overlays remain drawn after widget rendering; Preview continues to suppress them.
- Runtime-visible text continues to come only from explicit text/title/item/status/message/value properties.

## Known Parity Gaps

- Preview remains visual-only and therefore cannot demonstrate transient interaction states.
- Generated Tab Pages are represented by the Tab Control page surface rather than a separate runtime widget renderer.
- Menu Bar hover is available at the control surface level; the current runtime state does not store a separate hovered top-level item index.
- List Box has selected-row styling, but no per-row hover state exists in the current runtime model.
- Modal buttons use the shared raised treatment but the existing modal interaction path does not expose individual button hover/pressed state.
- Color Picker focus indication is available in generated runtime; Design/Preview show its stored static appearance only.
- Exact DPI, zoom, generated-build, runtime interaction, and save/reload parity require manual validation.

## Final Result Summary

- Updated the phase version from `1.0.11` to `1.0.12`.
- Reused the Phase 105 baseline primitives for raised and recessed surfaces, highlight/shadow edges, disabled text/fill derivation, accent selections, and focused Color Picker borders.
- Added a recessed List Box with selected rows and disabled treatment.
- Added recessed Progress Bar tracks with inset fills that preserve readable 0%, partial, and 100% bounds.
- Added raised/recessed Scroll Bar end controls and thumb treatment with existing hover/pressed/drag state.
- Added circular Radio Button depth, checked emphasis, disabled treatment, and generated hover/pressed feedback.
- Added Color Picker swatch, raised surface, generated hover/pressed/focus, and disabled treatment.
- Added restrained Frame, Group Box, Panel, Tab Control/page, Status Bar, Menu Bar, and Modal Dialog surface depth.
- Kept Sizer indicators editor-only and removed generated runtime decoration for layout-only Sizers.

## Remaining TODOs

- Build the normal Windows Debug `VisiForm` target through an explicitly approved exact command.
- Manually confirm version `1.0.12`.
- Run the full Design/Preview interaction checklist at multiple zoom levels.
- Export and build a representative generated project through the approved workflow.
- Record any visual differences found during manual runtime and generated-output validation.
- Confirm save/reload and autosave behavior remain unchanged.
