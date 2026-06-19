# Phase 90 Windows Subsystem And Application Icon Plan

## Objective

Convert the Windows `VisiForm` application target from a console subsystem executable to a native Windows subsystem executable, preserve the existing startup lifecycle through shared runner logic, generate and embed a multi-resolution application icon, and apply that icon to the native main window on Windows.

## Scope

- Keep the existing `VisiForm` target and current cross-platform startup behavior intact.
- Refactor startup so shared application initialization is not duplicated between platform entry points.
- Replace console-only fatal startup reporting with Windows-safe diagnostics and user-visible startup error handling.
- Add a repeatable icon-generation script and commit the generated `.ico` asset.
- Add Windows resource files and CMake wiring so the executable embeds the icon.
- Apply the icon to the native main window after the underlying Win32 window exists.
- Update the project specification and this phase plan for the new startup/resource behavior.

## Requirements

- Phase numbering check: existing `docs/agent_plans/` files include Phase 89 and no Phase 90 plan, so Phase 90 is the correct next unused phase number.
- Preserve the current startup order: construct `visiform::App`, initialize the main window, show it, verify it opened, run the event loop, and cleanly shut down on exit or failure.
- Do not keep both Windows `main` and `wWinMain` active in the Windows GUI target.
- Preserve non-Windows entry behavior.
- Prevent the normal Windows build from showing a console window.
- Keep diagnostics available through a debugger-safe path on Windows.
- Keep fatal startup failures visible to the user before the main window exists.
- Confirm the source PNG exists at `assets/source/VisiFormIcon.png` and leave it unchanged.
- Generate `assets/icons/windows/VisiForm.ico` with multiple icon sizes.
- Add `resources/windows/resource.h` and `resources/windows/VisiForm.rc`.
- Integrate Windows-only resource and subsystem behavior conditionally in `CMakeLists.txt`.
- Avoid machine-specific absolute paths.

## Investigation Report

### 1. Repo and phase state

- `git status --short --branch` at phase start: `## main...origin/main`, plus untracked `assets/source/` and `session-instructions/phase 90.txt`.
- Current branch during this phase: `main`.
- Most recent relevant commit at phase start: `b0bae6c Add tabs to Property Inspector for properties and events`.
- The highest existing numbered plan is Phase 89: `docs/agent_plans/phase_89_property_inspector_tabs_events_plan.md`.
- No existing Phase 90 plan file was present, so this file uses the next correct phase number without renumbering.

### 2. Current startup architecture

- `src/app/main.cpp` currently owns the only entry point and catches fatal exceptions by writing to `std::cerr`.
- `src/app/App.cpp` owns the real lifecycle:
  - `startup()` creates `ui::MainWindow`
  - `run()` shows the window, verifies `isShowing()`, runs the event loop, and shuts down
  - `shutdown()` closes and releases the main window
- This makes `App::run()` the right shared runner boundary for both Windows and non-Windows entry points.

### 3. Current diagnostics and console assumptions

- Current fatal startup reporting in `src/app/main.cpp` depends on `std::cerr`.
- Existing Windows-safe debug output already exists in `src/ui/MainWindow.cpp` through `OutputDebugStringA`.
- Repository search found no other `std::cout`, `printf`, or `fprintf` usage in the main application target beyond the current `main.cpp` fatal path.

### 4. Current main-window and native handle capabilities

- `ui::MainWindow` derives from `visage::ApplicationWindow`.
- The local Visage checkout shows `ApplicationEditor::window()` returns a `visage::Window*`.
- The Visage `Window` abstraction exposes `nativeHandle()`, and the Win32 implementation returns the underlying `HWND`.
- That gives Phase 90 a clean way to apply the large/small icon after the native window exists, while keeping Win32 code centralized in VisiForm's Windows-only path.

### 5. Current build wiring

- `CMakeLists.txt` defines a single `add_executable(VisiForm ...)` target without `WIN32`.
- Windows-specific linking currently adds `comdlg32`, but no resource script is included yet.
- CMake is the checked-in source of truth for build wiring, so Windows subsystem/resource changes should be made there rather than by editing generated IDE files.

### 6. Icon source state

- `assets/source/VisiFormIcon.png` exists and will remain the authoritative source artwork.
- No Windows `.ico` asset or `.rc` resource structure was present at the start of the phase.

## Architecture Decision

- Keep shared startup behavior in `visiform::App`.
- Introduce a small startup helper layer in `src/app/` so Windows and non-Windows entry points both call the same runner and the same fatal-error reporting path.
- Use `wWinMain` only on Windows builds and keep `main` only for non-Windows builds.
- Route debugger diagnostics through a Windows-safe helper that uses `OutputDebugStringW` on Windows and `std::cerr` elsewhere.
- Show Windows fatal startup errors with a native message box if the app fails before the main window is available.
- Centralize Win32 icon loading/application in a Windows-specific helper callable from `ui::MainWindow::showWindow()`.
- Generate the `.ico` from the authoritative PNG via a checked-in Pillow-based script, but do not require that script to run as part of every build.

## Files Expected To Change

- `CMakeLists.txt`
- `src/app/App.h`
- `src/app/App.cpp`
- `src/app/main.cpp`
- `src/ui/MainWindow.h`
- `src/ui/MainWindow.cpp`
- `docs/VISIFORM_PROJECT_SPEC.md`
- `docs/agent_plans/phase_90_windows_subsystem_application_icon_plan.md`
- `assets/icons/windows/VisiForm.ico`
- `resources/windows/resource.h`
- `resources/windows/VisiForm.rc`
- `tools/generate_windows_icon.py`

## TODO Checklist

- [x] Confirm the next correct phase number and create the official Phase 90 plan.
- [x] Inspect the current startup lifecycle, diagnostics, native window access, and build wiring.
- [x] Refactor startup into shared runner logic for Windows and non-Windows entry points.
- [x] Add a Windows subsystem entry point and remove console-only fatal startup reporting from the Windows target.
- [x] Add Windows-safe startup diagnostics and visible fatal error reporting.
- [x] Add centralized native window icon application on Windows.
- [x] Create the Windows icon-generation script.
- [x] Generate and commit the multi-resolution `.ico` asset from `assets/source/VisiFormIcon.png`.
- [x] Add Windows resource files and embed the icon in the `VisiForm` executable.
- [x] Update `CMakeLists.txt` so the Windows `VisiForm` target is a GUI subsystem executable and includes the resource script.
- [x] Update specification/documentation to describe the new startup/resource behavior.
- [x] Record validation status, final result summary, and remaining TODOs before finishing.

## Validation Plan

- Source inspection:
  - verify only one Windows entry point remains active for the GUI target
  - verify Windows-only code is guarded with `_WIN32`/`WIN32`
  - verify CMake includes the `.rc` file only on Windows
  - verify no absolute local paths are added
- Developer-run validation, because repository rules do not allow me to run build commands or launch `VisiForm.exe` without an explicit exact command:
  - build Debug x64 `VisiForm`
  - build Release x64 `VisiForm`
  - run existing automated tests if desired
  - manually confirm no console window appears
  - manually confirm the icon appears in Explorer, taskbar, Alt+Tab, and the title bar

## Compatibility Considerations

- Non-Windows entry behavior should remain valid through a retained `main` path.
- No `.vfb.json` schema changes are planned.
- No generator behavior changes are planned.
- Windows icon behavior may be subject to shell icon-cache delays even when resources are correct.

## Build / Test Status

- Build not run. Repository instructions require the developer to explicitly request the exact build command/path.
- Automated tests not run for the same reason.
- Manual UI/icon verification not run. Repository instructions forbid launching `VisiForm.exe`.
- Completed source inspection confirmed:
  - Windows uses `wWinMain` plus shared `startup::runApplication()`
  - non-Windows keeps `main()`
  - Windows-only build wiring adds `resources/windows/VisiForm.rc`
  - `CMakeLists.txt` marks `VisiForm` as `WIN32_EXECUTABLE TRUE` on Windows
  - `assets/icons/windows/VisiForm.ico` was generated successfully
  - Pillow reports the ICO contains the required sizes: 16, 20, 24, 32, 40, 48, 64, 128, and 256

## Final Result Summary

Phase 90 converted the Windows `VisiForm` target to a GUI subsystem application without changing the shared `visiform::App` lifecycle. `src/app/Startup.cpp` now owns the shared startup runner and fatal startup reporting, `src/app/main.cpp` uses `wWinMain` on Windows and retains `main()` elsewhere, and the old console-only fatal path was removed. Windows startup failures now emit debugger-visible diagnostics and a native error dialog instead of relying on a console window.

The phase also added a repeatable icon pipeline. `tools/generate_windows_icon.py` keeps `assets/source/VisiFormIcon.png` as the authoritative source image and generates `assets/icons/windows/VisiForm.ico`. `resources/windows/resource.h` and `resources/windows/VisiForm.rc` embed that icon into the executable, `CMakeLists.txt` wires the resource into the existing `VisiForm` target, and `ui::MainWindow` now applies the embedded large/small icon to the native Win32 window after `show()`. Native common dialogs now use the main window as their owner handle on Windows.

Files changed in this phase:

- `CMakeLists.txt`
- `docs/VISIFORM_PROJECT_SPEC.md`
- `docs/agent_plans/phase_90_windows_subsystem_application_icon_plan.md`
- `src/app/App.cpp`
- `src/app/App.h`
- `src/app/Startup.cpp`
- `src/app/Startup.h`
- `src/app/main.cpp`
- `src/ui/MainWindow.cpp`
- `src/ui/MainWindow.h`
- `src/utils/NativeFileDialogs.cpp`
- `src/utils/NativeFileDialogs.h`
- `resources/windows/resource.h`
- `resources/windows/VisiForm.rc`
- `tools/generate_windows_icon.py`
- `assets/icons/windows/VisiForm.ico`

Intentionally left untouched:

- `assets/source/VisiFormIcon.png` source artwork
- build, output, generated, and IDE folders
- generated-project export code

## Remaining TODOs

- Run the approved Visual Studio build path for Debug x64 and Release x64 `VisiForm`.
- Manually verify that the Windows build launches without a console window.
- Manually verify executable/taskbar/Alt+Tab/title-bar icon behavior, and note any temporary Explorer/taskbar icon-cache lag rather than changing correct resource code.
