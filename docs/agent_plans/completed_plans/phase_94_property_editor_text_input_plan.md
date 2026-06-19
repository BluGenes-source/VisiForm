# Phase 94 Property Editor Text Input Plan

## Scope

- Fix shared property text editor Delete handling and key routing.
- Correct property text editor caret hit testing and rendering to use real text metrics.
- Add a single active-editor caret blink timer.
- Add persistent versioning rules and update authoritative version records after successful build validation.
- Fix the runtime DPI regression exposed by Phase 94 text measurement so `TextEditControl`
  never measures with a zero-DPI `visage::Font`.

## Targeted Files

- `src/ui/editors/TextEditControl.h`
- `src/ui/editors/TextEditControl.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `src/utils/UiTimer.h`
- `src/utils/UiTimer.cpp`
- `CMakeLists.txt`
- `AGENTS.md`
- `docs/versioning.md`
- `docs/project_status.md`
- `src/app/Version.h`

## TODO Checklist

- [x] Inspect current branch and worktree.
- [x] Read Phase 94 brief, `AGENTS.md`, `docs/project_status.md`, and Phase 93 plan.
- [x] Confirm current version declarations.
- [x] Implement shared text editor Delete routing fix.
- [x] Implement font-metric caret hit testing and rendering.
- [x] Implement single active-editor caret blink timer.
- [x] Trace the launch-time `Font::dpiScale()` assertion to the Phase 94 text
      measurement path.
- [x] Initialize and refresh the shared text editor metrics font with a valid DPI.
- [x] Reapply text editor metrics when window DPI/canvas settings change.
- [x] Add compact versioning rule to `AGENTS.md`.
- [x] Run focused static validation.
- [ ] Build the normal Windows Debug `VisiForm` target if an approved path is available.
- [ ] After successful build validation, bump `1.0.0` to `1.0.1`.
- [x] Record final validation, version result, and remaining issues.

## Validation Plan

- Review the focused diff for text input routing, caret math, timer lifetime, and version locations.
- Run `git diff --check`.
- Build normal Windows Debug `VisiForm` once if permitted by repository rules and command availability.
- Manual launch/runtime checks remain developer-owned unless the developer explicitly performs them, because repository rules prohibit automated agents from launching `VisiForm.exe`.

## Timer Design

Use one `utils::UiTimer` owned by `MainWindow`, enabled only while the shared `TextEditControl` is active and focused. Timer ticks toggle caret visibility inside the editor and request a repaint of the active editor overlay. The timer is stopped whenever the editor is cleared, focus leaves the editor, or editing is committed/cancelled, so no property row owns a timer.

## Runtime DPI Regression

- Confirmed root cause: `MainWindow::loadLabelFont()` constructed `labelFont_`
  with `visage::Font(18.0f, std::string{ fontPath })`, leaving Visage
  `Font::dpi_scale_` at its default constructor argument of `0.0f`.
- Phase 94 exposed this latent issue because `TextEditControl::draw()` began
  storing `labelFont_` as `metricsFont_`, then calling `stringWidth()` and
  `lineHeight()` through caret hit testing, selection drawing, caret drawing,
  horizontal scrolling, focus-entry setup, and timer-triggered redraws.
  `Font::dpiScale()` asserts before returning its existing `1.0f` fallback, so
  the zero-DPI cached metrics font aborted during normal application drawing.
- Initialization-order fix: `MainWindow` now creates the label font with
  `normalizedDpiScale(dpiScale())`, refreshes the text editor metrics font after
  font load, before property/editor-modal text edit begin, during
  `applyCanvasSettings()`, and in `dpiChanged()`. `TextEditControl::draw()` also
  refreshes the metrics font from the live `Canvas::dpiScale()` before any text
  measurement in that draw pass.
- Defensive handling: `TextEditControl` normalizes invalid or missing DPI to an
  explicit `1.0f` before creating a metrics font. Visage already contains a
  `Font::dpiScale()` runtime return fallback to `1.0f` after its diagnostic
  assert; generated dependency files under `build/` were not modified.
- Measurement-path check: caret hit testing, caret drawing, selection drawing,
  horizontal text scrolling, focus-entry initialization, and caret blink repaint
  all route through `metricsFont_` or deliberately fall back to approximate
  widths only when no drawable font exists.

## Build / Validation Status

- Current branch: `main`
- Starting version: `1.0.0`
- Static validation: `git diff --check` passed with CRLF normalization warnings only.
- Build: attempted with `cmake --build --preset build-static-debug`, then
  `cmake --build --preset build-static-debug --parallel 1`. Both attempts failed
  before source diagnostics with MSVC `C1041` opening
  `build/vs2022-x64-static-debug/CMakeFiles/VisiForm.dir/vc140.pdb`; Ninja also
  reported `.ninja_lock` write permission failures. No `VisiForm.exe` launch was
  attempted.
- Manual runtime validation: not performed by the agent. Repository rules still
  prohibit automated launch of `VisiForm.exe`; developer runtime verification is
  required for the launch/focus/caret/type/delete/selection/DPI checklist.
- Version result: unchanged at `1.0.0`; `Build` was not incremented because required build validation did not run/pass.

## Final Summary

- Root cause of Delete-key failure/risk: the reusable text editor owned Delete behavior, but app-level routing could still proceed outside the focused text-control path. Phase 94 keeps Delete/Backspace inside `TextEditControl` and routes text input only while the editor is focused so global widget deletion cannot consume focused text-edit Delete.
- Text editing now erases selected ranges for Backspace/Delete and erases the previous or next UTF-8 boundary for unselected Backspace/Delete.
- Caret hit testing now uses measured insertion-boundary widths from the active Visage font instead of fixed-width character estimates, including horizontal padding and scroll offset.
- Caret drawing uses the same measured cursor offset as hit testing.
- Caret blinking uses one `utils::UiTimer` owned by `MainWindow` and active only while the shared `TextEditControl` is focused.
- The shared editor metrics font now receives a nonzero DPI before measurement,
  refreshes on edit begin, canvas settings, live draw, and window DPI changes,
  and falls back to an explicit `1.0f` DPI only when no valid DPI is available.
- `AGENTS.md` now documents the `Major.Minor.Build` version policy and build-gated version increment workflow.

## Remaining Issues

- Build validation remains required before Phase 94 can be considered complete;
  the current agent build attempts are blocked by generated Debug PDB/lock-file
  state rather than a reported source compile error.
- Manual runtime checks from the Phase 94 brief remain required because
  automated agents must not launch `VisiForm.exe`.
- Version declarations remain `1.0.0`; update to `1.0.1` only after successful approved build validation.
- Caret blink ticks currently request a `MainWindow` redraw because `TextEditControl` is a lightweight overlay, not a child `Frame` with independent invalidation.
- `docs/project_status.md` was not updated because version completion recording is gated on successful build validation, and the current file contains stale prompt text rather than a maintained project-status snapshot.
