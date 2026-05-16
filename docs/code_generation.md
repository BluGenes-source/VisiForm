# Code generation

`VisiForm` can export the current `ProjectDocument` as a standalone generated Visage C++ project.

## Export folder

Current fixed export folder:

- `Generated/ExportedVisageProject`

The generator overwrites the files it owns inside that folder.

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
- `Generated/ExportedVisageProject/src/main.cpp`
- `Generated/ExportedVisageProject/src/MainWindow.h`
- `Generated/ExportedVisageProject/src/MainWindow.cpp`

## Generated project contents

The generated project includes:

- CMake 3.24 minimum
- C++20
- `FetchContent` for `visage`
- the same core Visage options currently used by `VisiForm`
- static MSVC runtime settings for Debug and Release
- generated `CMakePresets.json` presets for static Debug and Release builds
- helper `.cmd` scripts that call those presets

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

## Widget mappings

Current generated widget rendering support:

- `FormWindow`
- `Frame`
- `Label`
- `Button`
- `TextBox`
- `CheckBox`
- `Slider`
- `Image`
- `Spacer`

Rendering is currently a static preview of the form and widget tree.

## Generated event handler stubs

Current generated C++ export supports event metadata stored as widget properties.

Supported event properties:

- `Button.onClick`
- `CheckBox.onToggle`
- `Slider.onChanged`
- `TextBox.onTextChanged`
- `FormWindow.onLoad`
- `FormWindow.onClose`

When a non-empty handler name is present, the generated project emits:

- a handler declaration in `src/MainWindow.h`
- a handler definition in `src/MainWindow.cpp`
- TODO comments near relevant generated widget drawing code

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
- overwrites the rest of the generated file
- removes handlers that are no longer referenced by the current project model

Current limitation:

- orphaned code from removed handlers is not preserved automatically yet

Current generated signatures:

- `onClick` -> `void handlerName()`
- `onToggle` -> `void handlerName(bool checked)`
- `onChanged` -> `void handlerName(float value)`
- `onTextChanged` -> `void handlerName(const std::string& text)`
- `onLoad` -> `void handlerName()`
- `onClose` -> `void handlerName()`

Handler names must be valid C++ identifiers.
If export finds an invalid handler name, export fails cleanly.
If the same handler name is reused with incompatible signatures, export fails with a conflict error.

## Overwrite behavior

The generator may overwrite files inside `Generated/ExportedVisageProject`.
It does not intentionally overwrite files outside that export folder.
It only writes known generated files inside that folder and refuses to write paths outside the requested export directory.

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
Generated UI event dispatch is still limited to comments and TODO placeholders for now.

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

## Future improvements

- Pin the exported Visage dependency to a known-good commit
- Generate safer user-edit regions inside generated files
- Generate event handler stubs and wiring
- Add richer export packaging as generated features expand
