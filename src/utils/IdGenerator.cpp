#include "utils/IdGenerator.h"

#include "utils/IdGenerator.h"

#include "model/ProjectDocument.h"

#include <sstream>

namespace visiform::utils {

std::string IdGenerator::next(model::WidgetType widgetType, const model::ProjectDocument& document)
{
    auto& nextValue = nextValues_[widgetType];
    if (nextValue == 0) {
        nextValue = 1;
    }

    const std::string prefix = prefixForType(widgetType);
    while (true) {
        std::ostringstream stream;
        stream << prefix << nextValue++;
        const std::string candidate = stream.str();

        if (document.findWidgetById(candidate) == nullptr) {
            return candidate;
        }
    }
}

std::string IdGenerator::prefixForType(model::WidgetType widgetType)
{
    switch (widgetType) {
    case model::WidgetType::FormWindow:
        return "form_";
    case model::WidgetType::Frame:
        return "frame_";
    case model::WidgetType::Label:
        return "label_";
    case model::WidgetType::Button:
        return "button_";
    case model::WidgetType::TextBox:
        return "textbox_";
    case model::WidgetType::CheckBox:
        return "checkbox_";
    case model::WidgetType::Slider:
        return "slider_";
    case model::WidgetType::Image:
        return "image_";
    case model::WidgetType::Spacer:
        return "spacer_";
    }

    return "widget_";
}

} // namespace visiform::utils
