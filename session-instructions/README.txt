Session instruction prompts for VisiForm phase work.

Keep this folder focused on active prompts and reusable notes.

Active files:
1. notes.txt - reusable debugging notes. Do not archive this file.
2. phase_81_export_build_and_ui_repair_prompt.txt - broad Phase 81 prompt; keep active while its plan remains pending.
3. phase_81C_palette_button_tree_tests_prompt.txt - pending split prompt.

Archived files:
- Completed dated prompts belong in old/.
- Do not archive active, pending, or ambiguous prompts until completion is clear.
- See AGENTS.md for the session-instruction accounting rules.

Guardrails for all split prompts:
- Do not edit anything under Generated/.
- Do not create temporary scripts, helper scripts, diagnostic scripts, or one-off files.
- Do not run VisiForm.exe.
- Do not launch generated applications.
- Do not run build scripts.
- Do not run terminal build commands.
- If validation is needed, use only the Visual Studio workspace build pipeline for the VisiForm target.
- If the Visual Studio workspace build pipeline is unavailable or ambiguous, stop and ask the user to build manually.
- Never select, build, launch, or validate against `freetype.vcxproj`; the intended app target is VisiForm.
- Preserve Windows CRLF line endings. Do not mix LF and CRLF in one file.
