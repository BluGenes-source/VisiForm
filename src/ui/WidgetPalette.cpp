#include "ui/WidgetPalette.h"

#include "model/WidgetDefinition.h"
#include "model/WidgetRegistry.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace visiform::ui {
namespace {

constexpr float kOuterPadding = 8.0f;
constexpr float kTabHeight = 30.0f;
constexpr float kTabGap = 4.0f;
constexpr float kItemGap = 6.0f;
constexpr float kItemHeight = 34.0f;
constexpr float kScrollStep = 72.0f;
constexpr float kScrollButtonWidth = 24.0f;

constexpr std::array<const char*, 7> kCategoryOrder{
    "Common",
    "Containers",
    "Layout",
    "Forms",
    "Data",
    "Menu/Toolbar",
    "Additional"
};

bool containsPoint(const model::Rect& bounds, float x, float y)
{
    return x >= bounds.x && y >= bounds.y && x <= bounds.x + bounds.width && y <= bounds.y + bounds.height;
}

int categoryRank(const std::string& category)
{
    const auto found = std::find(kCategoryOrder.begin(), kCategoryOrder.end(), category);
    return found == kCategoryOrder.end()
        ? static_cast<int>(kCategoryOrder.size())
        : static_cast<int>(std::distance(kCategoryOrder.begin(), found));
}

} // namespace

void WidgetPalette::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
    clampScrollOffsets();
}

bool WidgetPalette::contains(float x, float y) const
{
    return x >= x_ && y >= y_ && x <= x_ + width_ && y <= y_ + height_;
}

bool WidgetPalette::mouseDown(float x, float y)
{
    if (!contains(x, y)) {
        return false;
    }

    const auto categoryList = categories();
    const model::Rect tabs = tabStripBounds();
    const float maxTabScroll = maximumTabScroll(categoryList);
    if (containsPoint(tabs, x, y)) {
        if (maxTabScroll > 0.0f && x <= tabs.x + kScrollButtonWidth) {
            tabScrollOffset_ = std::max(0.0f, tabScrollOffset_ - kScrollStep);
            return true;
        }
        if (maxTabScroll > 0.0f && x >= tabs.x + tabs.width - kScrollButtonWidth) {
            tabScrollOffset_ = std::min(maxTabScroll, tabScrollOffset_ + kScrollStep);
            return true;
        }

        const float clipLeft = tabs.x + (maxTabScroll > 0.0f ? kScrollButtonWidth : 0.0f);
        const float clipRight = tabs.x + tabs.width - (maxTabScroll > 0.0f ? kScrollButtonWidth : 0.0f);
        float tabX = clipLeft - tabScrollOffset_;
        for (const auto& category : categoryList) {
            const float currentWidth = tabWidth(category.name);
            if (x >= std::max(tabX, clipLeft) && x <= std::min(tabX + currentWidth, clipRight)) {
                selectedCategory_ = category.name;
                itemScrollOffset_ = 0.0f;
                return true;
            }
            tabX += currentWidth + kTabGap;
        }
    }

    const Category* category = selectedCategory(categoryList);
    if (category == nullptr) {
        return false;
    }

    const model::Rect items = itemStripBounds();
    const float maxItemScroll = maximumItemScroll(*category);
    if (containsPoint(items, x, y) && maxItemScroll > 0.0f) {
        if (x <= items.x + kScrollButtonWidth) {
            itemScrollOffset_ = std::max(0.0f, itemScrollOffset_ - kScrollStep);
            return true;
        }
        if (x >= items.x + items.width - kScrollButtonWidth) {
            itemScrollOffset_ = std::min(maxItemScroll, itemScrollOffset_ + kScrollStep);
            return true;
        }
    }

    return false;
}

bool WidgetPalette::mouseDrag(float, float)
{
    return false;
}

bool WidgetPalette::mouseUp()
{
    return false;
}

bool WidgetPalette::mouseWheel(float deltaY, float x, float y)
{
    if (!contains(x, y)) {
        return false;
    }

    const auto categoryList = categories();
    if (containsPoint(tabStripBounds(), x, y)) {
        const float maximum = maximumTabScroll(categoryList);
        if (maximum <= 0.0f) {
            return false;
        }
        const float nextOffset = std::clamp(tabScrollOffset_ + (-deltaY * kScrollStep), 0.0f, maximum);
        const bool changed = std::abs(nextOffset - tabScrollOffset_) > 0.01f;
        tabScrollOffset_ = nextOffset;
        return changed;
    }

    const Category* category = selectedCategory(categoryList);
    if (category == nullptr) {
        return false;
    }

    const float maximum = maximumItemScroll(*category);
    if (maximum <= 0.0f) {
        return false;
    }

    const float nextOffset = std::clamp(itemScrollOffset_ + (-deltaY * kScrollStep), 0.0f, maximum);
    const bool changed = std::abs(nextOffset - itemScrollOffset_) > 0.01f;
    itemScrollOffset_ = nextOffset;
    return changed;
}

bool WidgetPalette::mouseMove(float x, float y)
{
    const auto previousCategory = hoveredCategory_;
    const auto previousWidget = hoveredWidgetType_;
    hoveredCategory_.reset();
    hoveredWidgetType_.reset();

    if (contains(x, y)) {
        const auto categoryList = categories();
        const model::Rect tabs = tabStripBounds();
        const float maxTabScroll = maximumTabScroll(categoryList);
        if (containsPoint(tabs, x, y)) {
            const float clipLeft = tabs.x + (maxTabScroll > 0.0f ? kScrollButtonWidth : 0.0f);
            const float clipRight = tabs.x + tabs.width - (maxTabScroll > 0.0f ? kScrollButtonWidth : 0.0f);
            float tabX = clipLeft - tabScrollOffset_;
            for (const auto& category : categoryList) {
                const float currentWidth = tabWidth(category.name);
                if (x >= std::max(tabX, clipLeft) && x <= std::min(tabX + currentWidth, clipRight)) {
                    hoveredCategory_ = category.name;
                    break;
                }
                tabX += currentWidth + kTabGap;
            }
        }
        hoveredWidgetType_ = hitTestWidgetType(x, y);
    }

    return previousCategory != hoveredCategory_ || previousWidget != hoveredWidgetType_;
}

std::optional<model::WidgetType> WidgetPalette::hitTestWidgetType(float x, float y) const
{
    if (!containsPoint(itemStripBounds(), x, y)) {
        return std::nullopt;
    }

    const auto categoryList = categories();
    const Category* category = selectedCategory(categoryList);
    if (category == nullptr) {
        return std::nullopt;
    }

    const model::Rect items = itemStripBounds();
    const float maxScroll = maximumItemScroll(*category);
    const float clipLeft = items.x + (maxScroll > 0.0f ? kScrollButtonWidth : 0.0f);
    const float clipRight = items.x + items.width - (maxScroll > 0.0f ? kScrollButtonWidth : 0.0f);
    if (x < clipLeft || x > clipRight) {
        return std::nullopt;
    }

    float itemX = clipLeft - itemScrollOffset_;
    for (const auto* entry : category->entries) {
        const float currentWidth = itemWidth(*entry);
        if (x >= std::max(itemX, clipLeft) && x <= std::min(itemX + currentWidth, clipRight)) {
            return entry->type;
        }
        itemX += currentWidth + kItemGap;
    }

    return std::nullopt;
}

std::optional<std::string> WidgetPalette::hitTestHint(float x, float y) const
{
    if (!contains(x, y)) {
        return std::nullopt;
    }

    if (const auto type = hitTestWidgetType(x, y)) {
        if (const auto* definition = model::WidgetRegistry::instance().find(*type)) {
            return definition->displayName + ": " + definition->defaultHint;
        }
    }

    if (hoveredCategory_.has_value()) {
        return "Widget category: " + *hoveredCategory_;
    }

    return std::string{ "Widget Palette" };
}

void WidgetPalette::draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const
{
    if (width_ <= 0.0f || height_ <= 0.0f) {
        return;
    }

    canvas.setColor(0xff202630);
    canvas.fill(x_, y_, width_, height_);
    canvas.setColor(0xff11151c);
    canvas.fill(x_, y_, width_, 1.0f);
    canvas.fill(x_, y_ + height_ - 1.0f, width_, 1.0f);

    const auto categoryList = categories();
    const model::Rect tabs = tabStripBounds();
    const float maxTabScroll = maximumTabScroll(categoryList);
    const float tabClipLeft = tabs.x + (maxTabScroll > 0.0f ? kScrollButtonWidth : 0.0f);
    const float tabClipRight = tabs.x + tabs.width - (maxTabScroll > 0.0f ? kScrollButtonWidth : 0.0f);
    float tabX = tabClipLeft - tabScrollOffset_;
    for (const auto& category : categoryList) {
        const float currentWidth = tabWidth(category.name);
        const float visibleLeft = std::max(tabX, tabClipLeft);
        const float visibleRight = std::min(tabX + currentWidth, tabClipRight);
        if (visibleRight > visibleLeft) {
            const bool selected = category.name == selectedCategory_;
            const bool hovered = hoveredCategory_ == category.name;
            canvas.setColor(selected ? 0xff3f78b7 : (hovered ? 0xff354252 : 0xff2a313c));
            canvas.fill(visibleLeft, tabs.y, visibleRight - visibleLeft, tabs.height);
            if (selected) {
                canvas.setColor(0xff79b7ff);
                canvas.fill(visibleLeft, tabs.y + tabs.height - 3.0f, visibleRight - visibleLeft, 3.0f);
            }
            if (drawText && tabX >= tabClipLeft && tabX + currentWidth <= tabClipRight) {
                canvas.setColor(0xfff2f5f8);
                canvas.text(category.name, font, visage::Font::kCenter, tabX, tabs.y, currentWidth, tabs.height - 2.0f);
            }
        }
        tabX += currentWidth + kTabGap;
    }

    if (maxTabScroll > 0.0f) {
        canvas.setColor(0xff171c24);
        canvas.fill(tabs.x, tabs.y, kScrollButtonWidth, tabs.height);
        canvas.fill(tabs.x + tabs.width - kScrollButtonWidth, tabs.y, kScrollButtonWidth, tabs.height);
        if (drawText) {
            canvas.setColor(0xffdbe2ea);
            canvas.text("<", font, visage::Font::kCenter, tabs.x, tabs.y, kScrollButtonWidth, tabs.height);
            canvas.text(">", font, visage::Font::kCenter,
                tabs.x + tabs.width - kScrollButtonWidth, tabs.y, kScrollButtonWidth, tabs.height);
        }
    }

    const Category* category = selectedCategory(categoryList);
    if (category == nullptr) {
        return;
    }

    const model::Rect items = itemStripBounds();
    const float maxItemScroll = maximumItemScroll(*category);
    const float itemClipLeft = items.x + (maxItemScroll > 0.0f ? kScrollButtonWidth : 0.0f);
    const float itemClipRight = items.x + items.width - (maxItemScroll > 0.0f ? kScrollButtonWidth : 0.0f);
    float itemX = itemClipLeft - itemScrollOffset_;
    for (const auto* entry : category->entries) {
        const float currentWidth = itemWidth(*entry);
        const float visibleLeft = std::max(itemX, itemClipLeft);
        const float visibleRight = std::min(itemX + currentWidth, itemClipRight);
        if (visibleRight > visibleLeft) {
            const bool hovered = hoveredWidgetType_ == entry->type;
            canvas.setColor(hovered ? 0xff44566c : 0xff303946);
            canvas.fill(visibleLeft, items.y, visibleRight - visibleLeft, items.height);
            canvas.setColor(hovered ? 0xff82b9f2 : 0xff536173);
            canvas.fill(visibleLeft, items.y, 3.0f, items.height);
            if (drawText && itemX >= itemClipLeft && itemX + currentWidth <= itemClipRight) {
                canvas.setColor(0xffedf1f5);
                canvas.text(entry->displayName, font, visage::Font::kCenter,
                    itemX + 6.0f, items.y, currentWidth - 12.0f, items.height);
            }
        }
        itemX += currentWidth + kItemGap;
    }

    if (maxItemScroll > 0.0f) {
        canvas.setColor(0xff171c24);
        canvas.fill(items.x, items.y, kScrollButtonWidth, items.height);
        canvas.fill(items.x + items.width - kScrollButtonWidth, items.y, kScrollButtonWidth, items.height);
        if (drawText) {
            canvas.setColor(0xffdbe2ea);
            canvas.text("<", font, visage::Font::kCenter, items.x, items.y, kScrollButtonWidth, items.height);
            canvas.text(">", font, visage::Font::kCenter,
                items.x + items.width - kScrollButtonWidth, items.y, kScrollButtonWidth, items.height);
        }
    }
}

std::vector<WidgetPalette::Category> WidgetPalette::categories() const
{
    std::vector<Category> result;
    for (const auto* definition : model::WidgetRegistry::instance().paletteDefinitions()) {
        auto found = std::find_if(result.begin(), result.end(), [definition](const Category& category) {
            return category.name == definition->paletteGroup;
        });
        if (found == result.end()) {
            result.push_back(Category{ definition->paletteGroup, { definition } });
        }
        else {
            found->entries.push_back(definition);
        }
    }

    std::stable_sort(result.begin(), result.end(), [](const Category& left, const Category& right) {
        const int leftRank = categoryRank(left.name);
        const int rightRank = categoryRank(right.name);
        return leftRank == rightRank ? left.name < right.name : leftRank < rightRank;
    });
    for (auto& category : result) {
        std::stable_sort(category.entries.begin(), category.entries.end(), [](const auto* left, const auto* right) {
            return left->paletteOrder == right->paletteOrder
                ? left->displayName < right->displayName
                : left->paletteOrder < right->paletteOrder;
        });
    }
    return result;
}

const WidgetPalette::Category* WidgetPalette::selectedCategory(const std::vector<Category>& categoryList) const
{
    const auto selected = std::find_if(categoryList.begin(), categoryList.end(), [this](const Category& category) {
        return category.name == selectedCategory_;
    });
    return selected == categoryList.end()
        ? (categoryList.empty() ? nullptr : &categoryList.front())
        : &*selected;
}

model::Rect WidgetPalette::tabStripBounds() const
{
    return { x_ + kOuterPadding, y_ + 6.0f, std::max(0.0f, width_ - kOuterPadding * 2.0f), kTabHeight };
}

model::Rect WidgetPalette::itemStripBounds() const
{
    return {
        x_ + kOuterPadding,
        y_ + kTabHeight + 10.0f,
        std::max(0.0f, width_ - kOuterPadding * 2.0f),
        std::max(0.0f, std::min(kItemHeight, height_ - kTabHeight - 14.0f))
    };
}

float WidgetPalette::tabWidth(const std::string& label) const
{
    return std::clamp(28.0f + static_cast<float>(label.size()) * 7.2f, 76.0f, 126.0f);
}

float WidgetPalette::itemWidth(const model::WidgetDefinition& definition) const
{
    return std::clamp(30.0f + static_cast<float>(definition.displayName.size()) * 7.0f, 86.0f, 142.0f);
}

float WidgetPalette::totalTabWidth(const std::vector<Category>& categoryList) const
{
    float total = 0.0f;
    for (const auto& category : categoryList) {
        total += tabWidth(category.name) + kTabGap;
    }
    return std::max(0.0f, total - kTabGap);
}

float WidgetPalette::totalItemWidth(const Category& category) const
{
    float total = 0.0f;
    for (const auto* definition : category.entries) {
        total += itemWidth(*definition) + kItemGap;
    }
    return std::max(0.0f, total - kItemGap);
}

float WidgetPalette::maximumTabScroll(const std::vector<Category>& categoryList) const
{
    const float fullWidth = tabStripBounds().width;
    const float totalWidth = totalTabWidth(categoryList);
    if (totalWidth <= fullWidth) {
        return 0.0f;
    }
    return std::max(0.0f, totalWidth - std::max(0.0f, fullWidth - kScrollButtonWidth * 2.0f));
}

float WidgetPalette::maximumItemScroll(const Category& category) const
{
    const float fullWidth = itemStripBounds().width;
    const float totalWidth = totalItemWidth(category);
    if (totalWidth <= fullWidth) {
        return 0.0f;
    }
    return std::max(0.0f, totalWidth - std::max(0.0f, fullWidth - kScrollButtonWidth * 2.0f));
}

void WidgetPalette::clampScrollOffsets()
{
    const auto categoryList = categories();
    if (!categoryList.empty()
        && std::none_of(categoryList.begin(), categoryList.end(), [this](const Category& category) {
            return category.name == selectedCategory_;
        })) {
        selectedCategory_ = categoryList.front().name;
    }

    tabScrollOffset_ = std::clamp(tabScrollOffset_, 0.0f, maximumTabScroll(categoryList));
    if (const Category* category = selectedCategory(categoryList)) {
        itemScrollOffset_ = std::clamp(itemScrollOffset_, 0.0f, maximumItemScroll(*category));
    }
    else {
        itemScrollOffset_ = 0.0f;
    }
}

} // namespace visiform::ui
