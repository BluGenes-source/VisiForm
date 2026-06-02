#pragma once

#include "model/WidgetNode.h"

namespace visiform::model {

class LayoutEngine {
public:
    [[nodiscard]] static Rect clientBoundsForParent(const WidgetNode& parent);
    static void applyDockLayout(WidgetNode& root);
    static void applyLayoutFromPrevious(WidgetNode& root, const WidgetNode& previousRoot);
};

} // namespace visiform::model
