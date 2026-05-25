# Code generation

`VisiForm` can export the current `ProjectDocument` as a standalone generated Visage C++ project.

## Export folder

Current default export fallback folder:

- `Generated/ExportedVisageProject`

The export flow may also write to a user-selected folder.
The generator overwrites only the files it owns inside the chosen export folder.
Managed project resources are copied into generated `assets/` folders and are not compiled by CMake in this phase.

Before export, `VisiForm` now validates the in-memory project document and writes a markdown report to:

- `Generated/validation_report.md`

Validation result behavior:

- errors block export before generated files are written
- warnings do not block export
- a clean validation pass keeps the generated export behavior unchanged
- the toolbar `Chk` action runs the same validation flow on demand
- the `Project > Validate / Check` menu command runs the same validation flow on demand
- the editor now shows a modal validation summary after `Chk`
- export-blocking validation errors also surface in an editor modal dialog before export stops

## Generated files

The export currently writes:

- `Generated/ExportedVisageProject/CMakeLists.txt`
- `Generated/ExportedVisageProject/CMakePresets.json`
- `Generated/ExportedVisageProject/README.md`
- `Generated/ExportedVisageProject/.gitignore`
- `Generated/ExportedVisageProject/scripts/configure_static_debug.cmd`
- `Generated/ExportedVisageProject/scripts/build_static_debug.cmd`
- `Generated/ExportedVisageProject/scripts/configure_static_release.cmd`
- `Generated/ExportedVisageProject/scripts/build_static_release.cmd`
- `Generated/ExportedVisageProject/scripts/configure_static_debug.ps1`
- `Generated/ExportedVisageProject/scripts/build_static_debug.ps1`
- `Generated/ExportedVisageProject/scripts/configure_static_release.ps1`
- `Generated/ExportedVisageProject/scripts/build_static_release.ps1`
- `Generated/ExportedVisageProject/src/main.cpp`
- `Generated/ExportedVisageProject/src/MainWindow.h`
- `Generated/ExportedVisageProject/src/MainWindow.cpp`
- `Generated/ExportedVisageProject/src/<UserSubclassName>.h`
- `Generated/ExportedVisageProject/src/<UserSubclassName>.cpp`

When resources exist, export also copies managed asset files such as:

- `Generated/ExportedVisageProject/assets/images/<filename>`
- `Generated/ExportedVisageProject/assets/fonts/<filename>`
- `Generated/ExportedVisageProject/assets/icons/<filename>`
- `Generated/ExportedVisageProject/assets/themes/<filename>`

## Generated project contents

The generated project includes:

- CMake 3.24 minimum
- C++20
- optional local-source support for `visage` via `VISIFORM_VISAGE_SOURCE_DIR`
- `FetchContent` fallback for `visage` when no valid local source is configured
- the same core Visage options currently used by `VisiForm`
- static MSVC runtime settings for Debug and Release
- generated `CMakePresets.json` presets for static Debug and Release builds
- helper `.cmd` scripts that locate Visual Studio with `vswhere`, load `VsDevCmd.bat`, and call those presets from the generated project root
- optional `.ps1` wrappers that perform the same Visual Studio environment bootstrap

Generated dependency variables in exported `CMakeLists.txt`:

- `VISIFORM_VISAGE_SOURCE_DIR`
- `VISIFORM_VISAGE_GIT_REPOSITORY`
- `VISIFORM_VISAGE_GIT_TAG`

Dependency behavior:

- if `VISIFORM_VISAGE_SOURCE_DIR` is non-empty and contains `CMakeLists.txt`, the generated project uses `add_subdirectory(...)` with the local Visage source tree
- otherwise the generated project falls back to `FetchContent`
- this avoids repeated dependency downloads when a developer keeps a local Visage checkout

## Managed assets

Project resources are exported with safe relative paths under `assets/`.

Current behavior:

- image resources default to `assets/images/<filename>`
- font resources default to `assets/fonts/<filename>`
- icon resources default to `assets/icons/<filename>`
- theme resources default to `assets/themes/<filename>`
- duplicate default export names are made unique when needed
- export copies managed files into the generated project folder
- export does not delete unknown user files

Generated image widgets currently stay safe by using managed resource paths as placeholder text and generated comments until runtime image loading is expanded.

## Generated project naming

The `New Project Wizard`, `Project Settings` dialog, and root `FormWindow` property editing flow all feed the same generated-project naming fields:

- `projectName`
- `executableName`
- `userSubclassName`
- `windowTitle`
- `lookAndFeelId`

Export uses those fields as follows:

- `projectName` drives the generated `project(...)` name in `CMakeLists.txt` after CMake-safe sanitization
- `executableName` drives `add_executable(...)` and therefore the produced Windows executable name
- `userSubclassName` drives `src/<UserSubclassName>.h`, `src/<UserSubclassName>.cpp`, and `main.cpp`
- `windowTitle` drives the generated runtime window title and title-bar caption text
- `lookAndFeelId` drives the project-level default look and feel used by generated runtime widgets

The wizard also sets the initial root `FormWindow` size by writing the selected width and height into the root widget bounds.

## Generated presets

The exported project currently includes these configure presets:

- `vs2022-x64-static-debug`
- `vs2022-x64-static-release`

And these build presets:

- `build-static-debug`
- `build-static-release`

The generated presets use `Ninja` and keep the static MSVC runtime strategy:

- `MultiThreadedDebug` for Debug
- `MultiThreaded` for Release

Because the generated presets use `Ninja`, `cl.exe` must already be available in the environment when CMake configures the project. `Visual Studio 2022` and the `x64 Native Tools Command Prompt for VS 2022` provide that environment automatically. A normal PowerShell session does not, so direct `cmake --preset ...` commands can fail with `No CMAKE_CXX_COMPILER could be found` unless `VsDevCmd.bat` is loaded first.

The exported helper scripts solve that by locating the latest `Visual Studio 2022` installation with `vswhere`, calling `VsDevCmd.bat -arch=x64 -host_arch=x64`, and then running the matching configure or build preset.

Recommended generated-project workflows:

- `Visual Studio 2022` - use `File > Open > Folder`, choose the exported project folder, select `vs2022-x64-static-debug` or `vs2022-x64-static-release`, then build from the IDE
- `x64 Native Tools Command Prompt for VS 2022` - run `cmake --preset ...` and `cmake --build --preset ...` directly
- normal PowerShell - run the generated scripts in `scripts/`, such as `configure_static_debug.cmd` and `build_static_debug.cmd`

Generated configure presets also include these dependency cache variables:

- `VISIFORM_VISAGE_GIT_REPOSITORY`
- `VISIFORM_VISAGE_GIT_TAG`

If `AppSettings.localVisageSourceDirectory` is configured before export, the generated presets also include:

- `VISIFORM_VISAGE_SOURCE_DIR`

The emitted path uses forward slashes, for example:

- `J:/Dev/CeePlusPlus/visage`

That local source path is still controlled by `VISIFORM_VISAGE_SOURCE_DIR`. If it is unset or invalid, the generated project still falls back to `FetchContent` using the configured repository and tag values.

The root `FormWindow` property inspector and `Project Settings` dialog both expose the app-level export dependency values. These values are stored in `AppSettings`, not in `.vfb.json` project files.

That means:

- `localVisageSourceDirectory` remains machine-specific and is read from `AppSettings` during export
- `visageGitRepository` remains machine-specific and is read from `AppSettings` during export
- `visageGitTag` remains machine-specific and is read from `AppSettings` during export

## Validation before export

`VisiForm` now validates the current project before export so bad generated code can be caught inside the editor first.

Current export validation highlights:

- project naming checks for `projectName`, `executableName`, and `userSubclassName`
- local Visage dependency-setting checks for `localVisageSourceDirectory`, `visageGitRepository`, and `visageGitTag`
- project resource checks for unique ids, existing source files, safe `assets/` export paths, and duplicate export path conflicts
- `Image` widget checks for `resourceId`, fallback `imagePath`, and `scaleMode`
- widget validation for duplicate ids, duplicate names, empty ids, bounds, colors, enum values, and numeric ranges
- callback validation for invalid handler names and incompatible signature reuse
- `RadioButton` group validation for empty groups, missing selections, and conflicting selected states

Export decision rules:

- if validation reports one or more errors, export is blocked
- if validation reports warnings only, export continues and the status text reports the warning count
- if validation reports no warnings or errors, export continues normally

Callback compatibility checks now reuse the same `handlerSignatureKind` metadata already stored in `WidgetRegistry` event definitions. A callback name can be shared only when every use maps to the same signature group:

- `void_event`
- `bool_event`
- `float_event`
- `string_event`

If the same callback name is assigned to incompatible signature kinds, validation reports an error before export.

## Widget mappings

Current generated widget rendering support:

- `FormWindow`
- `Frame`
- `Label`
- `Button`
- `TextBox`
- `CheckBox`
- `RadioButton`
- `Slider`
- `ScrollBar`
- `StatusBar`
- `ProgressBar`
- `ModalDialog`
- `ColorPicker`
- `Image`
- `Spacer`

Rendering now uses a simple generated runtime widget model.

Generated runtime type highlights:

- `WidgetEvent` now uses `std::string_view` sender metadata for synchronous callback dispatch
- `RuntimeWidgetType` now uses `std::uint8_t` with an `Unknown` default value
- `RuntimeOrientation` replaces string-based runtime orientation checks
- `RuntimeColor` replaces plain integer color fields in the generated runtime model
- `RuntimeWidget` now groups related text, toggle, range, style, event, and interaction state

## Look and feel aware preview rendering

Generated preview drawing now resolves a project-level look and feel plus optional per-widget style overrides at export time.

Built-in preset ids currently include:

- `VisiFormDark`
- `VisiFormLight`
- `ImGuiDark`
- `FlatClassic`

Supported generated preview style fields include:

- `lookAndFeelId`
- `fillColor`
- `textColor`
- `borderColor`
- `accentColor`
- `borderThickness`
- `cornerRadius`
- `fontSize`

Generated look and feel is baked into that runtime model for now.
Runtime theme switching in the generated app is future work.

## Generated interactive widget behavior

The exported `MainWindow` now generates a lightweight runtime widget list, hit testing, mouse input handling, basic text input handling, redraw calls, and sender-aware callback dispatch.

Current generated interactive behavior:

- `Button` - click completion fires `onClick`, release-over-button fires `onRelease`, double-click fires `onDoubleClick`, and toggle-mode buttons keep exported checked-state visuals
- `CheckBox` - click toggles and fires `onToggle`
- `RadioButton` - click selects within its group and fires `onSelected`
- `Slider` - drag updates value and fires `onChanged`
- `ScrollBar` - arrows, track paging, and thumb dragging update value and fire `onChanged`
- `TextBox` - click focuses, text input appends characters, Backspace removes a character, and changes fire `onTextChanged`
- `ProgressBar` - display only, but now renders from generated runtime state
- `StatusBar` - display only, but now renders from generated runtime state
- `ModalDialog` - exports runtime modal definitions, blocks underlying generated input while visible, supports `Enter` and `Escape`, and dispatches `onAccepted` or `onCancelled` from configured dialog buttons

The generated runtime is intentionally small and is not a full retained-mode widget framework.

## Generated runtime widget state API

The generated `MainWindow` base class now exposes protected runtime state helpers so the user subclass can read and update widgets inside exported callbacks without editing generated files.

Current protected helpers on `MainWindow`:

- `RuntimeWidget* findWidgetById(const std::string& id)`
- `const RuntimeWidget* findWidgetById(const std::string& id) const`
- `RuntimeWidget* findWidgetByName(const std::string& name)`
- `const RuntimeWidget* findWidgetByName(const std::string& name) const`
- `[[nodiscard]] bool setText(const std::string& idOrName, const std::string& text)`
- `std::optional<std::string> getText(const std::string& idOrName) const`
- `std::string getTextOr(const std::string& idOrName, std::string fallback) const`
- `[[nodiscard]] bool setChecked(const std::string& idOrName, bool checked)`
- `std::optional<bool> getChecked(const std::string& idOrName) const`
- `bool getCheckedOr(const std::string& idOrName, bool fallback) const`
- `[[nodiscard]] bool setSelected(const std::string& idOrName, bool selected)`
- `std::optional<bool> getSelected(const std::string& idOrName) const`
- `bool getSelectedOr(const std::string& idOrName, bool fallback) const`
- `[[nodiscard]] bool setValue(const std::string& idOrName, float value)`
- `std::optional<float> getValue(const std::string& idOrName) const`
- `float getValueOr(const std::string& idOrName, float fallback) const`
- `[[nodiscard]] bool setProgressValue(const std::string& idOrName, float value)`
- `[[nodiscard]] bool setStatusBarField(const std::string& idOrName, int fieldIndex, const std::string& text)`
- `[[nodiscard]] bool showMessageDialog(const std::string& title, const std::string& message)`
- `[[nodiscard]] bool showModalDialog(const std::string& idOrName)`
- `void closeModalDialog()`
- `std::optional<std::string> activeModalDialogId() const`
- `void requestGeneratedUiRepaint()`

Lookup behavior for `idOrName` helpers:

1. exact widget `id`
2. exact widget `name`

If no widget matches, or if the widget type does not support the requested getter:

- setters return `false`
- `getText(...)`, `getChecked(...)`, `getSelected(...)`, and `getValue(...)` return `std::nullopt`
- `getTextOr(...)`, `getCheckedOr(...)`, `getSelectedOr(...)`, and `getValueOr(...)` return the provided fallback

Current setter coverage:

- `setText(...)`
  - supported: `Label`, `Button`, `TextBox`, `CheckBox`, `RadioButton`, `ProgressBar`, `StatusBar` field 0, `Frame`, `ModalDialog`, `ColorPicker`
- `setChecked(...)`
  - supported: `CheckBox`, `Button`
- `setSelected(...)`
  - supported: `RadioButton`
  - selecting `true` enforces single selection for the matching radio `group`
- `setValue(...)`
  - supported: `Slider`, `ScrollBar`, `ProgressBar`
  - values are clamped to the widget `min` and `max`
- `setProgressValue(...)`
  - convenience wrapper over `setValue(...)`
- `setStatusBarField(...)`
  - supported: `StatusBar`
  - valid `fieldIndex` range: `0` through `3`

State setter behavior:

- generated helper setters are marked `[[nodiscard]]`, use safe return values, and do not throw exceptions
- helper-driven state changes request a generated UI repaint
- callback-driven helper setters do not auto-fire the widget's own generated callback again

Example user-subclass callback usage:

- `setText("statusLabel", "Working...")`
- `setProgressValue("progressBar_1", value)`
- `setStatusBarField("statusBar_1", 0, "Ready")`
- `setSelected("modeAdvanced", true)`

`ProgressBar` preview text behavior:

- if `showText` is `true` and `text` is empty, generated preview rendering shows percent text
- if `showText` is `true` and `text` is non-empty, generated preview rendering shows the custom text value
- if `showText` is `false`, generated preview rendering shows no ProgressBar text

## Generated event handler stubs

Current generated C++ export supports event metadata stored as widget properties.

Supported event properties:

- `Button.onClick`
- `Button.onRelease`
- `Button.onDoubleClick`
- `CheckBox.onToggle`
- `RadioButton.onSelected`
- `Slider.onChanged`
- `ScrollBar.onChanged`
- `ColorPicker.onChanged`
- `TextBox.onTextChanged`
- `FormWindow.onLoad`
- `FormWindow.onClose`
- `ModalDialog.onAccepted`
- `ModalDialog.onCancelled`

## Generated base class and user subclass

Current export uses a generated base class plus a user subclass pattern.

Default class names:

- generated base: `MainWindow`
- user subclass: `AppMainWindow`

Current generated file roles:

- generated base header and source are regenerated by export
- user subclass header and source are the user-edit layer
- `main.cpp` instantiates the user subclass

Generated base naming rule:

- `MainWindow.h`
- `MainWindow.cpp`
- `class MainWindow`

User subclass naming rule:

- `class <UserSubclassName> : public MainWindow`
- `main.cpp` includes `<UserSubclassName>.h`
- `main.cpp` instantiates `<UserSubclassName>`

When the root form is selected in the editor, `projectName`, `executableName`, `userSubclassName`, and `windowTitle` can be edited.

The generated user subclass keeps using `USER CODE` regions, and new handler stubs now include short examples that show how to call the protected runtime state helpers from callbacks, including dialog helpers such as `showMessageDialog(...)` and `showModalDialog(...)`.

When a non-empty handler name is present, the generated project emits:

- a handler declaration in `src/MainWindow.h`
- a handler definition in `src/MainWindow.cpp`
- TODO comments near relevant generated widget drawing code

## Sender-aware callback API

Generated code now emits a lightweight sender-aware event structure:

- `struct WidgetEvent`
  - `std::string_view senderId`
  - `std::string_view senderName`
  - `std::string_view senderType`

This provides a small practical listener/callback foundation similar in spirit to JUCE-style shared listener handlers, without generating a full messaging framework.

The generated runtime now also emits shared dispatch helpers per signature kind instead of generating one emit wrapper per widget binding. Those helpers inspect `widget.events.*`, build a `WidgetEvent` from the current runtime widget, and dispatch compatible callbacks by handler name.

Generated handler signatures now use sender-aware forms:

- `onClick` -> `void handlerName(const WidgetEvent& event)`
- `onRelease` -> `void handlerName(const WidgetEvent& event)`
- `onDoubleClick` -> `void handlerName(const WidgetEvent& event)`
- `onToggle` -> `void handlerName(const WidgetEvent& event, bool value)`
- `onSelected` -> `void handlerName(const WidgetEvent& event, bool value)`
- `onChanged` -> `void handlerName(const WidgetEvent& event, float value)`
- `ColorPicker.onChanged` -> `void handlerName(const WidgetEvent& event, const std::string& value)`
- `onTextChanged` -> `void handlerName(const WidgetEvent& event, const std::string& value)`
- `onLoad` -> `void handlerName(const WidgetEvent& event)`
- `onClose` -> `void handlerName(const WidgetEvent& event)`

The generated base class also emits per-widget helper methods for future interactive dispatch.

Examples:

- `emit_button_1_onClick()`
- `emit_radioButton_1_onSelected(bool value)`

These emit helpers construct the appropriate sender metadata and forward to the shared handler.

Callback compatibility for suggestion reuse is now grouped by:

- `void_event`
- `bool_event`
- `float_event`
- `string_event`

## Shared callbacks and conflict rules

If multiple widgets use the same callback name with a compatible signature, generated code emits:

- one handler declaration in the base class
- one handler definition in the base class
- one user override in the user subclass
- one emit helper per widget event that forwards distinct sender metadata to the shared handler

This lets one callback handle several widgets and inspect `event.senderId`, `event.senderName`, or `event.senderType` to identify the sender.

If the same callback name is reused with incompatible signatures, export fails with a clear conflict error.

## Generated hint comments

When a widget has a non-empty `hint` property, export also emits a nearby comment in generated drawing code:

- `// Hint: ...`

This is editor help text only. Generated runtime tooltip behavior is not implemented yet.

## User code preservation

Generated handler bodies in `src/MainWindow.cpp` use explicit preservation markers:

- `// USER CODE BEGIN handlerName`
- `// USER CODE END handlerName`

During re-export, `VisiForm` reads the existing generated `src/MainWindow.cpp` file and preserves the content inside matching handler markers.

Example:

- `// USER CODE BEGIN handleGenerateCodeClick`
- `// USER CODE END handleGenerateCodeClick`

Current preservation behavior:

- preserves recognized handler body regions only
- preserves by handler name
- overwrites the generated base files
- overwrites the non-user portions of the user subclass file
- removes handlers that are no longer referenced by the current project model

Current limitation:

- orphaned code from removed handlers is not preserved automatically yet

Current generated signatures:

- `onClick` -> `void handlerName(const WidgetEvent& event)`
- `onRelease` -> `void handlerName(const WidgetEvent& event)`
- `onDoubleClick` -> `void handlerName(const WidgetEvent& event)`
- `onToggle` -> `void handlerName(const WidgetEvent& event, bool value)`
- `onSelected` -> `void handlerName(const WidgetEvent& event, bool value)`
- `onChanged` -> `void handlerName(const WidgetEvent& event, float value)`
- `onTextChanged` -> `void handlerName(const WidgetEvent& event, const std::string& value)`
- `onLoad` -> `void handlerName(const WidgetEvent& event)`
- `onClose` -> `void handlerName(const WidgetEvent& event)`

Handler names must be valid C++ identifiers.
If export finds an invalid handler name, export fails cleanly.
If the same handler name is reused with incompatible signatures, export fails with a conflict error.

## Overwrite behavior

The generator may overwrite files inside `Generated/ExportedVisageProject`.
It does not intentionally overwrite files outside that export folder.
It only writes known generated files inside that folder and refuses to write paths outside the requested export directory.

## Export progress

The main editor now reports export progress through staged messages and a compact progress pane in the bottom status bar.
This progress is currently synchronous and stage-based rather than background threaded.

## Current limitations

Current export does not yet generate:

- event handlers
- interactive widget logic
- data binding
- layout managers
- property editor wiring
- asset copying for images
- custom widgets

Generated handler stubs exist, but users still implement the actual behavior manually.
Generated UI event dispatch is still intentionally limited compared with a full widget framework.

Current generated runtime limitations include:

- no text selection, clipboard, IME, or text cursor editing yet
- no tab traversal or full focus-management framework yet
- `ProgressBar` and `StatusBar` remain display-only for this phase
- no async event queue or threaded callback model

## Building the generated project manually

Example workflow after export:

1. Open a terminal in `Generated/ExportedVisageProject`
2. Configure with CMake
3. Build with your preferred generator and toolchain

Example using presets:

- `cmake --preset vs2022-x64-static-debug`
- `cmake --build --preset build-static-debug`
- `cmake --preset vs2022-x64-static-release`
- `cmake --build --preset build-static-release`

You can also use the generated helper scripts:

- `scripts\configure_static_debug.cmd`
- `scripts\build_static_debug.cmd`
- `scripts\configure_static_release.cmd`
- `scripts\build_static_release.cmd`

Visual Studio workflow:

- `File > Open > Folder`
- choose `Generated/ExportedVisageProject`
- select `vs2022-x64-static-debug` or `vs2022-x64-static-release`
- build and run from the IDE

## Future improvements

- Pin the exported Visage dependency to a known-good commit
- Generate safer user-edit regions inside generated files
- Generate event handler stubs and wiring
- Add richer export packaging as generated features expand
