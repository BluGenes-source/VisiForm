cmake_minimum_required(VERSION 3.24)

project(VisiForm
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(FetchContent)

# TODO: Pin Visage to a known-good commit hash instead of main once the project builds cleanly.
set(VISAGE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(VISAGE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(VISAGE_ENABLE_WIDGETS ON CACHE BOOL "" FORCE)
set(VISAGE_AMALGAMATED_BUILD ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    visage
    GIT_REPOSITORY https://github.com/VitalAudio/visage.git
    GIT_TAG main
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(visage)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_executable(VisiForm
    src/app/main.cpp
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
