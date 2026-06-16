#include "model/WidgetRegistry.h"
#include "serialization/JsonProjectReader.h"
#include "serialization/JsonProjectWriter.h"

#include <catch2/catch_test_macros.hpp>

using visiform::model::ProjectDocument;
using visiform::model::WidgetRegistry;
using visiform::model::WidgetType;
using visiform::serialization::JsonProjectReader;
using visiform::serialization::JsonProjectWriter;

TEST_CASE("WidgetRegistry exposes supported button events from metadata")
{
    const auto* definition = WidgetRegistry::instance().find(WidgetType::Button);

    REQUIRE(definition != nullptr);
    REQUIRE(definition->events.size() == 3);
    REQUIRE(definition->events[0].key == "onClick");
    REQUIRE(definition->events[1].key == "onRelease");
    REQUIRE(definition->events[2].key == "onDoubleClick");
}

TEST_CASE("Default widgets initialize event properties from widget metadata")
{
    const auto button = WidgetRegistry::instance().createDefaultWidget(WidgetType::Button, "button_test");

    REQUIRE(button.getProperty("onClick") != nullptr);
    REQUIRE(button.getProperty("onRelease") != nullptr);
    REQUIRE(button.getProperty("onDoubleClick") != nullptr);
    REQUIRE(button.getStringProperty("onClick", "missing").empty());
}

TEST_CASE("Event handler properties survive project JSON round-trip")
{
    ProjectDocument document = ProjectDocument::createDefault();
    auto* button = document.findWidgetById("button_hello");
    REQUIRE(button != nullptr);

    button->setProperty("onClick", "handleClick");
    button->setProperty("onRelease", "handleRelease");
    button->setProperty("onDoubleClick", "handleDoubleClick");

    JsonProjectWriter writer;
    JsonProjectReader reader;
    const std::string jsonText = writer.writeToString(document);

    std::string errorMessage;
    const auto loaded = reader.readFromString(jsonText, errorMessage);

    REQUIRE(errorMessage.empty());
    REQUIRE(loaded.has_value());

    const auto* loadedButton = loaded->findWidgetById("button_hello");
    REQUIRE(loadedButton != nullptr);
    REQUIRE(loadedButton->getStringProperty("onClick", {}) == "handleClick");
    REQUIRE(loadedButton->getStringProperty("onRelease", {}) == "handleRelease");
    REQUIRE(loadedButton->getStringProperty("onDoubleClick", {}) == "handleDoubleClick");
}
