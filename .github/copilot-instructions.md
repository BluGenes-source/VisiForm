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
- Build only the main project with the `build-static-debug` workflow unless requested otherwise.
- Always create or update a persistent phase plan file in `docs/agent_plans/` for multi-step changes.
- Keep model, serialization, and generator layers free of Visage UI headers; isolate UI-specific code in the `ui/` layer.
- Generated projects must not be compiled into the main `VisiForm` target.
- Preserve the generated base class name rule: `MainWindow`.
- Preserve `USER CODE BEGIN` / `USER CODE END` region markers when exporting.
- Do not delete arbitrary user files during export; only overwrite known generated files.
- Use `WidgetRegistry` to register new widget types and update `WidgetDefinition` accordingly.
- Update documentation when adding features or changing export behavior.
- Ensure repository builds successfully after changes and fix compile errors if introduced.

Contact / Notes

- These instructions are used by automated coding assistants to keep changes safe and consistent with project conventions.
