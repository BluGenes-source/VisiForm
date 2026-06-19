#pragma once

#include "model/WidgetNode.h"

#include <algorithm>

namespace visiform::model {

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

} // namespace visiform::model
