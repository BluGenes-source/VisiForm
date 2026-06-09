# VisiForm - GitHub Copilot Instructions

These instructions provide stable rules for automated agents and Copilot when working on the VisiForm repository.

Project overview

- Project: VisiForm
- Language: C++ (C++17/C++20)
- UI: Visage
- Build system: CMake (Ninja generator used in CI & developer flows)
- Primary IDE: Visual Studio 2022

Key rules for automated agents

- Do not change the `VisiForm` target name or the produced executable name `VisiForm.exe`.
- Do not change CMake presets or the vcpkg triplet unless explicitly requested.
- Do not change `CMAKE_MSVC_RUNTIME_LIBRARY` or static runtime settings unless explicitly requested.
- Do not run `VisiForm.exe` from automated agents; manual execution is done by the developer.
- Do not use Start-Process, automate keyboard input, or otherwise interact with the OS in a way that runs the built application.
- Do not run repository build scripts, generated build scripts, terminal build commands, PowerShell build commands, or `cmd.exe` build commands unless the developer explicitly asks for that exact command.
- For automated validation, use only the Visual Studio workspace build pipeline for the main `VisiForm` target when it is available and unambiguous.
- If the Visual Studio workspace build pipeline is unavailable, ambiguous, or appears to target a dependency such as `freetype.vcxproj`, stop and ask the developer to build manually.
- Never select, build, launch, or validate against `freetype.vcxproj`; the intended application target is `VisiForm`.
- Always create or update a persistent phase plan file in `docs/agent_plans/` for multi-step changes.
- Every phase must create a new `docs/agent_plans/phase_N_<name>_plan.md` file.
- The phase plan must include a markdown TODO checklist.
- The phase plan must be updated as work progresses.
- Completed checklist items in the phase plan must be marked with checked boxes.
- The phase plan must include build validation status, including whether validation was completed through the Visual Studio workspace pipeline or deferred to the developer.
- Agent Mode must not use scripts or terminal commands to build before finishing unless explicitly requested; if it cannot safely use the Visual Studio workspace build pipeline for `VisiForm`, it must ask the developer to build manually.
- The final result summary must be written into the phase plan file before finishing.
- Do not finish by asking whether to create or update the phase plan; it must already be updated.
- Do not claim completion unless the phase plan has already been updated with progress, build validation, and the final summary.
- Always summarize remaining TODOs in the phase plan file.
- Keep model, serialization, and generator layers free of Visage UI headers; isolate UI-specific code in the `ui/` layer.
- Generated projects must not be compiled into the main `VisiForm` target.
- Preserve the generated base class name rule: `MainWindow`.
- Preserve `USER CODE BEGIN` / `USER CODE END` region markers when exporting.
- Do not delete arbitrary user files during export; only overwrite known generated files.
- Use `WidgetRegistry` to register new widget types and update `WidgetDefinition` accordingly.
- Update documentation when adding features or changing export behavior.
- Ensure the repository is ready for the developer's Visual Studio build workflow and fix compile errors reported by the developer or by the approved workspace build pipeline.
- Do not launch generated apps from Agent Mode.

Contact / Notes

- These instructions are used by automated coding assistants to keep changes safe and consistent with project conventions.
