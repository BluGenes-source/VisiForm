# Phase 64 - Repository README Setup Guide

## Current problem

`README.md` is empty or incomplete, so the repository does not currently provide a reliable Windows setup guide for cloning, configuring, building, and running `VisiForm` with Visual Studio 2022.

## Goal

Create a complete repository `README.md` that explains how to clone, configure, build, and run `VisiForm` on Windows with Visual Studio 2022, while preserving existing project constraints and documented export workflows.

## Files to inspect

- `README.md`
- `CMakeLists.txt`
- `CMakePresets.json`
- `CMakeUserPresets.json` if present
- `vcpkg.json`
- `.gitignore`
- `docs/code_generation.md`
- `docs/settings.md`
- `docs/resources.md`
- `docs/menu_bar.md`
- `docs/new_project_wizard.md`
- `docs/project_validation.md`
- `docs/copilot_rules.md`
- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `docs/agent_plans/phase_64_repo_readme_setup_guide_plan.md`

## TODO checklist

- [x] Inspect the repository setup and documentation files listed above.
- [x] Draft a complete `README.md` for Windows setup, cloning, presets, build, run, export, and troubleshooting.
- [x] Verify the README documents local `Visage` source usage, `vcpkg`, `Ninja`, and Visual Studio 2022 workflows.
- [x] Confirm `CMakeUserPresets.json` handling is documented as local-only and ignored by Git.
- [x] Update this phase plan with progress after the README is written.
- [x] Build the main `VisiForm` app with `build-static-debug`.
- [x] Fix any compile errors introduced by this phase.
- [x] Update this phase plan with build validation, manual review notes, final result summary, and remaining TODOs.

## Build validation checklist

- [x] Build `VisiForm` with the `build-static-debug` preset.
- [x] Confirm the main `VisiForm` app built successfully.
- [x] Confirm no generated apps or `VisiForm.exe` were launched during Agent Mode.

## Manual test checklist

- [x] Review `README.md` for completeness against the requested section list.
- [x] Verify the documented Visual Studio 2022 `File > Open > Folder` workflow reads clearly.
- [x] Verify the documented command-line workflow reads clearly for Developer Command Prompt users.
- [x] Verify the documented export workflow matches current generated project behavior.
- [x] Verify placeholder repository clone URL guidance is clear.

## Final result summary

`README.md` was replaced with a complete Windows-focused setup guide for `VisiForm`. The new guide covers repository purpose, current feature status, OS and software requirements, `winget` install examples, recommended folder layout, clone steps for `VisiForm` and `Visage`, `vcpkg` setup, shared and local CMake presets, `.gitignore` guidance, Visual Studio 2022 build steps, command-line build steps, running notes, generated export workflow, local `Visage` export behavior, project-file expectations, troubleshooting, and documentation links.

Build validation was completed with the main `build-static-debug` workflow after entering the Visual Studio 2022 x64 developer environment. The main `VisiForm` app built successfully. No generated apps were launched, and `VisiForm.exe` was not run during Agent Mode.

## Remaining TODOs

- None.
