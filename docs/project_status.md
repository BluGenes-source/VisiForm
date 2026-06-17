> Phase 95 note (2026-06-17): Phase 95 implementation is ready for developer validation. The Widget Palette is now a top tabbed, registry-backed control with `Common`, `Containers`, `Layout`, `Forms`, `Data`, `Menu/Toolbar`, and `Additional` categories. The phase version is `1.0.1`, updated from `1.0.0` in `CMakeLists.txt`, `src/app/Version.h`, and `docs/versioning.md`. Static diff validation passed; the main `VisiForm` build and manual runtime checklist remain pending. This file still contains stale project-status creation prompt text below and should be rewritten into the authoritative status snapshot in a dedicated documentation pass.

Create an authoritative VisiForm project status document at:

`docs/PROJECT_STATUS.md`

This task is documentation and repository analysis only. Do not implement new features, refactor working code, or begin a new development phase.

Follow all repository instructions, including:

- `AGENTS.md`
- `copilot-instructions.md`
- `visiform.instructions.md`
- Any agent configuration files
- Existing documentation conventions
- Existing phase-numbering rules

# Objective

Inspect the current VisiForm repository and create a factual, evidence-based snapshot of the project as it exists now.

The document will become the primary source of truth used by ChatGPT, Codex, contributors, and future development agents before suggesting or planning new phases.

Do not rely only on README files, roadmaps, or phase documents. Compare documentation claims against the current source code, build configuration, tests, and resources.

# Phase Handling

This documentation task should not automatically create a new development phase.

The current known completed phase is:

Phase 90 - Windows Subsystem and Application Icon

Verify this against the repository.

If the repository shows a different current phase, document the discrepancy clearly rather than silently changing phase records.

Do not assign Phase 91 to this task unless the repository's established rules explicitly require documentation-only work to receive a phase number.

# Repository Inspection

Inspect at least the following, where present:

- `README.md`
- `AGENTS.md`
- `copilot-instructions.md`
- `visiform.instructions.md`
- `CMakeLists.txt` and related CMake files
- Visual Studio project files, if checked in
- `src/`
- `include/`
- `docs/`
- phase documentation
- `tests/`
- `assets/`
- `resources/`
- `tools/`
- `examples/`
- `scripts/`
- packaging or installer files
- Git history when available and useful
- Current build targets and platform-specific files

Use the configured VisiForm subagents where appropriate.

Explorer should perform the main repository inventory.

Architect should summarize the current architecture and major subsystems.

Reviewer should verify that status claims are supported by code or documentation.

Documenter should create and organize `docs/PROJECT_STATUS.md`.

# Evidence Rules

Every significant claim in `PROJECT_STATUS.md` should be traceable to repository evidence.

For important features and architectural claims, reference relevant files using repository-relative paths.

Examples:

- `src/designer/DesignerCanvas.cpp`
- `include/visiform/WidgetModel.h`
- `docs/phases/phase-090.md`
- `resources/windows/VisiForm.rc`

Do not claim that a feature is complete merely because it appears in a roadmap or old phase plan.

Use the following confidence labels where status cannot be proven fully:

- Verified
- Partially verified
- Documented but not verified
- Present but apparently incomplete
- Not found
- Requires manual runtime verification

# Required Document Structure

Create `docs/PROJECT_STATUS.md` with the following structure.

## 1. Document Purpose

Explain that this file is the authoritative current-status snapshot for VisiForm.

State that it should be reviewed before:

- proposing a new phase
- planning a feature
- investigating whether functionality already exists
- creating Codex prompts
- modifying architecture
- reporting project progress

Include the date the repository inspection was performed.

## 2. Executive Summary

Provide a concise summary covering:

- what VisiForm is
- current development maturity
- current completed or active phase
- primary framework and language
- supported platform or platforms
- build system
- major working capabilities
- most important incomplete or uncertain areas

## 3. Current Phase Status

Document:

- highest phase found
- phase title
- whether it appears complete
- evidence supporting completion
- any phase-number conflicts or missing phase documents
- any work present in source code but absent from phase documentation

Include Phase 90 explicitly and verify:

- Windows subsystem conversion
- removal of the normal console window
- application icon resource
- icon source and generated ICO locations
- Windows resource integration
- current application entry point

## 4. Build and Development Environment

Document what can be determined about:

- programming language and standard
- GUI framework
- CMake version requirements
- compiler and Visual Studio expectations
- supported architectures
- Debug and Release configurations
- external dependencies
- package-management approach
- build output locations
- test targets
- platform-specific requirements
- icon-generation or other asset-generation tools

Include exact repository-relative file references.

Do not invent versions that cannot be verified.

## 5. Repository Structure

Provide a concise tree or organized list of important directories.

For each major directory, explain its purpose.

Highlight:

- application entry point
- main-window implementation
- designer surface
- widget implementations
- property inspector
- event editing
- sizer or layout system
- serialization
- code generation
- undo/redo
- clipboard support
- tests
- phase documentation
- Windows resources
- application assets

Only include areas that actually exist.

## 6. Current Architecture

Describe the application architecture as implemented.

Include, where present:

- startup and shutdown flow
- main application/window ownership
- document or project model
- form model
- widget model
- designer canvas
- selection model
- property inspector
- event inspector or event editing
- command or undo system
- serialization
- preview system
- code generation
- sizer/layout architecture
- platform abstraction
- logging and error handling
- resource loading

Provide a high-level flow such as:

application entry point  
-> application initialization  
-> main window  
-> project/form model  
-> designer and inspectors  
-> persistence or code generation

Use actual class and file names.

## 7. Implemented Feature Inventory

Create a table with these columns:

| Feature | Status | Evidence | Notes |
| --- | --- | --- | --- |

Use statuses such as:

- Verified
- Partially verified
- Documented but not verified
- Requires runtime verification

Inspect and report on features including, where relevant:

- form creation
- widget placement
- widget selection
- moving and resizing
- multi-selection
- property editing
- event editing
- widget hierarchy or object tree
- sizers and layout containers
- nested sizers
- alignment and distribution
- cut, copy, and paste
- duplicate
- undo and redo
- project save and load
- autosave or recovery
- recent projects or files
- preview or test mode
- code generation
- menu system
- toolbar
- status bar
- theming
- preferences or settings persistence
- grid and snapping
- keyboard shortcuts
- application icon
- Windows subsystem startup
- logging
- tests

Do not mark a feature missing until the repository has been searched for likely alternate names and implementations.

## 8. Partially Implemented or Unverified Features

List features that appear present but incomplete, uncertain, or dependent on manual testing.

For each item include:

- observed implementation
- missing or uncertain behavior
- relevant files
- recommended verification step

Separate source-code uncertainty from runtime uncertainty.

## 9. Known Bugs and Limitations

Gather known issues from:

- phase documents
- `TODO` and `FIXME` comments
- tests
- issue templates
- project notes
- disabled code
- incomplete stubs
- recent bug-fix documentation

For each issue provide:

- description
- status
- severity if evident
- relevant files
- whether it is confirmed current or possibly stale

Do not present an old fixed bug as current unless evidence shows it still exists.

## 10. Testing Status

Document:

- test framework
- available test targets
- approximate areas covered
- areas apparently lacking tests
- whether tests were run during this task
- exact test command, when known
- results of any tests run
- limitations of the current environment

Do not claim runtime validation that was not performed.

## 11. Documentation Status

List major documentation currently available, including:

- `README`
- architecture documents
- phase index
- phase documents
- contributor instructions
- build instructions
- user documentation
- developer documentation

Identify:

- missing documentation
- stale documentation
- duplicate or conflicting documents
- phase gaps
- documents that should be treated as historical rather than authoritative

## 12. Current Assets and Windows Resources

Document:

- source icon location
- generated ICO location
- resource header
- resource script
- resource identifier
- icon-generation script
- executable-resource integration
- other branding assets

Verify the Phase 90 paths rather than assuming them.

## 13. Current Priorities

Derive a small list of priorities only from evidence already in the repository.

Group them as:

- Immediate maintenance
- Incomplete current work
- Architecture or quality improvements
- Candidate future features

Do not invent a detailed future roadmap.

Do not assign phase numbers to candidate work.

## 14. Recommended Manual Verification Checklist

Provide a checklist for features that cannot be proven by static inspection.

Examples may include:

- application launches without a console
- executable and taskbar icon
- drag-and-drop placement
- widget resizing
- sizer child resizing
- save and reload fidelity
- undo and redo
- event editing
- generated code compilation
- preview mode
- multi-selection
- clipboard operations

Include only applicable items.

## 15. Source-of-Truth Rules

Add a permanent section stating:

1. `docs/PROJECT_STATUS.md` describes the current verified project state.
2. Phase documents describe historical implementation work.
3. Source code and tests take precedence when documentation conflicts with implementation.
4. Roadmaps and plans do not prove that a feature was implemented.
5. Before proposing a new phase, agents must inspect:
   - `docs/PROJECT_STATUS.md`
   - the feature inventory
   - the phase index
   - relevant current source code
6. New completed phases must update this status document.
7. Bugs fixed in later phases must be removed from the current-bugs section or clearly marked resolved.
8. Unverified features must not be described as complete.

## 16. Update History

Add an initial entry containing:

- date
- author or agent
- reason for update
- inspected phase range
- summary of major findings

# Accuracy Requirements

- Be conservative.
- Prefer "not verified" over unsupported certainty.
- Distinguish implemented code from tested behavior.
- Distinguish planned work from completed work.
- Do not infer features solely from filenames.
- Search for alternate terminology before deciding a feature is absent.
- Do not modify application behavior.
- Do not delete or rewrite historical phase documentation.
- Do not manufacture test results.
- Do not report successful builds unless a build actually ran successfully.

# Optional Supporting Document

If `docs/PROJECT_STATUS.md` becomes excessively long, create:

`docs/FEATURE_INVENTORY.md`

Place the detailed feature table there and link it from `PROJECT_STATUS.md`.

However, `PROJECT_STATUS.md` must remain useful as a standalone overview.

Do not create multiple redundant status documents.

# Validation

Before finishing:

1. Confirm `docs/PROJECT_STATUS.md` exists.
2. Confirm all file references use repository-relative paths.
3. Confirm Phase 90 is represented accurately.
4. Confirm plans are not mislabeled as implemented features.
5. Confirm old fixed bugs are not listed as current without evidence.
6. Confirm uncertain claims have confidence labels.
7. Confirm no application source files were changed unless absolutely necessary to correct a broken documentation reference.
8. Run Markdown linting if the repository provides it.
9. Review the final document for internal contradictions.
10. Report which files were created or modified.

Complete the repository inspection and write the document. Do not stop after proposing an outline.
