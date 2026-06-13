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
- [x] Add JSON compatibility and round-trip tests.
- [ ] Add validation tests.
- [ ] Add generator source-inspection tests where feasible.
- [ ] Add undo/redo tests where feasible.

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
- Most recent relevant commit before this work: `a46317d added a sizer widget`.
- Files changed: model BoxSizer helpers/layout integration, JSON loading defaults, registry defaults, property inspector, designer canvas, main window drag behavior, validation, generator runtime output, CMake test wiring, docs, and focused Catch2 test source.
- Intentionally left untouched: active `session-instructions/phase 86 BoxSizer Layout.txt`; no archiving until Phase 86 is complete.
- Static validation run: `git diff --check` passed.
- Build/test validation: deferred to the developer because command-line build/test execution is prohibited and no unambiguous Visual Studio workspace pipeline was available to the agent.

## Remaining TODOs

- Add explicit designer insertion indicators between sizer children.
- Convert the current sizer outline into a true dashed design-time outline.
- Add validation, generator-inspection, undo/redo, and full Phase 86 acceptance-scenario tests.
- Developer should build the main `VisiForm` target in Visual Studio and manually verify editor behavior without launching generated apps from an agent.

## Final Result Summary

- Implemented model-level BoxSizer types, property helpers, defaults, recursive editor layout, inspector editing, designer visuals, validation, JSON compatibility, generated runtime metadata/layout, focused tests, and documentation.
- Build and test validation were deferred because the approved Visual Studio workspace build pipeline is not available to the agent and command-line build/test validation is prohibited by repository instructions.
