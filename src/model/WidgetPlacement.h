#pragma once

#include "model/WidgetNode.h"

#include <algorithm>

namespace visiform::model {

enum class ParentAlignment {
    Start,
    Center,
    End
};

[[nodiscard]] inline Rect clampWidgetBoundsToParent(const Rect& parentClientBounds, Rect bounds)
{
    const float maximumX = parentClientBounds.x + std::max(0.0f, parentClientBounds.width - bounds.width);
    const float maximumY = parentClientBounds.y + std::max(0.0f, parentClientBounds.height - bounds.height);

    bounds.x = maximumX <= parentClientBounds.x
        ? parentClientBounds.x
        : std::clamp(bounds.x, parentClientBounds.x, maximumX);
    bounds.y = maximumY <= parentClientBounds.y
        ? parentClientBounds.y
        : std::clamp(bounds.y, parentClientBounds.y, maximumY);
    return bounds;
}

[[nodiscard]] inline Rect safeWidgetPlacement(const Rect& parentClientBounds,
    float desiredWidth,
    float desiredHeight,
    float preferredX,
    float preferredY,
    float edgeInset)
{
    const float inset = std::max(0.0f, edgeInset);
    const float minimumX = parentClientBounds.x + inset;
    const float minimumY = parentClientBounds.y + inset;
    const float maximumX = parentClientBounds.x + parentClientBounds.width - inset - desiredWidth;
    const float maximumY = parentClientBounds.y + parentClientBounds.height - inset - desiredHeight;

    return {
        maximumX < minimumX ? minimumX : std::clamp(preferredX, minimumX, maximumX),
        maximumY < minimumY ? minimumY : std::clamp(preferredY, minimumY, maximumY),
        desiredWidth,
        desiredHeight
    };
}

[[nodiscard]] inline Rect fitWidgetWidthToParent(const Rect& parentClientBounds, Rect bounds)
{
    bounds.x = parentClientBounds.x;
    bounds.width = parentClientBounds.width;
    return bounds;
}

[[nodiscard]] inline Rect fitWidgetHeightToParent(const Rect& parentClientBounds, Rect bounds)
{
    bounds.y = parentClientBounds.y;
    bounds.height = parentClientBounds.height;
    return bounds;
}

[[nodiscard]] inline Rect alignWidgetHorizontallyToParent(const Rect& parentClientBounds,
    Rect bounds,
    ParentAlignment alignment)
{
    if (bounds.width >= parentClientBounds.width || alignment == ParentAlignment::Start) {
        bounds.x = parentClientBounds.x;
    }
    else if (alignment == ParentAlignment::Center) {
        bounds.x = parentClientBounds.x + (parentClientBounds.width - bounds.width) * 0.5f;
    }
    else {
        bounds.x = parentClientBounds.x + parentClientBounds.width - bounds.width;
    }
    return bounds;
}

[[nodiscard]] inline Rect alignWidgetVerticallyToParent(const Rect& parentClientBounds,
    Rect bounds,
    ParentAlignment alignment)
{
    if (bounds.height >= parentClientBounds.height || alignment == ParentAlignment::Start) {
        bounds.y = parentClientBounds.y;
    }
    else if (alignment == ParentAlignment::Center) {
        bounds.y = parentClientBounds.y + (parentClientBounds.height - bounds.height) * 0.5f;
    }
    else {
        bounds.y = parentClientBounds.y + parentClientBounds.height - bounds.height;
    }
    return bounds;
}

} // namespace visiform::model
