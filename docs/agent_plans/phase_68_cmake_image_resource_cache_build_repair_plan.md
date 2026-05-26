## Phase title

Repair the missing `ImageResourceCache` build integration so CMake can generate and the main `VisiForm` target can build again.

## Current state

- CMake generation fails because `src/ui/resources/ImageResourceCache.cpp` is listed in `CMakeLists.txt` but does not exist.
- `src/ui/MainWindow.h`, `src/ui/MainWindow.cpp`, and `src/ui/DesignerCanvas.cpp` already reference `ui/resources/ImageResourceCache.h` APIs.
- The image-preview phase is only partially integrated in the current workspace.

## Goal

- Restore the missing UI-layer image cache implementation files.
- Reconcile any local call-site drift caused by the partial integration.
- Validate the main `VisiForm` app with the required `build-static-debug` workflow.

## Files to inspect

- `CMakeLists.txt`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/model/ProjectDocument.h`
- `src/model/ProjectResource.h`
- `src/ui/resources/ImageResourceCache.h`
- `src/ui/resources/ImageResourceCache.cpp`
- `docs/agent_plans/phase_68_cmake_image_resource_cache_build_repair_plan.md`

## TODO checklist

- [x] Create the new phase plan before changing code.
- [ ] Inspect the current image cache call sites and required model dependencies.
- [ ] Implement `src/ui/resources/ImageResourceCache.h`.
- [ ] Implement `src/ui/resources/ImageResourceCache.cpp`.
- [ ] Reconcile `MainWindow` resource preview state with the restored cache API.
- [ ] Validate touched files for compile issues.
- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Fix any compile errors introduced by this phase.
- [ ] Confirm the main `VisiForm` target built successfully.
- [ ] Confirm `VisiForm.exe` was not run.
- [ ] Confirm no generated apps were launched.
- [ ] Update this phase plan with the final result summary and remaining TODOs.

## Build validation checklist

- [ ] Configure using the existing main static debug preset flow.
- [ ] Build the main `VisiForm` app with `build-static-debug`.
- [ ] Confirm the main `VisiForm` target built successfully.
- [ ] Confirm no new compile errors remain from this phase.
- [ ] Confirm `VisiForm.exe` was not run.
- [ ] Confirm no generated apps were launched.

## Final result summary

- In progress.

## Remaining TODOs

- Inspect the partial image-cache integration.
- Restore the missing files.
- Run the required build validation.
