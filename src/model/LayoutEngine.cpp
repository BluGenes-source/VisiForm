#include "model/LayoutEngine.h"

#include "model/WidgetRegistry.h"

#include <algorithm>
#include <cmath>

namespace visiform::model {
namespace {

constexpr float kTabControlPageInset = 6.0f;
constexpr float kTabControlHeaderHeight = 30.0f;
constexpr float kFrameHorizontalPadding = 12.0f;
constexpr float kFrameTopPadding = 28.0f;
constexpr float kFrameBottomPadding = 12.0f;
constexpr float kGroupBoxHorizontalPadding = 16.0f;
constexpr float kGroupBoxTopPadding = 32.0f;
constexpr float kGroupBoxBottomPadding = 16.0f;
constexpr float kMinimumClientSize = 20.0f;
constexpr float kMinimumWidgetSize = 20.0f;
constexpr float kFloatEpsilon = 0.01f;
constexpr float kDefaultSizerPadding = 8.0f;
constexpr float kDefaultSizerGap = 8.0f;

struct AnchorFlags {
    bool left = false;
    bool right = false;
    bool top = false;
    bool bottom = false;
};

const WidgetNode* findDirectChildById(const WidgetNode* parent, const std::string& childId)
{
    if (parent == nullptr) {
        return nullptr;
    }

    const auto iterator = std::find_if(parent->children.begin(), parent->children.end(), [&childId](const WidgetNode& child) {
        return child.id == childId;
    });
    return iterator == parent->children.end() ? nullptr : &*iterator;
}

Rect tabPageBoundsForTabControl(const WidgetNode& tabControl)
{
    return {
        kTabControlPageInset,
        kTabControlHeaderHeight + kTabControlPageInset,
        std::max(kMinimumClientSize, tabControl.bounds.width - kTabControlPageInset * 2.0f),
        std::max(kMinimumClientSize, tabControl.bounds.height - kTabControlHeaderHeight - kTabControlPageInset * 2.0f)
    };
}

void syncTabPageBounds(WidgetNode& parent)
{
    if (parent.type != WidgetType::TabControl) {
        return;
    }

    const Rect pageBounds = tabPageBoundsForTabControl(parent);
    for (auto& child : parent.children) {
        if (child.type == WidgetType::TabPage) {
            child.bounds = pageBounds;
        }
    }
}

AnchorFlags anchorFlagsForMode(AnchorMode mode)
{
    switch (mode) {
    case AnchorMode::None:
        return {};
    case AnchorMode::TopLeft:
        return { true, false, true, false };
    case AnchorMode::TopRight:
        return { false, true, true, false };
    case AnchorMode::BottomLeft:
        return { true, false, false, true };
    case AnchorMode::BottomRight:
        return { false, true, false, true };
    case AnchorMode::StretchWidthTop:
        return { true, true, true, false };
    case AnchorMode::StretchWidthBottom:
        return { true, true, false, true };
    case AnchorMode::StretchHeightLeft:
        return { true, false, true, true };
    case AnchorMode::StretchHeightRight:
        return { false, true, true, true };
    case AnchorMode::Fill:
        return { true, true, true, true };
    }

    return { true, false, true, false };
}

bool rectNearlyEqual(const Rect& first, const Rect& second)
{
    return std::abs(first.x - second.x) <= kFloatEpsilon
        && std::abs(first.y - second.y) <= kFloatEpsilon
        && std::abs(first.width - second.width) <= kFloatEpsilon
        && std::abs(first.height - second.height) <= kFloatEpsilon;
}

void applyAnchorResizeToChild(WidgetNode& child,
    const WidgetNode* previousChild,
    const Rect& previousClientBounds,
    const Rect& currentClientBounds)
{
    if (child.dockMode() != DockMode::None || previousChild == nullptr) {
        return;
    }

    const AnchorFlags flags = anchorFlagsForMode(child.anchorMode());
    if (!flags.left && !flags.right && !flags.top && !flags.bottom) {
        return;
    }

    const Rect previousBounds = previousChild->bounds;
    const float leftMargin = previousBounds.x - previousClientBounds.x;
    const float rightMargin = previousClientBounds.x + previousClientBounds.width - (previousBounds.x + previousBounds.width);
    const float topMargin = previousBounds.y - previousClientBounds.y;
    const float bottomMargin = previousClientBounds.y + previousClientBounds.height - (previousBounds.y + previousBounds.height);

    Rect updatedBounds = previousBounds;
    if (flags.left && flags.right) {
        updatedBounds.x = currentClientBounds.x + leftMargin;
        updatedBounds.width = std::max(kMinimumWidgetSize, currentClientBounds.width - leftMargin - rightMargin);
    }
    else if (flags.right) {
        updatedBounds.x = currentClientBounds.x + currentClientBounds.width - rightMargin - previousBounds.width;
    }
    else if (flags.left) {
        updatedBounds.x = currentClientBounds.x + leftMargin;
    }

    if (flags.top && flags.bottom) {
        updatedBounds.y = currentClientBounds.y + topMargin;
        updatedBounds.height = std::max(kMinimumWidgetSize, currentClientBounds.height - topMargin - bottomMargin);
    }
    else if (flags.bottom) {
        updatedBounds.y = currentClientBounds.y + currentClientBounds.height - bottomMargin - previousBounds.height;
    }
    else if (flags.top) {
        updatedBounds.y = currentClientBounds.y + topMargin;
    }

    child.bounds = updatedBounds;
}

void applyDockToDirectChildren(WidgetNode& parent)
{
    if (!WidgetRegistry::instance().canContainChildren(parent.type)) {
        return;
    }

    Rect remaining = LayoutEngine::clientBoundsForParent(parent);
    if (parent.type == WidgetType::Sizer) {
        if (parent.children.empty()) {
            return;
        }

        const bool horizontal = parent.getStringProperty("orientation", "Vertical") == "Horizontal";
        const float gap = std::clamp(parent.getFloatProperty("gap", kDefaultSizerGap), 0.0f, 64.0f);
        const float totalGap = gap * static_cast<float>(parent.children.size() - 1);
        const float availableMain = std::max(0.0f, (horizontal ? remaining.width : remaining.height) - totalGap);
        const float slotSize = availableMain / static_cast<float>(parent.children.size());
        float cursor = horizontal ? remaining.x : remaining.y;

        for (auto& child : parent.children) {
            if (horizontal) {
                child.bounds = {
                    cursor,
                    remaining.y,
                    std::max(kMinimumWidgetSize, slotSize),
                    std::max(kMinimumWidgetSize, remaining.height)
                };
                cursor += slotSize + gap;
            }
            else {
                child.bounds = {
                    remaining.x,
                    cursor,
                    std::max(kMinimumWidgetSize, remaining.width),
                    std::max(kMinimumWidgetSize, slotSize)
                };
                cursor += slotSize + gap;
            }
        }
        return;
    }

    for (auto& child : parent.children) {
        if (parent.type == WidgetType::TabControl && child.type == WidgetType::TabPage) {
            continue;
        }

        const DockMode dock = child.dockMode();
        const float originalWidth = child.bounds.width;
        const float originalHeight = child.bounds.height;

        switch (dock) {
        case DockMode::Top:
            child.bounds.x = remaining.x;
            child.bounds.y = remaining.y;
            child.bounds.width = remaining.width;
            remaining.y += originalHeight;
            remaining.height = std::max(0.0f, remaining.height - originalHeight);
            break;
        case DockMode::Bottom:
            child.bounds.x = remaining.x;
            child.bounds.y = std::max(remaining.y, remaining.y + remaining.height - originalHeight);
            child.bounds.width = remaining.width;
            remaining.height = std::max(0.0f, remaining.height - originalHeight);
            break;
        case DockMode::Left:
            child.bounds.x = remaining.x;
            child.bounds.y = remaining.y;
            child.bounds.height = remaining.height;
            remaining.x += originalWidth;
            remaining.width = std::max(0.0f, remaining.width - originalWidth);
            break;
        case DockMode::Right:
            child.bounds.x = std::max(remaining.x, remaining.x + remaining.width - originalWidth);
            child.bounds.y = remaining.y;
            child.bounds.height = remaining.height;
            remaining.width = std::max(0.0f, remaining.width - originalWidth);
            break;
        case DockMode::Fill:
            child.bounds = remaining;
            break;
        case DockMode::None:
            if (child.type == WidgetType::StatusBar && child.getBoolProperty("fillWidth", false)) {
                child.bounds.x = remaining.x;
                child.bounds.width = remaining.width;
            }
            break;
        }
    }
}

void applyLayoutRecursive(WidgetNode& current, const WidgetNode* previousRoot)
{
    syncTabPageBounds(current);

    const WidgetNode* previousCurrent = previousRoot != nullptr ? previousRoot->findById(current.id) : nullptr;
    const Rect previousClientBounds = previousCurrent != nullptr
        ? LayoutEngine::clientBoundsForParent(*previousCurrent)
        : LayoutEngine::clientBoundsForParent(current);
    const Rect currentClientBounds = LayoutEngine::clientBoundsForParent(current);

    if (previousCurrent != nullptr && !rectNearlyEqual(previousClientBounds, currentClientBounds)) {
        for (auto& child : current.children) {
            applyAnchorResizeToChild(child, findDirectChildById(previousCurrent, child.id), previousClientBounds, currentClientBounds);
        }
    }

    applyDockToDirectChildren(current);
    syncTabPageBounds(current);

    for (auto& child : current.children) {
        applyLayoutRecursive(child, previousRoot);
    }
}

} // namespace

Rect LayoutEngine::clientBoundsForParent(const WidgetNode& parent)
{
    switch (parent.type) {
    case WidgetType::Frame:
        return {
            kFrameHorizontalPadding,
            kFrameTopPadding,
            std::max(kMinimumClientSize, parent.bounds.width - kFrameHorizontalPadding * 2.0f),
            std::max(kMinimumClientSize, parent.bounds.height - kFrameTopPadding - kFrameBottomPadding)
        };
    case WidgetType::GroupBox:
        return {
            kGroupBoxHorizontalPadding,
            kGroupBoxTopPadding,
            std::max(kMinimumClientSize, parent.bounds.width - kGroupBoxHorizontalPadding * 2.0f),
            std::max(kMinimumClientSize, parent.bounds.height - kGroupBoxTopPadding - kGroupBoxBottomPadding)
        };
    case WidgetType::TabControl:
        return tabPageBoundsForTabControl(parent);
    case WidgetType::Sizer: {
        const float padding = std::clamp(parent.getFloatProperty("padding", kDefaultSizerPadding), 0.0f, 64.0f);
        return {
            padding,
            padding,
            std::max(kMinimumClientSize, parent.bounds.width - padding * 2.0f),
            std::max(kMinimumClientSize, parent.bounds.height - padding * 2.0f)
        };
    }
    case WidgetType::FormWindow:
    case WidgetType::Panel:
    case WidgetType::TabPage:
    default:
        return { 0.0f, 0.0f, std::max(kMinimumClientSize, parent.bounds.width), std::max(kMinimumClientSize, parent.bounds.height) };
    }
}

void LayoutEngine::applyDockLayout(WidgetNode& root)
{
    applyLayoutRecursive(root, nullptr);
}

void LayoutEngine::applyLayoutFromPrevious(WidgetNode& root, const WidgetNode& previousRoot)
{
    applyLayoutRecursive(root, &previousRoot);
}

} // namespace visiform::model
