# Copilot custom instructions and repository guidance

This document explains where Copilot/GitHub custom instructions live for the VisiForm repository and how they are used.

Files

- `.github/copilot-instructions.md` - repository-level instructions that describe project constraints and safe automation practices for Copilot/agents.
- `.github/instructions/visiform.instructions.md` - optional path-specific instructions that apply to all files via frontmatter and are read by Copilot setups that support path-specific instructions.
- `docs/agent_plans/` - persistent phase plans created/updated by automated agents for multi-step work.

Visual Studio

- Visual Studio 2022 is the primary IDE for VisiForm. Copilot custom instruction files are plain Markdown and are visible in Solution Explorer when the repository is opened as a folder.
- Some Copilot integrations may require enabling custom instructions in Visual Studio settings. If suggestions seem out of scope, verify that these files exist and are visible in the repository root.

Why agent plans live in `docs/agent_plans/`

- Agent plans are long-lived artifacts that document multi-step automated changes. Storing them in `docs/agent_plans/` makes them visible to developers and preserved in the repository history.

