# Phase 86 - BoxSizer Layout System

## Scope

Upgrade the existing starter `Sizer` widget into a wxBoxSizer-inspired layout system while keeping VisiForm a Visage-based C++ form builder.

## Existing Implementation Findings

- `Sizer` is already registered as `WidgetType::Sizer` with `orientation`, `padding`, and `gap` properties.
- Current sizer layout in `LayoutEngine` divides the usable main axis evenly among direct children.
- `WidgetNode` owns children by value in `std::vector<WidgetNode>`, and Project Tree display order follows that vector order.
- JSON persistence already preserves arbitrary widget properties, which supports compatible migration through new properties.
- Undo/redo already supports whole-document mutations through `DocumentStateCommand`.

## Architectural Decisions

- [x] Store per-child sizer-item metadata as namespaced properties on the direct child.
- [x] Interpret sizer-item metadata only when the direct parent is `WidgetType::Sizer`.
- [x] Keep model, validation, serialization, generator, and tests free of Visage UI headers.
- [x] Use one model-level BoxSizer math implementation for editor layout.
- [x] Mirror the same BoxSizer math in generated Visage runtime output.

## Files And Subsystems Expected To Change

- [x] Model: `WidgetNode`, `WidgetRegistry`, `LayoutEngine`, and new BoxSizer helper files.
- [x] UI: `PropertyInspector`, `DesignerCanvas`, `MainWindow`, and `ProjectTree` as needed.
- [x] Serialization: JSON tests and compatibility docs; reader/writer should mostly preserve properties unchanged.
- [x] Validation: `ProjectValidator`.
- [x] Generator: `VisageCppEmitter`.
- [x] Tests: Catch2 test sources and test CMake wiring.
- [x] Documentation: layout, widget catalog/registry, project file format, validation, and code generation docs.

## Model Changes

- [x] Add typed BoxSizer enums and structs.
- [x] Add helper functions to parse/write sizer and sizer-item properties.
- [x] Add spacer type/size helpers.
- [x] Centralize default sizer-item settings by widget type.

## Layout Algorithm

- [x] Add pure minimum-size calculation.
- [x] Add deterministic main-axis proportion distribution.
- [x] Add cross-axis expand and alignment.
- [x] Add padding, gap, borders, and border sides.
- [x] Add recursive nested sizer layout.
- [x] Add fixed and stretch spacer handling.
- [x] Clamp undersized parents without negative rectangles.

## Designer Changes

- [ ] Keep dashed design-time sizer outline and orientation label.
- [x] Add or improve fixed/stretch spacer visual representation.
- [x] Disable misleading free resize behavior for direct sizer children, or map it to minimum-size overrides.
- [x] Map direct sizer-child mouse resize gestures to `Sizer Item` minimum-size properties.
- [x] Keep selected sizer-child resize handles repeatable after layout rewrites child bounds.
- [x] Start selected-widget resize handles before general widget hit testing so visible handles can restart resizing.
- [x] Relayout children while resizing a Sizer and commit the result as one undoable document state.
- [x] Clamp moved widgets and root-level sizers to their parent client canvas during drag, nudge, and bounds edits.
- [x] Snap-connect dragged widgets into nearby sizers within the editor drop threshold.
- [ ] Add insertion/reorder feedback where compatible with current interaction architecture.

## Property Inspector Changes

- [x] Show Sizer properties on selected sizers.
- [x] Show Sizer Item properties on direct children of sizers.
- [x] Mark inactive absolute bounds, Dock, and Anchor as controlled by the parent sizer.

## Project Tree Changes

- [x] Ensure sizers, nested sizers, and spacers display in layout order.
- [x] Preserve or initialize sizer-item metadata during reparent/reorder operations.

## Serialization And Migration Strategy

- [x] Keep `schemaVersion` at 1 unless a blocker appears.
- [x] Preserve existing Phase 85 `padding` while adding side-specific padding fields.
- [x] Round-trip new sizer, sizer-item, and spacer metadata through properties.
- [x] Preserve unknown properties.

## Validation Changes

- [x] Validate sizer properties.
- [x] Validate sizer-item properties.
- [x] Validate spacer properties.
- [x] Add warnings for empty sizers and ignored absolute/dock/anchor/alignment state.
- [x] Add minimum-size warning where practical.

## Undo/Redo Changes

- [x] Use existing undoable document changes for property edits.
- [x] Preserve previous and new document state for reparent/reorder operations.
- [x] Commit sizer-child resize gestures as a single `DocumentStateCommand`.
- [x] Commit sizer resize plus child relayout as a single `DocumentStateCommand`.
- [x] Add focused command support only if needed by the current architecture.

## Generated Runtime Changes

- [x] Emit runtime metadata for BoxSizer properties.
- [x] Emit runtime metadata for per-child sizer-item settings.
- [x] Emit generated layout helper for nested sizers and spacers.
- [x] Avoid visible runtime components for spacers.

## Documentation Changes

- [x] Update `README.md`.
- [x] Update layout, widget catalog, widget registry, project file format, validation, and code generation docs.

## Automated Tests

- [x] Add pure BoxSizer layout tests.
- [x] Add focused sizer-item minimum-size layout tests.
- [x] Add focused resized-sizer child relayout test.
- [x] Add focused nested-sizer relayout propagation test.
- [x] Add focused JSON round-trip test for resized sizer-item minimum sizes.
- [x] Add JSON compatibility and round-trip tests.
- [x] Add validation tests.
- [ ] Add generator source-inspection tests where feasible.
- [ ] Add undo/redo tests where feasible.

## Sizer Resize Bug Investigation

- Issue 1, repeated child resize: current follow-up code maps direct sizer-child resize gestures to `sizerItem.minimumWidth` and `sizerItem.minimumHeight`, starts selected-widget handle hit testing before general widget hit testing, relayouts from the pre-drag document, and clears drag state after committing a `DocumentStateCommand`.
- Issue 2, child relayout after resizing a Sizer: current follow-up code relayouts while the Sizer is resized and commits the Sizer bounds plus relaid-out child bounds as one `DocumentStateCommand`.
- The apparent width or height lock is intentional where BoxSizer rules own that dimension. `expand` controls cross-axis fill, `proportion` controls main-axis growth, and default Button items use `proportion=0` and `expand=false`.
- No new production-code fix was made for this investigation because the reports appear addressed by the current Phase 86 follow-up worktree and remaining uncertainty is manual UI verification.

## Verification Matrix

| Case | Expected behavior | Observed evidence | Defect? | Coverage |
| --- | --- | --- | --- | --- |
| Button outside a sizer | Corner resize edits widget bounds directly with normal resize command behavior. | Existing `MainWindow` resize path remains available when selected widget is not a direct sizer child. | No current evidence. | Manual UI check required. |
| Button in horizontal BoxSizer | Width is main-axis and grows only with positive proportion; height fills only with expand. | BoxSizer contract in `BoxSizerLayout` and architect review. | No, unless configured expand/proportion fails. | Model tests cover proportion and expand behavior; manual UI check required. |
| Button in vertical BoxSizer | Height is main-axis and grows only with positive proportion; width fills only with expand. | BoxSizer contract in `BoxSizerLayout` and architect review. | No, unless configured expand/proportion fails. | Model tests cover minimum height and resized-sizer relayout; manual UI check required. |
| Expand disabled | Cross-axis size stays at minimum or preferred size with alignment. | Existing alignment/border test and sizer contract. | No. | `BoxSizer supports item borders, alignment, and fixed spacers`. |
| Expand enabled | Cross-axis size tracks available sizer content size. | Existing and added model tests. | No current evidence. | `Resized Sizer recomputes child layout from new bounds`; nested propagation test. |
| Proportion 0 | Main-axis size stays at minimum or preferred size. | Existing proportion test. | No. | `BoxSizer distributes extra main-axis space by proportion deterministically`. |
| Nonzero proportion | Main-axis receives weighted extra space. | Existing proportion test. | No current evidence. | `BoxSizer distributes extra main-axis space by proportion deterministically`. |
| Button inside nested BoxSizer | Parent resize assigns nested sizer slot, then nested children relayout recursively. | Added focused model test. | No current evidence. | `Nested BoxSizer relayout propagates from parent resize`. |
| First resize drag | Direct sizer child resize edits sizer-item minimum size. | Current `MainWindow` code path and minimum-size layout tests. | No current evidence. | Model tests cover resulting minimum behavior; manual UI check required. |
| Second resize drag after release | Selected handle can restart resizing after layout rewrites child bounds. | Current `DesignerCanvas` selected-handle-first hit testing and `clearCanvasInteraction` path. | Original report appears fixed in current worktree. | Manual UI check required. |
| Resize from each applicable handle | Corner handles should restart and update minimum size request. | Current handle detection covers all four corner handles. | No current evidence. | Manual UI check required. |
| Resize containing sizer larger | Children relayout according to expand/proportion/minimum rules. | Current `MainWindow` Sizer resize path and model tests. | No current evidence. | `Resized Sizer recomputes child layout from new bounds`; nested propagation test. |
| Resize containing sizer smaller | Children relayout and clamp to effective minimums without negative bounds. | BoxSizer layout clamps available sizes. | No current evidence. | Manual UI check required; model undersize coverage remains indirect. |
| Save and reload before another resize | Sizer-item minimum overrides persist and reproduce layout. | Added JSON round-trip test for minimum width/height. | No current evidence. | `Sizer item minimum sizes round-trip through JSON`; manual follow-up resize check required. |
| Undo and redo | Each completed sizer-child or Sizer resize is one document-state command. | Current `MainWindow` uses `DocumentStateCommand` for both paths. | No current evidence. | Manual UI check required; automated undo/redo test still TODO. |

## Risks

- Generated runtime parity may require careful duplication of model-level math because generated projects cannot directly link VisiForm internals.
- Existing drag/drop behavior is centered on freeform positioning, so full insertion indicators may need incremental UI work.
- The current test target does not yet include all model/generator/validation files needed for expanded tests.

## Acceptance Criteria

- [x] Vertical and horizontal sizers support ordered children, proportions, expand, alignment, padding, gap, borders, minimum overrides, nested sizers, fixed spacers, and stretch spacers.
- [x] Designer, JSON, validation, undo/redo, Project Tree, and generated runtime behavior remain consistent.
- [x] Existing Phase 85 sizer projects load with compatible defaults.
- [ ] Phase 86 acceptance scenario is covered by automated tests where feasible and manual notes otherwise.

## Build-Validation Status

- Not run by agent.
- Command-line builds and tests are prohibited unless the developer explicitly requests the exact command.
- Visual Studio workspace build pipeline was not available through the current tool set.
- Validation is deferred to the developer for the main `VisiForm` target.

## Work Continuity

- Current branch: `main`.
- Most recent relevant commit before this follow-up: `6f95b09 Add BoxSizer layout system with enhanced features`.
- Files changed: model BoxSizer helpers/layout integration, JSON loading defaults, registry defaults, property inspector, designer canvas, main window drag behavior, validation, generator runtime output, CMake test wiring, docs, and focused Catch2 test source.
- Follow-up files changed: `src/ui/DesignerCanvas.cpp`, `src/ui/MainWindow.cpp`, `src/ui/MainWindow.h`, `tests/test_box_sizer_layout.cpp`, `docs/layout_tools.md`, and this plan.
- Adjustment files changed: `src/ui/MainWindow.cpp`, `docs/layout_tools.md`, and this plan.
- Validation follow-up files changed: `tests/CMakeLists.txt`, `tests/test_project_validation.cpp`, and this plan.
- Intentionally left untouched: active `session-instructions/phase 86 BoxSizer Layout.txt`; no archiving until Phase 86 is complete.
- Static validation run: `git diff --check` passed after the repeatable sizer resizing and clamp follow-ups.
- Build/test validation: deferred to the developer because command-line build/test execution is prohibited and no unambiguous Visual Studio workspace pipeline was available to the agent.

## Remaining TODOs

- Add explicit designer insertion indicators between sizer children.
- Convert the current sizer outline into a true dashed design-time outline.
- Add generator-inspection, undo/redo, and full Phase 86 acceptance-scenario tests.
- Developer should manually verify repeat sizer-child mouse resizing, root-level Sizer drag clamping at the form edges, nudge/bounds-edit clamping, resizing a Sizer with children inside it, undo/redo after Sizer resize, snap-connect reparenting into sizers, and single-selection Fill Width/Fill Height menu behavior.
- Developer should build the main `VisiForm` target in Visual Studio and manually verify editor behavior without launching generated apps from an agent.

## Final Result Summary

- Implemented model-level BoxSizer types, property helpers, defaults, recursive editor layout, inspector editing, designer visuals, repeatable sizer-child resize-to-minimum behavior, selected-handle-first resize restart, parent-canvas movement clamping, live Sizer resize relayout with undoable document-state commits, sizer snap-connect drop targeting, validation, JSON compatibility, focused validation tests, generated runtime metadata/layout, and documentation.
- Build and test validation were deferred because the approved Visual Studio workspace build pipeline is not available to the agent and command-line build/test validation is prohibited by repository instructions.
