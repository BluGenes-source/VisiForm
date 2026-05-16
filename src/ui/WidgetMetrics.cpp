#include "ui/WidgetMetrics.h"

namespace visiform::ui {

WidgetSizeMetrics getWidgetSizeMetrics(model::WidgetType type)
{
    switch (type) {
    case model::WidgetType::FormWindow:
        return { 900.0f, 600.0f, 300.0f, 200.0f };
    case model::WidgetType::Label:
        return { 260.0f, 64.0f, 140.0f, 58.0f };
    case model::WidgetType::Button:
        return { 260.0f, 56.0f, 140.0f, 52.0f };
    case model::WidgetType::TextBox:
        return { 260.0f, 48.0f, 160.0f, 44.0f };
    case model::WidgetType::CheckBox:
        return { 300.0f, 68.0f, 200.0f, 62.0f };
    case model::WidgetType::Slider:
        return { 240.0f, 44.0f, 120.0f, 40.0f };
    case model::WidgetType::Frame:
        return { 300.0f, 180.0f, 180.0f, 120.0f };
    case model::WidgetType::Image:
        return { 200.0f, 140.0f, 100.0f, 80.0f };
    case model::WidgetType::Spacer:
        return { 180.0f, 50.0f, 40.0f, 30.0f };
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
