#include "model/ProjectDocument.h"
#include "serialization/JsonProjectReader.h"
#include "serialization/JsonProjectWriter.h"

#include <catch2/catch_test_macros.hpp>

using visiform::model::ProjectDocument;
using visiform::serialization::JsonProjectReader;
using visiform::serialization::JsonProjectWriter;

TEST_CASE("ProjectDocument createDefault builds the expected starter model")
{
    const ProjectDocument document = ProjectDocument::createDefault();

    REQUIRE(document.schemaVersion == 1);
    REQUIRE(document.projectName == "UntitledVisiFormProject");
    REQUIRE(document.mainFormClassName == "MainWindow");
    REQUIRE(document.root.id == "form_main");
    REQUIRE(document.root.name == "MainWindow");
    REQUIRE(document.root.typeName() == "FormWindow");
    REQUIRE(document.root.children.size() == 1);
    REQUIRE(document.selectedWidgetId == "button_hello");
}

TEST_CASE("ProjectDocument round-trips through JSON")
{
    const ProjectDocument document = ProjectDocument::createDefault();
    JsonProjectWriter writer;
    JsonProjectReader reader;

    const std::string jsonText = writer.writeToString(document);
    std::string errorMessage;
    const auto loaded = reader.readFromString(jsonText, errorMessage);

    REQUIRE(errorMessage.empty());
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->schemaVersion == document.schemaVersion);
    REQUIRE(loaded->projectName == document.projectName);
    REQUIRE(loaded->mainFormClassName == document.mainFormClassName);
    REQUIRE(loaded->root.id == document.root.id);
    REQUIRE(loaded->root.name == document.root.name);
    REQUIRE(loaded->root.typeName() == document.root.typeName());
    REQUIRE(loaded->selectedWidgetId == document.selectedWidgetId);
    REQUIRE(loaded->root.children.size() == 1);
    REQUIRE(loaded->root.children.front().getStringProperty("text", "") == "Click Me");
}

TEST_CASE("Invalid JSON returns an error")
{
    JsonProjectReader reader;
    std::string errorMessage;

    const auto loaded = reader.readFromString("{ not valid json", errorMessage);

    REQUIRE_FALSE(loaded.has_value());
    REQUIRE_FALSE(errorMessage.empty());
}

TEST_CASE("Invalid widget type returns an error")
{
    JsonProjectReader reader;
    std::string errorMessage;

    const auto loaded = reader.readFromString(R"json(
{
  "schemaVersion": 1,
  "projectName": "BrokenProject",
  "mainFormClassName": "MainWindow",
  "selectedWidgetId": "form_main",
  "root": {
    "id": "form_main",
    "name": "MainWindow",
    "type": "NotAWidget",
    "bounds": {
      "x": 0,
      "y": 0,
      "width": 900,
      "height": 600
    },
    "properties": {},
    "children": []
  }
}
)json", errorMessage);

    REQUIRE_FALSE(loaded.has_value());
    REQUIRE_FALSE(errorMessage.empty());
}
