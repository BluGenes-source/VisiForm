#pragma once

#include "model/WidgetNode.h"

#include <string>

namespace visiform::ui {

struct WidgetSizeMetrics {
    float defaultWidth = 20.0f;
    float defaultHeight = 20.0f;
    float minWidth = 20.0f;
    float minHeight = 20.0f;
};

[[nodiscard]] WidgetSizeMetrics getWidgetSizeMetrics(model::WidgetType type);
[[nodiscard]] float defaultDesignerFontSize();
[[nodiscard]] float estimatedCharacterWidth(float fontSize);
[[nodiscard]] float estimatedLineHeight(float fontSize);
[[nodiscard]] float estimatedTextBaselineOffset(float fontSize);
[[nodiscard]] float estimateDesignerTextWidth(const std::string& text);

} // namespace visiform::ui
