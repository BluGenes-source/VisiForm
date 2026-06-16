#include "model/WidgetRegistry.h"
#include "serialization/JsonProjectReader.h"
#include "serialization/JsonProjectWriter.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <vector>

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

TEST_CASE("WidgetRegistry exposes event signature metadata for supported widgets")
{
    struct ExpectedEvent {
        WidgetType type;
        std::string key;
        std::string signatureKind;
    };

    const std::vector<ExpectedEvent> expectedEvents{
        { WidgetType::FormWindow, "onLoad", "void_event" },
        { WidgetType::FormWindow, "onClose", "void_event" },
        { WidgetType::Button, "onClick", "void_event" },
        { WidgetType::Button, "onRelease", "void_event" },
        { WidgetType::Button, "onDoubleClick", "void_event" },
        { WidgetType::TextBox, "onTextChanged", "string_event" },
        { WidgetType::CheckBox, "onToggle", "bool_event" },
        { WidgetType::RadioButton, "onSelected", "bool_event" },
        { WidgetType::Slider, "onChanged", "float_event" },
        { WidgetType::ScrollBar, "onChanged", "float_event" },
        { WidgetType::ColorPicker, "onChanged", "string_event" },
        { WidgetType::ComboBox, "onChanged", "void_event" },
        { WidgetType::ListBox, "onChanged", "void_event" },
        { WidgetType::ListBox, "onDoubleClick", "void_event" },
        { WidgetType::TableGrid, "onSelectionChanged", "void_event" },
        { WidgetType::TableGrid, "onCellDoubleClick", "void_event" },
        { WidgetType::TreeView, "onChanged", "void_event" },
        { WidgetType::TreeView, "onDoubleClick", "void_event" },
        { WidgetType::ModalDialog, "onAccepted", "void_event" },
        { WidgetType::ModalDialog, "onCancelled", "void_event" }
    };

    for (const auto& expected : expectedEvents) {
        const auto* definition = WidgetRegistry::instance().find(expected.type);
        REQUIRE(definition != nullptr);
        const auto iterator = std::find_if(definition->events.begin(), definition->events.end(),
            [&expected](const auto& event) {
                return event.key == expected.key;
            });
        REQUIRE(iterator != definition->events.end());
        CHECK(iterator->handlerSignatureKind == expected.signatureKind);
    }
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

TEST_CASE("Blank event handler properties survive project JSON round-trip")
{
    ProjectDocument document = ProjectDocument::createDefault();
    auto* button = document.findWidgetById("button_hello");
    REQUIRE(button != nullptr);

    button->setProperty("onClick", "");

    JsonProjectWriter writer;
    JsonProjectReader reader;
    const std::string jsonText = writer.writeToString(document);

    std::string errorMessage;
    const auto loaded = reader.readFromString(jsonText, errorMessage);

    REQUIRE(errorMessage.empty());
    REQUIRE(loaded.has_value());

    const auto* loadedButton = loaded->findWidgetById("button_hello");
    REQUIRE(loadedButton != nullptr);
    REQUIRE(loadedButton->getProperty("onClick") != nullptr);
    REQUIRE(loadedButton->getStringProperty("onClick", "missing").empty());
}
