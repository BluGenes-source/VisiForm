#include "model/WidgetRegistry.h"

#include <algorithm>
#include <set>

#include <catch2/catch_test_macros.hpp>

using visiform::model::WidgetRegistry;
using visiform::model::WidgetType;
using visiform::model::PropertyEditKind;

TEST_CASE("Widget palette includes Slider with registry-backed metadata")
{
    const auto paletteDefinitions = WidgetRegistry::instance().paletteDefinitions();

    const auto sliderIt = std::find_if(paletteDefinitions.begin(), paletteDefinitions.end(), [](const auto* definition) {
        return definition != nullptr && definition->type == WidgetType::Slider;
    });
    REQUIRE(sliderIt != paletteDefinitions.end());
    REQUIRE((*sliderIt)->displayName == "Slider");
    REQUIRE((*sliderIt)->paletteVisible);
    REQUIRE((*sliderIt)->paletteGroup == "Forms");

    const auto scrollBarIt = std::find_if(paletteDefinitions.begin(), paletteDefinitions.end(), [](const auto* definition) {
        return definition != nullptr && definition->type == WidgetType::ScrollBar;
    });
    REQUIRE(scrollBarIt != paletteDefinitions.end());
    REQUIRE(std::distance(paletteDefinitions.begin(), sliderIt) < std::distance(paletteDefinitions.begin(), scrollBarIt));

    const auto sizerIt = std::find_if(paletteDefinitions.begin(), paletteDefinitions.end(), [](const auto* definition) {
        return definition != nullptr && definition->type == WidgetType::Sizer;
    });
    REQUIRE(sizerIt != paletteDefinitions.end());
    REQUIRE((*sizerIt)->paletteGroup == "Layout");

    const std::set<std::string> expectedGroups{
        "Common",
        "Containers",
        "Layout",
        "Forms",
        "Data",
        "Menu/Toolbar",
        "Additional"
    };
    std::set<WidgetType> uniqueTypes;
    for (const auto* definition : paletteDefinitions) {
        REQUIRE(definition != nullptr);
        REQUIRE(expectedGroups.contains(definition->paletteGroup));
        REQUIRE(uniqueTypes.insert(definition->type).second);
    }
}

TEST_CASE("Appearance geometry metadata preserves zero-valued slider semantics")
{
    const auto* button = WidgetRegistry::instance().find(WidgetType::Button);
    REQUIRE(button != nullptr);

    const auto findProperty = [button](const std::string& key) {
        return std::find_if(
            button->properties.begin(),
            button->properties.end(),
            [&key](const auto& property) { return property.key == key; });
    };

    const auto borderThickness = findProperty("borderThickness");
    REQUIRE(borderThickness != button->properties.end());
    CHECK(borderThickness->editKind == PropertyEditKind::Slider);
    CHECK(borderThickness->minimumValue == 0.0f);
    CHECK(borderThickness->maximumValue == 25.0f);
    CHECK(borderThickness->stepValue == 1.0f);

    const auto cornerRadius = findProperty("cornerRadius");
    REQUIRE(cornerRadius != button->properties.end());
    CHECK(cornerRadius->editKind == PropertyEditKind::Slider);
    CHECK(cornerRadius->minimumValue == 0.0f);
    CHECK(cornerRadius->maximumValue == 25.0f);
    CHECK(cornerRadius->stepValue == 1.0f);
}
