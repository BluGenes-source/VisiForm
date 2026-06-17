#include "ui/Splitter.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using visiform::ui::Splitter;

TEST_CASE("Vertical splitter computes pane bounds from split position")
{
    Splitter splitter;
    splitter.setOrientation(Splitter::Orientation::Vertical);
    splitter.setBounds(10.0f, 20.0f, 500.0f, 300.0f);
    splitter.setDividerThickness(6.0f);
    splitter.setMinimumFirstPaneSize(120.0f);
    splitter.setMinimumSecondPaneSize(180.0f);
    splitter.setSplitPosition(250.0f);

    const Splitter::Bounds firstPane = splitter.firstPaneBounds();
    const Splitter::Bounds divider = splitter.dividerBounds();
    const Splitter::Bounds secondPane = splitter.secondPaneBounds();

    REQUIRE(firstPane.x == Catch::Approx(10.0f));
    REQUIRE(firstPane.y == Catch::Approx(20.0f));
    REQUIRE(firstPane.width == Catch::Approx(250.0f));
    REQUIRE(firstPane.height == Catch::Approx(300.0f));

    REQUIRE(divider.x == Catch::Approx(260.0f));
    REQUIRE(divider.width == Catch::Approx(6.0f));

    REQUIRE(secondPane.x == Catch::Approx(266.0f));
    REQUIRE(secondPane.width == Catch::Approx(244.0f));
    REQUIRE(secondPane.height == Catch::Approx(300.0f));
}

TEST_CASE("Vertical splitter clamps split position to second pane minimum")
{
    Splitter splitter;
    splitter.setOrientation(Splitter::Orientation::Vertical);
    splitter.setBounds(0.0f, 0.0f, 500.0f, 240.0f);
    splitter.setDividerThickness(6.0f);
    splitter.setMinimumFirstPaneSize(120.0f);
    splitter.setMinimumSecondPaneSize(180.0f);
    splitter.setSplitPosition(400.0f);

    REQUIRE(splitter.splitPosition() == Catch::Approx(314.0f));
    REQUIRE(splitter.secondPaneSize() == Catch::Approx(180.0f));
}

TEST_CASE("Vertical splitter drag updates position without jumping")
{
    Splitter splitter;
    splitter.setOrientation(Splitter::Orientation::Vertical);
    splitter.setBounds(0.0f, 0.0f, 500.0f, 240.0f);
    splitter.setDividerThickness(6.0f);
    splitter.setHitThickness(18.0f);
    splitter.setMinimumFirstPaneSize(120.0f);
    splitter.setMinimumSecondPaneSize(180.0f);
    splitter.setSplitPosition(220.0f);

    REQUIRE(splitter.mouseDown(223.0f, 40.0f));
    REQUIRE(splitter.isDragging());
    REQUIRE(splitter.mouseDrag(263.0f, 40.0f));
    REQUIRE(splitter.splitPosition() == Catch::Approx(260.0f));
    REQUIRE(splitter.mouseUp());
    REQUIRE(!splitter.isDragging());
}

TEST_CASE("Horizontal splitter computes pane bounds")
{
    Splitter splitter;
    splitter.setOrientation(Splitter::Orientation::Horizontal);
    splitter.setBounds(4.0f, 8.0f, 300.0f, 400.0f);
    splitter.setDividerThickness(8.0f);
    splitter.setMinimumFirstPaneSize(100.0f);
    splitter.setMinimumSecondPaneSize(120.0f);
    splitter.setSplitPosition(140.0f);

    const Splitter::Bounds firstPane = splitter.firstPaneBounds();
    const Splitter::Bounds secondPane = splitter.secondPaneBounds();

    REQUIRE(firstPane.height == Catch::Approx(140.0f));
    REQUIRE(secondPane.y == Catch::Approx(156.0f));
    REQUIRE(secondPane.height == Catch::Approx(252.0f));
}
