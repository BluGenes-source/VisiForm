#include "model/ProjectDocument.h"
#include "model/LookAndFeelRegistry.h"
#include "serialization/JsonProjectReader.h"
#include "serialization/JsonProjectWriter.h"

#include <catch2/catch_test_macros.hpp>

#include <utility>

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

TEST_CASE("Legacy project without look and feel uses the established default")
{
    JsonProjectReader reader;
    std::string errorMessage;
    const auto loaded = reader.readFromString(R"json(
{
  "schemaVersion": 1,
  "projectName": "LegacyProject",
  "mainFormClassName": "MainWindow",
  "selectedWidgetId": "form_main",
  "root": {
    "id": "form_main",
    "name": "MainWindow",
    "type": "FormWindow",
    "bounds": { "x": 0, "y": 0, "width": 900, "height": 600 },
    "properties": {},
    "children": []
  }
}
)json", errorMessage);

    REQUIRE(errorMessage.empty());
    REQUIRE(loaded.has_value());
    CHECK(loaded->lookAndFeelId == "VisiFormDark");
}

TEST_CASE("Look and feel resolution preserves presets overrides and fallback")
{
    ProjectDocument document = ProjectDocument::createDefault();
    const auto& registry = visiform::model::LookAndFeelRegistry::instance();

    document.lookAndFeelId = "VisiFormLight";
    const auto light = registry.resolve(document, document.root);
    CHECK(light.id == "VisiFormLight");
    CHECK(light.applicationSurfaceColor == "#F2F4F8");
    CHECK(light.recessedSurfaceColor == "#E8ECF2");
    CHECK(light.hoverStateColor == "#EDF4FF");

    document.root.setProperty("fillColor", "#123456");
    const auto overridden = registry.resolve(document, document.root);
    CHECK(overridden.controlSurfaceColor == "#123456");

    document.lookAndFeelId = "UnknownPreset";
    document.root.setProperty("fillColor", "");
    const auto fallback = registry.resolve(document, document.root);
    CHECK(fallback.id == "VisiFormDark");
    CHECK(fallback.applicationSurfaceColor == "#1F242D");
    CHECK(fallback.highlightEdgeColor == "#C8D2E2");
}

TEST_CASE("ProjectDocument z-order commands move one sibling step and preserve selection")
{
    ProjectDocument document = ProjectDocument::createDefault();
    auto second = document.root.children.front();
    second.id = "button_second";
    second.name = "Second";
    auto third = second;
    third.id = "button_third";
    third.name = "Third";
    document.root.appendChild(std::move(second));
    document.root.appendChild(std::move(third));
    document.setSelection("button_hello");
    document.addToSelection("button_second");

    REQUIRE(document.selectedWidgetId == "button_second");
    REQUIRE(document.selectedWidgetIds().size() == 2);
    REQUIRE(document.bringWidgetForward("button_second"));
    CHECK(document.root.children[0].id == "button_hello");
    CHECK(document.root.children[1].id == "button_third");
    CHECK(document.root.children[2].id == "button_second");
    CHECK(document.selectedWidgetId == "button_second");
    CHECK(document.selectedWidgetIds().size() == 2);

    REQUIRE(document.sendWidgetBackward("button_second"));
    CHECK(document.root.children[0].id == "button_hello");
    CHECK(document.root.children[1].id == "button_second");
    CHECK(document.root.children[2].id == "button_third");
    CHECK_FALSE(document.sendWidgetBackward("button_hello"));
    CHECK_FALSE(document.bringWidgetForward("button_third"));
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
