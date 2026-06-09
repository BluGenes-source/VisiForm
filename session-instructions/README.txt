Session instruction prompts for VisiForm phase work.

Use these files as active Copilot prompts instead of the older broad Phase 81 prompt.

Recommended order:
1. phase_81A_generated_batch_script_fix_prompt.txt
2. phase_81B_menubar_toolbar_insert_menu_prompt.txt
3. phase_81D_generated_code_compile_repair_prompt is not present because the generator compile repair was handled directly; see `docs/agent_plans/phase_81D_generated_code_compile_repair_plan.md`
4. phase_81E_item_editor_hit_test_layout_prompt.txt
5. phase_81C_palette_button_tree_tests_prompt.txt

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
