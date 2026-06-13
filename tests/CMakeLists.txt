find_package(Catch2 CONFIG REQUIRED)

add_executable(VisiFormTests
    test_project_serialization.cpp
    test_box_sizer_layout.cpp
    ${CMAKE_SOURCE_DIR}/src/model/ProjectDocument.cpp
    ${CMAKE_SOURCE_DIR}/src/model/FormNode.cpp
    ${CMAKE_SOURCE_DIR}/src/model/BoxSizerLayout.cpp
    ${CMAKE_SOURCE_DIR}/src/model/LayoutEngine.cpp
    ${CMAKE_SOURCE_DIR}/src/model/WidgetRegistry.cpp
    ${CMAKE_SOURCE_DIR}/src/model/WidgetDefinition.cpp
    ${CMAKE_SOURCE_DIR}/src/model/WidgetItemUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/model/WidgetNode.cpp
    ${CMAKE_SOURCE_DIR}/src/model/PropertyValue.cpp
    ${CMAKE_SOURCE_DIR}/src/serialization/JsonProjectReader.cpp
    ${CMAKE_SOURCE_DIR}/src/serialization/JsonProjectWriter.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/FileUtils.cpp
)

target_include_directories(VisiFormTests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(VisiFormTests
    PRIVATE
        Catch2::Catch2WithMain
        nlohmann_json::nlohmann_json
)

include(Catch)
catch_discover_tests(VisiFormTests)
