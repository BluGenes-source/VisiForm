cmake_minimum_required(VERSION 3.24)

project(VisiForm
    VERSION 1.0.17
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

set(_VISIFORM_VISAGE_SOURCE_CANDIDATES)
if (VISIFORM_VISAGE_SOURCE_DIR)
    list(APPEND _VISIFORM_VISAGE_SOURCE_CANDIDATES "${VISIFORM_VISAGE_SOURCE_DIR}")
endif()
if (DEFINED ENV{VISIFORM_VISAGE_SOURCE_DIR} AND NOT "$ENV{VISIFORM_VISAGE_SOURCE_DIR}" STREQUAL "")
    list(APPEND _VISIFORM_VISAGE_SOURCE_CANDIDATES "$ENV{VISIFORM_VISAGE_SOURCE_DIR}")
endif()
list(APPEND _VISIFORM_VISAGE_SOURCE_CANDIDATES
    "${CMAKE_CURRENT_LIST_DIR}/../visage"
    "${CMAKE_CURRENT_LIST_DIR}/../../visage"
    "${CMAKE_CURRENT_LIST_DIR}/../../../visage"
)

set(_VISIFORM_RESOLVED_VISAGE_SOURCE_DIR "")
foreach (_VISIFORM_VISAGE_SOURCE_CANDIDATE IN LISTS _VISIFORM_VISAGE_SOURCE_CANDIDATES)
    if (EXISTS "${_VISIFORM_VISAGE_SOURCE_CANDIDATE}/CMakeLists.txt")
        get_filename_component(
            _VISIFORM_RESOLVED_VISAGE_SOURCE_DIR
            "${_VISIFORM_VISAGE_SOURCE_CANDIDATE}"
            ABSOLUTE
        )
        break()
    endif()
endforeach()

if (_VISIFORM_RESOLVED_VISAGE_SOURCE_DIR)
    message(STATUS "Using local Visage source: ${_VISIFORM_RESOLVED_VISAGE_SOURCE_DIR}")

    add_subdirectory(
        "${_VISIFORM_RESOLVED_VISAGE_SOURCE_DIR}"
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
    src/app/Startup.cpp
    src/app/main.cpp
    src/ui/MainWindow.cpp
    src/ui/DesignerCanvas.cpp
    src/ui/resources/ImageResourceCache.cpp
    src/ui/WidgetMetrics.cpp
    src/ui/WidgetPalette.cpp
    src/ui/PropertyInspector.cpp
    src/ui/ProjectTree.cpp
    src/ui/Splitter.cpp
    src/ui/editors/TextEditControl.cpp
    src/ui/editors/DropdownControl.cpp
    src/model/ProjectDocument.cpp
    src/model/ProjectResource.cpp
    src/model/FormNode.cpp
    src/model/BoxSizerLayout.cpp
    src/model/LayoutEngine.cpp
    src/model/WidgetDefinition.cpp
    src/model/WidgetItemUtils.cpp
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
    src/commands/CommandRegistry.cpp
    src/commands/UndoRedoStack.cpp
    src/utils/FileUtils.cpp
    src/utils/CppIdentifier.cpp
    src/utils/IdGenerator.cpp
    src/utils/AppSettings.cpp
    src/utils/LookAndFeelPresetStore.cpp
    src/utils/UiTimer.cpp
    src/utils/ProjectRecovery.cpp
    src/utils/NativeFileDialogs.cpp
    src/utils/RecentFiles.cpp
)

target_include_directories(VisiForm
    PRIVATE
        src
        .
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
    target_sources(VisiForm PRIVATE
        resources/windows/VisiForm.rc
    )

    set_target_properties(VisiForm PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    target_link_libraries(VisiForm PRIVATE comdlg32)
endif()

# Optional test wiring. Tests are built only when explicitly enabled.
option(VISIFORM_BUILD_TESTS "Build VisiForm unit tests (Catch2)" OFF)
if (VISIFORM_BUILD_TESTS)
    if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    else()
        message(WARNING "VISIFORM_BUILD_TESTS is ON but tests/CMakeLists.txt is missing.")
    endif()
endif()
