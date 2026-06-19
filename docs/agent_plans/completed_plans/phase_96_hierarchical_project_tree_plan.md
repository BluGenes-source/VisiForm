# Phase 96 Hierarchical Project Tree Plan

## Scope

- Remove Recent Files from the Project Tree while preserving File-menu recent-file behavior and settings persistence.
- Replace the always-expanded flat presentation with an expandable hierarchical view.
- Derive every displayed widget node directly from `ProjectDocument::root` and recursive `WidgetNode::children`.
- Correct Project Tree row metrics, indentation, label spacing, clipping, and row-aligned scrolling using the active rendered font.
- Preserve canvas, Property Inspector, Events tab, command, undo/redo, and multi-selection behavior.
- Update authoritative version declarations from `1.0.1` to `1.0.2`.

## Requirements

- Use Phase 96 and version `1.0.2`.
- Use no subagents.
- Do not launch `VisiForm.exe`.
- Do not run a build command without an exact developer request.
- Do not change `.vfb.json`, generator behavior, validation rules, or widget ownership semantics.

## Version Notes

- Previous Phase 95 version: `1.0.1`.
- Phase 96 version: `1.0.2`.
- Authoritative declarations: `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`.

## Architecture Decisions

- Hierarchy source of truth: `ProjectDocument::root` and each `WidgetNode::children` vector.
- Tree-node mapping: a lightweight render entry generated from each model node; no duplicate persistent hierarchy.
- Project root: a UI-only row labeled from `ProjectDocument::projectName`, with the top-level `FormWindow` beneath it.
- Selection synchronization: tree clicks continue through `MainWindow::handleWidgetClicked`; drawing reads the document selection, and the tree reveals the primary selected widget.
- Refresh strategy: visible entries are derived on demand. Expanded widget IDs are retained in `ProjectTree`; missing IDs are pruned and selected ancestors are expanded automatically.
- Focused updates: property/name changes redraw from current model data without resetting expansion state. Full model replacement on new/load naturally rebuilds entries while retaining only still-valid expansion IDs.
- Formatting root cause: Project Tree rows were fixed at 24 px while the active UI font is loaded at 18 px and reports a larger rendered line height. Text was top-aligned inside a smaller text box, while row backgrounds, expanders, hit testing, reveal logic, and scrolling independently reused the undersized constant.
- Row metric rule: `ceil(font.lineHeight() + 12 px)` with a minimum of `kExpanderSize + 10 px`. The resulting `rowHeight_` is the single source of truth for layout, drawing, selection bounds, hit testing, reveal calculations, and scroll steps.
- Horizontal layout rule: each depth adds 16 px; rows then reserve a 12 px expander region and an 8 px control-to-label gap before the name, separator, and type regions. Name and type are measured separately so they cannot overlap.
- Clipping/elision rule: all rows remain clamped to the Project Tree content bounds. Long UTF-8 names and types are measured with the active Visage font and elided with an ellipsis; stored model values are unchanged. No tooltip was added because Project Tree has no existing hover/tooltip integration and adding one would expand this formatting-only pass into MainWindow input plumbing.
- Hover hint mechanism: reuse `MainWindow`'s existing status-bar hover-hint path. `ProjectTree` stores only the hovered project/widget row identity and resolves the displayed text live from `ProjectDocument`.
- Hover hit testing: use the same visible content bounds, generated row layouts, `rowHeight_`, and scroll offset used by drawing and selection hit testing.
- Hint content: project root uses `projectName : Project root`; widget rows use `widgetName : WidgetType`, with ` - Parent: parentName` for nested widgets.
- Stale-hover cleanup: clear on tree exit through pointer hit testing, document reset, scrolling, and expand/collapse. Live document lookup prevents deleted rows from displaying, and rename/type changes update without retaining copied label text.
- Scroll rule: the viewport height and scroll offsets are aligned to complete rows. Wheel, arrow, page, thumb-drag, and selected-item reveal paths all snap to the same row metric.
- Frame hierarchy bug root cause: the Project Tree already rendered the recursive model hierarchy correctly, but palette insertion and canvas reparent hit testing used separate hard-coded parent-type lists. Those lists included GroupBox, Sizer, and TabPage but omitted Frame and Panel, so visually overlapping widgets could remain children of the root form.
- Parent assignment fix: palette insertion and canvas reparenting now consult `WidgetRegistry::canContainChild`, the same model capability used by `ProjectDocument::addChildToParent` and `ProjectDocument::canReparentWidget`. This keeps Frame, Panel, GroupBox, Sizer, and tab-page containment on one shared rule.
- Hierarchy refresh fix: add, move/reparent, undo, and redo explicitly reveal the selected widget in `ProjectTree`. This expands its current model ancestors even when the selected widget ID did not change during a reparent operation.

## TODO Checklist

- [x] Inspect branch and worktree.
- [x] Read the Phase 96 brief, project status, Phase 95 plan, relevant specification sections, and directly related files.
- [x] Confirm Phase 96 is unused and increment version from `1.0.1` to `1.0.2`.
- [x] Create this persistent phase plan.
- [x] Remove Recent Files from the Project Tree UI and related layout/input code.
- [x] Implement project/document root, recursive expandable rows, indentation, guides, and readable labels.
- [x] Preserve tree/canvas selection synchronization and reveal the primary selection.
- [x] Preserve expansion and scrolling state during routine redraws.
- [x] Confirm no focused Project Tree automated test target currently exists.
- [x] Run focused static validation.
- [x] Record build/manual validation status, changed files, final summary, and remaining issues.
- [x] Confirm the Project Tree formatting root cause against the active font loading and Visage font metrics API.
- [x] Replace the fixed row height with active-font-derived metrics shared by drawing and hit testing.
- [x] Separate indentation, expander, name, separator, and type regions.
- [x] Add measured UTF-8-safe label elision and content clipping.
- [x] Align scrolling and selected-item reveal to complete rows.
- [x] Add Project Tree row hover tracking and status-bar hints using full, untruncated model text.
- [x] Clear stale Project Tree hover state on exit, scrolling, expand/collapse, and document reset.
- [x] Trace Frame add/reparent ownership from insertion target through model mutation and Project Tree rendering.
- [x] Replace hard-coded add/reparent container lists with the shared model containment rule.
- [x] Reveal the affected selection after add, reparent, undo, and redo.
- [x] Confirm serialization already persists recursive `WidgetNode::children` and requires no schema change.

## Validation Plan

- Run focused static checks, including `git diff --check`.
- Run existing relevant tests only through an approved path.
- Defer the Windows Debug build until the developer supplies or approves an exact command.
- Do not perform automated runtime validation because repository rules prohibit launching `VisiForm.exe`.

## Compatibility

- No project schema changes.
- No serialization, validation, or generated-code changes.
- Existing recent-file persistence and File-menu behavior remain intact.
- Existing primary/secondary selection semantics remain intact.

## Build / Test Status

- Branch: `main`.
- Most recent relevant commit at completion: `a61e688 Redesign Widget Palette with Top Tabbed Layout`.
- Starting worktree contained pre-existing session-instruction changes; they were preserved and not modified as part of Phase 96.
- Static validation: `git diff --check` passed with line-ending normalization warnings only.
- Formatting-pass static validation: `git diff --check -- src/ui/ProjectTree.cpp src/ui/ProjectTree.h` passed with line-ending normalization warnings only.
- Hover-hint static validation: `git diff --check -- src/ui/ProjectTree.h src/ui/ProjectTree.cpp src/ui/MainWindow.cpp docs/agent_plans/phase_96_hierarchical_project_tree_plan.md` passed with line-ending normalization warnings only.
- Frame hierarchy fix static validation: `git diff --check -- src/ui/MainWindow.cpp docs/agent_plans/phase_96_hierarchical_project_tree_plan.md` passed with line-ending normalization warnings only.
- Visage API inspection confirmed `Font::lineHeight()`, `Font::stringWidth()`, and `Font::widthOverflowIndex()` are available for rendered row metrics and label elision.
- Targeted searches confirmed Project Tree Recent Files APIs/text were removed and File-menu recent-file handling remains.
- Targeted source tracing confirmed add/reparent now assign ownership through `ProjectDocument` and the tree reads the resulting recursive `WidgetNode::children` directly. JSON save/load already writes and restores that same recursive relationship.
- Automated tests: not run. No focused Project Tree test target exists, and existing tests require an approved build path.
- Windows Debug build: deferred because no exact approved build command or unambiguous Visual Studio workspace build tool was available.
- Manual runtime validation: not performed because automated agents may not launch `VisiForm.exe`.

## Files Changed

- `CMakeLists.txt`
- `src/app/Version.h`
- `src/ui/ProjectTree.h`
- `src/ui/ProjectTree.cpp`
- `src/ui/MainWindow.cpp`
- `docs/versioning.md`
- `docs/project_status.md`
- `docs/agent_plans/phase_96_hierarchical_project_tree_plan.md`

## Final Result Summary

- The Project Tree is dedicated to the active project hierarchy; Recent Files remain available from the File menu.
- A UI-only project row contains the top-level form, and widget rows recursively mirror `WidgetNode::children`, including containers, sizers, tab pages, and deeper descendants.
- Rows use `name : WidgetType`, indentation guides, expand/collapse controls, selected-row styling, clipping, and the existing vertical scrollbar.
- Rows now derive their height from the active rendered font, vertically center text and expanders, use the full shared row bounds for selection and hit testing, and scroll in complete-row increments.
- Hovering any visible row now displays its full untruncated name and type in the existing status-bar hint area. Nested rows also identify their parent, and the project row identifies itself as the project root.
- Hover state uses the same row geometry as drawing and selection, updates only when the row changes, and clears on exit, scrolling, expand/collapse, document reset, or missing/deleted model nodes.
- Name, separator, and widget type use measured non-overlapping regions. Long labels are UTF-8-safe and end in an ellipsis instead of clipping abruptly.
- Expansion is tracked by widget ID and pruned when widgets disappear. New/load operations reset the tree, while routine property edits and hierarchy changes retain valid expansion state.
- Tree selection continues through the existing `MainWindow` selection path. Canvas/command selection changes reveal the primary selected widget by expanding ancestors and scrolling it into view.
- Palette insertion and canvas reparenting now recognize every container supported by the model registry, including Frame and Panel, instead of relying on incomplete UI-only type lists.
- Add, move/reparent, undo, and redo reveal the selected widget so the corrected model branch is visible immediately without a project reload.
- No model, serialization, validation, generator, or ownership behavior changed.

## Remaining TODOs

- Build the normal Windows Debug `VisiForm` target through an explicitly approved command.
- Manually launch VisiForm and complete the Phase 96 runtime checklist, including nested hierarchy, selection synchronization, rename, delete, undo/redo, save/reload, and scrolling.
