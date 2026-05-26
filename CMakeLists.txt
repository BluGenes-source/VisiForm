cmake_minimum_required(VERSION 3.24)

project(VisiForm
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(FetchContent)

# Optional local Visage source checkout.
# Recommended for VisiForm development to avoid repeated FetchContent downloads.
set(VISIFORM_VISAGE_SOURCE_DIR "" CACHE PATH "Optional local Visage source directory")
set(VISIFORM_VISAGE_GIT_REPOSITORY "https://github.com/VitalAudio/visage.git" CACHE STRING "Visage Git repository")
set(VISIFORM_VISAGE_GIT_TAG "main" CACHE STRING "Visage Git tag or commit")

# TODO: Replace VISIFORM_VISAGE_GIT_TAG=main with a tested commit hash after validation.
set(VISAGE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(VISAGE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(VISAGE_ENABLE_WIDGETS ON CACHE BOOL "" FORCE)
set(VISAGE_AMALGAMATED_BUILD ON CACHE BOOL "" FORCE)

if (VISIFORM_VISAGE_SOURCE_DIR AND EXISTS "${VISIFORM_VISAGE_SOURCE_DIR}/CMakeLists.txt")
    message(STATUS "Using local Visage source: ${VISIFORM_VISAGE_SOURCE_DIR}")

    add_subdirectory(
        "${VISIFORM_VISAGE_SOURCE_DIR}"
        "${CMAKE_BINARY_DIR}/_deps/visage-build"
    )
else()
    message(STATUS "Using FetchContent Visage fallback")
    message(STATUS "Visage repository: ${VISIFORM_VISAGE_GIT_REPOSITORY}")
    message(STATUS "Visage tag/commit: ${VISIFORM_VISAGE_GIT_TAG}")

    FetchContent_Declare(
        visage
        GIT_REPOSITORY "${VISIFORM_VISAGE_GIT_REPOSITORY}"
        GIT_TAG "${VISIFORM_VISAGE_GIT_TAG}"
        GIT_SHALLOW TRUE
    )

    FetchContent_MakeAvailable(visage)
endif()
find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(VisiForm
    src/app/App.cpp
    src/app/main.cpp
    src/ui/MainWindow.cpp
    src/ui/DesignerCanvas.cpp
    src/ui/WidgetMetrics.cpp
    src/ui/WidgetPalette.cpp
    src/ui/PropertyInspector.cpp
    src/ui/ProjectTree.cpp
    src/ui/editors/TextEditControl.cpp
    src/ui/editors/DropdownControl.cpp
    src/model/ProjectDocument.cpp
    src/model/ProjectResource.cpp
    src/model/FormNode.cpp
    src/model/WidgetDefinition.cpp
    src/model/LookAndFeelRegistry.cpp
    src/model/WidgetRegistry.cpp
    src/model/WidgetNode.cpp
    src/model/PropertyValue.cpp
    src/serialization/JsonProjectReader.cpp
    src/serialization/JsonProjectWriter.cpp
    src/generator/CodeGenerator.cpp
    src/generator/VisageCppEmitter.cpp
    src/generator/CMakeEmitter.cpp
    src/validation/ProjectValidator.cpp
    src/commands/Command.cpp
    src/commands/UndoRedoStack.cpp
    src/utils/FileUtils.cpp
    src/utils/CppIdentifier.cpp
    src/utils/IdGenerator.cpp
    src/utils/AppSettings.cpp
    src/utils/NativeFileDialogs.cpp
    src/utils/RecentFiles.cpp
)

target_include_directories(VisiForm
    PRIVATE
        src
)

target_link_libraries(VisiForm
    PRIVATE
        visage
        nlohmann_json::nlohmann_json
        fmt::fmt
        spdlog::spdlog
)

if (MSVC)
    target_compile_options(VisiForm PRIVATE /W4 /permissive-)
endif()

if (WIN32)
    target_link_libraries(VisiForm PRIVATE comdlg32)
endif()
