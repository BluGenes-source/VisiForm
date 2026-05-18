# Phase 46 generated listener API plan

## Goal

Design and implement a small generated listener/callback API inspired by practical JUCE-style listener usage, using sender-aware `WidgetEvent` metadata, shared callback support, and generated emit helpers without introducing a full messaging framework.

## Current problems

- Sender-aware handler signatures already exist, but the generated API does not yet provide explicit emit/helper methods per widget event.
- The generated base class needs a clearer listener-oriented foundation for future interactive widget generation.
- Documentation does not yet fully describe shared callbacks, generated sender helpers, and conflict handling as a small listener API.

## Files to inspect

- `src/generator/VisageCppEmitter.cpp`
- `src/generator/CodeGenerator.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetNode.h`
- `docs/code_generation.md`
- `docs/widget_registry.md`
- `docs/agent_plans/phase_46_generated_listener_api_plan.md`

## Step-by-step TODO list with checkboxes

- [x] Create persistent phase plan file before code changes
- [x] Inspect current generated sender-aware callback foundation and identify missing listener helpers
- [x] Add generated `WidgetEvent` listener helper or emit methods with sender metadata
- [x] Keep shared callback generation working for compatible signatures
- [x] Preserve clear conflict errors for incompatible signatures
- [x] Update generated base and user subclass output to keep warning-free stubs and USER CODE preservation
- [x] Update `docs/code_generation.md`
- [x] Update `docs/widget_registry.md`
- [x] Build with `build-static-debug`
- [x] Write final result summary

## Current progress notes

- Phase plan file created before edits.
- Inspection confirmed that sender-aware `WidgetEvent` handler signatures already existed, but the generated API did not yet emit per-widget listener helper methods.
- `VisageCppEmitter` now records sender metadata per event binding and generates per-widget emit helper methods such as `emit_button_1_onClick()` and `emit_radioButton_1_onSelected(bool value)`.
- Shared callbacks still generate one handler declaration and definition when the signature is compatible, while each widget event gets its own emit helper carrying distinct sender metadata.
- Incompatible callback signature reuse still fails through the existing handler conflict checks.
- Generated user subclass stubs remain warning-free with `(void)event;` and `(void)value;` defaults and USER CODE preservation stays unchanged.
- Documentation now describes the small generated listener pattern, shared callback behavior, and signature-kind mapping.

## Build validation checklist

- [x] Build the main `VisiForm` project with `build-static-debug`
- [x] Fix any compile errors introduced by this phase
- [x] Do not run `VisiForm.exe`
- [x] Do not launch the generated app

## Manual test checklist

- [ ] Export a project and verify generated `MainWindow.h` contains `WidgetEvent`
- [ ] Verify generated base handler declarations include `WidgetEvent`
- [ ] Verify generated user subclass overrides include `WidgetEvent`
- [ ] Verify generated emit/helper methods include correct `senderId`, `senderName`, and `senderType`
- [ ] Verify two widgets can share one compatible callback and generated code emits a single handler declaration/definition
- [ ] Verify incompatible callback signature reuse still fails cleanly during export
- [ ] Verify USER CODE preservation still works after re-export
- [ ] Build the exported project manually in Debug and Release

## Final result summary

Completed.

- The generated listener API now includes sender-aware `WidgetEvent` plus per-widget emit helper methods in the generated base class.
- Handler signatures remain `WidgetEvent`-based and shared callbacks continue to collapse to one generated handler for compatible signatures.
- Emit helpers carry correct sender metadata so future interactive widget hookups can dispatch through a small practical listener foundation.
- Conflict handling remains strict: the same callback name with incompatible signatures still fails export clearly.

Remaining TODOs:

- Manual export verification is still needed to inspect generated helper methods, shared callback emission, USER CODE preservation after re-export, and exported project Debug/Release builds.
