# Phase 72 - Repair GroupBox drag-out exception plan

## TODO
- [x] Create phase plan file.
- [x] Inspect the canvas drag reparenting flow for GroupBox children.
- [x] Identify the runtime exception trigger when dragging a widget out of a GroupBox.
- [x] Implement the drag-out repair.
- [x] Validate the `VisiForm` debug build.
- [x] Write the final summary and remaining TODOs.

## Progress
- Phase plan created for investigating the exception thrown when dragging a button out of a GroupBox.
- Traced the drag-out path through `resolveDropParentId(...)`, `mouseUp(...)`, and `ProjectDocument::reparentWidget(...)`.
- Identified that `mouseUp(...)` kept using the original `widget` pointer after `DocumentStateCommand::execute()` replaced `document_` during reparenting.
- Updated `src/ui/MainWindow.cpp` to capture the dragged widget id and display name before executing reparent, move, or resize commands, so status updates no longer dereference invalid widget memory.

## Build validation
- `run_build` completed successfully after the repair.
- `cmake --build --preset build-static-debug --target VisiForm` reported an environment-specific x86/x64 linker mismatch in this shell session and did not indicate a source-level compile failure from the patch.

## Final summary
- Repaired the GroupBox drag-out exception in `src/ui/MainWindow.cpp`.
- Root cause: `mouseUp(...)` reused a `WidgetNode*` after `DocumentStateCommand::execute()` replaced `document_` during reparenting, leaving a stale pointer that was later dereferenced while building status text.
- Fix: capture the dragged widget id and display name before command execution and use those stable values after move, resize, and reparent commands.

## Remaining TODOs
- Manual runtime verification by the developer: drag a child widget out of a GroupBox and confirm the exception no longer occurs.
