#include "model/WidgetRegistry.h"

#include <algorithm>

#include <catch2/catch_test_macros.hpp>

using visiform::model::WidgetRegistry;
using visiform::model::WidgetType;

TEST_CASE("Widget palette includes Slider with registry-backed metadata")
{
    const auto paletteDefinitions = WidgetRegistry::instance().paletteDefinitions();

    const auto sliderIt = std::find_if(paletteDefinitions.begin(), paletteDefinitions.end(), [](const auto* definition) {
        return definition != nullptr && definition->type == WidgetType::Slider;
    });
    REQUIRE(sliderIt != paletteDefinitions.end());
    REQUIRE((*sliderIt)->displayName == "Slider");
    REQUIRE((*sliderIt)->paletteVisible);
    REQUIRE((*sliderIt)->paletteGroup == "Value/Feedback");

    const auto scrollBarIt = std::find_if(paletteDefinitions.begin(), paletteDefinitions.end(), [](const auto* definition) {
        return definition != nullptr && definition->type == WidgetType::ScrollBar;
    });
    REQUIRE(scrollBarIt != paletteDefinitions.end());
    REQUIRE(std::distance(paletteDefinitions.begin(), sliderIt) < std::distance(paletteDefinitions.begin(), scrollBarIt));
}
