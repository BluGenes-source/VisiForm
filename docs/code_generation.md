# Code generation

`VisiForm` can export the current `ProjectDocument` as a standalone generated Visage C++ project.

## Export folder

Current fixed export folder:

- `Generated/ExportedVisageProject`

The generator overwrites the files it owns inside that folder.

## Generated files

The export currently writes:

- `Generated/ExportedVisageProject/CMakeLists.txt`
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

## Overwrite behavior

The generator may overwrite files inside `Generated/ExportedVisageProject`.
It does not intentionally overwrite files outside that export folder.

## Current limitations

Current export does not yet generate:

- event handlers
- interactive widget logic
- data binding
- layout managers
- property editor wiring
- asset copying for images
- custom widgets

Button output includes `TODO` comments for future generated event handlers.

## Building the generated project manually

Example workflow after export:

1. Open a terminal in `Generated/ExportedVisageProject`
2. Configure with CMake
3. Build with your preferred generator and toolchain

Example:

- `cmake -S . -B build -G Ninja`
- `cmake --build build`

If your environment requires vcpkg or other toolchain settings later, the generated `CMakeLists.txt` includes a `TODO` note for future improvements.
