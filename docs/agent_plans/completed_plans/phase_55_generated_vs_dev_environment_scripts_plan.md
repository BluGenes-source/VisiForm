# Phase 55 - Generated VS dev environment scripts plan
# Phase 55 - Generated VS dev environment scripts plan

## Phase title
Generated VS dev environment scripts

## Current bug
Generated exported projects use the `Ninja` generator. Running `cmake --preset vs2022-x64-static-debug` from a normal PowerShell terminal fails with `No CMAKE_CXX_COMPILER could be found` because the Visual Studio C++ compiler environment is not loaded before CMake runs.

## Files to inspect
- `.github/copilot-instructions.md`
- `.github/instructions/visiform.instructions.md`
- `src/generator/CMakeEmitter.h`
- `src/generator/CMakeEmitter.cpp`
- `src/generator/CodeGenerator.cpp`
- `docs/code_generation.md`
- generated README emitter code if separate

## TODO checklist
- [x] Create this phase plan file with required sections.
- [x] Inspect repository Copilot instructions and target generator files.
- [x] Update generated `.cmd` configure scripts to locate Visual Studio with `vswhere`, load `VsDevCmd.bat`, switch to the generated project root, and fail with a nonzero exit code on errors.
- [x] Update generated `.cmd` build scripts to locate Visual Studio with `vswhere`, load `VsDevCmd.bat`, switch to the generated project root, and fail with a nonzero exit code on errors.
- [x] Optionally emit matching `.ps1` helper scripts if the change stays low risk.
- [x] Update the generated `README.md` content to document Visual Studio 2022, x64 Native Tools Command Prompt, and normal PowerShell usage with generated scripts.
- [x] Keep generated `CMakePresets.json` on `Ninja` unless implementation changes prove necessary.
- [x] Update `docs/code_generation.md` to describe the generated script behavior, compiler environment requirements, and local `VISIFORM_VISAGE_SOURCE_DIR` behavior.
- [x] Build the main `VisiForm` project with the `build-static-debug` workflow.
- [x] Fix any compile errors introduced by this phase.
- [x] Update this plan with build validation results, remaining TODOs, and the final result summary before finishing.

## Build validation checklist
- [x] Build the main `VisiForm` project with `build-static-debug`.
- [x] Confirm the main app build completed successfully.
- [x] Confirm no generated-project executables were launched.

### Latest build validation
- [x] Revalidated on 2026-05-20 by loading `VsDevCmd.bat` for `x64`, running `cmake --preset vs2022-x64-static-debug`, and then running `cmake --build --preset build-static-debug` from `J:\Dev\CeePlusPlus\VisiForm`.
- [x] Confirmed the main `VisiForm` target linked successfully as `VisiForm.exe` without introducing new phase 55 compile errors.
- [x] Confirmed no generated-project applications were launched during validation.

## Manual test checklist
- [ ] Export a generated project and inspect `scripts/configure_static_debug.cmd`.
- [ ] Run `scripts\configure_static_debug.cmd` from a normal PowerShell window in the exported project.
- [ ] Run `scripts\build_static_debug.cmd` from a normal PowerShell window in the exported project.
- [ ] Open the exported project with `Visual Studio 2022` using `File > Open > Folder` and select a generated preset.
- [ ] Configure and build from `x64 Native Tools Command Prompt for VS 2022`.
- [ ] Confirm the generated README explains the compiler environment requirement and local `VISIFORM_VISAGE_SOURCE_DIR` behavior.

## Final result summary
`src/generator/CMakeEmitter.cpp`, `src/generator/CodeGenerator.cpp`, and `docs/code_generation.md` already contain the requested phase 55 behavior: generated `.cmd` and optional `.ps1` helper scripts locate `Visual Studio 2022` with `vswhere`, load `VsDevCmd.bat` for `x64`, switch to the generated project root, and fail with a nonzero exit code on errors before invoking the `Ninja` presets; generated `README.md` content explains Visual Studio IDE usage, `x64 Native Tools Command Prompt for VS 2022`, normal PowerShell helper-script usage, and the `VISIFORM_VISAGE_SOURCE_DIR` local-source fallback behavior; generated `CMakePresets.json` remains on `Ninja` while continuing to emit the local Visage source path when configured. Revalidated the main `VisiForm` project successfully with the required `build-static-debug` workflow in this session; no additional phase 55 source edits were required, no new compile errors were introduced, and no generated-project executables were launched.

## Remaining TODOs
- Manual exported-project verification remains pending:
  - Export a generated project and inspect `scripts/configure_static_debug.cmd`.
  - Run `scripts\configure_static_debug.cmd` from a normal PowerShell window in the exported project.
  - Run `scripts\build_static_debug.cmd` from a normal PowerShell window in the exported project.
  - Open the exported project with `Visual Studio 2022` using `File > Open > Folder` and select a generated preset.
  - Configure and build from `x64 Native Tools Command Prompt for VS 2022`.
  - Confirm the generated README explains the compiler environment requirement and local `VISIFORM_VISAGE_SOURCE_DIR` behavior.
