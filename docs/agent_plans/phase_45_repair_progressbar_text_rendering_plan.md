# Phase 45 repair ProgressBar text rendering plan

## Goal

Fix `ProgressBar` text rendering so it is always visible and predictable in the designer, the bottom export progress display, and generated preview rendering while preserving the `text` property for optional custom labels.

## Current remaining bugs

- `ProgressBar` currently shows no visible text in at least one rendering path.
- The current contrast/rendering strategy is not reliably showing percent or custom text.
- Generated `ProgressBar` preview rendering may have the same issue.

## Files to inspect

- `src/ui/DesignerCanvas.h`
- `src/ui/DesignerCanvas.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/model/WidgetDefinition.h`
- `src/model/WidgetRegistry.h`
- `src/model/WidgetNode.h`
- `src/generator/VisageCppEmitter.cpp`
- `docs/widget_catalog.md`
- `docs/project_file_format.md`
- `docs/code_generation.md`
- `docs/agent_plans/phase_45_repair_progressbar_text_rendering_plan.md`

## Step-by-step TODO list with checkboxes

- [x] Create persistent phase plan file before code changes
- [x] Inspect current ProgressBar text rendering paths and document why text disappears
- [x] Add or repair a shared helper for ProgressBar display text behavior
- [x] Fix DesignerCanvas ProgressBar text rendering
- [x] Fix bottom export progress bar text rendering
- [x] Fix generated ProgressBar rendering in `VisageCppEmitter`
- [x] Verify ProgressBar default property behavior still matches intended rules
- [x] Update ProgressBar documentation
- [x] Build with `build-static-debug`
- [x] Write final result summary

## Current progress notes

- Phase plan file created before edits.
- Diagnosis: ProgressBar text disappeared because the previous contrast repair moved text into alternative side areas and status-pane variants instead of using the intended in-bar text behavior. That made the display inconsistent and effectively invisible in tested cases.
- `showText` and `text` properties were still present and readable, but the drawing strategy was no longer aligned with the intended behavior of showing either centered percent text or centered custom text.
- The repair now uses one predictable rule set across all ProgressBar paths: if `showText` is false, draw nothing; else show custom `text` when non-empty; else show percent text.
- DesignerCanvas now draws centered ProgressBar text directly over the bar using a single readable color chosen by fill percent.
- Bottom export progress display now uses the same reliable centered-text-over-bar approach.
- Generated ProgressBar preview rendering now mirrors the same behavior.

## Build validation checklist

- [x] Build the main `VisiForm` project with `build-static-debug`
- [x] Fix any compile errors introduced by this phase
- [x] Do not run `VisiForm.exe`
- [x] Do not launch the generated app

## Manual test checklist

- [ ] New `ProgressBar` with default properties shows `25%`
- [ ] `ProgressBar` value `10` shows readable `10%`
- [ ] `ProgressBar` value `50` shows readable `50%`
- [ ] `ProgressBar` value `90` shows readable `90%`
- [ ] `ProgressBar` text set to `Loading...` shows `Loading...`
- [ ] `ProgressBar` `showText = false` hides the label
- [ ] Bottom export progress bar shows readable text during export
- [ ] Generated `ProgressBar` preview shows readable text
- [ ] Save/load preserves `showText` and `text`

## Final result summary

Completed.

- ProgressBar text rendering now follows the intended rules in all relevant paths.
- If `showText = true` and `text` is empty, the ProgressBar shows percent text such as `25%`.
- If `showText = true` and `text` is non-empty, the ProgressBar shows the custom text value.
- If `showText = false`, the ProgressBar shows no text.
- Designer preview, bottom export progress display, and generated ProgressBar preview now all use a predictable centered text strategy with simple readable contrast.

Remaining TODOs:

- Manual Visual Studio verification is still needed for default `25%`, values `10/50/90`, custom text like `Loading...`, `showText = false`, export progress readability, save/load preservation, and generated project preview behavior.
