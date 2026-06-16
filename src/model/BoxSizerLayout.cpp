#include "model/BoxSizerLayout.h"

#include "model/WidgetRegistry.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace visiform::model {
namespace {

constexpr int kDefaultSizerPadding = 8;
constexpr int kDefaultSizerGap = 8;
constexpr float kMinimumWidgetSize = 0.0f;

std::string propertyKey(std::string_view key)
{
    return std::string{ key };
}

std::string trim(std::string value)
{
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char character) {
        return std::isspace(character) != 0;
    });
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char character) {
        return std::isspace(character) != 0;
    }).base();
    if (first >= last) {
        return {};
    }
    return { first, last };
}

int intProperty(const WidgetNode& widget, std::string_view key, int fallback)
{
    const auto* property = widget.getProperty(propertyKey(key));
    if (property == nullptr) {
        return fallback;
    }
    if (property->isInt()) {
        return property->asInt(fallback);
    }
    if (property->isFloat()) {
        return static_cast<int>(std::lround(property->asFloat(static_cast<float>(fallback))));
    }
    if (property->isString()) {
        std::istringstream stream(trim(property->asString()));
        int value = fallback;
        char trailing = '\0';
        if ((stream >> value) && !(stream >> trailing)) {
            return value;
        }
    }
    return fallback;
}

bool boolProperty(const WidgetNode& widget, std::string_view key, bool fallback)
{
    const auto* property = widget.getProperty(propertyKey(key));
    if (property == nullptr) {
        return fallback;
    }
    if (property->isBool()) {
        return property->asBool(fallback);
    }
    if (property->isString()) {
        const std::string value = trim(property->asString());
        if (value == "true" || value == "True" || value == "1") {
            return true;
        }
        if (value == "false" || value == "False" || value == "0") {
            return false;
        }
    }
    return fallback;
}

std::string stringProperty(const WidgetNode& widget, std::string_view key, std::string fallback = {})
{
    return trim(widget.getStringProperty(propertyKey(key), fallback));
}

int nonNegativeProperty(const WidgetNode& widget, std::string_view key, int fallback)
{
    return std::max(0, intProperty(widget, key, fallback));
}

float positiveOrZero(float value)
{
    return std::max(0.0f, value);
}

float mainSize(const Size& size, SizerOrientation orientation)
{
    return orientation == SizerOrientation::Horizontal ? size.width : size.height;
}

float crossSize(const Size& size, SizerOrientation orientation)
{
    return orientation == SizerOrientation::Horizontal ? size.height : size.width;
}

float mainPosition(const Rect& rect, SizerOrientation orientation)
{
    return orientation == SizerOrientation::Horizontal ? rect.x : rect.y;
}

float crossPosition(const Rect& rect, SizerOrientation orientation)
{
    return orientation == SizerOrientation::Horizontal ? rect.y : rect.x;
}

float rectMainSize(const Rect& rect, SizerOrientation orientation)
{
    return orientation == SizerOrientation::Horizontal ? rect.width : rect.height;
}

float rectCrossSize(const Rect& rect, SizerOrientation orientation)
{
    return orientation == SizerOrientation::Horizontal ? rect.height : rect.width;
}

float borderLeft(const SizerItemLayout& layout)
{
    return hasBorderSide(layout.borderSides, SizerBorderSide::Left) ? static_cast<float>(layout.border) : 0.0f;
}

float borderTop(const SizerItemLayout& layout)
{
    return hasBorderSide(layout.borderSides, SizerBorderSide::Top) ? static_cast<float>(layout.border) : 0.0f;
}

float borderRight(const SizerItemLayout& layout)
{
    return hasBorderSide(layout.borderSides, SizerBorderSide::Right) ? static_cast<float>(layout.border) : 0.0f;
}

float borderBottom(const SizerItemLayout& layout)
{
    return hasBorderSide(layout.borderSides, SizerBorderSide::Bottom) ? static_cast<float>(layout.border) : 0.0f;
}

float mainBorder(const SizerItemLayout& layout, SizerOrientation orientation)
{
    return orientation == SizerOrientation::Horizontal
        ? borderLeft(layout) + borderRight(layout)
        : borderTop(layout) + borderBottom(layout);
}

float crossBorder(const SizerItemLayout& layout, SizerOrientation orientation)
{
    return orientation == SizerOrientation::Horizontal
        ? borderTop(layout) + borderBottom(layout)
        : borderLeft(layout) + borderRight(layout);
}

Size naturalMinimumSize(const WidgetNode& widget)
{
    if (widget.type == WidgetType::Sizer) {
        return calculateBoxSizerMinimumSize(widget);
    }
    if (widget.type == WidgetType::Spacer) {
        const SpacerKind kind = parseSpacerKind(widget);
        const int fixedSize = nonNegativeProperty(widget, sizer_properties::kSpacerSize, 24);
        return kind == SpacerKind::Fixed
            ? Size{ static_cast<float>(fixedSize), static_cast<float>(fixedSize) }
            : Size{ 0.0f, 0.0f };
    }

    Size size{ 0.0f, 0.0f };
    if (const auto* definition = WidgetRegistry::instance().find(widget.type)) {
        size.width = std::max(definition->size.minWidth, kMinimumWidgetSize);
        size.height = std::max(definition->size.minHeight, kMinimumWidgetSize);
    }
    else {
        size.width = kMinimumWidgetSize;
        size.height = kMinimumWidgetSize;
    }
    return size;
}

Size effectiveMinimumSize(const WidgetNode& widget, const SizerItemLayout& layout)
{
    Size size = naturalMinimumSize(widget);
    const float preferredWidth = layout.preferredWidth >= 0
        ? static_cast<float>(layout.preferredWidth)
        : positiveOrZero(widget.bounds.width);
    const float preferredHeight = layout.preferredHeight >= 0
        ? static_cast<float>(layout.preferredHeight)
        : positiveOrZero(widget.bounds.height);
    size.width = std::max(size.width, preferredWidth);
    size.height = std::max(size.height, preferredHeight);
    if (layout.minimumWidth >= 0) {
        size.width = std::max(size.width, static_cast<float>(layout.minimumWidth));
    }
    if (layout.minimumHeight >= 0) {
        size.height = std::max(size.height, static_cast<float>(layout.minimumHeight));
    }
    return size;
}

struct LayoutItem {
    WidgetNode* widget = nullptr;
    SizerItemLayout layout{};
    Size minimum{};
    float borderedMainMinimum = 0.0f;
    float borderedCrossMinimum = 0.0f;
    float assignedMain = 0.0f;
};

std::vector<LayoutItem> participatingItems(WidgetNode& sizer, SizerOrientation orientation)
{
    std::vector<LayoutItem> items;
    items.reserve(sizer.children.size());
    for (auto& child : sizer.children) {
        SizerItemLayout layout = sizerItemLayoutFor(child);
        if (!layout.shown) {
            child.bounds = {};
            continue;
        }

        Size minimum = effectiveMinimumSize(child, layout);
        items.push_back(LayoutItem{
            &child,
            layout,
            minimum,
            mainSize(minimum, orientation) + mainBorder(layout, orientation),
            crossSize(minimum, orientation) + crossBorder(layout, orientation),
            0.0f });
    }
    return items;
}

void assignWeightedExtra(std::vector<LayoutItem>& items, float extraSpace)
{
    int totalProportion = 0;
    for (const auto& item : items) {
        totalProportion += std::max(0, item.layout.proportion);
    }
    if (totalProportion <= 0 || extraSpace <= 0.0f) {
        return;
    }

    const int integerExtra = std::max(0, static_cast<int>(std::floor(extraSpace)));
    int assigned = 0;
    for (auto& item : items) {
        if (item.layout.proportion <= 0) {
            continue;
        }

        const int share = integerExtra * item.layout.proportion / totalProportion;
        item.assignedMain += static_cast<float>(share);
        assigned += share;
    }

    int remainder = integerExtra - assigned;
    for (auto& item : items) {
        if (remainder <= 0) {
            break;
        }
        if (item.layout.proportion <= 0) {
            continue;
        }
        item.assignedMain += 1.0f;
        --remainder;
    }

    const float fractional = extraSpace - static_cast<float>(integerExtra);
    if (fractional > 0.0f) {
        for (auto& item : items) {
            if (item.layout.proportion > 0) {
                item.assignedMain += fractional;
                break;
            }
        }
    }
}

Rect childRectForSlot(const Rect& content,
    SizerOrientation orientation,
    float cursor,
    const LayoutItem& item,
    float availableCross)
{
    const float leftBorder = borderLeft(item.layout);
    const float topBorder = borderTop(item.layout);
    const float rightBorder = borderRight(item.layout);
    const float bottomBorder = borderBottom(item.layout);

    const float mainStart = cursor + (orientation == SizerOrientation::Horizontal ? leftBorder : topBorder);
    const float mainLength = std::max(0.0f, item.assignedMain - mainBorder(item.layout, orientation));
    const float crossStart = crossPosition(content, orientation)
        + (orientation == SizerOrientation::Horizontal ? topBorder : leftBorder);
    const float crossLength = std::max(0.0f, availableCross - crossBorder(item.layout, orientation));
    const float preferredCross = std::min(crossSize(item.minimum, orientation), crossLength);

    float finalCrossStart = crossStart;
    float finalCrossLength = crossLength;
    if (!item.layout.expand) {
        finalCrossLength = preferredCross;
        const float remainingCross = std::max(0.0f, crossLength - preferredCross);
        switch (item.layout.alignment) {
        case SizerAlignment::Start:
            break;
        case SizerAlignment::Center:
            finalCrossStart += std::floor(remainingCross * 0.5f);
            break;
        case SizerAlignment::End:
            finalCrossStart += remainingCross;
            break;
        }
    }

    if (orientation == SizerOrientation::Horizontal) {
        return {
            mainStart,
            finalCrossStart,
            mainLength,
            finalCrossLength
        };
    }

    return {
        finalCrossStart,
        mainStart,
        finalCrossLength,
        mainLength
    };
}

} // namespace

const char* toString(SizerOrientation orientation)
{
    return orientation == SizerOrientation::Horizontal ? "Horizontal" : "Vertical";
}

const char* toString(SizerAlignment alignment)
{
    switch (alignment) {
    case SizerAlignment::Start:
        return "Start";
    case SizerAlignment::Center:
        return "Center";
    case SizerAlignment::End:
        return "End";
    }
    return "Start";
}

const char* toString(SizerBorderSide sides)
{
    switch (sides) {
    case SizerBorderSide::None:
        return "None";
    case SizerBorderSide::Left:
        return "Left";
    case SizerBorderSide::Top:
        return "Top";
    case SizerBorderSide::Right:
        return "Right";
    case SizerBorderSide::Bottom:
        return "Bottom";
    case SizerBorderSide::All:
        return "All";
    }
    return "None";
}

const char* toString(SpacerKind kind)
{
    return kind == SpacerKind::Stretch ? "Stretch" : "Fixed";
}

SizerOrientation parseSizerOrientation(const WidgetNode& widget)
{
    return stringProperty(widget, sizer_properties::kOrientation, "Vertical") == "Horizontal"
        ? SizerOrientation::Horizontal
        : SizerOrientation::Vertical;
}

SizerAlignment parseSizerAlignment(const WidgetNode& widget, std::string_view key, SizerAlignment fallback)
{
    const std::string value = stringProperty(widget, key);
    if (value == "Center") {
        return SizerAlignment::Center;
    }
    if (value == "End") {
        return SizerAlignment::End;
    }
    if (value == "Start") {
        return SizerAlignment::Start;
    }
    return fallback;
}

SizerBorderSide parseSizerBorderSides(const WidgetNode& widget, std::string_view key, SizerBorderSide fallback)
{
    const std::string value = stringProperty(widget, key);
    if (value.empty()) {
        return fallback;
    }
    if (value == "None") {
        return SizerBorderSide::None;
    }
    if (value == "All") {
        return SizerBorderSide::All;
    }

    SizerBorderSide sides = SizerBorderSide::None;
    std::istringstream stream(value);
    std::string token;
    while (std::getline(stream, token, '|')) {
        token = trim(token);
        if (token == "Left") {
            sides = sides | SizerBorderSide::Left;
        }
        else if (token == "Top") {
            sides = sides | SizerBorderSide::Top;
        }
        else if (token == "Right") {
            sides = sides | SizerBorderSide::Right;
        }
        else if (token == "Bottom") {
            sides = sides | SizerBorderSide::Bottom;
        }
    }
    return sides;
}

SpacerKind parseSpacerKind(const WidgetNode& widget)
{
    return stringProperty(widget, sizer_properties::kSpacerKind, "Fixed") == "Stretch"
        ? SpacerKind::Stretch
        : SpacerKind::Fixed;
}

BoxSizerLayout boxSizerLayoutFor(const WidgetNode& sizer)
{
    const int legacyPadding = nonNegativeProperty(sizer, sizer_properties::kLegacyPadding, kDefaultSizerPadding);
    return {
        parseSizerOrientation(sizer),
        nonNegativeProperty(sizer, sizer_properties::kPaddingLeft, legacyPadding),
        nonNegativeProperty(sizer, sizer_properties::kPaddingTop, legacyPadding),
        nonNegativeProperty(sizer, sizer_properties::kPaddingRight, legacyPadding),
        nonNegativeProperty(sizer, sizer_properties::kPaddingBottom, legacyPadding),
        nonNegativeProperty(sizer, sizer_properties::kGap, kDefaultSizerGap)
    };
}

SizerItemLayout defaultSizerItemLayoutFor(WidgetType type)
{
    SizerItemLayout layout;
    switch (type) {
    case WidgetType::TextBox:
    case WidgetType::ComboBox:
        layout.expand = true;
        break;
    case WidgetType::ListBox:
    case WidgetType::TreeView:
    case WidgetType::TableGrid:
    case WidgetType::Panel:
    case WidgetType::Sizer:
        layout.proportion = 1;
        layout.expand = true;
        break;
    case WidgetType::Spacer:
        layout.proportion = 0;
        layout.expand = false;
        break;
    default:
        break;
    }
    return layout;
}

SizerItemLayout sizerItemLayoutFor(const WidgetNode& child)
{
    SizerItemLayout layout = defaultSizerItemLayoutFor(child.type);
    if (child.type == WidgetType::Spacer && parseSpacerKind(child) == SpacerKind::Stretch) {
        layout.proportion = 1;
    }
    layout.proportion = std::max(0, intProperty(child, sizer_properties::kItemProportion, layout.proportion));
    layout.expand = boolProperty(child, sizer_properties::kItemExpand, layout.expand);
    layout.alignment = parseSizerAlignment(child, sizer_properties::kItemAlignment, layout.alignment);
    layout.border = std::max(0, intProperty(child, sizer_properties::kItemBorder, layout.border));
    layout.borderSides = parseSizerBorderSides(child, sizer_properties::kItemBorderSides, layout.borderSides);
    layout.preferredWidth = intProperty(child, sizer_properties::kItemPreferredWidth, layout.preferredWidth);
    layout.preferredHeight = intProperty(child, sizer_properties::kItemPreferredHeight, layout.preferredHeight);
    layout.minimumWidth = intProperty(child, sizer_properties::kItemMinimumWidth, layout.minimumWidth);
    layout.minimumHeight = intProperty(child, sizer_properties::kItemMinimumHeight, layout.minimumHeight);
    layout.shown = boolProperty(child, sizer_properties::kItemShown, layout.shown);
    return layout;
}

void applyDefaultSizerItemLayout(WidgetNode& child)
{
    const SizerItemLayout layout = defaultSizerItemLayoutFor(child.type);
    const auto setMissing = [&child](std::string_view key, PropertyValue value) {
        const std::string storedKey = propertyKey(key);
        if (child.getProperty(storedKey) == nullptr) {
            child.setProperty(storedKey, std::move(value));
        }
    };

    setMissing(sizer_properties::kItemProportion, layout.proportion);
    setMissing(sizer_properties::kItemExpand, layout.expand);
    setMissing(sizer_properties::kItemAlignment, toString(layout.alignment));
    setMissing(sizer_properties::kItemBorder, layout.border);
    setMissing(sizer_properties::kItemBorderSides, toString(layout.borderSides));
    setMissing(sizer_properties::kItemPreferredWidth, layout.preferredWidth);
    setMissing(sizer_properties::kItemPreferredHeight, layout.preferredHeight);
    setMissing(sizer_properties::kItemMinimumWidth, layout.minimumWidth);
    setMissing(sizer_properties::kItemMinimumHeight, layout.minimumHeight);
    setMissing(sizer_properties::kItemShown, layout.shown);
}

void normalizeBoxSizerProperties(WidgetNode& widget)
{
    if (widget.type == WidgetType::Sizer) {
        const BoxSizerLayout layout = boxSizerLayoutFor(widget);
        widget.setProperty(propertyKey(sizer_properties::kOrientation), toString(layout.orientation));
        widget.setProperty(propertyKey(sizer_properties::kPaddingLeft), layout.paddingLeft);
        widget.setProperty(propertyKey(sizer_properties::kPaddingTop), layout.paddingTop);
        widget.setProperty(propertyKey(sizer_properties::kPaddingRight), layout.paddingRight);
        widget.setProperty(propertyKey(sizer_properties::kPaddingBottom), layout.paddingBottom);
        widget.setProperty(propertyKey(sizer_properties::kGap), layout.gap);
    }
    else if (widget.type == WidgetType::Spacer) {
        const SpacerKind kind = parseSpacerKind(widget);
        widget.setProperty(propertyKey(sizer_properties::kSpacerKind), toString(kind));
        widget.setProperty(propertyKey(sizer_properties::kSpacerSize),
            nonNegativeProperty(widget, sizer_properties::kSpacerSize, kind == SpacerKind::Fixed ? 24 : 0));
    }

    for (auto& child : widget.children) {
        normalizeBoxSizerProperties(child);
    }
}

Size calculateBoxSizerMinimumSize(const WidgetNode& sizer)
{
    const BoxSizerLayout layout = boxSizerLayoutFor(sizer);
    float mainTotal = 0.0f;
    float crossMaximum = 0.0f;
    int participatingCount = 0;

    for (const auto& child : sizer.children) {
        const SizerItemLayout itemLayout = sizerItemLayoutFor(child);
        if (!itemLayout.shown) {
            continue;
        }

        const Size minimum = effectiveMinimumSize(child, itemLayout);
        mainTotal += mainSize(minimum, layout.orientation) + mainBorder(itemLayout, layout.orientation);
        crossMaximum = std::max(crossMaximum, crossSize(minimum, layout.orientation) + crossBorder(itemLayout, layout.orientation));
        ++participatingCount;
    }

    const float totalGap = static_cast<float>(layout.gap * std::max(0, participatingCount - 1));
    if (layout.orientation == SizerOrientation::Horizontal) {
        return {
            static_cast<float>(layout.paddingLeft + layout.paddingRight) + mainTotal + totalGap,
            static_cast<float>(layout.paddingTop + layout.paddingBottom) + crossMaximum
        };
    }

    return {
        static_cast<float>(layout.paddingLeft + layout.paddingRight) + crossMaximum,
        static_cast<float>(layout.paddingTop + layout.paddingBottom) + mainTotal + totalGap
    };
}

void layoutBoxSizerChildren(WidgetNode& sizer)
{
    if (sizer.type != WidgetType::Sizer) {
        return;
    }

    const BoxSizerLayout layout = boxSizerLayoutFor(sizer);
    Rect content{
        static_cast<float>(layout.paddingLeft),
        static_cast<float>(layout.paddingTop),
        std::max(0.0f, sizer.bounds.width - static_cast<float>(layout.paddingLeft + layout.paddingRight)),
        std::max(0.0f, sizer.bounds.height - static_cast<float>(layout.paddingTop + layout.paddingBottom))
    };

    auto items = participatingItems(sizer, layout.orientation);
    if (items.empty()) {
        return;
    }

    const float totalGap = static_cast<float>(layout.gap * std::max(0, static_cast<int>(items.size()) - 1));
    const float availableMain = std::max(0.0f, rectMainSize(content, layout.orientation) - totalGap);
    const float availableCross = std::max(0.0f, rectCrossSize(content, layout.orientation));
    float minimumMainTotal = 0.0f;
    for (auto& item : items) {
        item.assignedMain = item.borderedMainMinimum;
        minimumMainTotal += item.borderedMainMinimum;
    }

    assignWeightedExtra(items, std::max(0.0f, availableMain - minimumMainTotal));

    float cursor = mainPosition(content, layout.orientation);
    for (auto& item : items) {
        item.widget->bounds = childRectForSlot(content, layout.orientation, cursor, item, availableCross);
        if (item.widget->type == WidgetType::Sizer) {
            layoutBoxSizerChildren(*item.widget);
        }
        cursor += item.assignedMain + static_cast<float>(layout.gap);
    }
}

} // namespace visiform::model
