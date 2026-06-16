#include "model/BoxSizerLayout.h"
#include "model/LayoutEngine.h"
#include "model/ProjectDocument.h"
#include "model/WidgetRegistry.h"
#include "serialization/JsonProjectReader.h"
#include "serialization/JsonProjectWriter.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <utility>

using visiform::model::WidgetRegistry;
using visiform::model::WidgetType;

namespace {

visiform::model::WidgetNode makeWidget(WidgetType type, const char* id, float width, float height)
{
    auto widget = WidgetRegistry::instance().createDefaultWidget(type, id);
    widget.bounds = { 0.0f, 0.0f, width, height };
    return widget;
}

void appendSizerChild(visiform::model::WidgetNode& sizer, visiform::model::WidgetNode child)
{
    visiform::model::applyDefaultSizerItemLayout(child);
    sizer.appendChild(std::move(child));
}

} // namespace

TEST_CASE("Vertical BoxSizer honors padding gap and zero proportions")
{
    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 200.0f, 160.0f);
    sizer.setProperty("orientation", "Vertical");
    sizer.setProperty("paddingLeft", 10);
    sizer.setProperty("paddingTop", 5);
    sizer.setProperty("paddingRight", 10);
    sizer.setProperty("paddingBottom", 5);
    sizer.setProperty("gap", 4);

    auto first = makeWidget(WidgetType::Spacer, "spacer_1", 0.0f, 0.0f);
    first.setProperty("spacer.kind", "Fixed");
    first.setProperty("spacer.size", 20);
    first.setProperty("sizerItem.expand", false);
    auto second = makeWidget(WidgetType::Spacer, "spacer_2", 0.0f, 0.0f);
    second.setProperty("spacer.kind", "Fixed");
    second.setProperty("spacer.size", 30);
    second.setProperty("sizerItem.expand", true);

    appendSizerChild(sizer, std::move(first));
    appendSizerChild(sizer, std::move(second));

    visiform::model::layoutBoxSizerChildren(sizer);

    REQUIRE(sizer.children[0].bounds.x == Catch::Approx(10.0f));
    REQUIRE(sizer.children[0].bounds.y == Catch::Approx(5.0f));
    REQUIRE(sizer.children[0].bounds.width == Catch::Approx(20.0f));
    REQUIRE(sizer.children[0].bounds.height == Catch::Approx(20.0f));

    REQUIRE(sizer.children[1].bounds.x == Catch::Approx(10.0f));
    REQUIRE(sizer.children[1].bounds.y == Catch::Approx(29.0f));
    REQUIRE(sizer.children[1].bounds.width == Catch::Approx(180.0f));
    REQUIRE(sizer.children[1].bounds.height == Catch::Approx(30.0f));
}

TEST_CASE("BoxSizer distributes extra main-axis space by proportion deterministically")
{
    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 100.0f, 100.0f);
    sizer.setProperty("orientation", "Vertical");
    sizer.setProperty("paddingLeft", 0);
    sizer.setProperty("paddingTop", 0);
    sizer.setProperty("paddingRight", 0);
    sizer.setProperty("paddingBottom", 0);
    sizer.setProperty("gap", 0);

    auto fixed = makeWidget(WidgetType::Spacer, "spacer_1", 0.0f, 0.0f);
    fixed.setProperty("spacer.kind", "Fixed");
    fixed.setProperty("spacer.size", 10);
    fixed.setProperty("sizerItem.proportion", 0);
    auto one = makeWidget(WidgetType::Spacer, "spacer_2", 0.0f, 0.0f);
    one.setProperty("spacer.kind", "Fixed");
    one.setProperty("spacer.size", 10);
    one.setProperty("sizerItem.proportion", 1);
    auto two = makeWidget(WidgetType::Spacer, "spacer_3", 0.0f, 0.0f);
    two.setProperty("spacer.kind", "Fixed");
    two.setProperty("spacer.size", 10);
    two.setProperty("sizerItem.proportion", 2);

    appendSizerChild(sizer, std::move(fixed));
    appendSizerChild(sizer, std::move(one));
    appendSizerChild(sizer, std::move(two));

    visiform::model::layoutBoxSizerChildren(sizer);

    REQUIRE(sizer.children[0].bounds.height == Catch::Approx(10.0f));
    REQUIRE(sizer.children[1].bounds.height == Catch::Approx(34.0f));
    REQUIRE(sizer.children[2].bounds.height == Catch::Approx(56.0f));
}

TEST_CASE("BoxSizer supports item borders, alignment, and fixed spacers")
{
    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 120.0f, 80.0f);
    sizer.setProperty("orientation", "Horizontal");
    sizer.setProperty("paddingLeft", 0);
    sizer.setProperty("paddingTop", 0);
    sizer.setProperty("paddingRight", 0);
    sizer.setProperty("paddingBottom", 0);
    sizer.setProperty("gap", 0);

    auto spacer = makeWidget(WidgetType::Spacer, "spacer_1", 0.0f, 0.0f);
    spacer.setProperty("spacer.kind", "Fixed");
    spacer.setProperty("spacer.size", 24);

    auto centered = makeWidget(WidgetType::Spacer, "spacer_2", 0.0f, 0.0f);
    centered.setProperty("spacer.kind", "Fixed");
    centered.setProperty("spacer.size", 20);
    centered.setProperty("sizerItem.expand", false);
    centered.setProperty("sizerItem.alignment", "Center");
    centered.setProperty("sizerItem.border", 4);
    centered.setProperty("sizerItem.borderSides", "Top|Bottom");

    appendSizerChild(sizer, std::move(spacer));
    appendSizerChild(sizer, std::move(centered));

    visiform::model::layoutBoxSizerChildren(sizer);

    REQUIRE(sizer.children[0].bounds.width == Catch::Approx(24.0f));
    REQUIRE(sizer.children[1].bounds.x == Catch::Approx(24.0f));
    REQUIRE(sizer.children[1].bounds.y == Catch::Approx(30.0f));
    REQUIRE(sizer.children[1].bounds.height == Catch::Approx(20.0f));
}

TEST_CASE("Sizer item minimum width controls vertical sizer cross-axis size")
{
    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 260.0f, 80.0f);
    sizer.setProperty("orientation", "Vertical");
    sizer.setProperty("paddingLeft", 0);
    sizer.setProperty("paddingTop", 0);
    sizer.setProperty("paddingRight", 0);
    sizer.setProperty("paddingBottom", 0);
    sizer.setProperty("gap", 0);

    auto button = makeWidget(WidgetType::Button, "button_1", 40.0f, 20.0f);
    button.setProperty("sizerItem.expand", false);
    button.setProperty("sizerItem.minimumWidth", 180);

    appendSizerChild(sizer, std::move(button));
    visiform::model::layoutBoxSizerChildren(sizer);

    REQUIRE(sizer.children[0].bounds.width == Catch::Approx(180.0f));
}

TEST_CASE("Sizer item minimum height controls vertical sizer main-axis size")
{
    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 120.0f, 160.0f);
    sizer.setProperty("orientation", "Vertical");
    sizer.setProperty("paddingLeft", 0);
    sizer.setProperty("paddingTop", 0);
    sizer.setProperty("paddingRight", 0);
    sizer.setProperty("paddingBottom", 0);
    sizer.setProperty("gap", 0);

    auto button = makeWidget(WidgetType::Button, "button_1", 40.0f, 20.0f);
    button.setProperty("sizerItem.minimumHeight", 72);

    appendSizerChild(sizer, std::move(button));
    visiform::model::layoutBoxSizerChildren(sizer);

    REQUIRE(sizer.children[0].bounds.height == Catch::Approx(72.0f));
}

TEST_CASE("Sizer item preferred height can grow and later shrink without rewriting the minimum")
{
    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 120.0f, 160.0f);
    sizer.setProperty("orientation", "Vertical");
    sizer.setProperty("paddingLeft", 0);
    sizer.setProperty("paddingTop", 0);
    sizer.setProperty("paddingRight", 0);
    sizer.setProperty("paddingBottom", 0);
    sizer.setProperty("gap", 0);

    auto button = makeWidget(WidgetType::Button, "button_1", 40.0f, 20.0f);
    button.setProperty("sizerItem.minimumHeight", 50);
    button.setProperty("sizerItem.preferredHeight", 72);

    appendSizerChild(sizer, std::move(button));
    visiform::model::layoutBoxSizerChildren(sizer);
    REQUIRE(sizer.children[0].bounds.height == Catch::Approx(72.0f));

    sizer.children[0].setProperty("sizerItem.preferredHeight", 36);
    visiform::model::layoutBoxSizerChildren(sizer);
    REQUIRE(sizer.children[0].bounds.height == Catch::Approx(50.0f));

    sizer.children[0].setProperty("sizerItem.preferredHeight", 96);
    visiform::model::layoutBoxSizerChildren(sizer);
    REQUIRE(sizer.children[0].bounds.height == Catch::Approx(96.0f));
}

TEST_CASE("Resized Sizer recomputes child layout from new bounds")
{
    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 120.0f, 80.0f);
    sizer.setProperty("orientation", "Vertical");
    sizer.setProperty("paddingLeft", 0);
    sizer.setProperty("paddingTop", 0);
    sizer.setProperty("paddingRight", 0);
    sizer.setProperty("paddingBottom", 0);
    sizer.setProperty("gap", 0);

    auto button = makeWidget(WidgetType::Button, "button_1", 40.0f, 20.0f);
    button.setProperty("sizerItem.expand", true);
    button.setProperty("sizerItem.proportion", 1);

    appendSizerChild(sizer, std::move(button));
    visiform::model::layoutBoxSizerChildren(sizer);

    REQUIRE(sizer.children[0].bounds.width == Catch::Approx(120.0f));
    REQUIRE(sizer.children[0].bounds.height == Catch::Approx(80.0f));

    sizer.bounds.width = 220.0f;
    sizer.bounds.height = 140.0f;
    visiform::model::layoutBoxSizerChildren(sizer);

    REQUIRE(sizer.children[0].bounds.width == Catch::Approx(220.0f));
    REQUIRE(sizer.children[0].bounds.height == Catch::Approx(140.0f));
}

TEST_CASE("Nested BoxSizer relayout propagates from parent resize")
{
    auto outer = makeWidget(WidgetType::Sizer, "sizer_outer", 300.0f, 200.0f);
    outer.setProperty("orientation", "Vertical");
    outer.setProperty("paddingLeft", 0);
    outer.setProperty("paddingTop", 0);
    outer.setProperty("paddingRight", 0);
    outer.setProperty("paddingBottom", 0);
    outer.setProperty("gap", 0);

    auto inner = makeWidget(WidgetType::Sizer, "sizer_inner", 10.0f, 10.0f);
    inner.setProperty("orientation", "Horizontal");
    inner.setProperty("paddingLeft", 0);
    inner.setProperty("paddingTop", 0);
    inner.setProperty("paddingRight", 0);
    inner.setProperty("paddingBottom", 0);
    inner.setProperty("gap", 0);
    inner.setProperty("sizerItem.expand", true);
    inner.setProperty("sizerItem.proportion", 1);

    auto button = makeWidget(WidgetType::Button, "button_1", 40.0f, 20.0f);
    button.setProperty("sizerItem.expand", true);
    button.setProperty("sizerItem.proportion", 1);

    appendSizerChild(inner, std::move(button));
    appendSizerChild(outer, std::move(inner));

    visiform::model::layoutBoxSizerChildren(outer);

    REQUIRE(outer.children[0].bounds.width == Catch::Approx(300.0f));
    REQUIRE(outer.children[0].bounds.height == Catch::Approx(200.0f));
    REQUIRE(outer.children[0].children[0].bounds.width == Catch::Approx(300.0f));
    REQUIRE(outer.children[0].children[0].bounds.height == Catch::Approx(200.0f));

    outer.bounds.width = 180.0f;
    outer.bounds.height = 90.0f;
    visiform::model::layoutBoxSizerChildren(outer);

    REQUIRE(outer.children[0].bounds.width == Catch::Approx(180.0f));
    REQUIRE(outer.children[0].bounds.height == Catch::Approx(90.0f));
    REQUIRE(outer.children[0].children[0].bounds.width == Catch::Approx(180.0f));
    REQUIRE(outer.children[0].children[0].bounds.height == Catch::Approx(90.0f));
}

TEST_CASE("Sizer item minimum sizes round-trip through JSON")
{
    visiform::model::ProjectDocument document = visiform::model::ProjectDocument::createDefault();
    document.root.children.clear();

    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 240.0f, 160.0f);
    sizer.setProperty("orientation", "Vertical");
    sizer.setProperty("paddingLeft", 0);
    sizer.setProperty("paddingTop", 0);
    sizer.setProperty("paddingRight", 0);
    sizer.setProperty("paddingBottom", 0);
    sizer.setProperty("gap", 0);

    auto button = makeWidget(WidgetType::Button, "button_1", 40.0f, 20.0f);
    button.setProperty("sizerItem.expand", false);
    button.setProperty("sizerItem.minimumWidth", 180);
    button.setProperty("sizerItem.minimumHeight", 72);

    appendSizerChild(sizer, std::move(button));
    document.root.appendChild(std::move(sizer));
    document.refreshHierarchyMetadata();

    visiform::serialization::JsonProjectWriter writer;
    visiform::serialization::JsonProjectReader reader;
    std::string error;
    const auto loaded = reader.readFromString(writer.writeToString(document), error);

    REQUIRE(error.empty());
    REQUIRE(loaded.has_value());

    const auto* loadedButton = loaded->findWidgetById("button_1");
    REQUIRE(loadedButton != nullptr);
    REQUIRE(loadedButton->getIntProperty("sizerItem.minimumWidth", -1) == 180);
    REQUIRE(loadedButton->getIntProperty("sizerItem.minimumHeight", -1) == 72);
}

TEST_CASE("Sizer item preferred sizes round-trip through JSON")
{
    visiform::model::ProjectDocument document = visiform::model::ProjectDocument::createDefault();
    document.root.children.clear();

    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 240.0f, 160.0f);
    sizer.setProperty("orientation", "Vertical");
    sizer.setProperty("paddingLeft", 0);
    sizer.setProperty("paddingTop", 0);
    sizer.setProperty("paddingRight", 0);
    sizer.setProperty("paddingBottom", 0);
    sizer.setProperty("gap", 0);

    auto button = makeWidget(WidgetType::Button, "button_1", 40.0f, 20.0f);
    button.setProperty("sizerItem.preferredWidth", 180);
    button.setProperty("sizerItem.preferredHeight", 72);

    appendSizerChild(sizer, std::move(button));
    document.root.appendChild(std::move(sizer));
    document.refreshHierarchyMetadata();

    visiform::serialization::JsonProjectWriter writer;
    visiform::serialization::JsonProjectReader reader;
    std::string error;
    const auto loaded = reader.readFromString(writer.writeToString(document), error);

    REQUIRE(error.empty());
    REQUIRE(loaded.has_value());

    const auto* loadedButton = loaded->findWidgetById("button_1");
    REQUIRE(loadedButton != nullptr);
    REQUIRE(loadedButton->getIntProperty("sizerItem.preferredWidth", -1) == 180);
    REQUIRE(loadedButton->getIntProperty("sizerItem.preferredHeight", -1) == 72);
}

TEST_CASE("Legacy Sizer padding migrates to side padding on JSON load")
{
    const std::string json = R"json(
{
  "schemaVersion": 1,
  "projectName": "SizerProject",
  "mainFormClassName": "AppMainWindow",
  "selectedWidgetId": "sizer_main",
  "root": {
    "id": "form_main",
    "name": "MainWindow",
    "type": "FormWindow",
    "bounds": { "x": 0, "y": 0, "width": 400, "height": 300 },
    "properties": {},
    "children": [
      {
        "id": "sizer_main",
        "name": "sizer1",
        "type": "Sizer",
        "bounds": { "x": 0, "y": 0, "width": 300, "height": 200 },
        "properties": { "orientation": "Vertical", "padding": 11, "gap": 3 },
        "children": []
      }
    ]
  }
}
)json";

    visiform::serialization::JsonProjectReader reader;
    std::string error;
    const auto loaded = reader.readFromString(json, error);

    REQUIRE(error.empty());
    REQUIRE(loaded.has_value());

    const auto* sizer = loaded->findWidgetById("sizer_main");
    REQUIRE(sizer != nullptr);
    REQUIRE(sizer->getIntProperty("paddingLeft", 0) == 11);
    REQUIRE(sizer->getIntProperty("paddingTop", 0) == 11);
    REQUIRE(sizer->getIntProperty("paddingRight", 0) == 11);
    REQUIRE(sizer->getIntProperty("paddingBottom", 0) == 11);
}
