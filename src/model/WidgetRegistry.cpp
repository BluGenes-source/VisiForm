#include "model/WidgetRegistry.h"
#include "model/WidgetRegistry.h"

#include <algorithm>

namespace visiform::model {
namespace {

std::vector<WidgetPropertyDefinition> commonTextProperties()
{
    return {
        { "hint", "hint", PropertyValue{}, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
}

std::vector<WidgetPropertyDefinition> commonStyleProperties(bool includeLookAndFeelId = true)
{
    std::vector<WidgetPropertyDefinition> properties;
    if (includeLookAndFeelId) {
        properties.push_back({ "lookAndFeelId", "Look and Feel", "", PropertyEditKind::Text, true, "Optional widget look and feel override. Empty means inherit from the project." });
    }
    properties.push_back({ "fillColor", "Fill Color", "", PropertyEditKind::Color, true, "Optional fill color override. Empty means inherit." });
    properties.push_back({ "textColor", "Text Color", "", PropertyEditKind::Color, true, "Optional text color override. Empty means inherit." });
    properties.push_back({ "borderColor", "Border Color", "", PropertyEditKind::Color, true, "Optional border color override. Empty means inherit." });
    properties.push_back({ "accentColor", "Accent Color", "", PropertyEditKind::Color, true, "Optional accent color override. Empty means inherit." });
    properties.push_back({ "borderThickness", "Border Thickness", PropertyValue{}, PropertyEditKind::Float, true, "Optional border thickness override. Empty means inherit." });
    properties.push_back({ "cornerRadius", "Corner Radius", PropertyValue{}, PropertyEditKind::Float, true, "Optional corner radius override. Empty means inherit." });
    properties.push_back({ "fontSize", "Font Size", PropertyValue{}, PropertyEditKind::Float, true, "Optional font size override. Empty means inherit." });
    return properties;
}

std::vector<WidgetPropertyDefinition> commonFontProperties()
{
    return {
        { "fontFamily", "Font Family", "Default", PropertyEditKind::Text, true, "Font family name. \"Default\" uses the editor fallback font." },
        { "fontBold", "Bold", false, PropertyEditKind::Bool, true, "Use bold text style when supported by the preview font." },
        { "fontItalic", "Italic", false, PropertyEditKind::Bool, true, "Use italic text style when supported by the preview font." }
    };
}

void appendProperties(std::vector<WidgetPropertyDefinition>& target, const std::vector<WidgetPropertyDefinition>& source)
{
    target.insert(target.end(), source.begin(), source.end());
}

WidgetDefinition makeFormWindowDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::FormWindow;
    definition.typeName = "FormWindow";
    definition.displayName = "Form Window";
    definition.paletteGroup = "Root";
    definition.defaultNamePrefix = "form";
    definition.defaultHint = "Main form window.";
    definition.size = { 900.0f, 600.0f, 300.0f, 200.0f };
    definition.properties = {
        { "title", "title", "MainWindow", PropertyEditKind::Text, true, "Window title text." },
        { "backgroundColor", "backgroundColor", "", PropertyEditKind::Color, true, "Form background color override. Empty means inherit from the look and feel." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties(false));
    definition.events = {
        { "onLoad", "onLoad", "void_event", "Called when the form loads." },
        { "onClose", "onClose", "void_event", "Called when the form closes." }
    };
    return definition;
}

WidgetDefinition makeStatusBarDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::StatusBar;
    definition.typeName = "StatusBar";
    definition.displayName = "Status Bar";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "statusBar";
    definition.defaultHint = "Displays status messages in one or more fields.";
    definition.size = { 600.0f, 50.0f, 200.0f, 44.0f };
    definition.properties = {
        { "fields", "fields", 1, PropertyEditKind::Integer, true, "Number of status fields (1-4)." },
        { "text0", "text0", "Ready", PropertyEditKind::Text, true, "Text for field 0." },
        { "text1", "text1", "", PropertyEditKind::Text, true, "Text for field 1." },
        { "text2", "text2", "", PropertyEditKind::Text, true, "Text for field 2." },
        { "fieldWidths", "fieldWidths", "1", PropertyEditKind::Text, true, "Relative field widths e.g. \"1,2,1\"." },
        { "dock", "dock", "Bottom", PropertyEditKind::Text, true, "Simple docking mode for the editor.", { "Bottom", "None" } },
        { "fillWidth", "fillWidth", true, PropertyEditKind::Bool, true, "Stretch the status bar to the root form width when docked." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeProgressBarDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::ProgressBar;
    definition.typeName = "ProgressBar";
    definition.displayName = "Progress Bar";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "progressBar";
    definition.defaultHint = "Displays task progress.";
    definition.size = { 240.0f, 32.0f, 100.0f, 24.0f };
    definition.properties = {
        { "min", "min", 0, PropertyEditKind::Integer, true, "Minimum value." },
        { "max", "max", 100, PropertyEditKind::Integer, true, "Maximum value." },
        { "value", "value", 25, PropertyEditKind::Integer, true, "Current value." },
        { "showText", "showText", true, PropertyEditKind::Bool, true, "Show text overlay." },
        { "text", "text", "", PropertyEditKind::Text, true, "Optional text to display." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeFrameDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Frame;
    definition.typeName = "Frame";
    definition.displayName = "Frame";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "frame";
    definition.defaultHint = "Groups related controls visually.";
    definition.size = { 300.0f, 180.0f, 180.0f, 120.0f };
    definition.properties = {
        { "title", "title", "Frame", PropertyEditKind::Text, true, "Frame caption text." },
        { "backgroundColor", "backgroundColor", "", PropertyEditKind::Color, true, "Frame background color override. Empty means inherit from the look and feel." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeLabelDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Label;
    definition.typeName = "Label";
    definition.displayName = "Label";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "label";
    definition.defaultHint = "Displays static text.";
    definition.size = { 260.0f, 64.0f, 140.0f, 58.0f };
    definition.properties = {
        { "text", "text", "Label", PropertyEditKind::Text, true, "Displayed label text." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeButtonDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Button;
    definition.typeName = "Button";
    definition.displayName = "Button";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "button";
    definition.defaultHint = "Runs an action when clicked.";
    definition.size = { 260.0f, 56.0f, 140.0f, 52.0f };
    definition.properties = {
        { "text", "text", "Button", PropertyEditKind::Text, true, "Button label text." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onClick", "onClick", "void_event", "Called when the button is clicked." }
    };
    return definition;
}

WidgetDefinition makeTextBoxDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::TextBox;
    definition.typeName = "TextBox";
    definition.displayName = "Text Box";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "textBox";
    definition.defaultHint = "Allows text entry.";
    definition.size = { 260.0f, 48.0f, 160.0f, 44.0f };
    definition.properties = {
        { "text", "text", "", PropertyEditKind::Text, true, "Text box contents." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onTextChanged", "onTextChanged", "string_event", "Called when the text changes." }
    };
    return definition;
}

WidgetDefinition makeCheckBoxDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::CheckBox;
    definition.typeName = "CheckBox";
    definition.displayName = "Check Box";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "checkBox";
    definition.defaultHint = "Toggles an option on or off.";
    definition.size = { 300.0f, 68.0f, 200.0f, 62.0f };
    definition.properties = {
        { "text", "text", "CheckBox", PropertyEditKind::Text, true, "Check box label text." },
        { "checked", "checked", false, PropertyEditKind::Bool, true, "Initial checked state." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onToggle", "onToggle", "bool_event", "Called when the check box toggles." }
    };
    return definition;
}

WidgetDefinition makeRadioButtonDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::RadioButton;
    definition.typeName = "RadioButton";
    definition.displayName = "Radio Button";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "radioButton";
    definition.defaultHint = "Selects one option from a group.";
    definition.size = { 280.0f, 52.0f, 180.0f, 48.0f };
    definition.properties = {
        { "text", "text", "Radio Button", PropertyEditKind::Text, true, "Radio button label text." },
        { "selected", "selected", false, PropertyEditKind::Bool, true, "Initial selected state." },
        { "group", "group", "default", PropertyEditKind::Text, true, "Logical radio group name." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onSelected", "onSelected", "bool_event", "Called when the radio button is selected." }
    };
    return definition;
}

WidgetDefinition makeSliderDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Slider;
    definition.typeName = "Slider";
    definition.displayName = "Slider";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "slider";
    definition.defaultHint = "Adjusts a numeric value.";
    definition.size = { 240.0f, 44.0f, 120.0f, 40.0f };
    definition.properties = {
        { "min", "min", 0, PropertyEditKind::Integer, true, "Minimum slider value." },
        { "max", "max", 100, PropertyEditKind::Integer, true, "Maximum slider value." },
        { "value", "value", 50, PropertyEditKind::Integer, true, "Current slider value." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    definition.events = {
        { "onChanged", "onChanged", "float_event", "Called when the slider value changes." }
    };
    return definition;
}

WidgetDefinition makeScrollBarDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::ScrollBar;
    definition.typeName = "ScrollBar";
    definition.displayName = "Scroll Bar";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "scrollBar";
    definition.defaultHint = "Scrolls through a range of values.";
    definition.size = { 240.0f, 36.0f, 100.0f, 28.0f };
    definition.properties = {
        { "orientation", "orientation", "Horizontal", PropertyEditKind::Text, true, "Scroll bar orientation: Horizontal or Vertical.", { "Horizontal", "Vertical" } },
        { "min", "min", 0, PropertyEditKind::Integer, true, "Minimum scroll value." },
        { "max", "max", 100, PropertyEditKind::Integer, true, "Maximum scroll value." },
        { "value", "value", 0, PropertyEditKind::Integer, true, "Current scroll value." },
        { "pageSize", "pageSize", 10, PropertyEditKind::Integer, true, "Visible page size for the thumb." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    definition.events = {
        { "onChanged", "onChanged", "float_event", "Called when the scroll value changes." }
    };
    return definition;
}

WidgetDefinition makeImageDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Image;
    definition.typeName = "Image";
    definition.displayName = "Image";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "image";
    definition.defaultHint = "Displays or reserves space for an image.";
    definition.size = { 200.0f, 140.0f, 100.0f, 80.0f };
    definition.properties = {
        { "resourceId", "Resource", "", PropertyEditKind::Text, true, "Selects a managed image resource from the project Resource Manager." },
        { "imagePath", "Image Path", "", PropertyEditKind::FilePath, true, "Optional direct image file path used when no managed resource is selected." },
        { "scaleMode", "Scale Mode", "Fit", PropertyEditKind::Text, true, "Controls how the image is fitted inside the widget bounds.", { "Stretch", "Fit", "Fill", "Center" } },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    return definition;
}

WidgetDefinition makeColorPickerDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::ColorPicker;
    definition.typeName = "ColorPicker";
    definition.displayName = "Color Picker";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "colorPicker";
    definition.defaultHint = "Selects a color value.";
    definition.size = { 220.0f, 40.0f, 140.0f, 34.0f };
    definition.properties = {
        { "value", "value", "#2D7DFF", PropertyEditKind::Color, true, "Selected color value." },
        { "text", "text", "Color", PropertyEditKind::Text, true, "Optional color picker label text." },
        { "showText", "showText", true, PropertyEditKind::Bool, true, "Show the label text beside the swatch." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onChanged", "onChanged", "string_event", "Called when the selected color changes." }
    };
    return definition;
}

WidgetDefinition makeModalDialogDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::ModalDialog;
    definition.typeName = "ModalDialog";
    definition.displayName = "Modal Dialog";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "modalDialog";
    definition.defaultHint = "Displays a modal dialog.";
    definition.size = { 420.0f, 240.0f, 260.0f, 160.0f };
    definition.properties = {
        { "title", "title", "Dialog", PropertyEditKind::Text, true, "Dialog title text." },
        { "message", "message", "Message text", PropertyEditKind::Text, true, "Dialog message text." },
        { "buttons", "buttons", "OK", PropertyEditKind::Text, true, "Comma-separated dialog buttons such as OK or OK,Cancel." },
        { "modal", "modal", true, PropertyEditKind::Bool, true, "Keep the dialog modal at runtime." },
        { "visibleAtStartup", "visibleAtStartup", false, PropertyEditKind::Bool, true, "Show this dialog when the generated window first opens." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onAccepted", "onAccepted", "void_event", "Called when an accept-style dialog button is clicked." },
        { "onCancelled", "onCancelled", "void_event", "Called when a cancel-style dialog button is clicked." }
    };
    return definition;
}

WidgetDefinition makeSpacerDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Spacer;
    definition.typeName = "Spacer";
    definition.displayName = "Spacer";
    definition.paletteGroup = "Basic";
    definition.defaultNamePrefix = "spacer";
    definition.defaultHint = "Adds spacing between widgets.";
    definition.size = { 180.0f, 50.0f, 40.0f, 30.0f };
    definition.properties = {
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonStyleProperties());
    return definition;
}

std::string defaultNameFromId(const WidgetDefinition& definition, const std::string& id)
{
    const auto underscore = id.find_last_of('_');
    const std::string suffix = underscore == std::string::npos ? std::string{} : id.substr(underscore + 1);
    return definition.defaultNamePrefix + suffix;
}

} // namespace

WidgetRegistry::WidgetRegistry()
    : definitions_{
        makeFormWindowDefinition(),
        makeFrameDefinition(),
        makeLabelDefinition(),
        makeButtonDefinition(),
        makeTextBoxDefinition(),
        makeCheckBoxDefinition(),
        makeRadioButtonDefinition(),
        makeSliderDefinition(),
        makeScrollBarDefinition(),
        makeStatusBarDefinition(),
        makeProgressBarDefinition(),
        makeModalDialogDefinition(),
        makeColorPickerDefinition(),
        makeImageDefinition(),
        makeSpacerDefinition() }
{
}

const WidgetRegistry& WidgetRegistry::instance()
{
    static const WidgetRegistry registry;
    return registry;
}

const WidgetDefinition* WidgetRegistry::find(WidgetType type) const
{
    const auto iterator = std::find_if(definitions_.begin(), definitions_.end(),
        [type](const WidgetDefinition& definition) { return definition.type == type; });
    return iterator == definitions_.end() ? nullptr : &*iterator;
}

const WidgetDefinition* WidgetRegistry::findByTypeName(const std::string& typeName) const
{
    const auto iterator = std::find_if(definitions_.begin(), definitions_.end(),
        [&typeName](const WidgetDefinition& definition) { return definition.typeName == typeName; });
    return iterator == definitions_.end() ? nullptr : &*iterator;
}

const std::vector<WidgetDefinition>& WidgetRegistry::definitions() const
{
    return definitions_;
}

WidgetNode WidgetRegistry::createDefaultWidget(WidgetType type, const std::string& id) const
{
    if (const WidgetDefinition* definition = find(type)) {
        WidgetNode widget{ id, defaultNameFromId(*definition, id), type,
            Rect{ 0.0f, 0.0f, definition->size.defaultWidth, definition->size.defaultHeight } };
        for (const auto& property : definition->properties) {
            widget.setProperty(property.key, property.defaultValue);
        }
        for (const auto& event : definition->events) {
            if (widget.getProperty(event.key) == nullptr) {
                widget.setProperty(event.key, "");
            }
        }
        return widget;
    }

    return WidgetNode{ id, id, type };
}

} // namespace visiform::model
