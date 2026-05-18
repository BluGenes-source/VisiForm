---
applyTo: "**/*"
---

# VisiForm path-specific Copilot instructions

These instructions apply to all files in the repository and mirror the central Copilot rules for VisiForm.

- Project: VisiForm (C++ / Visage)
- Primary IDE: Visual Studio 2022
- Use CMake presets and Ninja generator for CI/development
- Preserve static runtime and vcpkg triplet unless explicitly asked
- Do not run or automate execution of `VisiForm.exe` from automated agents
- Build main project with `build-static-debug` only
- Keep UI (Visage) code in `src/ui/` layer; keep model/serialization/generator independent of Visage
- Create or update persistent phase plans in `docs/agent_plans/` when performing multi-step changes
- Every phase must create or update `docs/agent_plans/phase_N_<name>_plan.md`
- Update the phase plan as work progresses
- Mark completed checklist items in the phase plan with checked boxes
- Write the final result summary into the phase plan before finishing
- Do not finish by asking whether to create or update the phase plan
- Summarize any remaining TODOs in the phase plan file
- Preserve `MainWindow` generated base class rule and USER CODE regions
- Use `WidgetRegistry` to add widget types
- Update docs when adding features
- Ensure builds succeed after changes
