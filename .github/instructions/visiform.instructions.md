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
- Do not run repository build scripts, generated build scripts, terminal build commands, PowerShell build commands, or `cmd.exe` build commands unless the developer explicitly asks for that exact command
- For automated validation, use only the Visual Studio workspace build pipeline for the main `VisiForm` target when it is available and unambiguous
- If the Visual Studio workspace build pipeline is unavailable, ambiguous, or appears to target a dependency such as `freetype.vcxproj`, stop and ask the developer to build manually
- Never select, build, launch, or validate against `freetype.vcxproj`; the intended application target is `VisiForm`
- Keep UI (Visage) code in `src/ui/` layer; keep model/serialization/generator independent of Visage
- Create or update persistent phase plans in `docs/agent_plans/` when performing multi-step changes
- Every phase must create a new `docs/agent_plans/phase_N_<name>_plan.md`
- The phase plan must include a markdown TODO checklist
- Update the phase plan as work progresses
- Mark completed checklist items in the phase plan with checked boxes
- The phase plan must include build validation status, including whether validation was completed through the Visual Studio workspace pipeline or deferred to the developer
- Agent Mode must not use scripts or terminal commands to build before finishing unless explicitly requested; if it cannot safely use the Visual Studio workspace build pipeline for `VisiForm`, it must ask the developer to build manually
- Write the final result summary into the phase plan before finishing
- Do not finish by asking whether to create or update the phase plan
- Do not claim completion unless the phase plan already contains progress updates, build validation, and the final result summary
- Summarize any remaining TODOs in the phase plan file
- Preserve `MainWindow` generated base class rule and USER CODE regions
- Use `WidgetRegistry` to add widget types
- Update docs when adding features
- Ensure the repository is ready for the developer's Visual Studio build workflow and fix compile errors reported by the developer or by the approved workspace build pipeline
- Do not launch generated apps from Agent Mode
