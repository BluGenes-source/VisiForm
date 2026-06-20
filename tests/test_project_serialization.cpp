#include "model/ProjectDocument.h"
#include "model/LookAndFeelRegistry.h"
#include "serialization/JsonProjectReader.h"
#include "serialization/JsonProjectWriter.h"

#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

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

TEST_CASE("Project look and feel overrides serialize sparsely and resolve over a changed base preset")
{
    ProjectDocument document = ProjectDocument::createDefault();
    document.lookAndFeelId = "VisiFormLight";
    document.lookAndFeelOverrides.controlSurfaceColor = "#123456";
    document.lookAndFeelOverrides.borderThickness = 3.5f;
    document.lookAndFeelOverrides.controlPadding = 12.0f;

    JsonProjectWriter writer;
    JsonProjectReader reader;
    const std::string jsonText = writer.writeToString(document);
    const auto json = nlohmann::json::parse(jsonText);
    REQUIRE(json.contains("lookAndFeelOverrides"));
    CHECK(json["lookAndFeelOverrides"].size() == 3);
    CHECK(json["lookAndFeelOverrides"]["controlSurfaceColor"] == "#123456");

    std::string errorMessage;
    const auto loaded = reader.readFromString(jsonText, errorMessage);
    REQUIRE(errorMessage.empty());
    REQUIRE(loaded.has_value());
    CHECK(loaded->lookAndFeelOverrides == document.lookAndFeelOverrides);

    const auto& registry = visiform::model::LookAndFeelRegistry::instance();
    auto resolved = registry.resolve(*loaded, loaded->root);
    CHECK(resolved.id == "VisiFormLight");
    CHECK(resolved.controlSurfaceColor == "#123456");
    CHECK(resolved.borderThickness == 3.5f);
    CHECK(resolved.controlPadding == 12.0f);

    ProjectDocument changedBase = *loaded;
    changedBase.lookAndFeelId = "FlatClassic";
    resolved = registry.resolve(changedBase, changedBase.root);
    CHECK(resolved.id == "FlatClassic");
    CHECK(resolved.applicationSurfaceColor == "#E7EAEE");
    CHECK(resolved.controlSurfaceColor == "#123456");
}

TEST_CASE("Projects without look and feel overrides remain compact")
{
    const ProjectDocument document = ProjectDocument::createDefault();
    const auto json = nlohmann::json::parse(JsonProjectWriter{}.writeToString(document));
    CHECK_FALSE(json.contains("lookAndFeelOverrides"));
}

TEST_CASE("Invalid project look and feel overrides fall back or clamp safely")
{
    ProjectDocument document = ProjectDocument::createDefault();
    document.lookAndFeelOverrides.borderColor = "not-a-color";
    document.lookAndFeelOverrides.borderThickness = -20.0f;
    document.lookAndFeelOverrides.cornerRadius = 500.0f;

    const auto resolved = visiform::model::LookAndFeelRegistry::instance().resolve(document, document.root);
    CHECK(resolved.borderColor == "#97A3B7");
    CHECK(resolved.borderThickness == 0.0f);
    CHECK(resolved.cornerRadius == 50.0f);
}

TEST_CASE("Widget appearance overrides serialize sparsely and resolve after project overrides")
{
    ProjectDocument document = ProjectDocument::createDefault();
    document.lookAndFeelId = "VisiFormLight";
    document.lookAndFeelOverrides.controlSurfaceColor = "#112233";
    document.lookAndFeelOverrides.borderThickness = 2.0f;

    auto& button = document.root.children.front();
    button.appearanceOverrides.controlSurfaceColor = "#445566";
    button.appearanceOverrides.textColor = "#F0E0D0";
    button.appearanceOverrides.focusOutlineColor = "#ABCDEF";
    button.appearanceOverrides.borderThickness = 5.0f;
    button.appearanceOverrides.cornerRadius = 13.0f;
    button.appearanceOverrides.controlPadding = 11.0f;

    const auto json = nlohmann::json::parse(JsonProjectWriter{}.writeToString(document));
    REQUIRE(json["root"]["children"][0].contains("appearanceOverrides"));
    const auto& serialized = json["root"]["children"][0]["appearanceOverrides"];
    CHECK(serialized.size() == 6);
    CHECK(serialized["controlSurfaceColor"] == "#445566");
    CHECK_FALSE(serialized.contains("borderColor"));

    std::string errorMessage;
    const auto loaded = JsonProjectReader{}.readFromString(json.dump(), errorMessage);
    REQUIRE(errorMessage.empty());
    REQUIRE(loaded.has_value());
    const auto& loadedButton = loaded->root.children.front();
    CHECK(loadedButton.appearanceOverrides == button.appearanceOverrides);

    const auto resolved = visiform::model::LookAndFeelRegistry::instance().resolve(*loaded, loadedButton);
    CHECK(resolved.controlSurfaceColor == "#445566");
    CHECK(resolved.primaryTextColor == "#F0E0D0");
    CHECK(resolved.focusOutlineColor == "#ABCDEF");
    CHECK(resolved.borderThickness == 5.0f);
    CHECK(resolved.cornerRadius == 13.0f);
    CHECK(resolved.controlPadding == 11.0f);
    CHECK(resolved.borderColor == "#B8C2D0");
}

TEST_CASE("Widgets without appearance overrides remain compact and value copies preserve overrides")
{
    ProjectDocument document = ProjectDocument::createDefault();
    auto& button = document.root.children.front();
    const auto compact = nlohmann::json::parse(JsonProjectWriter{}.writeToString(document));
    CHECK_FALSE(compact["root"]["children"][0].contains("appearanceOverrides"));

    button.appearanceOverrides.accentColor = "#123ABC";
    button.appearanceOverrides.cornerRadius = 7.0f;
    const auto copied = button;
    CHECK(copied.appearanceOverrides == button.appearanceOverrides);
}

TEST_CASE("Invalid widget appearance override values fall back or clamp safely")
{
    ProjectDocument document = ProjectDocument::createDefault();
    auto& button = document.root.children.front();
    button.appearanceOverrides.borderColor = "invalid";
    button.appearanceOverrides.borderThickness = -5.0f;
    button.appearanceOverrides.cornerRadius = 500.0f;
    button.appearanceOverrides.controlPadding = 100.0f;

    const auto resolved = visiform::model::LookAndFeelRegistry::instance().resolve(document, button);
    CHECK(resolved.borderColor == "#97A3B7");
    CHECK(resolved.borderThickness == 0.0f);
    CHECK(resolved.cornerRadius == 50.0f);
    CHECK(resolved.controlPadding == 40.0f);
}

TEST_CASE("Widget state appearance overrides serialize sparsely and resolve after normal appearance")
{
    ProjectDocument document = ProjectDocument::createDefault();
    auto& button = document.root.children.front();
    button.appearanceOverrides.controlSurfaceColor = "#112233";
    button.appearanceOverrides.borderColor = "#445566";
    auto& hover = button.stateAppearanceOverrides[
        visiform::model::WidgetAppearanceState::Hover];
    hover.controlSurfaceColor = "#778899";
    hover.borderColor = "#AABBCC";
    auto& focused = button.stateAppearanceOverrides[
        visiform::model::WidgetAppearanceState::Focused];
    focused.focusOutlineColor = "#DDEEFF";

    const auto json = nlohmann::json::parse(JsonProjectWriter{}.writeToString(document));
    const auto& appearance = json["root"]["children"][0]["appearanceOverrides"];
    REQUIRE(appearance.contains("states"));
    REQUIRE(appearance["states"].contains("hover"));
    CHECK(appearance["states"]["hover"].size() == 2);
    CHECK(appearance["states"]["hover"]["controlSurfaceColor"] == "#778899");
    CHECK_FALSE(appearance["states"].contains("normal"));

    std::string errorMessage;
    const auto loaded = JsonProjectReader{}.readFromString(json.dump(), errorMessage);
    REQUIRE(errorMessage.empty());
    REQUIRE(loaded.has_value());
    const auto& loadedButton = loaded->root.children.front();
    CHECK(loadedButton.stateAppearanceOverrides == button.stateAppearanceOverrides);

    const auto resolved = visiform::model::LookAndFeelRegistry::instance().resolve(
        *loaded,
        loadedButton,
        visiform::model::WidgetAppearanceState::Hover,
        true);
    CHECK(resolved.controlSurfaceColor == "#112233");
    CHECK(resolved.hoverStateColor == "#778899");
    CHECK(resolved.borderColor == "#AABBCC");
    CHECK(resolved.focusOutlineColor == "#DDEEFF");

    const auto copied = button;
    CHECK(copied.stateAppearanceOverrides == button.stateAppearanceOverrides);
}

TEST_CASE("Unsupported widget states are filtered by the shared compatibility map")
{
    using visiform::model::LookAndFeelRegistry;
    using visiform::model::WidgetAppearanceState;
    using visiform::model::WidgetType;

    CHECK(LookAndFeelRegistry::supportsWidgetState(WidgetType::Button, WidgetAppearanceState::Pressed));
    CHECK_FALSE(LookAndFeelRegistry::supportsWidgetState(
        WidgetType::Button, WidgetAppearanceState::CheckedOrSelected));
    CHECK(LookAndFeelRegistry::supportsWidgetState(
        WidgetType::CheckBox, WidgetAppearanceState::CheckedOrSelected));
    CHECK_FALSE(LookAndFeelRegistry::supportsWidgetState(
        WidgetType::ProgressBar, WidgetAppearanceState::Hover));
    CHECK(LookAndFeelRegistry::supportsWidgetState(
        WidgetType::ProgressBar, WidgetAppearanceState::Disabled));
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
