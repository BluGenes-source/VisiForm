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

std::string IdGenerator::next(model::ProjectResourceType resourceType, const model::ProjectDocument& document)
{
    auto& nextValue = nextResourceValues_[resourceType];
    if (nextValue == 0) {
        nextValue = 1;
    }

    const std::string prefix = prefixForType(resourceType);
    while (true) {
        std::ostringstream stream;
        stream << prefix << nextValue++;
        const std::string candidate = stream.str();

        if (document.findResourceById(candidate) == nullptr) {
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
    case model::WidgetType::GroupBox:
        return "groupbox_";
    case model::WidgetType::Panel:
        return "panel_";
    case model::WidgetType::Sizer:
        return "sizer_";
    case model::WidgetType::TabControl:
        return "tabcontrol_";
    case model::WidgetType::TabPage:
        return "tabpage_";
    case model::WidgetType::Label:
        return "label_";
    case model::WidgetType::Button:
        return "button_";
    case model::WidgetType::TextBox:
        return "textbox_";
    case model::WidgetType::ComboBox:
        return "combobox_";
    case model::WidgetType::ListBox:
        return "listbox_";
    case model::WidgetType::CheckBox:
        return "checkbox_";
    case model::WidgetType::RadioButton:
        return "radiobutton_";
    case model::WidgetType::Slider:
        return "slider_";
    case model::WidgetType::ScrollBar:
        return "scrollbar_";
    case model::WidgetType::StatusBar:
        return "statusbar_";
    case model::WidgetType::ProgressBar:
        return "progressbar_";
    case model::WidgetType::ModalDialog:
        return "modaldialog_";
    case model::WidgetType::ColorPicker:
        return "colorpicker_";
    case model::WidgetType::Image:
        return "image_";
    case model::WidgetType::Spacer:
        return "spacer_";
    }

    return "widget_";
}

std::string IdGenerator::prefixForType(model::ProjectResourceType resourceType)
{
    switch (resourceType) {
    case model::ProjectResourceType::Image:
        return "image_";
    case model::ProjectResourceType::Font:
        return "font_";
    case model::ProjectResourceType::Icon:
        return "icon_";
    case model::ProjectResourceType::Theme:
        return "theme_";
    case model::ProjectResourceType::Other:
        return "resource_";
    }

    return "resource_";
}

} // namespace visiform::utils
