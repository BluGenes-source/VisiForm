#pragma once

#include "model/WidgetNode.h"

#include <cstdint>
#include <string_view>

namespace visiform::model {

enum class SizerOrientation {
    Vertical,
    Horizontal
};

enum class SizerAlignment {
    Start,
    Center,
    End
};

enum class SizerBorderSide : std::uint8_t {
    None = 0,
    Left = 1u << 0,
    Top = 1u << 1,
    Right = 1u << 2,
    Bottom = 1u << 3,
    All = 15
};

enum class SpacerKind {
    Fixed,
    Stretch
};

struct Size {
    float width = 0.0f;
    float height = 0.0f;
};

struct SizerItemLayout {
    int proportion = 0;
    bool expand = false;
    SizerAlignment alignment = SizerAlignment::Start;
    int border = 0;
    SizerBorderSide borderSides = SizerBorderSide::None;
    int preferredWidth = -1;
    int preferredHeight = -1;
    int minimumWidth = -1;
    int minimumHeight = -1;
    bool shown = true;
};

struct BoxSizerLayout {
    SizerOrientation orientation = SizerOrientation::Vertical;
    int paddingLeft = 0;
    int paddingTop = 0;
    int paddingRight = 0;
    int paddingBottom = 0;
    int gap = 0;
};

[[nodiscard]] constexpr SizerBorderSide operator|(SizerBorderSide first, SizerBorderSide second)
{
    return static_cast<SizerBorderSide>(static_cast<std::uint8_t>(first) | static_cast<std::uint8_t>(second));
}

[[nodiscard]] constexpr SizerBorderSide operator&(SizerBorderSide first, SizerBorderSide second)
{
    return static_cast<SizerBorderSide>(static_cast<std::uint8_t>(first) & static_cast<std::uint8_t>(second));
}

[[nodiscard]] constexpr bool hasBorderSide(SizerBorderSide sides, SizerBorderSide side)
{
    return (static_cast<std::uint8_t>(sides) & static_cast<std::uint8_t>(side)) != 0;
}

namespace sizer_properties {
inline constexpr std::string_view kOrientation = "orientation";
inline constexpr std::string_view kLegacyPadding = "padding";
inline constexpr std::string_view kPaddingLeft = "paddingLeft";
inline constexpr std::string_view kPaddingTop = "paddingTop";
inline constexpr std::string_view kPaddingRight = "paddingRight";
inline constexpr std::string_view kPaddingBottom = "paddingBottom";
inline constexpr std::string_view kGap = "gap";
inline constexpr std::string_view kItemProportion = "sizerItem.proportion";
inline constexpr std::string_view kItemExpand = "sizerItem.expand";
inline constexpr std::string_view kItemAlignment = "sizerItem.alignment";
inline constexpr std::string_view kItemBorder = "sizerItem.border";
inline constexpr std::string_view kItemBorderSides = "sizerItem.borderSides";
inline constexpr std::string_view kItemPreferredWidth = "sizerItem.preferredWidth";
inline constexpr std::string_view kItemPreferredHeight = "sizerItem.preferredHeight";
inline constexpr std::string_view kItemMinimumWidth = "sizerItem.minimumWidth";
inline constexpr std::string_view kItemMinimumHeight = "sizerItem.minimumHeight";
inline constexpr std::string_view kItemShown = "sizerItem.shown";
inline constexpr std::string_view kSpacerKind = "spacer.kind";
inline constexpr std::string_view kSpacerSize = "spacer.size";
}

[[nodiscard]] const char* toString(SizerOrientation orientation);
[[nodiscard]] const char* toString(SizerAlignment alignment);
[[nodiscard]] const char* toString(SizerBorderSide sides);
[[nodiscard]] const char* toString(SpacerKind kind);

[[nodiscard]] SizerOrientation parseSizerOrientation(const WidgetNode& widget);
[[nodiscard]] SizerAlignment parseSizerAlignment(const WidgetNode& widget, std::string_view key, SizerAlignment fallback);
[[nodiscard]] SizerBorderSide parseSizerBorderSides(const WidgetNode& widget, std::string_view key, SizerBorderSide fallback);
[[nodiscard]] SpacerKind parseSpacerKind(const WidgetNode& widget);

[[nodiscard]] BoxSizerLayout boxSizerLayoutFor(const WidgetNode& sizer);
[[nodiscard]] SizerItemLayout sizerItemLayoutFor(const WidgetNode& child);
[[nodiscard]] SizerItemLayout defaultSizerItemLayoutFor(WidgetType type);

void applyDefaultSizerItemLayout(WidgetNode& child);
void normalizeBoxSizerProperties(WidgetNode& widget);

[[nodiscard]] Size calculateBoxSizerMinimumSize(const WidgetNode& sizer);
void layoutBoxSizerChildren(WidgetNode& sizer);

} // namespace visiform::model
