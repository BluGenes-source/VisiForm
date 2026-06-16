# Phase 91 Property Inspector Events Prototype

Open `index.html` in a browser to review the interactive prototype.

## What It Demonstrates

- The existing VisiForm desktop shell with a focused Property Inspector.
- Fixed `Properties` and `Events` tabs.
- Empty selection messaging.
- A selected `Button` with blank event handlers.
- A selected `Button` with `onClick` assigned.
- Creating a new handler from an event row with a proposed default name based on widget name and event name.
- Selecting an existing compatible handler from a signature-filtered dropdown.
- Removing an event assignment immediately with `Clear`.
- Row-local invalid C++ identifier and incompatible duplicate-handler conflict messaging.

## Review Notes

The prototype intentionally keeps the Phase 89 implementation model: event assignments are still string properties edited through the inspector. The revised interaction removes direct handler-name text entry from the main row and exposes explicit `Create`, `Existing`, and `Clear` controls instead.

Compatible duplicate handler reuse is allowed by current VisiForm rules. The `Existing` dropdown therefore lists only matching signature handlers, while the conflict state demonstrates an imported or otherwise pre-existing incompatible assignment that validation/export should explain beneath the affected row.

## Interaction States

- `No widget selected`: the Events tab remains visible and shows a clear empty state.
- `Button, no events`: all Button event rows show `<unset>` plus compact `Create`, `Existing`, and `Clear` actions.
- `Button, click assigned`: `onClick` shows the assigned handler and keeps the same compact row actions available.
- `Create`: assigns a proposed default handler such as `handleHelloButtonClick`.
- `Existing`: opens a row-local dropdown containing only compatible `void_event` handlers.
- `Clear`: removes the assignment immediately without confirmation.
- `Validation error`: displays the invalid-name message directly beneath `On Click`.
- `Duplicate conflict`: displays the incompatible-signature conflict directly beneath `On Click`.

## Open Design Questions

- Should `Create` assign the proposed default immediately, or open a small rename field/dialog before committing?
- Should the `Existing` dropdown stay inline as shown here, or reuse the current inspector dropdown/suggestion surface?
- Should `Clear` be disabled when a row is already unset, or remain enabled as a harmless no-op?
