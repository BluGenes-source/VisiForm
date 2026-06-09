Session instruction prompts for VisiForm phase work.

Use these files as active Copilot prompts instead of the older broad Phase 81 prompt.

Recommended order:
1. phase_81A_generated_batch_script_fix_prompt.txt
2. phase_81B_menubar_toolbar_insert_menu_prompt.txt
3. phase_81C_palette_button_tree_tests_prompt.txt

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
