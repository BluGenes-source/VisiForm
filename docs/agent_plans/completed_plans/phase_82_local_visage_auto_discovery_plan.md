# Phase 82 - Local Visage auto discovery

## Goal

Make the main VisiForm project and generated projects try nearby local Visage
source checkouts before falling back to `FetchContent`.

## Checklist

- [x] Inspect the existing root and generated CMake dependency logic.
- [x] Update root `CMakeLists.txt` to resolve local Visage candidates before
  `FetchContent`.
- [x] Update generated `CMakeLists.txt` emission with the same local discovery
  behavior.
- [x] Update generated-project documentation to describe auto-discovery.
- [x] Record validation status and final summary.

## Notes

Previous phases added support for an explicitly configured
`VISIFORM_VISAGE_SOURCE_DIR`, but CMake still went straight to `FetchContent`
when that setting was empty. This phase adds automatic local-source candidates
so common sibling checkouts such as `../visage` can be used without repeatedly
downloading Visage dependencies.

## Validation

Completed non-build validation:

- reviewed the root CMake dependency block
- reviewed the generated `CMakeLists.txt` emitter output
- ran `git diff --check`

Build/configure validation was not run because repository agent rules prohibit
terminal build/configure commands unless the developer explicitly requests the
exact command.

## Final Summary

Completed. The main repository `CMakeLists.txt` and generated project
`CMakeLists.txt` output now build a local Visage candidate list in this order:
configured `VISIFORM_VISAGE_SOURCE_DIR`, `VISIFORM_VISAGE_SOURCE_DIR`
environment variable, then nearby sibling checkout paths `../visage`,
`../../visage`, and `../../../visage`. The first candidate containing
`CMakeLists.txt` is used with `add_subdirectory(...)`; only when no valid local
source is found does CMake fall back to `FetchContent`. Updated
`README.md`, `docs/code_generation.md`, `docs/settings.md`, and the generated
README template text to describe the new behavior.
