#include "ui/WidgetMetrics.h"

#include "model/WidgetRegistry.h"

namespace visiform::ui {

WidgetSizeMetrics getWidgetSizeMetrics(model::WidgetType type)
{
    if (const auto* definition = model::WidgetRegistry::instance().find(type)) {
        return { definition->size.defaultWidth, definition->size.defaultHeight, definition->size.minWidth, definition->size.minHeight };
    }

    return {};
}

float defaultDesignerFontSize()
{
    return 16.0f;
}

float estimatedCharacterWidth(float fontSize)
{
    return fontSize * 0.70f;
}

float estimatedLineHeight(float fontSize)
{
    return fontSize * 1.60f;
}

float estimatedTextBaselineOffset(float fontSize)
{
    return fontSize * 0.35f;
}

float estimateDesignerTextWidth(const std::string& text)
{
    if (text.empty()) {
        return 0.0f;
    }

    return static_cast<float>(text.length()) * estimatedCharacterWidth(defaultDesignerFontSize()) + 24.0f;
}

} // namespace visiform::ui
