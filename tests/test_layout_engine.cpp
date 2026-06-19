#include "model/ProjectDocument.h"
#include "model/LayoutEngine.h"
#include "model/WidgetPlacement.h"
#include "model/WidgetRegistry.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <utility>

using visiform::model::ProjectDocument;
using visiform::model::WidgetRegistry;
using visiform::model::WidgetType;

namespace {

visiform::model::WidgetNode makeWidget(WidgetType type, const char* id, float width, float height)
{
    auto widget = WidgetRegistry::instance().createDefaultWidget(type, id);
    widget.bounds = { 0.0f, 0.0f, width, height };
    return widget;
}

} // namespace

TEST_CASE("Anchored child relayouts after resizing its parent container")
{
    ProjectDocument document = ProjectDocument::createDefault();
    document.root.children.clear();

    auto panel = makeWidget(WidgetType::Panel, "panel_main", 100.0f, 100.0f);
    auto button = makeWidget(WidgetType::Button, "button_1", 30.0f, 20.0f);
    button.bounds = { 10.0f, 15.0f, 30.0f, 20.0f };
    button.setProperty("anchor", "Bottom Right");
    panel.appendChild(std::move(button));
    document.root.appendChild(std::move(panel));
    document.refreshHierarchyMetadata();

    const ProjectDocument beforeDocument = document;
    auto* resizedPanel = document.findWidgetById("panel_main");
    REQUIRE(resizedPanel != nullptr);
    resizedPanel->bounds.width = 160.0f;
    resizedPanel->bounds.height = 150.0f;

    document.applyLayoutFromPrevious(beforeDocument);

    const auto* resizedButton = document.findWidgetById("button_1");
    REQUIRE(resizedButton != nullptr);
    REQUIRE(resizedButton->bounds.x == Catch::Approx(70.0f));
    REQUIRE(resizedButton->bounds.y == Catch::Approx(65.0f));
    REQUIRE(resizedButton->bounds.width == Catch::Approx(30.0f));
    REQUIRE(resizedButton->bounds.height == Catch::Approx(20.0f));
}

TEST_CASE("Parent resize relayout propagates through nested sizer descendants")
{
    ProjectDocument document = ProjectDocument::createDefault();
    document.root.children.clear();

    auto panel = makeWidget(WidgetType::Panel, "panel_main", 120.0f, 80.0f);
    auto sizer = makeWidget(WidgetType::Sizer, "sizer_main", 120.0f, 80.0f);
    sizer.setProperty("dock", "Fill");
    sizer.setProperty("orientation", "Vertical");
    sizer.setProperty("paddingLeft", 0);
    sizer.setProperty("paddingTop", 0);
    sizer.setProperty("paddingRight", 0);
    sizer.setProperty("paddingBottom", 0);
    sizer.setProperty("gap", 0);

    auto button = makeWidget(WidgetType::Button, "button_1", 40.0f, 20.0f);
    button.setProperty("sizerItem.expand", true);
    button.setProperty("sizerItem.proportion", 1);
    sizer.appendChild(std::move(button));
    panel.appendChild(std::move(sizer));
    document.root.appendChild(std::move(panel));
    document.refreshHierarchyMetadata();

    const ProjectDocument beforeDocument = document;
    auto* resizedPanel = document.findWidgetById("panel_main");
    REQUIRE(resizedPanel != nullptr);
    resizedPanel->bounds.width = 220.0f;
    resizedPanel->bounds.height = 140.0f;

    document.applyLayoutFromPrevious(beforeDocument);

    const auto* resizedSizer = document.findWidgetById("sizer_main");
    REQUIRE(resizedSizer != nullptr);
    REQUIRE(resizedSizer->bounds.width == Catch::Approx(220.0f));
    REQUIRE(resizedSizer->bounds.height == Catch::Approx(140.0f));

    const auto* resizedButton = document.findWidgetById("button_1");
    REQUIRE(resizedButton != nullptr);
    REQUIRE(resizedButton->bounds.width == Catch::Approx(220.0f));
    REQUIRE(resizedButton->bounds.height == Catch::Approx(140.0f));
}

TEST_CASE("Safe widget placement clamps complete bounds inside a parent client area")
{
    const visiform::model::Rect placed = visiform::model::safeWidgetPlacement(
        { 10.0f, 20.0f, 200.0f, 120.0f },
        80.0f,
        40.0f,
        190.0f,
        130.0f,
        12.0f);

    REQUIRE(placed.x == Catch::Approx(118.0f));
    REQUIRE(placed.y == Catch::Approx(88.0f));
    REQUIRE(placed.width == Catch::Approx(80.0f));
    REQUIRE(placed.height == Catch::Approx(40.0f));
}

TEST_CASE("Safe widget placement preserves valid size when the parent is too small")
{
    const visiform::model::Rect placed = visiform::model::safeWidgetPlacement(
        { 0.0f, 0.0f, 60.0f, 40.0f },
        100.0f,
        50.0f,
        -30.0f,
        -20.0f,
        8.0f);

    REQUIRE(placed.x == Catch::Approx(8.0f));
    REQUIRE(placed.y == Catch::Approx(8.0f));
    REQUIRE(placed.width == Catch::Approx(100.0f));
    REQUIRE(placed.height == Catch::Approx(50.0f));
}

TEST_CASE("Parent fit changes only the requested dimension")
{
    const visiform::model::Rect parentBounds{ 12.0f, 28.0f, 240.0f, 160.0f };
    const visiform::model::Rect original{ 300.0f, 220.0f, 80.0f, 40.0f };

    const auto fitWidth = visiform::model::fitWidgetWidthToParent(parentBounds, original);
    REQUIRE(fitWidth.x == Catch::Approx(12.0f));
    REQUIRE(fitWidth.y == Catch::Approx(220.0f));
    REQUIRE(fitWidth.width == Catch::Approx(240.0f));
    REQUIRE(fitWidth.height == Catch::Approx(40.0f));

    const auto fitHeight = visiform::model::fitWidgetHeightToParent(parentBounds, original);
    REQUIRE(fitHeight.x == Catch::Approx(300.0f));
    REQUIRE(fitHeight.y == Catch::Approx(28.0f));
    REQUIRE(fitHeight.width == Catch::Approx(80.0f));
    REQUIRE(fitHeight.height == Catch::Approx(160.0f));
}

TEST_CASE("Parent alignment changes only the requested position axis")
{
    const visiform::model::Rect parentBounds{ 12.0f, 28.0f, 240.0f, 160.0f };
    const visiform::model::Rect original{ 300.0f, 220.0f, 80.0f, 40.0f };

    const auto left = visiform::model::alignWidgetHorizontallyToParent(
        parentBounds, original, visiform::model::ParentAlignment::Start);
    REQUIRE(left.x == Catch::Approx(12.0f));
    REQUIRE(left.y == Catch::Approx(220.0f));
    REQUIRE(left.width == Catch::Approx(80.0f));
    REQUIRE(left.height == Catch::Approx(40.0f));

    const auto right = visiform::model::alignWidgetHorizontallyToParent(
        parentBounds, original, visiform::model::ParentAlignment::End);
    REQUIRE(right.x == Catch::Approx(172.0f));
    REQUIRE(right.y == Catch::Approx(220.0f));

    const auto horizontalCenter = visiform::model::alignWidgetHorizontallyToParent(
        parentBounds, original, visiform::model::ParentAlignment::Center);
    REQUIRE(horizontalCenter.x == Catch::Approx(92.0f));
    REQUIRE(horizontalCenter.y == Catch::Approx(220.0f));

    const auto top = visiform::model::alignWidgetVerticallyToParent(
        parentBounds, original, visiform::model::ParentAlignment::Start);
    REQUIRE(top.x == Catch::Approx(300.0f));
    REQUIRE(top.y == Catch::Approx(28.0f));
    REQUIRE(top.width == Catch::Approx(80.0f));
    REQUIRE(top.height == Catch::Approx(40.0f));

    const auto bottom = visiform::model::alignWidgetVerticallyToParent(
        parentBounds, original, visiform::model::ParentAlignment::End);
    REQUIRE(bottom.x == Catch::Approx(300.0f));
    REQUIRE(bottom.y == Catch::Approx(148.0f));

    const auto verticalCenter = visiform::model::alignWidgetVerticallyToParent(
        parentBounds, original, visiform::model::ParentAlignment::Center);
    REQUIRE(verticalCenter.x == Catch::Approx(300.0f));
    REQUIRE(verticalCenter.y == Catch::Approx(88.0f));
}

TEST_CASE("Oversized parent alignment uses the starting content edge")
{
    const visiform::model::Rect parentBounds{ 12.0f, 28.0f, 60.0f, 30.0f };
    const visiform::model::Rect original{ 300.0f, 220.0f, 80.0f, 40.0f };

    const auto horizontal = visiform::model::alignWidgetHorizontallyToParent(
        parentBounds, original, visiform::model::ParentAlignment::End);
    REQUIRE(horizontal.x == Catch::Approx(12.0f));
    REQUIRE(horizontal.y == Catch::Approx(220.0f));
    REQUIRE(horizontal.width == Catch::Approx(80.0f));

    const auto vertical = visiform::model::alignWidgetVerticallyToParent(
        parentBounds, original, visiform::model::ParentAlignment::Center);
    REQUIRE(vertical.x == Catch::Approx(300.0f));
    REQUIRE(vertical.y == Catch::Approx(28.0f));
    REQUIRE(vertical.height == Catch::Approx(40.0f));
}

TEST_CASE("Complete widget bounds clamp to parent content edges")
{
    const auto clamped = visiform::model::clampWidgetBoundsToParent(
        { 10.0f, 20.0f, 200.0f, 100.0f },
        { 180.0f, 105.0f, 60.0f, 30.0f });

    REQUIRE(clamped.x == Catch::Approx(150.0f));
    REQUIRE(clamped.y == Catch::Approx(90.0f));
    REQUIRE(clamped.width == Catch::Approx(60.0f));
    REQUIRE(clamped.height == Catch::Approx(30.0f));
}

TEST_CASE("Adding a free-position widget enforces direct parent content bounds")
{
    ProjectDocument document = ProjectDocument::createDefault();
    auto panel = WidgetRegistry::instance().createDefaultWidget(WidgetType::Panel, "panel_1");
    panel.bounds = { 100.0f, 100.0f, 200.0f, 150.0f };
    REQUIRE(document.addChildToParent(document.root.id, panel));

    auto button = WidgetRegistry::instance().createDefaultWidget(WidgetType::Button, "button_nested");
    button.bounds = { 190.0f, 140.0f, 80.0f, 40.0f };
    REQUIRE(document.addChildToParent("panel_1", button));

    const auto* added = document.findWidgetById("button_nested");
    REQUIRE(added != nullptr);
    REQUIRE(added->bounds.x == Catch::Approx(120.0f));
    REQUIRE(added->bounds.y == Catch::Approx(110.0f));
    REQUIRE(added->bounds.width == Catch::Approx(80.0f));
    REQUIRE(added->bounds.height == Catch::Approx(40.0f));
}

TEST_CASE("Default project button begins inside the form in model space")
{
    const ProjectDocument document = ProjectDocument::createDefault();
    REQUIRE(document.root.children.size() == 1);

    const auto& button = document.root.children.front();
    const auto clientBounds = visiform::model::LayoutEngine::clientBoundsForParent(document.root);
    const auto* definition = WidgetRegistry::instance().find(WidgetType::Button);
    REQUIRE(definition != nullptr);
    REQUIRE(clientBounds.x == Catch::Approx(0.0f));
    REQUIRE(clientBounds.y == Catch::Approx(28.0f));
    REQUIRE(clientBounds.width == Catch::Approx(900.0f));
    REQUIRE(clientBounds.height == Catch::Approx(572.0f));
    REQUIRE(button.bounds.x == Catch::Approx(40.0f));
    REQUIRE(button.bounds.y == Catch::Approx(48.0f));
    REQUIRE(button.bounds.width == Catch::Approx(definition->size.defaultWidth));
    REQUIRE(button.bounds.height == Catch::Approx(definition->size.defaultHeight));
    REQUIRE(button.bounds.x >= clientBounds.x);
    REQUIRE(button.bounds.y >= clientBounds.y);
    REQUIRE(button.bounds.x + button.bounds.width <= clientBounds.x + clientBounds.width);
    REQUIRE(button.bounds.y + button.bounds.height <= clientBounds.y + clientBounds.height);
}
