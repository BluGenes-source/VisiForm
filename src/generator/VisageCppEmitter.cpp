#include "generator/VisageCppEmitter.h"

#include "app/Version.h"
#include "model/BoxSizerLayout.h"
#include "model/LookAndFeelRegistry.h"
#include "model/WidgetItemUtils.h"
#include "model/WidgetRegistry.h"
#include "utils/CppIdentifier.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace visiform::generator {
namespace {

const std::string kGeneratedFileHeader = "// " + generatedByComment()
    + ".\n"
      "// This file is generated.\n"
      "// Manual changes may be overwritten.\n\n";

std::string widgetLabel(const visiform::model::WidgetNode& widget);

std::string sanitizeClassName(const std::string& value)
{
    std::string sanitized = utils::sanitizeCppIdentifier(value);
    if (!sanitized.empty() && std::islower(static_cast<unsigned char>(sanitized.front())) != 0) {
        sanitized.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(sanitized.front())));
    }

    return sanitized;
}

std::string generatedBaseClassName(const visiform::model::ProjectDocument& document)
{
    (void)document;
    return "MainWindow";
}

std::string userSubclassName(const visiform::model::ProjectDocument& document)
{
    const std::string fallback = document.userSubclassName.empty()
        ? (document.mainFormClassName.empty() ? "AppMainWindow" : document.mainFormClassName)
        : document.userSubclassName;
    const std::string sanitized = sanitizeClassName(fallback);
    return sanitized.empty() || sanitized == "MainWindow" ? "AppMainWindow" : sanitized;
}

std::string escapeCppStringLiteral(const std::string& value)
{
    std::ostringstream stream;
    for (char character : value) {
        switch (character) {
        case '\\': stream << "\\\\"; break;
        case '"': stream << "\\\""; break;
        case '\n': stream << "\\n"; break;
        case '\r': stream << "\\r"; break;
        case '\t': stream << "\\t"; break;
        default: stream << character; break;
        }
    }
    return stream.str();
}

std::vector<std::string> splitCommaSeparatedValues(const std::string& text)
{
    std::vector<std::string> values;
    std::istringstream stream(text);
    std::string item;
    while (std::getline(stream, item, ',')) {
        const auto first = std::find_if_not(item.begin(), item.end(), [](unsigned char character) {
            return std::isspace(character) != 0;
        });
        const auto last = std::find_if_not(item.rbegin(), item.rend(), [](unsigned char character) {
            return std::isspace(character) != 0;
        }).base();
        if (first < last) {
            values.emplace_back(first, last);
        }
    }

    if (values.empty()) {
        values.push_back("OK");
    }

    return values;
}

std::vector<std::string> tabLabels(const visiform::model::WidgetNode& widget)
{
    std::vector<std::string> labels;
    std::istringstream stream(widget.getStringProperty("tabs", "Tab 1,Tab 2"));
    std::string item;
    while (std::getline(stream, item, ',')) {
        const auto first = std::find_if_not(item.begin(), item.end(), [](unsigned char character) {
            return std::isspace(character) != 0;
        });
        const auto last = std::find_if_not(item.rbegin(), item.rend(), [](unsigned char character) {
            return std::isspace(character) != 0;
        }).base();
        if (first < last) {
            labels.emplace_back(first, last);
        }
    }

    if (labels.empty()) {
        labels.push_back("Tab 1");
    }

    return labels;
}

int selectedTabIndex(const visiform::model::WidgetNode& widget)
{
    const std::vector<std::string> labels = tabLabels(widget);
    return std::clamp(widget.getIntProperty("selectedTab", 0), 0, std::max(0, static_cast<int>(labels.size()) - 1));
}

std::string progressBarDisplayText(const visiform::model::WidgetNode& widget)
{
    if (!widget.getBoolProperty("showText", true)) {
        return {};
    }

    const std::string explicitText = widget.getStringProperty("text", {});
    if (!explicitText.empty()) {
        return explicitText;
    }

    const float minimum = widget.getFloatProperty("min", 0.0f);
    const float maximum = std::max(minimum, widget.getFloatProperty("max", 100.0f));
    const float safeMaximum = maximum <= minimum ? minimum + 1.0f : maximum;
    const float currentValue = std::clamp(widget.getFloatProperty("value", minimum), minimum, safeMaximum);
    const float normalized = safeMaximum <= minimum ? 0.0f : std::clamp((currentValue - minimum) / (safeMaximum - minimum), 0.0f, 1.0f);
    return std::to_string(static_cast<int>(std::round(normalized * 100.0f))) + "%";
}

std::string imageWidgetDisplaySource(const visiform::model::ProjectDocument& document, const visiform::model::WidgetNode& widget)
{
    const std::string resourceId = widget.getStringProperty("resourceId", {});
    if (!resourceId.empty()) {
        if (const auto* resource = document.findResourceById(resourceId)) {
            return resource->exportRelativePath.empty() ? resource->id : resource->exportRelativePath;
        }

        return "Missing resource: " + resourceId;
    }

    const std::string imagePath = widget.getStringProperty("imagePath", widget.getStringProperty("source", {}));
    if (!imagePath.empty()) {
        return imagePath;
    }

    return widgetLabel(widget);
}

std::string emitFloat(float value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value << 'f';
    return stream.str();
}

std::string emitStringLiteral(const std::string& value)
{
    return std::string{"\""} + escapeCppStringLiteral(value) + "\"";
}

struct RuntimeColorLiteral {
    unsigned int r = 0;
    unsigned int g = 0;
    unsigned int b = 0;
    unsigned int a = 0xff;
};

bool tryParseHexByte(std::string_view value, unsigned int& result)
{
    if (value.size() != 2) {
        return false;
    }

    unsigned int parsed = 0;
    for (char character : value) {
        parsed <<= 4;
        if (character >= '0' && character <= '9') {
            parsed |= static_cast<unsigned int>(character - '0');
        }
        else if (character >= 'a' && character <= 'f') {
            parsed |= static_cast<unsigned int>(character - 'a' + 10);
        }
        else if (character >= 'A' && character <= 'F') {
            parsed |= static_cast<unsigned int>(character - 'A' + 10);
        }
        else {
            return false;
        }
    }

    result = parsed;
    return true;
}

std::optional<RuntimeColorLiteral> parseRuntimeColorLiteral(std::string_view value)
{
    if (value.size() == 7 && value.front() == '#') {
        RuntimeColorLiteral color;
        if (!tryParseHexByte(value.substr(1, 2), color.r)
            || !tryParseHexByte(value.substr(3, 2), color.g)
            || !tryParseHexByte(value.substr(5, 2), color.b)) {
            return std::nullopt;
        }
        return color;
    }

    if (value.size() == 9 && value.front() == '#') {
        RuntimeColorLiteral color;
        if (!tryParseHexByte(value.substr(1, 2), color.a)
            || !tryParseHexByte(value.substr(3, 2), color.r)
            || !tryParseHexByte(value.substr(5, 2), color.g)
            || !tryParseHexByte(value.substr(7, 2), color.b)) {
            return std::nullopt;
        }
        return color;
    }

    return std::nullopt;
}

std::string emitHexByteLiteral(unsigned int value)
{
    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (value & 0xffu);
    return stream.str();
}

std::string emitRuntimeColorLiteral(const RuntimeColorLiteral& color)
{
    return "makeColor(" + emitHexByteLiteral(color.r)
        + ", " + emitHexByteLiteral(color.g)
        + ", " + emitHexByteLiteral(color.b)
        + ", " + emitHexByteLiteral(color.a) + ")";
}

std::string emitRuntimeColorLiteral(std::string_view value, const std::string& defaultColorLiteral)
{
    if (const auto parsed = parseRuntimeColorLiteral(value)) {
        return emitRuntimeColorLiteral(*parsed);
    }

    return defaultColorLiteral;
}

std::string emitColorExpression(std::string_view value, const std::string& defaultColorLiteral)
{
    if (const auto parsed = parseRuntimeColorLiteral(value)) {
        std::ostringstream stream;
        stream << "0x"
               << emitHexByteLiteral(parsed->a).substr(2)
               << emitHexByteLiteral(parsed->r).substr(2)
               << emitHexByteLiteral(parsed->g).substr(2)
               << emitHexByteLiteral(parsed->b).substr(2);
        return stream.str();
    }

    return defaultColorLiteral;
}

std::string runtimeOrientationLiteral(std::string_view value)
{
    return value == "Vertical" ? "RuntimeOrientation::Vertical" : "RuntimeOrientation::Horizontal";
}

struct ResolvedWidgetStyle {
    std::string panelColor;
    std::string fillColor;
    std::string recessedColor;
    std::string raisedColor;
    std::string textColor;
    std::string secondaryTextColor;
    std::string disabledTextColor;
    std::string borderColor;
    std::string focusColor;
    std::string accentColor;
    std::string disabledColor;
    std::string selectedColor;
    std::string hoverColor;
    std::string pressedColor;
    std::string checkedColor;
    std::string highlightColor;
    std::string shadowColor;
    float borderThickness = 1.0f;
    float cornerRadius = 0.0f;
    std::string fontFamily = "Default";
    float fontSize = 16.0f;
    int fontWeight = 400;
    bool italic = false;
    float controlPadding = 8.0f;
    float textPadding = 8.0f;
    std::string horizontalTextAlignment = "Default";
    std::string verticalTextAlignment = "Default";
    bool multiline = false;
    bool wordWrap = false;
    std::string overflowMode = "Clip";
};

ResolvedWidgetStyle resolveWidgetStyle(const visiform::model::ProjectDocument& document, const visiform::model::WidgetNode& widget)
{
    const visiform::model::ResolvedLookAndFeelStyle resolved =
        visiform::model::LookAndFeelRegistry::instance().resolve(document, widget);
    return {
        resolved.applicationSurfaceColor,
        resolved.controlSurfaceColor,
        resolved.recessedSurfaceColor,
        resolved.raisedSurfaceColor,
        resolved.primaryTextColor,
        resolved.secondaryTextColor,
        resolved.disabledTextColor,
        resolved.borderColor,
        resolved.focusOutlineColor,
        resolved.accentColor,
        resolved.disabledSurfaceColor,
        resolved.selectedStateColor,
        resolved.hoverStateColor,
        resolved.pressedStateColor,
        resolved.checkedStateColor,
        resolved.highlightEdgeColor,
        resolved.shadowEdgeColor,
        resolved.borderThickness,
        resolved.cornerRadius,
        resolved.fontFamily,
        resolved.fontSize,
        resolved.fontWeight,
        resolved.italic,
        resolved.controlPadding,
        resolved.textPadding,
        resolved.horizontalTextAlignment,
        resolved.verticalTextAlignment,
        resolved.multiline,
        resolved.wordWrap,
        resolved.overflowMode };
}

std::string widgetLabel(const visiform::model::WidgetNode& widget)
{
    if (!widget.name.empty()) {
        return widget.name;
    }
    if (!widget.id.empty()) {
        return widget.id;
    }

    return widget.typeName();
}

enum class HandlerSignature {
    Void,
    Bool,
    Float,
    String
};

struct EventBinding {
    std::string eventKey;
    std::string handlerName;
    std::string widgetId;
    std::string widgetName;
    std::string widgetType;
    HandlerSignature signature = HandlerSignature::Void;
};

struct HandlerInfo {
    std::string handlerName;
    HandlerSignature signature = HandlerSignature::Void;
    std::vector<EventBinding> bindings{};
};

using PreservedUserCodeBlocks = std::unordered_map<std::string, std::string>;

std::string trimNewlines(std::string text)
{
    while (!text.empty() && (text.front() == '\r' || text.front() == '\n')) {
        text.erase(text.begin());
    }
    while (!text.empty() && (text.back() == '\r' || text.back() == '\n')) {
        text.pop_back();
    }
    return text;
}

PreservedUserCodeBlocks extractPreservedUserCodeBlocks(const std::string& existingMainWindowCpp)
{
    PreservedUserCodeBlocks blocks;
    std::size_t searchStart = 0;
    while (true) {
        const std::size_t beginMarker = existingMainWindowCpp.find("// USER CODE BEGIN ", searchStart);
        if (beginMarker == std::string::npos) {
            break;
        }

        const std::size_t handlerNameStart = beginMarker + std::string("// USER CODE BEGIN ").size();
        const std::size_t handlerNameEnd = existingMainWindowCpp.find('\n', handlerNameStart);
        if (handlerNameEnd == std::string::npos) {
            break;
        }

        const std::string handlerName = trimNewlines(existingMainWindowCpp.substr(handlerNameStart, handlerNameEnd - handlerNameStart));
        const std::string endMarkerText = "// USER CODE END " + handlerName;
        const std::size_t endMarker = existingMainWindowCpp.find(endMarkerText, handlerNameEnd);
        if (endMarker == std::string::npos) {
            searchStart = handlerNameEnd;
            continue;
        }

        std::string preservedBody = existingMainWindowCpp.substr(handlerNameEnd + 1, endMarker - (handlerNameEnd + 1));
        blocks.insert_or_assign(handlerName, trimNewlines(std::move(preservedBody)));
        searchStart = endMarker + endMarkerText.size();
    }

    return blocks;
}

std::vector<std::string> relevantEventKeys(visiform::model::WidgetType type)
{
    switch (type) {
    case visiform::model::WidgetType::Button:
        return { "onClick", "onRelease", "onDoubleClick" };
    case visiform::model::WidgetType::ComboBox:
        return { "onChanged" };
    case visiform::model::WidgetType::ListBox:
        return { "onChanged", "onDoubleClick" };
    case visiform::model::WidgetType::CheckBox:
        return { "onToggle" };
    case visiform::model::WidgetType::RadioButton:
        return { "onSelected" };
    case visiform::model::WidgetType::Slider:
    case visiform::model::WidgetType::ScrollBar:
        return { "onChanged" };
    case visiform::model::WidgetType::ColorPicker:
        return { "onChanged" };
    case visiform::model::WidgetType::TextBox:
        return { "onTextChanged" };
    case visiform::model::WidgetType::FormWindow:
        return { "onLoad", "onClose" };
    case visiform::model::WidgetType::ModalDialog:
        return { "onAccepted", "onCancelled" };
    case visiform::model::WidgetType::Label:
    case visiform::model::WidgetType::Frame:
    case visiform::model::WidgetType::GroupBox:
    case visiform::model::WidgetType::Panel:
    case visiform::model::WidgetType::Sizer:
    case visiform::model::WidgetType::Image:
    case visiform::model::WidgetType::Spacer:
        return {};
    }

    return {};
}

const visiform::model::WidgetEventDefinition* findEventDefinition(visiform::model::WidgetType type, const std::string& eventKey)
{
    if (const auto* definition = visiform::model::WidgetRegistry::instance().find(type)) {
        for (const auto& event : definition->events) {
            if (event.key == eventKey) {
                return &event;
            }
        }
    }

    return nullptr;
}

HandlerSignature signatureForEventKind(const std::string& handlerSignatureKind)
{
    if (handlerSignatureKind == "bool_event") {
        return HandlerSignature::Bool;
    }
    if (handlerSignatureKind == "float_event") {
        return HandlerSignature::Float;
    }
    if (handlerSignatureKind == "string_event") {
        return HandlerSignature::String;
    }

    return HandlerSignature::Void;
}

std::string signatureDescription(HandlerSignature signature)
{
    switch (signature) {
    case HandlerSignature::Void:
        return "void(const WidgetEvent&)";
    case HandlerSignature::Bool:
        return "void(const WidgetEvent&, bool)";
    case HandlerSignature::Float:
        return "void(const WidgetEvent&, float)";
    case HandlerSignature::String:
        return "void(const WidgetEvent&, const std::string&)";
    }

    return "void(const WidgetEvent&)";
}

std::string handlerDeclaration(const HandlerInfo& handler)
{
    switch (handler.signature) {
    case HandlerSignature::Void:
        return "void " + handler.handlerName + "(const WidgetEvent& event);";
    case HandlerSignature::Bool:
        return "void " + handler.handlerName + "(const WidgetEvent& event, bool value);";
    case HandlerSignature::Float:
        return "void " + handler.handlerName + "(const WidgetEvent& event, float value);";
    case HandlerSignature::String:
        return "void " + handler.handlerName + "(const WidgetEvent& event, const std::string& value);";
    }

    return "void " + handler.handlerName + "(const WidgetEvent& event);";
}

std::string handlerDefinitionSignature(const std::string& className, const HandlerInfo& handler)
{
    switch (handler.signature) {
    case HandlerSignature::Void:
        return "void " + className + "::" + handler.handlerName + "(const WidgetEvent& event)";
    case HandlerSignature::Bool:
        return "void " + className + "::" + handler.handlerName + "(const WidgetEvent& event, bool value)";
    case HandlerSignature::Float:
        return "void " + className + "::" + handler.handlerName + "(const WidgetEvent& event, float value)";
    case HandlerSignature::String:
        return "void " + className + "::" + handler.handlerName + "(const WidgetEvent& event, const std::string& value)";
    }

    return "void " + className + "::" + handler.handlerName + "(const WidgetEvent& event)";
}

std::string handlerTodoLine(const EventBinding& binding)
{
    return "// TODO: Implement " + binding.eventKey + " handler for " + binding.widgetId + ".";
}

std::vector<std::string> handlerExampleLines(const HandlerInfo& handler)
{
    if (handler.bindings.empty()) {
        return {};
    }

    const EventBinding& binding = handler.bindings.front();
    if (binding.widgetType == "Button" && binding.eventKey == "onClick") {
        return {
            "// Example:",
            "// showMessageDialog(\"Hello\", \"Button clicked.\");",
            "// showModalDialog(\"modalDialog_1\");"
        };
    }
    if ((binding.widgetType == "Slider" || binding.widgetType == "ScrollBar") && binding.eventKey == "onChanged") {
        return {
            "// Example:",
            "// setProgressValue(\"progressBar_1\", value);"
        };
    }
    if (binding.widgetType == "TextBox" && binding.eventKey == "onTextChanged") {
        return {
            "// Example:",
            "// setStatusBarField(\"statusBar_1\", 0, value);"
        };
    }
    if ((binding.widgetType == "CheckBox" && binding.eventKey == "onToggle")
        || (binding.widgetType == "RadioButton" && binding.eventKey == "onSelected")) {
        return {
            "// Example:",
            "// setText(\"label_1\", value ? \"Enabled\" : \"Disabled\");"
        };
    }
    if (binding.widgetType == "ColorPicker" && binding.eventKey == "onChanged") {
        return {
            "// Example:",
            "// setStatusBarField(\"statusBar_1\", 0, value);"
        };
    }
    if ((binding.widgetType == "MenuBar" || binding.widgetType == "ToolBar") && binding.eventKey == "onItemAction") {
        return {
            "// Example:",
            "// setStatusBarField(\"statusBar_1\", 0, event.itemAction.empty() ? event.itemLabel.data() : std::string{ event.itemAction });"
        };
    }
    if (binding.widgetType == "FormWindow" && binding.eventKey == "onLoad") {
        return {
            "// Example:",
            "// setStatusBarField(\"statusBar_1\", 0, \"Ready\");"
        };
    }
    if (binding.widgetType == "ModalDialog" && binding.eventKey == "onAccepted") {
        return {
            "// Example:",
            "// setStatusBarField(\"statusBar\", 0, \"Dialog accepted\");"
        };
    }
    if (binding.widgetType == "ModalDialog" && binding.eventKey == "onCancelled") {
        return {
            "// Example:",
            "// setStatusBarField(\"statusBar\", 0, \"Dialog cancelled\");"
        };
    }

    return {};
}

std::string handlerReferenceList(const HandlerInfo& handler)
{
    std::ostringstream stream;
    for (std::size_t index = 0; index < handler.bindings.size(); ++index) {
        if (index > 0) {
            stream << ", ";
        }
        stream << handler.bindings[index].widgetId << "." << handler.bindings[index].eventKey;
    }
    return stream.str();
}

std::string emitMethodName(const EventBinding& binding)
{
    return "emit_" + utils::sanitizeCppIdentifier(binding.widgetId) + "_" + binding.eventKey;
}

std::string widgetEventLiteral(const EventBinding& binding)
{
    return "WidgetEvent{ "
        + emitStringLiteral(binding.widgetId)
        + ", " + emitStringLiteral(binding.widgetName)
        + ", " + emitStringLiteral(binding.widgetType)
        + " }";
}

std::string runtimeEventHandlerAccessor(std::string_view eventKey)
{
    if (eventKey == "onClick") {
        return "widget.events.onClick";
    }
    if (eventKey == "onRelease") {
        return "widget.events.onRelease";
    }
    if (eventKey == "onDoubleClick") {
        return "widget.events.onDoubleClick";
    }
    if (eventKey == "onToggle") {
        return "widget.events.onToggle";
    }
    if (eventKey == "onSelected") {
        return "widget.events.onSelected";
    }
    if (eventKey == "onChanged") {
        return "widget.events.onChanged";
    }
    if (eventKey == "onTextChanged") {
        return "widget.events.onTextChanged";
    }
    if (eventKey == "onAccepted") {
        return "widget.events.onAccepted";
    }
    if (eventKey == "onCancelled") {
        return "widget.events.onCancelled";
    }
    if (eventKey == "onSelectionChanged") {
        return "widget.events.onSelectionChanged";
    }
    if (eventKey == "onCellDoubleClick") {
        return "widget.events.onCellDoubleClick";
    }

    return {};
}

std::string emitMethodDeclaration(const EventBinding& binding)
{
    switch (binding.signature) {
    case HandlerSignature::Void:
        return "void " + emitMethodName(binding) + "();";
    case HandlerSignature::Bool:
        return "void " + emitMethodName(binding) + "(bool value);";
    case HandlerSignature::Float:
        return "void " + emitMethodName(binding) + "(float value);";
    case HandlerSignature::String:
        return "void " + emitMethodName(binding) + "(const std::string& value);";
    }

    return "void " + emitMethodName(binding) + "();";
}

std::string emitMethodDefinitionSignature(const std::string& className, const EventBinding& binding)
{
    switch (binding.signature) {
    case HandlerSignature::Void:
        return "void " + className + "::" + emitMethodName(binding) + "()";
    case HandlerSignature::Bool:
        return "void " + className + "::" + emitMethodName(binding) + "(bool value)";
    case HandlerSignature::Float:
        return "void " + className + "::" + emitMethodName(binding) + "(float value)";
    case HandlerSignature::String:
        return "void " + className + "::" + emitMethodName(binding) + "(const std::string& value)";
    }

    return "void " + className + "::" + emitMethodName(binding) + "()";
}

void collectEventBindings(const visiform::model::WidgetNode& widget, std::vector<EventBinding>& bindings, std::string& errorMessage)
{
    for (const auto& eventKey : relevantEventKeys(widget.type)) {
        const std::string handlerName = widget.getStringProperty(eventKey, {});
        if (handlerName.empty()) {
            continue;
        }

        if (!utils::isValidCppIdentifier(handlerName)) {
            errorMessage = "Invalid event handler name: " + handlerName;
            return;
        }

        const auto* eventDefinition = findEventDefinition(widget.type, eventKey);
        bindings.push_back(EventBinding{
            eventKey,
            handlerName,
            widget.id,
            widget.name.empty() ? widget.id : widget.name,
            widget.typeName(),
            signatureForEventKind(eventDefinition == nullptr ? std::string{} : eventDefinition->handlerSignatureKind)
        });
    }

    if (visiform::model::supportsItemActions(widget.type)) {
        for (const auto& handlerName : visiform::model::getWidgetItemActions(widget)) {
            if (handlerName.empty()) {
                continue;
            }

            if (!utils::isValidCppIdentifier(handlerName)) {
                errorMessage = "Invalid event handler name: " + handlerName;
                return;
            }

            bindings.push_back(EventBinding{
                "onItemAction",
                handlerName,
                widget.id,
                widget.name.empty() ? widget.id : widget.name,
                widget.typeName(),
                HandlerSignature::Void
            });
        }
    }

    for (const auto& child : widget.children) {
        collectEventBindings(child, bindings, errorMessage);
        if (!errorMessage.empty()) {
            return;
        }
    }
}

bool collectHandlers(const visiform::model::ProjectDocument& document, std::vector<HandlerInfo>& handlers, std::string& errorMessage)
{
    std::vector<EventBinding> bindings;
    collectEventBindings(document.root, bindings, errorMessage);
    if (!errorMessage.empty()) {
        return false;
    }

    std::map<std::string, std::size_t> handlerIndexByName;
    for (const auto& binding : bindings) {
        const auto [iterator, inserted] = handlerIndexByName.emplace(binding.handlerName, handlers.size());
        if (inserted) {
            handlers.push_back(HandlerInfo{ binding.handlerName, binding.signature, { binding } });
            continue;
        }

        auto& handler = handlers[iterator->second];
        if (handler.signature != binding.signature) {
            errorMessage = "Event handler name conflict: " + binding.handlerName + " has multiple signatures";
            return false;
        }

        handler.bindings.push_back(binding);
    }

    return true;
}

void emitEventComments(std::ostringstream& stream, const std::string& indent, const visiform::model::WidgetNode& widget)
{
    for (const auto& eventKey : relevantEventKeys(widget.type)) {
        const std::string handlerName = widget.getStringProperty(eventKey, {});
        if (handlerName.empty()) {
            continue;
        }

        stream << indent << "// Event: " << eventKey << " -> " << handlerName << "\n";
        stream << indent << "// Sender: " << widgetLabel(widget) << " (" << widget.id << ", " << widget.typeName() << ")\n";
        stream << indent << "// TODO: Connect this handler when generated interactive widgets are implemented.\n";
    }
}

void emitWidgetDraw(std::ostringstream& stream,
    const visiform::model::ProjectDocument& document,
    const visiform::model::WidgetNode& widget,
    float parentX,
    float parentY,
    int indentLevel)
{
    const std::string indent(indentLevel * 4, ' ');
    const std::string xExpr = emitFloat(parentX + widget.bounds.x);
    const std::string yExpr = emitFloat(parentY + widget.bounds.y);
    const std::string widthExpr = emitFloat(widget.bounds.width);
    const std::string heightExpr = emitFloat(widget.bounds.height);
    const ResolvedWidgetStyle style = resolveWidgetStyle(document, widget);
    const std::string panelRuntimeColor = emitRuntimeColorLiteral(style.panelColor, "makeColor(0x2B, 0x31, 0x3D)");
    const std::string fillRuntimeColor = emitRuntimeColorLiteral(style.fillColor, "makeColor(0x2B, 0x31, 0x3D)");
    const std::string formFillRuntimeColor = emitRuntimeColorLiteral(style.fillColor, "makeColor(0x1F, 0x24, 0x2D)");
    const std::string whiteFillRuntimeColor = emitRuntimeColorLiteral(style.fillColor, "makeColor(0xFF, 0xFF, 0xFF)");
    const std::string lightFillRuntimeColor = emitRuntimeColorLiteral(style.fillColor, "makeColor(0xEE, 0xF2, 0xF7)");
    const std::string disabledFillRuntimeColor = emitRuntimeColorLiteral(style.fillColor, "makeColor(0xD3, 0xDA, 0xE6)");
    const std::string scrollbarFillRuntimeColor = emitRuntimeColorLiteral(style.fillColor, "makeColor(0xD7, 0xDE, 0xE8)");
    const std::string borderRuntimeColor = emitRuntimeColorLiteral(style.borderColor, "makeColor(0x97, 0xA3, 0xB7)");
    const std::string subduedBorderRuntimeColor = emitRuntimeColorLiteral(style.borderColor, "makeColor(0x6C, 0x77, 0x88)");
    emitEventComments(stream, indent, widget);
    const std::string widgetHint = widget.getStringProperty("hint", {});
    if (!widgetHint.empty()) {
        stream << indent << "// Hint: " << escapeCppStringLiteral(widgetHint) << "\n";
    }

    switch (widget.type) {
    case visiform::model::WidgetType::FormWindow:
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << formFillRuntimeColor << ", " << subduedBorderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        stream << indent << "canvas.setColor(" << emitColorExpression(style.panelColor, "0xff2B313D") << ");\n";
        stream << indent << "canvas.fill(" << xExpr << ", " << yExpr << ", " << widthExpr << ", 28.0f);\n";
        stream << indent << "if (drawText) {\n";
        stream << indent << "    canvas.setColor(" << emitColorExpression(style.textColor, "0xffEEF2F8") << ");\n";
        stream << indent << "    canvas.text(" << emitStringLiteral(widget.getStringProperty("title", {}))
               << ", labelFont_, visage::Font::kTopLeft, " << xExpr << " + 10.0f, " << yExpr << " + 4.0f, " << widthExpr << " - 20.0f, 22.0f);\n";
        stream << indent << "}\n";
        break;
    case visiform::model::WidgetType::Frame:
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << fillRuntimeColor << ", " << borderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        stream << indent << "if (drawText) {\n";
        stream << indent << "    canvas.setColor(" << emitColorExpression(style.textColor, "0xffEEF2F8") << ");\n";
        stream << indent << "    canvas.text(" << emitStringLiteral(widget.getStringProperty("title", {}))
               << ", labelFont_, visage::Font::kTopLeft, " << xExpr << " + 8.0f, " << yExpr << " + 6.0f, " << widthExpr << " - 16.0f, 20.0f);\n";
        stream << indent << "}\n";
        break;
    case visiform::model::WidgetType::GroupBox:
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << " + 10.0f, " << widthExpr << ", std::max(0.0f, " << heightExpr << " - 10.0f), " << fillRuntimeColor << ", " << borderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        stream << indent << "if (drawText) {\n";
        stream << indent << "    canvas.setColor(" << emitColorExpression(style.fillColor, "0xff2B313D") << ");\n";
        stream << indent << "    canvas.fill(" << xExpr << " + 12.0f, " << yExpr << ", 96.0f, 20.0f);\n";
        stream << indent << "    canvas.setColor(" << emitColorExpression(style.textColor, "0xffEEF2F8") << ");\n";
        stream << indent << "    canvas.text(" << emitStringLiteral(widget.getStringProperty("title", {}))
               << ", labelFont_, visage::Font::kTopLeft, " << xExpr << " + 18.0f, " << yExpr << " + 1.0f, " << widthExpr << " - 28.0f, 18.0f);\n";
        stream << indent << "}\n";
        break;
    case visiform::model::WidgetType::Panel:
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << fillRuntimeColor << ", " << borderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        break;
    case visiform::model::WidgetType::Sizer:
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << fillRuntimeColor << ", blendColor(" << borderRuntimeColor << ", " << fillRuntimeColor << ", 0.25f), " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        break;
    case visiform::model::WidgetType::Label:
        stream << indent << "if (drawText) {\n";
        stream << indent << "    canvas.setColor(" << emitColorExpression(style.textColor, "0xffEEF2F8") << ");\n";
        stream << indent << "    canvas.text(" << emitStringLiteral(widget.getStringProperty("text", {}))
               << ", labelFont_, visage::Font::kTopLeft, " << xExpr << " + 4.0f, " << yExpr << " + 3.0f, " << widthExpr << " - 4.0f, " << heightExpr << " - 4.0f);\n";
        stream << indent << "}\n";
        break;
    case visiform::model::WidgetType::Button:
    {
        const bool pressedState = widget.getBoolProperty("toggleMode", false) && widget.getBoolProperty("checked", false);
        const std::string normalText = widget.getStringProperty("normalText", widget.getStringProperty("text", {}));
        const std::string pressedText = widget.getStringProperty("pressedText", normalText);
        const std::string normalFillColor = emitRuntimeColorLiteral(widget.getStringProperty("normalFillColor", {}), fillRuntimeColor);
        const std::string pressedFillColor = emitRuntimeColorLiteral(widget.getStringProperty("pressedFillColor", {}), emitRuntimeColorLiteral(style.accentColor, normalFillColor));
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << (pressedState ? pressedFillColor : normalFillColor) << ", " << borderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        stream << indent << "if (drawText) {\n";
        stream << indent << "    canvas.setColor(" << emitColorExpression(style.textColor, "0xffEEF2F8") << ");\n";
        stream << indent << "    canvas.text(" << emitStringLiteral(pressedState ? pressedText : normalText)
               << ", labelFont_, visage::Font::kCenter, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ");\n";
        stream << indent << "}\n";
        break;
    }
    case visiform::model::WidgetType::TextBox:
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << whiteFillRuntimeColor << ", " << borderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        stream << indent << "if (drawText) {\n";
        stream << indent << "    canvas.setColor(" << emitColorExpression(style.textColor, "0xffEEF2F8") << ");\n";
        stream << indent << "    canvas.text(" << emitStringLiteral(widget.getStringProperty("text", ""))
               << ", labelFont_, visage::Font::kTopLeft, " << xExpr << " + 8.0f, " << yExpr << " + 6.0f, " << widthExpr << " - 12.0f, " << heightExpr << " - 8.0f);\n";
        stream << indent << "}\n";
        break;
    case visiform::model::WidgetType::CheckBox:
        stream << indent << "canvas.setColor(" << emitColorExpression(style.fillColor, "0xffFFFFFF") << ");\n";
        stream << indent << "canvas.fill(" << xExpr << ", " << yExpr << " + (" << heightExpr << " - 18.0f) * 0.5f, 18.0f, 18.0f);\n";
        stream << indent << "drawBorder(canvas, " << xExpr << ", " << yExpr << " + (" << heightExpr << " - 18.0f) * 0.5f, 18.0f, 18.0f, " << emitColorExpression(style.borderColor, "0xff97A3B7") << ", " << emitFloat(style.borderThickness) << ");\n";
        if (widget.getBoolProperty("checked", false)) {
            stream << indent << "canvas.setColor(" << emitColorExpression(style.accentColor, "0xff2D7FF9") << ");\n";
            stream << indent << "canvas.fill(" << xExpr << " + 4.0f, " << yExpr << " + (" << heightExpr << " - 10.0f) * 0.5f, 10.0f, 10.0f);\n";
        }
        stream << indent << "if (drawText) {\n";
        stream << indent << "    canvas.setColor(" << emitColorExpression(style.textColor, "0xffEEF2F8") << ");\n";
        stream << indent << "    canvas.text(" << emitStringLiteral(widget.getStringProperty("text", {}))
               << ", labelFont_, visage::Font::kTopLeft, " << xExpr << " + 26.0f, " << yExpr << " + 4.0f, " << widthExpr << " - 30.0f, " << heightExpr << " - 6.0f);\n";
        stream << indent << "}\n";
        break;
    case visiform::model::WidgetType::RadioButton:
        stream << indent << "canvas.setColor(" << emitColorExpression(style.fillColor, "0xffFFFFFF") << ");\n";
        stream << indent << "canvas.fill(" << xExpr << ", " << yExpr << " + (" << heightExpr << " - 18.0f) * 0.5f, 18.0f, 18.0f);\n";
        stream << indent << "drawBorder(canvas, " << xExpr << ", " << yExpr << " + (" << heightExpr << " - 18.0f) * 0.5f, 18.0f, 18.0f, " << emitColorExpression(style.borderColor, "0xff97A3B7") << ", " << emitFloat(style.borderThickness) << ");\n";
        if (widget.getBoolProperty("selected", false)) {
            stream << indent << "canvas.setColor(" << emitColorExpression(style.accentColor, "0xff2D7FF9") << ");\n";
            stream << indent << "canvas.fill(" << xExpr << " + 5.0f, " << yExpr << " + (" << heightExpr << " - 8.0f) * 0.5f, 8.0f, 8.0f);\n";
        }
        stream << indent << "if (drawText) {\n";
        stream << indent << "    canvas.setColor(" << emitColorExpression(style.textColor, "0xffEEF2F8") << ");\n";
        stream << indent << "    canvas.text(" << emitStringLiteral(widget.getStringProperty("text", {}))
               << ", labelFont_, visage::Font::kTopLeft, " << xExpr << " + 26.0f, " << yExpr << " + 4.0f, " << widthExpr << " - 30.0f, " << heightExpr << " - 6.0f);\n";
        stream << indent << "}\n";
        break;
    case visiform::model::WidgetType::Slider: {
        const float minValue = widget.getFloatProperty("min", 0.0f);
        const float maxValue = widget.getFloatProperty("max", 100.0f);
        const float currentValue = widget.getFloatProperty("value", 50.0f);
        const float denominator = maxValue > minValue ? (maxValue - minValue) : 1.0f;
        const float normalized = std::clamp((currentValue - minValue) / denominator, 0.0f, 1.0f);
        stream << indent << "canvas.setColor(" << emitColorExpression(style.borderColor, "0xff97A3B7") << ");\n";
        stream << indent << "canvas.fill(" << xExpr << " + 8.0f, " << yExpr << " + " << heightExpr << " * 0.5f - 2.0f, " << widthExpr << " - 16.0f, 4.0f);\n";
        stream << indent << "canvas.setColor(" << emitColorExpression(style.accentColor, "0xff2D7FF9") << ");\n";
        stream << indent << "canvas.fill(" << xExpr << " + 8.0f + (" << widthExpr << " - 28.0f) * " << emitFloat(normalized)
               << ", " << yExpr << " + " << heightExpr << " * 0.5f - 8.0f, 12.0f, 16.0f);\n";
        break;
    }
    case visiform::model::WidgetType::ScrollBar: {
        const bool vertical = widget.getStringProperty("orientation", "Horizontal") == "Vertical";
        const float minValue = widget.getFloatProperty("min", 0.0f);
        const float maxValue = std::max(minValue + 1.0f, widget.getFloatProperty("max", 100.0f));
        const float currentValue = std::clamp(widget.getFloatProperty("value", 0.0f), minValue, maxValue);
        const float pageSize = std::max(1.0f, widget.getFloatProperty("pageSize", 10.0f));
        const float normalized = std::clamp((currentValue - minValue) / (maxValue - minValue), 0.0f, 1.0f);
        const float thumbFactor = std::clamp(pageSize / (maxValue - minValue + pageSize), 0.18f, 0.55f);
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << scrollbarFillRuntimeColor << ", " << borderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        if (vertical) {
            stream << indent << "const float arrowSize = std::min(" << widthExpr << ", 20.0f);\n";
            stream << indent << "const float trackTop = " << yExpr << " + arrowSize;\n";
            stream << indent << "const float trackHeight = std::max(0.0f, " << heightExpr << " - arrowSize * 2.0f);\n";
            stream << indent << "const float thumbHeight = std::clamp(trackHeight * " << emitFloat(thumbFactor) << ", 18.0f, std::max(18.0f, trackHeight));\n";
            stream << indent << "const float thumbY = trackTop + std::max(0.0f, trackHeight - thumbHeight) * " << emitFloat(normalized) << ";\n";
            stream << indent << "canvas.setColor(" << emitColorExpression(style.fillColor, "0xffEEF2F8") << ");\n";
            stream << indent << "canvas.fill(" << xExpr << ", " << yExpr << ", " << widthExpr << ", arrowSize);\n";
            stream << indent << "canvas.fill(" << xExpr << ", " << yExpr << " + " << heightExpr << " - arrowSize, " << widthExpr << ", arrowSize);\n";
            stream << indent << "drawBorder(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", arrowSize, " << emitColorExpression(style.borderColor, "0xff97A3B7") << ", " << emitFloat(style.borderThickness) << ");\n";
            stream << indent << "drawBorder(canvas, " << xExpr << ", " << yExpr << " + " << heightExpr << " - arrowSize, " << widthExpr << ", arrowSize, " << emitColorExpression(style.borderColor, "0xff97A3B7") << ", " << emitFloat(style.borderThickness) << ");\n";
            stream << indent << "canvas.setColor(" << emitColorExpression(style.panelColor, "0xff1F242D") << ");\n";
            stream << indent << "canvas.fill(" << xExpr << " + 2.0f, trackTop, " << widthExpr << " - 4.0f, trackHeight);\n";
            stream << indent << "canvas.setColor(" << emitColorExpression(style.accentColor, "0xff2D7FF9") << ");\n";
            stream << indent << "canvas.fill(" << xExpr << " + 4.0f, thumbY, " << widthExpr << " - 8.0f, thumbHeight);\n";
            stream << indent << "drawBorder(canvas, " << xExpr << " + 4.0f, thumbY, " << widthExpr << " - 8.0f, thumbHeight, " << emitColorExpression(style.borderColor, "0xff97A3B7") << ", " << emitFloat(style.borderThickness) << ");\n";
        }
        else {
            stream << indent << "const float arrowSize = std::min(" << heightExpr << ", 20.0f);\n";
            stream << indent << "const float trackLeft = " << xExpr << " + arrowSize;\n";
            stream << indent << "const float trackWidth = std::max(0.0f, " << widthExpr << " - arrowSize * 2.0f);\n";
            stream << indent << "const float thumbWidth = std::clamp(trackWidth * " << emitFloat(thumbFactor) << ", 18.0f, std::max(18.0f, trackWidth));\n";
            stream << indent << "const float thumbX = trackLeft + std::max(0.0f, trackWidth - thumbWidth) * " << emitFloat(normalized) << ";\n";
            stream << indent << "canvas.setColor(" << emitColorExpression(style.fillColor, "0xffEEF2F8") << ");\n";
            stream << indent << "canvas.fill(" << xExpr << ", " << yExpr << ", arrowSize, " << heightExpr << ");\n";
            stream << indent << "canvas.fill(" << xExpr << " + " << widthExpr << " - arrowSize, " << yExpr << ", arrowSize, " << heightExpr << ");\n";
            stream << indent << "drawBorder(canvas, " << xExpr << ", " << yExpr << ", arrowSize, " << heightExpr << ", " << emitColorExpression(style.borderColor, "0xff97A3B7") << ", " << emitFloat(style.borderThickness) << ");\n";
            stream << indent << "drawBorder(canvas, " << xExpr << " + " << widthExpr << " - arrowSize, " << yExpr << ", arrowSize, " << heightExpr << ", " << emitColorExpression(style.borderColor, "0xff97A3B7") << ", " << emitFloat(style.borderThickness) << ");\n";
            stream << indent << "canvas.setColor(" << emitColorExpression(style.panelColor, "0xff1F242D") << ");\n";
            stream << indent << "canvas.fill(trackLeft, " << yExpr << " + 2.0f, trackWidth, " << heightExpr << " - 4.0f);\n";
            stream << indent << "canvas.setColor(" << emitColorExpression(style.accentColor, "0xff2D7FF9") << ");\n";
            stream << indent << "canvas.fill(thumbX, " << yExpr << " + 4.0f, thumbWidth, " << heightExpr << " - 8.0f);\n";
            stream << indent << "drawBorder(canvas, thumbX, " << yExpr << " + 4.0f, thumbWidth, " << heightExpr << " - 8.0f, " << emitColorExpression(style.borderColor, "0xff97A3B7") << ", " << emitFloat(style.borderThickness) << ");\n";
        }
        break;
    }
    case visiform::model::WidgetType::MenuBar: {
        const auto items = visiform::model::splitItems(widget.getStringProperty("items", {}));
        const std::string selectedIndexKey = std::string(visiform::model::selectedItemIndexPropertyKey(widget.type));
        const int selectedIndex = visiform::model::sanitizeSelectedIndex(items,
            widget.getIntProperty(selectedIndexKey, items.empty() ? -1 : 0));
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << panelRuntimeColor << ", " << borderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        float itemOffset = 6.0f;
        for (std::size_t index = 0; index < items.size(); ++index) {
            const float itemWidth = std::max(56.0f, static_cast<float>(items[index].size()) * 11.2f + 36.0f);
            if (static_cast<int>(index) == selectedIndex) {
                stream << indent << "canvas.setColor(" << emitColorExpression(style.accentColor, "0xff2D7FF9") << ");\n";
                stream << indent << "canvas.fill(" << xExpr << " + " << emitFloat(itemOffset) << ", " << yExpr << " + 4.0f, " << emitFloat(itemWidth) << ", " << heightExpr << " - 8.0f);\n";
            }
            stream << indent << "if (drawText) {\n";
            stream << indent << "    canvas.setColor(" << emitColorExpression(style.textColor, "0xffEEF2F8") << ");\n";
            stream << indent << "    canvas.text(" << emitStringLiteral(items[index])
                   << ", labelFont_, visage::Font::kCenter, " << xExpr << " + " << emitFloat(itemOffset + 4.0f)
                   << ", " << yExpr << " + 4.0f, " << emitFloat(std::max(0.0f, itemWidth - 8.0f)) << ", " << heightExpr << " - 8.0f);\n";
            stream << indent << "}\n";
            itemOffset += itemWidth + 4.0f;
        }
        break;
    }
    case visiform::model::WidgetType::ToolBar: {
        const auto items = visiform::model::splitItems(widget.getStringProperty("items", {}));
        const std::string selectedIndexKey = std::string(visiform::model::selectedItemIndexPropertyKey(widget.type));
        const int selectedIndex = visiform::model::sanitizeSelectedIndex(items,
            widget.getIntProperty(selectedIndexKey, items.empty() ? -1 : 0));
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << panelRuntimeColor << ", " << borderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        float itemOffset = 6.0f;
        for (std::size_t index = 0; index < items.size(); ++index) {
            const float itemWidth = std::max(64.0f, static_cast<float>(items[index].size()) * 11.2f + 42.0f);
            stream << indent << "canvas.setColor(" << emitColorExpression(static_cast<int>(index) == selectedIndex ? style.accentColor : style.fillColor, "0xff2B313D") << ");\n";
            stream << indent << "canvas.fill(" << xExpr << " + " << emitFloat(itemOffset) << ", " << yExpr << " + 5.0f, " << emitFloat(itemWidth) << ", " << heightExpr << " - 10.0f);\n";
            stream << indent << "drawBorder(canvas, " << xExpr << " + " << emitFloat(itemOffset) << ", " << yExpr << " + 5.0f, " << emitFloat(itemWidth) << ", " << heightExpr << " - 10.0f, " << emitColorExpression(style.borderColor, "0xff97A3B7") << ", " << emitFloat(style.borderThickness) << ");\n";
            stream << indent << "if (drawText) {\n";
            stream << indent << "    canvas.setColor(" << emitColorExpression(style.textColor, "0xffEEF2F8") << ");\n";
            stream << indent << "    canvas.text(" << emitStringLiteral(items[index])
                   << ", labelFont_, visage::Font::kCenter, " << xExpr << " + " << emitFloat(itemOffset + 4.0f)
                   << ", " << yExpr << " + 5.0f, " << emitFloat(std::max(0.0f, itemWidth - 8.0f)) << ", " << heightExpr << " - 10.0f);\n";
            stream << indent << "}\n";
            itemOffset += itemWidth + 6.0f;
        }
        break;
    }
    case visiform::model::WidgetType::StatusBar: {
        const int fields = std::clamp(widget.getIntProperty("fields", 1), 1, 4);
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << fillRuntimeColor << ", " << subduedBorderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        stream << indent << "if (drawText) {\n";
        stream << indent << "    const float fieldWidth = " << widthExpr << " / " << fields << ".0f;\n";
        for (int index = 0; index < fields; ++index) {
            const std::string textKey = "text" + std::to_string(index);
            stream << indent << "    canvas.setColor(" << emitColorExpression(style.textColor, "0xffEEF2F8") << ");\n";
            stream << indent << "    canvas.text(" << emitStringLiteral(widget.getStringProperty(textKey, index == 0 ? "Ready" : ""))
                   << ", labelFont_, visage::Font::kTopLeft, " << xExpr << " + fieldWidth * " << index << ".0f + 6.0f, " << yExpr << " + 6.0f, fieldWidth - 12.0f, " << heightExpr << " - 12.0f);\n";
            if (index + 1 < fields) {
                stream << indent << "    canvas.setColor(" << emitColorExpression(style.borderColor, "0xff6C7788") << ");\n";
                stream << indent << "    canvas.fill(" << xExpr << " + fieldWidth * " << (index + 1) << ".0f - 1.0f, " << yExpr << " + 4.0f, 1.0f, " << heightExpr << " - 8.0f);\n";
            }
        }
        stream << indent << "}\n";
        break;
    }
    case visiform::model::WidgetType::ProgressBar: {
        const float minValue = widget.getFloatProperty("min", 0.0f);
        const float maxValue = std::max(minValue + 1.0f, widget.getFloatProperty("max", 100.0f));
        const float currentValue = std::clamp(widget.getFloatProperty("value", 25.0f), minValue, maxValue);
        const float normalized = std::clamp((currentValue - minValue) / (maxValue - minValue), 0.0f, 1.0f);
        const std::string displayText = progressBarDisplayText(widget);
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << lightFillRuntimeColor << ", " << borderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        stream << indent << "canvas.setColor(" << emitColorExpression(style.accentColor, "0xff2D7FF9") << ");\n";
        stream << indent << "canvas.fill(" << xExpr << ", " << yExpr << ", " << widthExpr << " * " << emitFloat(normalized) << ", " << heightExpr << ");\n";
        if (!displayText.empty()) {
            stream << indent << "canvas.setColor(" << (normalized >= 0.5f ? "0xffF8FBFF" : emitColorExpression(style.textColor, "0xff182333")) << ");\n";
            stream << indent << "canvas.text(" << emitStringLiteral(displayText)
                   << ", labelFont_, visage::Font::kCenter, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ");\n";
        }
        break;
    }
    case visiform::model::WidgetType::ColorPicker: {
        const std::string value = widget.getStringProperty("value", "#2D7DFF");
        const std::string configuredLabel = widget.getStringProperty("text", {});
        const std::string label = widget.getBoolProperty("showText", true) && !configuredLabel.empty()
            ? configuredLabel + "  " + value
            : value;
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << fillRuntimeColor << ", " << borderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        stream << indent << "canvas.setColor(" << emitColorExpression(value, emitColorExpression(style.accentColor, "0xff2D7FF9")) << ");\n";
        stream << indent << "canvas.fill(" << xExpr << " + 6.0f, " << yExpr << " + 6.0f, 22.0f, " << heightExpr << " - 12.0f);\n";
        stream << indent << "drawBorder(canvas, " << xExpr << " + 6.0f, " << yExpr << " + 6.0f, 22.0f, " << heightExpr << " - 12.0f, " << emitColorExpression(style.borderColor, "0xff97A3B7") << ", " << emitFloat(style.borderThickness) << ");\n";
        stream << indent << "if (drawText) {\n";
        stream << indent << "    canvas.setColor(" << emitColorExpression(style.textColor, "0xffEEF2F8") << ");\n";
        stream << indent << "    canvas.text(" << emitStringLiteral(label)
               << ", labelFont_, visage::Font::kTopLeft, " << xExpr << " + 36.0f, " << yExpr << " + 6.0f, " << widthExpr << " - 42.0f, " << heightExpr << " - 8.0f);\n";
        stream << indent << "}\n";
        break;
    }
    case visiform::model::WidgetType::Image:
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << disabledFillRuntimeColor << ", " << borderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        stream << indent << "// Image resource: " << escapeCppStringLiteral(imageWidgetDisplaySource(document, widget)) << "\n";
        break;
    case visiform::model::WidgetType::Spacer:
        stream << indent << "drawRoundedBox(canvas, " << xExpr << ", " << yExpr << ", " << widthExpr << ", " << heightExpr << ", " << fillRuntimeColor << ", " << borderRuntimeColor << ", " << emitFloat(style.borderThickness) << ", " << emitFloat(style.cornerRadius) << ");\n";
        break;
    }

    for (const auto& child : widget.children) {
        emitWidgetDraw(stream, document, child, parentX + widget.bounds.x, parentY + widget.bounds.y, indentLevel);
    }
}

struct RuntimeWidgetSpec {
    const visiform::model::WidgetNode* widget = nullptr;
    float x = 0.0f;
    float y = 0.0f;
    ResolvedWidgetStyle style{};
};

std::string runtimeWidgetTypeLiteral(visiform::model::WidgetType type)
{
    switch (type) {
    case visiform::model::WidgetType::Label:
        return "RuntimeWidgetType::Label";
    case visiform::model::WidgetType::Button:
        return "RuntimeWidgetType::Button";
    case visiform::model::WidgetType::TextBox:
        return "RuntimeWidgetType::TextBox";
    case visiform::model::WidgetType::ComboBox:
        return "RuntimeWidgetType::ComboBox";
    case visiform::model::WidgetType::ListBox:
        return "RuntimeWidgetType::ListBox";
    case visiform::model::WidgetType::TableGrid:
        return "RuntimeWidgetType::TableGrid";
    case visiform::model::WidgetType::CheckBox:
        return "RuntimeWidgetType::CheckBox";
    case visiform::model::WidgetType::RadioButton:
        return "RuntimeWidgetType::RadioButton";
    case visiform::model::WidgetType::Slider:
        return "RuntimeWidgetType::Slider";
    case visiform::model::WidgetType::ScrollBar:
        return "RuntimeWidgetType::ScrollBar";
    case visiform::model::WidgetType::StatusBar:
        return "RuntimeWidgetType::StatusBar";
    case visiform::model::WidgetType::ProgressBar:
        return "RuntimeWidgetType::ProgressBar";
    case visiform::model::WidgetType::ModalDialog:
        return "RuntimeWidgetType::ModalDialog";
    case visiform::model::WidgetType::ColorPicker:
        return "RuntimeWidgetType::ColorPicker";
    case visiform::model::WidgetType::Frame:
        return "RuntimeWidgetType::Frame";
    case visiform::model::WidgetType::GroupBox:
        return "RuntimeWidgetType::GroupBox";
    case visiform::model::WidgetType::Panel:
        return "RuntimeWidgetType::Panel";
    case visiform::model::WidgetType::Sizer:
        return "RuntimeWidgetType::Sizer";
    case visiform::model::WidgetType::TabControl:
        return "RuntimeWidgetType::TabControl";
    case visiform::model::WidgetType::MenuBar:
        return "RuntimeWidgetType::MenuBar";
    case visiform::model::WidgetType::ToolBar:
        return "RuntimeWidgetType::ToolBar";
    case visiform::model::WidgetType::Image:
        return "RuntimeWidgetType::Image";
    case visiform::model::WidgetType::Spacer:
        return "RuntimeWidgetType::Spacer";
    case visiform::model::WidgetType::FormWindow:
        break;
    }

    return "RuntimeWidgetType::Unknown";
}

std::string emitStringVectorLiteral(const std::vector<std::string>& values)
{
    std::ostringstream stream;
    stream << "{";
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index > 0) {
            stream << ", ";
        }
        stream << emitStringLiteral(values[index]);
    }
    stream << "}";
    return stream.str();
}

std::string emitStringMatrixLiteral(const std::vector<std::vector<std::string>>& values)
{
    std::ostringstream stream;
    stream << "{";
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index > 0) {
            stream << ", ";
        }
        stream << emitStringVectorLiteral(values[index]);
    }
    stream << "}";
    return stream.str();
}

void collectRuntimeWidgetSpecs(const visiform::model::ProjectDocument& document,
    const visiform::model::WidgetNode& widget,
    float parentX,
    float parentY,
    std::vector<RuntimeWidgetSpec>& specs)
{
    const float absoluteX = parentX + widget.bounds.x;
    const float absoluteY = parentY + widget.bounds.y;
    if (widget.type != visiform::model::WidgetType::FormWindow) {
        specs.push_back(RuntimeWidgetSpec{ &widget, absoluteX, absoluteY, resolveWidgetStyle(document, widget) });
    }

    for (const auto& child : widget.children) {
        collectRuntimeWidgetSpecs(document, child, absoluteX, absoluteY, specs);
    }
}

void emitRuntimeWidgetInitialization(std::ostringstream& stream, const RuntimeWidgetSpec& spec, int indentLevel)
{
    const auto& widget = *spec.widget;
    const std::string indent(indentLevel * 4, ' ');
    const std::string innerIndent = indent + "    ";
    stream << indent << "{\n";
    stream << innerIndent << "RuntimeWidget widget{};\n";
    stream << innerIndent << "widget.type = " << runtimeWidgetTypeLiteral(widget.type) << ";\n";
    stream << innerIndent << "widget.id = " << emitStringLiteral(widget.id) << ";\n";
    stream << innerIndent << "widget.name = " << emitStringLiteral(widget.name.empty() ? widget.id : widget.name) << ";\n";
    stream << innerIndent << "widget.parentId = " << emitStringLiteral(widget.parentId) << ";\n";
    stream << innerIndent << "widget.bounds = RuntimeRect{ " << emitFloat(spec.x) << ", " << emitFloat(spec.y) << ", "
           << emitFloat(widget.bounds.width) << ", " << emitFloat(widget.bounds.height) << " };\n";
    const auto sizerItemLayout = visiform::model::sizerItemLayoutFor(widget);
    const float preferredWidth = sizerItemLayout.preferredWidth >= 0
        ? static_cast<float>(sizerItemLayout.preferredWidth)
        : widget.bounds.width;
    const float preferredHeight = sizerItemLayout.preferredHeight >= 0
        ? static_cast<float>(sizerItemLayout.preferredHeight)
        : widget.bounds.height;
    stream << innerIndent << "widget.preferredWidth = " << emitFloat(preferredWidth) << ";\n";
    stream << innerIndent << "widget.preferredHeight = " << emitFloat(preferredHeight) << ";\n";
    stream << innerIndent << "widget.hint = " << emitStringLiteral(widget.getStringProperty("hint", {})) << ";\n";
    stream << innerIndent << "widget.dialogTitle = " << emitStringLiteral(widget.getStringProperty("title", {})) << ";\n";
    stream << innerIndent << "widget.text.value = " << emitStringLiteral(widget.getStringProperty("text", {})) << ";\n";
    stream << innerIndent << "widget.source = " << emitStringLiteral(widget.getStringProperty("source", "Image")) << ";\n";
    stream << innerIndent << "widget.colorValue = " << emitStringLiteral(widget.getStringProperty("value", "#2D7DFF")) << ";\n";
    stream << innerIndent << "widget.tabIndex = " << widget.getIntProperty("tabIndex", 0) << ";\n";
    stream << innerIndent << "widget.selectedTab = " << selectedTabIndex(widget) << ";\n";
    stream << innerIndent << "widget.text.showText = " << (widget.getBoolProperty("showText", true) ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.modal = " << (widget.getBoolProperty("modal", true) ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.visibleAtStartup = " << (widget.getBoolProperty("visibleAtStartup", false) ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.enabled = " << (widget.getBoolProperty("enabled", true) && !widget.getBoolProperty("disabled", false) ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.readOnly = " << (widget.getBoolProperty("readOnly", false) ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.toggle.checked = " << (widget.getBoolProperty("checked", false) ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.toggle.selected = " << (widget.getBoolProperty("selected", false) ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.toggle.group = " << emitStringLiteral(widget.getStringProperty("group", "default")) << ";\n";
    stream << innerIndent << "widget.range.min = " << emitFloat(widget.getFloatProperty("min", 0.0f)) << ";\n";
    stream << innerIndent << "widget.range.max = " << emitFloat(widget.getFloatProperty("max", 100.0f)) << ";\n";
    stream << innerIndent << "widget.range.value = " << emitFloat(widget.getFloatProperty("value", 0.0f)) << ";\n";
    stream << innerIndent << "widget.range.pageSize = " << emitFloat(widget.getFloatProperty("pageSize", 10.0f)) << ";\n";
    stream << innerIndent << "widget.range.orientation = " << runtimeOrientationLiteral(widget.getStringProperty("orientation", "Horizontal")) << ";\n";
    const auto boxSizerLayout = visiform::model::boxSizerLayoutFor(widget);
    stream << innerIndent << "widget.sizerPaddingLeft = " << boxSizerLayout.paddingLeft << ";\n";
    stream << innerIndent << "widget.sizerPaddingTop = " << boxSizerLayout.paddingTop << ";\n";
    stream << innerIndent << "widget.sizerPaddingRight = " << boxSizerLayout.paddingRight << ";\n";
    stream << innerIndent << "widget.sizerPaddingBottom = " << boxSizerLayout.paddingBottom << ";\n";
    stream << innerIndent << "widget.sizerGap = " << boxSizerLayout.gap << ";\n";
    stream << innerIndent << "widget.sizerItemProportion = " << sizerItemLayout.proportion << ";\n";
    stream << innerIndent << "widget.sizerItemExpand = " << (sizerItemLayout.expand ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.sizerItemAlignment = RuntimeSizerAlignment::" << visiform::model::toString(sizerItemLayout.alignment) << ";\n";
    stream << innerIndent << "widget.sizerItemBorder = " << sizerItemLayout.border << ";\n";
    stream << innerIndent << "widget.sizerItemBorderSides = " << static_cast<int>(static_cast<std::uint8_t>(sizerItemLayout.borderSides)) << ";\n";
    stream << innerIndent << "widget.sizerItemMinimumWidth = " << sizerItemLayout.minimumWidth << ";\n";
    stream << innerIndent << "widget.sizerItemMinimumHeight = " << sizerItemLayout.minimumHeight << ";\n";
    stream << innerIndent << "widget.sizerItemShown = " << (sizerItemLayout.shown ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.spacerStretch = " << (visiform::model::parseSpacerKind(widget) == visiform::model::SpacerKind::Stretch ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.spacerSize = " << widget.getIntProperty(std::string{ visiform::model::sizer_properties::kSpacerSize }, 24) << ";\n";
    stream << innerIndent << "widget.events.onClick = " << emitStringLiteral(widget.getStringProperty("onClick", {})) << ";\n";
    stream << innerIndent << "widget.events.onRelease = " << emitStringLiteral(widget.getStringProperty("onRelease", {})) << ";\n";
    stream << innerIndent << "widget.events.onDoubleClick = " << emitStringLiteral(widget.getStringProperty("onDoubleClick", {})) << ";\n";
    stream << innerIndent << "widget.events.onToggle = " << emitStringLiteral(widget.getStringProperty("onToggle", {})) << ";\n";
    stream << innerIndent << "widget.events.onSelected = " << emitStringLiteral(widget.getStringProperty("onSelected", {})) << ";\n";
    stream << innerIndent << "widget.events.onChanged = " << emitStringLiteral(widget.getStringProperty("onChanged", {})) << ";\n";
    stream << innerIndent << "widget.events.onTextChanged = " << emitStringLiteral(widget.getStringProperty("onTextChanged", {})) << ";\n";
    stream << innerIndent << "widget.events.onAccepted = " << emitStringLiteral(widget.getStringProperty("onAccepted", {})) << ";\n";
    stream << innerIndent << "widget.events.onCancelled = " << emitStringLiteral(widget.getStringProperty("onCancelled", {})) << ";\n";
    stream << innerIndent << "widget.events.onSelectionChanged = " << emitStringLiteral(widget.getStringProperty("onSelectionChanged", {})) << ";\n";
    stream << innerIndent << "widget.events.onCellDoubleClick = " << emitStringLiteral(widget.getStringProperty("onCellDoubleClick", {})) << ";\n";
    stream << innerIndent << "widget.style.panelColor = " << emitRuntimeColorLiteral(spec.style.panelColor, "makeColor(0x1F, 0x24, 0x2D)") << ";\n";
    stream << innerIndent << "widget.style.fillColor = " << emitRuntimeColorLiteral(spec.style.fillColor, "makeColor(0x2B, 0x31, 0x3D)") << ";\n";
    stream << innerIndent << "widget.style.recessedColor = " << emitRuntimeColorLiteral(spec.style.recessedColor, "makeColor(0x20, 0x26, 0x30)") << ";\n";
    stream << innerIndent << "widget.style.raisedColor = " << emitRuntimeColorLiteral(spec.style.raisedColor, "makeColor(0x30, 0x37, 0x44)") << ";\n";
    stream << innerIndent << "widget.style.textColor = " << emitRuntimeColorLiteral(spec.style.textColor, "makeColor(0xEE, 0xF2, 0xF8)") << ";\n";
    stream << innerIndent << "widget.style.secondaryTextColor = " << emitRuntimeColorLiteral(spec.style.secondaryTextColor, "makeColor(0xAE, 0xB8, 0xC8)") << ";\n";
    stream << innerIndent << "widget.style.disabledTextColor = " << emitRuntimeColorLiteral(spec.style.disabledTextColor, "makeColor(0x6C, 0x77, 0x88)") << ";\n";
    stream << innerIndent << "widget.style.borderColor = " << emitRuntimeColorLiteral(spec.style.borderColor, "makeColor(0x97, 0xA3, 0xB7)") << ";\n";
    stream << innerIndent << "widget.style.focusColor = " << emitRuntimeColorLiteral(spec.style.focusColor, "makeColor(0x2D, 0x7F, 0xF9)") << ";\n";
    stream << innerIndent << "widget.style.accentColor = " << emitRuntimeColorLiteral(spec.style.accentColor, "makeColor(0x2D, 0x7F, 0xF9)") << ";\n";
    stream << innerIndent << "widget.style.disabledColor = " << emitRuntimeColorLiteral(spec.style.disabledColor, "makeColor(0x6C, 0x77, 0x88)") << ";\n";
    stream << innerIndent << "widget.style.selectedColor = " << emitRuntimeColorLiteral(spec.style.selectedColor, "makeColor(0x35, 0x53, 0x82)") << ";\n";
    stream << innerIndent << "widget.style.hoverColor = " << emitRuntimeColorLiteral(spec.style.hoverColor, "makeColor(0x35, 0x40, 0x52)") << ";\n";
    stream << innerIndent << "widget.style.pressedColor = " << emitRuntimeColorLiteral(spec.style.pressedColor, "makeColor(0x23, 0x2A, 0x35)") << ";\n";
    stream << innerIndent << "widget.style.checkedColor = " << emitRuntimeColorLiteral(spec.style.checkedColor, "makeColor(0x35, 0x53, 0x82)") << ";\n";
    stream << innerIndent << "widget.style.highlightColor = " << emitRuntimeColorLiteral(spec.style.highlightColor, "makeColor(0xC8, 0xD2, 0xE2)") << ";\n";
    stream << innerIndent << "widget.style.shadowColor = " << emitRuntimeColorLiteral(spec.style.shadowColor, "makeColor(0x11, 0x15, 0x1C)") << ";\n";
    stream << innerIndent << "widget.style.borderThickness = " << emitFloat(spec.style.borderThickness) << ";\n";
    stream << innerIndent << "widget.style.cornerRadius = " << emitFloat(spec.style.cornerRadius) << ";\n";
    stream << innerIndent << "widget.style.fontFamily = \"" << escapeCppStringLiteral(spec.style.fontFamily) << "\";\n";
    stream << innerIndent << "widget.style.fontSize = " << emitFloat(spec.style.fontSize) << ";\n";
    stream << innerIndent << "widget.style.fontWeight = " << spec.style.fontWeight << ";\n";
    stream << innerIndent << "widget.style.italic = " << (spec.style.italic ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.style.controlPadding = " << emitFloat(spec.style.controlPadding) << ";\n";
    stream << innerIndent << "widget.style.textPadding = " << emitFloat(spec.style.textPadding) << ";\n";
    stream << innerIndent << "widget.style.horizontalTextAlignment = \"" << escapeCppStringLiteral(spec.style.horizontalTextAlignment) << "\";\n";
    stream << innerIndent << "widget.style.verticalTextAlignment = \"" << escapeCppStringLiteral(spec.style.verticalTextAlignment) << "\";\n";
    stream << innerIndent << "widget.style.multiline = " << (spec.style.multiline ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.style.wordWrap = " << (spec.style.wordWrap ? "true" : "false") << ";\n";
    stream << innerIndent << "widget.style.overflowMode = \"" << escapeCppStringLiteral(spec.style.overflowMode) << "\";\n";
    const auto emitStateOverrides = [&](visiform::model::WidgetAppearanceState state, const char* member) {
        const auto found = widget.stateAppearanceOverrides.find(state);
        if (found == widget.stateAppearanceOverrides.end() || found->second.empty()) {
            return;
        }
        const auto& overrides = found->second;
        const auto emitColor = [&](const char* field, const std::optional<std::string>& value) {
            if (!value.has_value() || !parseRuntimeColorLiteral(*value).has_value()) return;
            stream << innerIndent << "widget." << member << ".has" << field << " = true;\n";
            stream << innerIndent << "widget." << member << "." << static_cast<char>(std::tolower(field[0]))
                   << std::string{ field + 1 } << " = "
                   << emitRuntimeColorLiteral(*value, "makeColor(0x00, 0x00, 0x00)") << ";\n";
        };
        emitColor("ControlSurfaceColor", overrides.controlSurfaceColor);
        emitColor("TextColor", overrides.textColor);
        emitColor("BorderColor", overrides.borderColor);
        emitColor("AccentColor", overrides.accentColor);
        emitColor("FocusColor", overrides.focusOutlineColor);
        emitColor("HighlightColor", overrides.highlightEdgeColor);
        emitColor("ShadowColor", overrides.shadowEdgeColor);
    };
    emitStateOverrides(visiform::model::WidgetAppearanceState::Hover, "hoverAppearance");
    emitStateOverrides(visiform::model::WidgetAppearanceState::Pressed, "pressedAppearance");
    emitStateOverrides(visiform::model::WidgetAppearanceState::Focused, "focusedAppearance");
    emitStateOverrides(visiform::model::WidgetAppearanceState::CheckedOrSelected, "checkedOrSelectedAppearance");
    emitStateOverrides(visiform::model::WidgetAppearanceState::Disabled, "disabledAppearance");

    if (widget.type == visiform::model::WidgetType::Frame) {
        stream << innerIndent << "widget.text.value = " << emitStringLiteral(widget.getStringProperty("title", {})) << ";\n";
    }
    else if (widget.type == visiform::model::WidgetType::GroupBox) {
        stream << innerIndent << "widget.text.value = " << emitStringLiteral(widget.getStringProperty("title", {})) << ";\n";
    }
    else if (widget.type == visiform::model::WidgetType::TabControl) {
        stream << innerIndent << "widget.items = " << emitStringVectorLiteral(tabLabels(widget)) << ";\n";
    }
    else if (widget.type == visiform::model::WidgetType::Button) {
        const std::string text = widget.getStringProperty("text", {});
        const std::string configuredNormalText = widget.getStringProperty("normalText", {});
        const std::string configuredPressedText = widget.getStringProperty("pressedText", {});
        const std::string normalText = !configuredNormalText.empty()
            ? configuredNormalText
            : text;
        const std::string pressedText = !configuredPressedText.empty() ? configuredPressedText : normalText;
        stream << innerIndent << "widget.text.value = " << emitStringLiteral(text) << ";\n";
        stream << innerIndent << "widget.button.toggleMode = " << (widget.getBoolProperty("toggleMode", false) ? "true" : "false") << ";\n";
        stream << innerIndent << "widget.button.normalText = " << emitStringLiteral(normalText) << ";\n";
        stream << innerIndent << "widget.button.pressedText = " << emitStringLiteral(pressedText) << ";\n";
        stream << innerIndent << "widget.style.fillColor = " << emitRuntimeColorLiteral(widget.getStringProperty("normalFillColor", {}), emitRuntimeColorLiteral(spec.style.fillColor, "makeColor(0x2B, 0x31, 0x3D)")) << ";\n";
        stream << innerIndent << "widget.button.pressedFillColor = " << emitRuntimeColorLiteral(widget.getStringProperty("pressedFillColor", {}), "blendColor(widget.style.fillColor, widget.style.accentColor, 0.18f)") << ";\n";
    }
    else if (widget.type == visiform::model::WidgetType::TextBox) {
        stream << innerIndent << "widget.text.value = " << emitStringLiteral(widget.getStringProperty("text", {})) << ";\n";
    }
    else if (widget.type == visiform::model::WidgetType::ComboBox
        || widget.type == visiform::model::WidgetType::ListBox
        || widget.type == visiform::model::WidgetType::MenuBar
        || widget.type == visiform::model::WidgetType::ToolBar) {
        const auto items = visiform::model::splitItems(widget.getStringProperty("items", {}));
        const std::string selectedIndexKey = std::string(visiform::model::selectedItemIndexPropertyKey(widget.type));
        const int selectedIndex = visiform::model::sanitizeSelectedIndex(items,
            widget.getIntProperty(selectedIndexKey, items.empty() ? -1 : 0));
        stream << innerIndent << "widget.items = " << emitStringVectorLiteral(items) << ";\n";
        stream << innerIndent << "widget.selectedIndex = " << selectedIndex << ";\n";
        if (widget.type == visiform::model::WidgetType::ListBox) {
            stream << innerIndent << "widget.multiSelect = " << (widget.getBoolProperty("multiSelect", false) ? "true" : "false") << ";\n";
        }
        if (widget.type == visiform::model::WidgetType::ComboBox) {
            stream << innerIndent << "widget.text.value = " << emitStringLiteral(visiform::model::getSelectedItemText(items, selectedIndex)) << ";\n";
        }
    }
    else if (widget.type == visiform::model::WidgetType::TableGrid) {
        const auto data = visiform::model::normalizeTableData(
            widget.getStringProperty("columns", {}),
            widget.getStringProperty("rows", {}));
        const auto selection = visiform::model::clampSelectedCell(
            data.columns,
            data.rows,
            widget.getIntProperty("selectedRow", data.rows.empty() ? -1 : 0),
            widget.getIntProperty("selectedColumn", data.columns.empty() ? -1 : 0));
        stream << innerIndent << "widget.tableColumns = " << emitStringVectorLiteral(data.columns) << ";\n";
        stream << innerIndent << "widget.tableRows = " << emitStringMatrixLiteral(data.rows) << ";\n";
        stream << innerIndent << "widget.selectedRow = " << selection.row << ";\n";
        stream << innerIndent << "widget.selectedColumn = " << selection.column << ";\n";
        stream << innerIndent << "widget.showHeader = " << (widget.getBoolProperty("showHeader", true) ? "true" : "false") << ";\n";
        stream << innerIndent << "widget.showGridLines = " << (widget.getBoolProperty("showGridLines", true) ? "true" : "false") << ";\n";
        stream << innerIndent << "widget.rowHeight = " << emitFloat(std::max(1.0f, widget.getFloatProperty("rowHeight", 28.0f))) << ";\n";
        stream << innerIndent << "widget.headerHeight = " << emitFloat(std::max(0.0f, widget.getFloatProperty("headerHeight", 30.0f))) << ";\n";
    }
    else if (widget.type == visiform::model::WidgetType::ProgressBar) {
        stream << innerIndent << "widget.text.value = " << emitStringLiteral(widget.getStringProperty("text", {})) << ";\n";
    }
    else if (widget.type == visiform::model::WidgetType::ModalDialog) {
        stream << innerIndent << "widget.text.value = " << emitStringLiteral(widget.getStringProperty("message", "Message text")) << ";\n";
        stream << innerIndent << "widget.items = " << emitStringVectorLiteral(splitCommaSeparatedValues(widget.getStringProperty("buttons", "OK"))) << ";\n";
    }
    else if (widget.type == visiform::model::WidgetType::ColorPicker) {
        stream << innerIndent << "widget.text.value = " << emitStringLiteral(widget.getStringProperty("text", {})) << ";\n";
    }
    else if (widget.type == visiform::model::WidgetType::Image) {
        stream << innerIndent << "widget.text.value = " << emitStringLiteral(widget.getStringProperty("source", "Image")) << ";\n";
    }
    if (widget.type == visiform::model::WidgetType::StatusBar) {
        const int fields = std::clamp(widget.getIntProperty("fields", 1), 1, 4);
        std::vector<std::string> fieldTexts;
        fieldTexts.reserve(static_cast<std::size_t>(fields));
        for (int index = 0; index < fields; ++index) {
            const std::string key = "text" + std::to_string(index);
            fieldTexts.push_back(widget.getStringProperty(key, {}));
        }
        stream << innerIndent << "widget.items = " << emitStringVectorLiteral(fieldTexts) << ";\n";
    }

    stream << innerIndent << "runtimeWidgets_.push_back(std::move(widget));\n";
    stream << indent << "}\n";
}

void emitRuntimeEventDispatcher(std::ostringstream& stream,
    const std::string& signature,
    HandlerSignature handlerSignature,
    const std::vector<EventBinding>& bindings)
{
    stream << signature << "\n";
    stream << "{\n";
    stream << "    const WidgetEvent event = makeWidgetEvent(widget);\n";

    bool emittedMatch = false;
    std::vector<std::string> emittedDispatchKeys;
    for (const auto& binding : bindings) {
        if (binding.signature != handlerSignature) {
            continue;
        }

        const std::string handlerAccessor = runtimeEventHandlerAccessor(binding.eventKey);
        if (handlerAccessor.empty()) {
            continue;
        }

        const std::string dispatchKey = binding.eventKey + "|" + binding.handlerName;
        if (std::find(emittedDispatchKeys.begin(), emittedDispatchKeys.end(), dispatchKey) != emittedDispatchKeys.end()) {
            continue;
        }
        emittedDispatchKeys.push_back(dispatchKey);

        emittedMatch = true;
        stream << "    if (eventKey == " << emitStringLiteral(binding.eventKey) << " && " << handlerAccessor << " == " << emitStringLiteral(binding.handlerName) << ") {\n";
        if (handlerSignature == HandlerSignature::Void) {
            stream << "        " << binding.handlerName << "(event);\n";
        }
        else {
            stream << "        " << binding.handlerName << "(event, value);\n";
        }
        stream << "        return;\n";
        stream << "    }\n";
    }

    if (!emittedMatch) {
        stream << "    (void)widget;\n";
        stream << "    (void)eventKey;\n";
        if (handlerSignature != HandlerSignature::Void) {
            stream << "    (void)value;\n";
        }
        stream << "    return;\n";
    }
    else {
        stream << "    (void)widget;\n";
        stream << "    (void)eventKey;\n";
        if (handlerSignature != HandlerSignature::Void) {
            stream << "    (void)value;\n";
        }
    }

    stream << "}\n";
}

std::string emitGeneratedBaseHeader(const visiform::model::ProjectDocument& document)
{
    const std::string className = generatedBaseClassName(document);
    std::vector<HandlerInfo> handlers;
    std::vector<EventBinding> bindings;
    std::string ignoredError;
    collectHandlers(document, handlers, ignoredError);
    collectEventBindings(document.root, bindings, ignoredError);
    std::ostringstream stream;
    stream << kGeneratedFileHeader;
    stream << "#pragma once\n\n";
    stream << "#include <cstdint>\n";
    stream << "#include <optional>\n";
    stream << "#include <string>\n";
    stream << "#include <string_view>\n";
    stream << "#include <vector>\n";
    stream << "#include <visage/app.h>\n";
    stream << "#include <visage/graphics.h>\n\n";
    stream << "struct WidgetEvent {\n";
    stream << "    std::string_view senderId{};\n";
    stream << "    std::string_view senderName{};\n";
    stream << "    std::string_view senderType{};\n";
    stream << "    int itemIndex = -1;\n";
    stream << "    std::string_view itemLabel{};\n";
    stream << "    std::string_view itemAction{};\n";
    stream << "};\n\n";
    stream << "struct RuntimeColor {\n";
    stream << "    std::uint8_t r = 0;\n";
    stream << "    std::uint8_t g = 0;\n";
    stream << "    std::uint8_t b = 0;\n";
    stream << "    std::uint8_t a = 255;\n";
    stream << "};\n\n";
    stream << "constexpr RuntimeColor makeColor(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255)\n";
    stream << "{\n";
    stream << "    return RuntimeColor{ r, g, b, a };\n";
    stream << "}\n\n";
    stream << "enum class RuntimeWidgetType : std::uint8_t {\n";
    stream << "    Unknown,\n";
    stream << "    Button,\n";
    stream << "    TextBox,\n";
    stream << "    ComboBox,\n";
    stream << "    ListBox,\n";
    stream << "    TableGrid,\n";
    stream << "    CheckBox,\n";
    stream << "    RadioButton,\n";
    stream << "    Slider,\n";
    stream << "    ScrollBar,\n";
    stream << "    ProgressBar,\n";
    stream << "    StatusBar,\n";
    stream << "    ModalDialog,\n";
    stream << "    Label,\n";
    stream << "    Frame,\n";
    stream << "    GroupBox,\n";
    stream << "    Panel,\n";
    stream << "    Sizer,\n";
    stream << "    TabControl,\n";
    stream << "    MenuBar,\n";
    stream << "    ToolBar,\n";
    stream << "    Image,\n";
    stream << "    Spacer,\n";
    stream << "    ColorPicker\n";
    stream << "};\n\n";
    stream << "enum class RuntimeOrientation : std::uint8_t {\n";
    stream << "    Horizontal,\n";
    stream << "    Vertical\n";
    stream << "};\n\n";
    stream << "enum class RuntimeSizerAlignment : std::uint8_t {\n";
    stream << "    Start,\n";
    stream << "    Center,\n";
    stream << "    End\n";
    stream << "};\n\n";
    stream << "struct RuntimeRect {\n";
    stream << "    float x = 0.0f;\n";
    stream << "    float y = 0.0f;\n";
    stream << "    float width = 0.0f;\n";
    stream << "    float height = 0.0f;\n\n";
    stream << "    [[nodiscard]] bool contains(float px, float py) const\n";
    stream << "    {\n";
    stream << "        return px >= x && py >= y && px <= x + width && py <= y + height;\n";
    stream << "    }\n";
    stream << "};\n\n";
    stream << "struct RuntimeTextState {\n";
    stream << "    std::string value;\n";
    stream << "    bool showText = true;\n";
    stream << "};\n\n";
    stream << "struct RuntimeButtonState {\n";
    stream << "    bool toggleMode = false;\n";
    stream << "    std::string normalText;\n";
    stream << "    std::string pressedText;\n";
    stream << "    RuntimeColor pressedFillColor = makeColor(0x35, 0x53, 0x82);\n";
    stream << "};\n\n";
    stream << "struct RuntimeToggleState {\n";
    stream << "    bool checked = false;\n";
    stream << "    bool selected = false;\n";
    stream << "    std::string group;\n";
    stream << "};\n\n";
    stream << "struct RuntimeRangeState {\n";
    stream << "    float min = 0.0f;\n";
    stream << "    float max = 100.0f;\n";
    stream << "    float value = 0.0f;\n";
    stream << "    float pageSize = 10.0f;\n";
    stream << "    RuntimeOrientation orientation = RuntimeOrientation::Horizontal;\n";
    stream << "};\n\n";
    stream << "struct RuntimeStyleState {\n";
    stream << "    RuntimeColor panelColor = makeColor(0x1F, 0x24, 0x2D);\n";
    stream << "    RuntimeColor fillColor = makeColor(0x2B, 0x31, 0x3D);\n";
    stream << "    RuntimeColor recessedColor = makeColor(0x20, 0x26, 0x30);\n";
    stream << "    RuntimeColor raisedColor = makeColor(0x30, 0x37, 0x44);\n";
    stream << "    RuntimeColor textColor = makeColor(0xEE, 0xF2, 0xF8);\n";
    stream << "    RuntimeColor secondaryTextColor = makeColor(0xAE, 0xB8, 0xC8);\n";
    stream << "    RuntimeColor disabledTextColor = makeColor(0x6C, 0x77, 0x88);\n";
    stream << "    RuntimeColor borderColor = makeColor(0x97, 0xA3, 0xB7);\n";
    stream << "    RuntimeColor focusColor = makeColor(0x2D, 0x7F, 0xF9);\n";
    stream << "    RuntimeColor accentColor = makeColor(0x2D, 0x7F, 0xF9);\n";
    stream << "    RuntimeColor disabledColor = makeColor(0x6C, 0x77, 0x88);\n";
    stream << "    RuntimeColor selectedColor = makeColor(0x35, 0x53, 0x82);\n";
    stream << "    RuntimeColor hoverColor = makeColor(0x35, 0x40, 0x52);\n";
    stream << "    RuntimeColor pressedColor = makeColor(0x23, 0x2A, 0x35);\n";
    stream << "    RuntimeColor checkedColor = makeColor(0x35, 0x53, 0x82);\n";
    stream << "    RuntimeColor highlightColor = makeColor(0xC8, 0xD2, 0xE2);\n";
    stream << "    RuntimeColor shadowColor = makeColor(0x11, 0x15, 0x1C);\n";
    stream << "    float borderThickness = 1.0f;\n";
    stream << "    float cornerRadius = 0.0f;\n";
    stream << "    std::string fontFamily = \"Default\";\n";
    stream << "    float fontSize = 16.0f;\n";
    stream << "    int fontWeight = 400;\n";
    stream << "    bool italic = false;\n";
    stream << "    float controlPadding = 8.0f;\n";
    stream << "    float textPadding = 8.0f;\n";
    stream << "    std::string horizontalTextAlignment = \"Default\";\n";
    stream << "    std::string verticalTextAlignment = \"Default\";\n";
    stream << "    bool multiline = false;\n";
    stream << "    bool wordWrap = false;\n";
    stream << "    std::string overflowMode = \"Clip\";\n";
    stream << "};\n\n";
    stream << "struct RuntimeEventHandlers {\n";
    stream << "    std::string onClick;\n";
    stream << "    std::string onRelease;\n";
    stream << "    std::string onDoubleClick;\n";
    stream << "    std::string onToggle;\n";
    stream << "    std::string onSelected;\n";
    stream << "    std::string onChanged;\n";
    stream << "    std::string onTextChanged;\n";
    stream << "    std::string onAccepted;\n";
    stream << "    std::string onCancelled;\n";
    stream << "    std::string onSelectionChanged;\n";
    stream << "    std::string onCellDoubleClick;\n";
    stream << "};\n\n";
    stream << "struct RuntimeInteractionState {\n";
    stream << "    bool pressed = false;\n";
    stream << "    bool focused = false;\n";
    stream << "    bool hovered = false;\n";
    stream << "};\n\n";
    stream << "struct RuntimeStateAppearanceOverride {\n";
    stream << "    bool hasControlSurfaceColor = false;\n";
    stream << "    bool hasTextColor = false;\n";
    stream << "    bool hasBorderColor = false;\n";
    stream << "    bool hasAccentColor = false;\n";
    stream << "    bool hasFocusColor = false;\n";
    stream << "    bool hasHighlightColor = false;\n";
    stream << "    bool hasShadowColor = false;\n";
    stream << "    RuntimeColor controlSurfaceColor{};\n";
    stream << "    RuntimeColor textColor{};\n";
    stream << "    RuntimeColor borderColor{};\n";
    stream << "    RuntimeColor accentColor{};\n";
    stream << "    RuntimeColor focusColor{};\n";
    stream << "    RuntimeColor highlightColor{};\n";
    stream << "    RuntimeColor shadowColor{};\n";
    stream << "};\n\n";
    stream << "struct RuntimeWidget {\n";
    stream << "    RuntimeWidgetType type = RuntimeWidgetType::Unknown;\n";
    stream << "    std::string id;\n";
    stream << "    std::string name;\n";
    stream << "    std::string parentId;\n";
    stream << "    RuntimeRect bounds;\n";
    stream << "    float preferredWidth = 0.0f;\n";
    stream << "    float preferredHeight = 0.0f;\n";
    stream << "    std::string hint;\n";
    stream << "    std::string dialogTitle;\n";
    stream << "    std::string source;\n";
    stream << "    std::string colorValue = \"#2D7DFF\";\n";
    stream << "    std::vector<std::string> items;\n";
    stream << "    std::vector<std::string> itemActions;\n";
    stream << "    std::vector<std::string> tableColumns;\n";
    stream << "    std::vector<std::vector<std::string>> tableRows;\n";
    stream << "    int tabIndex = 0;\n";
    stream << "    int selectedTab = 0;\n";
    stream << "    int selectedIndex = -1;\n";
    stream << "    int selectedRow = -1;\n";
    stream << "    int selectedColumn = -1;\n";
    stream << "    bool multiSelect = false;\n";
    stream << "    bool showHeader = true;\n";
    stream << "    bool showGridLines = true;\n";
    stream << "    bool modal = true;\n";
    stream << "    bool visibleAtStartup = false;\n";
    stream << "    bool enabled = true;\n";
    stream << "    bool readOnly = false;\n";
    stream << "    float rowHeight = 28.0f;\n";
    stream << "    float headerHeight = 30.0f;\n";
    stream << "    int sizerPaddingLeft = 0;\n";
    stream << "    int sizerPaddingTop = 0;\n";
    stream << "    int sizerPaddingRight = 0;\n";
    stream << "    int sizerPaddingBottom = 0;\n";
    stream << "    int sizerGap = 0;\n";
    stream << "    int sizerItemProportion = 0;\n";
    stream << "    bool sizerItemExpand = false;\n";
    stream << "    RuntimeSizerAlignment sizerItemAlignment = RuntimeSizerAlignment::Start;\n";
    stream << "    int sizerItemBorder = 0;\n";
    stream << "    int sizerItemBorderSides = 0;\n";
    stream << "    int sizerItemMinimumWidth = -1;\n";
    stream << "    int sizerItemMinimumHeight = -1;\n";
    stream << "    bool sizerItemShown = true;\n";
    stream << "    bool spacerStretch = false;\n";
    stream << "    int spacerSize = 0;\n";
    stream << "    RuntimeTextState text;\n";
    stream << "    RuntimeButtonState button;\n";
    stream << "    RuntimeToggleState toggle;\n";
    stream << "    RuntimeRangeState range;\n";
    stream << "    RuntimeStyleState style;\n";
    stream << "    RuntimeStateAppearanceOverride hoverAppearance;\n";
    stream << "    RuntimeStateAppearanceOverride pressedAppearance;\n";
    stream << "    RuntimeStateAppearanceOverride focusedAppearance;\n";
    stream << "    RuntimeStateAppearanceOverride checkedOrSelectedAppearance;\n";
    stream << "    RuntimeStateAppearanceOverride disabledAppearance;\n";
    stream << "    RuntimeEventHandlers events;\n";
    stream << "    RuntimeInteractionState interaction;\n";
    stream << "};\n\n";
    stream << "struct RuntimeModalState {\n";
    stream << "    bool visible = false;\n";
    stream << "    std::string dialogId;\n";
    stream << "    std::string title;\n";
    stream << "    std::string message;\n";
    stream << "    std::vector<std::string> buttons;\n";
    stream << "};\n\n";
    stream << "class " << className << " : public visage::ApplicationWindow {\n";
    stream << "public:\n";
    stream << "    " << className << "();\n";
    stream << "    ~" << className << "() override = default;\n\n";
    stream << "    void showWindow();\n";
    stream << "    void draw(visage::Canvas& canvas) override;\n\n";
    stream << "    void mouseDown(const visage::MouseEvent& e) override;\n";
    stream << "    void mouseMove(const visage::MouseEvent& e) override;\n";
    stream << "    void mouseDrag(const visage::MouseEvent& e) override;\n";
    stream << "    void mouseUp(const visage::MouseEvent& e) override;\n";
    stream << "    bool keyPress(const visage::KeyEvent& e) override;\n";
    stream << "    bool receivesTextInput() override;\n";
    stream << "    void textInput(const std::string& text) override;\n\n";
    stream << "protected:\n";
    stream << "    RuntimeWidget* findWidgetById(const std::string& id);\n";
    stream << "    const RuntimeWidget* findWidgetById(const std::string& id) const;\n";
    stream << "    RuntimeWidget* findWidgetByName(const std::string& name);\n";
    stream << "    const RuntimeWidget* findWidgetByName(const std::string& name) const;\n\n";
    stream << "    [[nodiscard]] bool setText(const std::string& idOrName, const std::string& text);\n";
    stream << "    [[nodiscard]] std::optional<std::string> getText(const std::string& idOrName) const;\n";
    stream << "    [[nodiscard]] std::string getTextOr(const std::string& idOrName, std::string fallback) const;\n\n";
    stream << "    [[nodiscard]] bool setChecked(const std::string& idOrName, bool checked);\n";
    stream << "    [[nodiscard]] std::optional<bool> getChecked(const std::string& idOrName) const;\n";
    stream << "    [[nodiscard]] bool getCheckedOr(const std::string& idOrName, bool fallback) const;\n\n";
    stream << "    [[nodiscard]] bool setSelected(const std::string& idOrName, bool selected);\n";
    stream << "    [[nodiscard]] std::optional<bool> getSelected(const std::string& idOrName) const;\n";
    stream << "    [[nodiscard]] bool getSelectedOr(const std::string& idOrName, bool fallback) const;\n\n";
    stream << "    [[nodiscard]] std::optional<int> getSelectedRow(const std::string& idOrName) const;\n";
    stream << "    [[nodiscard]] std::optional<int> getSelectedColumn(const std::string& idOrName) const;\n";
    stream << "    [[nodiscard]] bool setSelectedCell(const std::string& idOrName, int row, int column);\n";
    stream << "    [[nodiscard]] std::optional<std::string> getCellText(const std::string& idOrName, int row, int column) const;\n";
    stream << "    [[nodiscard]] bool setCellText(const std::string& idOrName, int row, int column, const std::string& text);\n\n";
    stream << "    [[nodiscard]] bool setValue(const std::string& idOrName, float value);\n";
    stream << "    [[nodiscard]] std::optional<float> getValue(const std::string& idOrName) const;\n";
    stream << "    [[nodiscard]] float getValueOr(const std::string& idOrName, float fallback) const;\n\n";
    stream << "    [[nodiscard]] bool setProgressValue(const std::string& idOrName, float value);\n";
    stream << "    [[nodiscard]] bool setStatusBarField(const std::string& idOrName, int fieldIndex, const std::string& text);\n\n";
    stream << "    [[nodiscard]] bool showMessageDialog(const std::string& title, const std::string& message);\n";
    stream << "    [[nodiscard]] bool showModalDialog(const std::string& idOrName);\n";
    stream << "    void closeModalDialog();\n";
    stream << "    [[nodiscard]] std::optional<std::string> activeModalDialogId() const;\n\n";
    stream << "    void requestGeneratedUiRepaint();\n\n";
    for (const auto& handler : handlers) {
        stream << "    // Referenced by: " << handlerReferenceList(handler) << "\n";
        std::string declaration = handlerDeclaration(handler);
        if (!declaration.empty() && declaration.back() == ';') {
            declaration.pop_back();
        }
        stream << "    virtual " << declaration << ";\n";
    }
    if (!handlers.empty()) {
        stream << "\n";
    }
    stream << "private:\n";
    stream << "    bool canDrawText() const;\n\n";
    stream << "    void initializeRuntimeWidgets();\n";
    stream << "    void applyRuntimeSizerLayouts();\n";
    stream << "    RuntimeWidget* findWidgetByIdOrName(const std::string& idOrName);\n";
    stream << "    const RuntimeWidget* findWidgetByIdOrName(const std::string& idOrName) const;\n";
    stream << "    RuntimeWidget* activeModalWidget();\n";
    stream << "    const RuntimeWidget* activeModalWidget() const;\n";
    stream << "    [[nodiscard]] bool isWidgetVisible(const RuntimeWidget& widget) const;\n";
    stream << "    [[nodiscard]] std::optional<int> hitTestTabHeader(const RuntimeWidget& widget, float x, float y) const;\n";
    stream << "    [[nodiscard]] std::optional<int> menuBarItemIndexAt(const RuntimeWidget& widget, float x, float y) const;\n";
    stream << "    [[nodiscard]] std::optional<int> toolBarItemIndexAt(const RuntimeWidget& widget, float x, float y) const;\n";
    stream << "    RuntimeWidget* hitTest(float x, float y);\n";
    stream << "    RuntimeWidget* focusedTextBox();\n";
    stream << "    const RuntimeWidget* focusedTextBox() const;\n";
    stream << "    void setFocusedWidget(const std::string& widgetId);\n";
    stream << "    void clearPressedState();\n";
    stream << "    bool isInteractive(const RuntimeWidget& widget) const;\n";
    stream << "    [[nodiscard]] WidgetEvent makeWidgetEvent(const RuntimeWidget& widget, int itemIndex = -1) const;\n";
    stream << "    [[nodiscard]] RuntimeRect activeModalDialogRect() const;\n";
    stream << "    [[nodiscard]] RuntimeRect activeModalButtonRect(std::size_t buttonIndex) const;\n";
    stream << "    void handleActiveModalButton(std::size_t buttonIndex);\n";
    stream << "    void drawActiveModalDialog(visage::Canvas& canvas, bool drawText) const;\n";
    stream << "    bool setWidgetValue(RuntimeWidget& widget, float value, bool emitEvent);\n";
    stream << "    bool setItemSelection(RuntimeWidget& widget, int selectedIndex, bool emitEvent);\n";
    stream << "    bool setTableGridSelection(RuntimeWidget& widget, int row, int column, bool emitEvent);\n";
    stream << "    bool updateSliderFromPoint(RuntimeWidget& widget, float formX);\n";
    stream << "    RuntimeRect scrollBarThumbRect(const RuntimeWidget& widget) const;\n";
    stream << "    std::optional<int> listBoxRowIndexAt(const RuntimeWidget& widget, float x, float y) const;\n";
    stream << "    std::optional<std::pair<int, int>> tableGridCellAt(const RuntimeWidget& widget, float x, float y) const;\n";
    stream << "    bool updateScrollBarFromPointer(RuntimeWidget& widget, float formX, float formY);\n";
    stream << "    bool updateTextBoxText(RuntimeWidget& widget, const std::string& text, bool emitEvent);\n";
    stream << "    void emitItemAction(const RuntimeWidget& widget, int itemIndex);\n";
    stream << "    void emitVoidEvent(const RuntimeWidget& widget, std::string_view eventKey);\n";
    stream << "    void emitBoolEvent(const RuntimeWidget& widget, std::string_view eventKey, bool value);\n";
    stream << "    void emitFloatEvent(const RuntimeWidget& widget, std::string_view eventKey, float value);\n";
    stream << "    void emitStringEvent(const RuntimeWidget& widget, std::string_view eventKey, const std::string& value);\n\n";
    stream << "    visage::Font labelFont_{};\n";
    stream << "    RuntimeRect formBounds_{};\n";
    stream << "    std::string formTitle_{};\n";
    stream << "    RuntimeColor formPanelColor_ = makeColor(0x2B, 0x31, 0x3D);\n";
    stream << "    RuntimeColor formFillColor_ = makeColor(0x1F, 0x24, 0x2D);\n";
    stream << "    RuntimeColor formTextColor_ = makeColor(0xEE, 0xF2, 0xF8);\n";
    stream << "    RuntimeColor formBorderColor_ = makeColor(0x97, 0xA3, 0xB7);\n";
    stream << "    float formBorderThickness_ = 1.0f;\n";
    stream << "    std::vector<RuntimeWidget> runtimeWidgets_{};\n";
    stream << "    std::string pressedWidgetId_{};\n";
    stream << "    std::string focusedWidgetId_{};\n";
    stream << "    std::string draggingWidgetId_{};\n";
    stream << "    bool draggingSlider_ = false;\n";
    stream << "    bool draggingScrollBar_ = false;\n";
    stream << "    float dragPointerOffset_ = 0.0f;\n";
    stream << "    RuntimeModalState modalState_{};\n";
    stream << "};\n";
    return stream.str();
}

std::string emitMainCpp(const visiform::model::ProjectDocument& document)
{
    const std::string className = userSubclassName(document);
    std::ostringstream stream;
    stream << kGeneratedFileHeader;
    stream << "#include \"" << className << ".h\"\n\n";
    stream << "#include <exception>\n";
    stream << "#include <iostream>\n\n";
    stream << "int main()\n";
    stream << "{\n";
    stream << "    try {\n";
    stream << "        " << className << " window;\n";
    stream << "        window.showWindow();\n";
    stream << "        if (!window.isShowing()) {\n";
    stream << "            throw std::runtime_error(\"The generated Visage window did not open.\");\n";
    stream << "        }\n";
    stream << "\n";
    stream << "        window.runEventLoop();\n";
    stream << "        return 0;\n";
    stream << "    }\n";
    stream << "    catch (const std::exception& exception) {\n";
    stream << "        std::cerr << \"Generated Visage project failed to start: \" << exception.what() << '\\n';\n";
    stream << "    }\n";
    stream << "    catch (...) {\n";
    stream << "        std::cerr << \"Generated Visage project failed to start with an unknown error.\\n\";\n";
    stream << "    }\n\n";
    stream << "    return 1;\n";
    stream << "}\n";
    return stream.str();
}

std::string emitGeneratedBaseCpp(const visiform::model::ProjectDocument& document)
{
    const std::string className = generatedBaseClassName(document);
    const std::string projectName = document.projectName.empty() ? std::string{"VisiFormProject"} : document.projectName;
    const std::string windowTitle = document.windowTitle.empty()
        ? document.root.getStringProperty("title", projectName)
        : document.windowTitle;
    const float windowWidth = std::max(1000.0f, document.root.bounds.width + 120.0f);
    const float windowHeight = std::max(800.0f, document.root.bounds.height + 120.0f);
    const ResolvedWidgetStyle rootStyle = resolveWidgetStyle(document, document.root);
    std::vector<HandlerInfo> handlers;
    std::vector<EventBinding> bindings;
    std::vector<RuntimeWidgetSpec> runtimeWidgets;
    std::string ignoredError;
    collectHandlers(document, handlers, ignoredError);
    collectEventBindings(document.root, bindings, ignoredError);
    collectRuntimeWidgetSpecs(document, document.root, -document.root.bounds.x, -document.root.bounds.y, runtimeWidgets);

    std::ostringstream stream;
    stream << kGeneratedFileHeader;
    stream << "#include \"" << className << ".h\"\n\n";
    stream << "#include <algorithm>\n";
    stream << "#include <array>\n";
    stream << "#include <cctype>\n";
    stream << "#include <cmath>\n";
    stream << "#include <cstdint>\n";
    stream << "#include <fstream>\n";
    stream << "#include <sstream>\n";
    stream << "#include <string>\n";
    stream << "#include <string_view>\n";
    stream << "#include <utility>\n\n";
    stream << "namespace {\n\n";
    stream << "constexpr float kFormOffsetX = 40.0f;\n";
    stream << "constexpr float kFormOffsetY = 60.0f;\n";
    stream << "constexpr float kTitleBarHeight = 28.0f;\n\n";
    stream << "constexpr float kTabHeaderHeight = 30.0f;\n\n";
    stream << "std::uint32_t runtimeColorToArgb(RuntimeColor color)\n";
    stream << "{\n";
    stream << "    return (static_cast<std::uint32_t>(color.a) << 24)\n";
    stream << "        | (static_cast<std::uint32_t>(color.r) << 16)\n";
    stream << "        | (static_cast<std::uint32_t>(color.g) << 8)\n";
    stream << "        | static_cast<std::uint32_t>(color.b);\n";
    stream << "}\n\n";
    stream << "int canvasColor(RuntimeColor color)\n";
    stream << "{\n";
    stream << "    return static_cast<int>(runtimeColorToArgb(color));\n";
    stream << "}\n\n";
    stream << "void drawBorder(visage::Canvas& canvas, float x, float y, float width, float height, RuntimeColor color, float thickness = 1.0f)\n";
    stream << "{\n";
    stream << "    canvas.setColor(canvasColor(color));\n";
    stream << "    canvas.fill(x, y, width, thickness);\n";
    stream << "    canvas.fill(x, y + height - thickness, width, thickness);\n";
    stream << "    canvas.fill(x, y, thickness, height);\n";
    stream << "    canvas.fill(x + width - thickness, y, thickness, height);\n";
    stream << "}\n\n";
    stream << "RuntimeColor blendColor(RuntimeColor colorA, RuntimeColor colorB, float amount)\n";
    stream << "{\n";
    stream << "    const auto blendChannel = [amount](std::uint8_t first, std::uint8_t second) {\n";
    stream << "        return static_cast<std::uint8_t>(std::round(static_cast<float>(first) * (1.0f - amount) + static_cast<float>(second) * amount));\n";
    stream << "    };\n\n";
    stream << "    return makeColor(\n";
    stream << "        blendChannel(colorA.r, colorB.r),\n";
    stream << "        blendChannel(colorA.g, colorB.g),\n";
    stream << "        blendChannel(colorA.b, colorB.b),\n";
    stream << "        blendChannel(colorA.a, colorB.a));\n";
    stream << "}\n\n";
    stream << "enum class RuntimeVisualBaseState {\n";
    stream << "    Normal,\n";
    stream << "    Hovered,\n";
    stream << "    CheckedOrSelected,\n";
    stream << "    Pressed,\n";
    stream << "    Disabled\n";
    stream << "};\n\n";
    stream << "RuntimeVisualBaseState resolveVisualBaseState(const RuntimeWidget& widget, bool checkedOrSelected = false, bool active = false)\n";
    stream << "{\n";
    stream << "    if (!widget.enabled) {\n";
    stream << "        return RuntimeVisualBaseState::Disabled;\n";
    stream << "    }\n";
    stream << "    if (widget.interaction.pressed) {\n";
    stream << "        return RuntimeVisualBaseState::Pressed;\n";
    stream << "    }\n";
    stream << "    if (checkedOrSelected || active) {\n";
    stream << "        return RuntimeVisualBaseState::CheckedOrSelected;\n";
    stream << "    }\n";
    stream << "    return widget.interaction.hovered ? RuntimeVisualBaseState::Hovered : RuntimeVisualBaseState::Normal;\n";
    stream << "}\n\n";
    stream << "void applyStateAppearance(RuntimeWidget& widget, const RuntimeStateAppearanceOverride& overrides, RuntimeVisualBaseState baseState)\n";
    stream << "{\n";
    stream << "    if (overrides.hasControlSurfaceColor) {\n";
    stream << "        switch (baseState) {\n";
    stream << "        case RuntimeVisualBaseState::Hovered: widget.style.hoverColor = overrides.controlSurfaceColor; break;\n";
    stream << "        case RuntimeVisualBaseState::Pressed: widget.style.pressedColor = overrides.controlSurfaceColor; break;\n";
    stream << "        case RuntimeVisualBaseState::CheckedOrSelected:\n";
    stream << "            widget.style.checkedColor = overrides.controlSurfaceColor;\n";
    stream << "            widget.style.selectedColor = overrides.controlSurfaceColor;\n";
    stream << "            break;\n";
    stream << "        case RuntimeVisualBaseState::Disabled: widget.style.disabledColor = overrides.controlSurfaceColor; break;\n";
    stream << "        case RuntimeVisualBaseState::Normal: widget.style.fillColor = overrides.controlSurfaceColor; break;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    if (overrides.hasTextColor) {\n";
    stream << "        if (baseState == RuntimeVisualBaseState::Disabled) widget.style.disabledTextColor = overrides.textColor;\n";
    stream << "        else widget.style.textColor = overrides.textColor;\n";
    stream << "    }\n";
    stream << "    if (overrides.hasBorderColor) widget.style.borderColor = overrides.borderColor;\n";
    stream << "    if (overrides.hasAccentColor) widget.style.accentColor = overrides.accentColor;\n";
    stream << "    if (overrides.hasFocusColor) widget.style.focusColor = overrides.focusColor;\n";
    stream << "    if (overrides.hasHighlightColor) widget.style.highlightColor = overrides.highlightColor;\n";
    stream << "    if (overrides.hasShadowColor) widget.style.shadowColor = overrides.shadowColor;\n";
    stream << "}\n\n";
    stream << "void applyActiveStateAppearance(RuntimeWidget& widget)\n";
    stream << "{\n";
    stream << "    const bool checkedOrSelected = widget.toggle.checked || widget.toggle.selected\n";
    stream << "        || (widget.type == RuntimeWidgetType::ListBox && widget.selectedIndex >= 0)\n";
    stream << "        || (widget.type == RuntimeWidgetType::TabControl && widget.selectedTab >= 0);\n";
    stream << "    const RuntimeVisualBaseState baseState = resolveVisualBaseState(widget, checkedOrSelected, checkedOrSelected);\n";
    stream << "    switch (baseState) {\n";
    stream << "    case RuntimeVisualBaseState::Hovered: applyStateAppearance(widget, widget.hoverAppearance, baseState); break;\n";
    stream << "    case RuntimeVisualBaseState::Pressed: applyStateAppearance(widget, widget.pressedAppearance, baseState); break;\n";
    stream << "    case RuntimeVisualBaseState::CheckedOrSelected: applyStateAppearance(widget, widget.checkedOrSelectedAppearance, baseState); break;\n";
    stream << "    case RuntimeVisualBaseState::Disabled: applyStateAppearance(widget, widget.disabledAppearance, baseState); break;\n";
    stream << "    case RuntimeVisualBaseState::Normal: break;\n";
    stream << "    }\n";
    stream << "    if (widget.interaction.focused && widget.enabled) {\n";
    stream << "        applyStateAppearance(widget, widget.focusedAppearance, baseState);\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "RuntimeColor visualStateFill(const RuntimeWidget& widget, RuntimeColor fillColor, bool checkedOrSelected = false, bool active = false)\n";
    stream << "{\n";
    stream << "    switch (resolveVisualBaseState(widget, checkedOrSelected, active)) {\n";
    stream << "    case RuntimeVisualBaseState::Disabled:\n";
    stream << "        return blendColor(fillColor, widget.style.disabledColor, 0.55f);\n";
    stream << "    case RuntimeVisualBaseState::Pressed:\n";
    stream << "        return widget.style.pressedColor;\n";
    stream << "    case RuntimeVisualBaseState::CheckedOrSelected:\n";
    stream << "        return checkedOrSelected ? widget.style.checkedColor : widget.style.selectedColor;\n";
    stream << "    case RuntimeVisualBaseState::Hovered:\n";
    stream << "        return widget.style.hoverColor;\n";
    stream << "    case RuntimeVisualBaseState::Normal:\n";
    stream << "        return fillColor;\n";
    stream << "    }\n";
    stream << "    return fillColor;\n";
    stream << "}\n\n";
    stream << "RuntimeColor visualTextColor(const RuntimeWidget& widget)\n";
    stream << "{\n";
    stream << "    return widget.enabled ? widget.style.textColor : widget.style.disabledTextColor;\n";
    stream << "}\n\n";
    stream << "void drawVisualBevel(visage::Canvas& canvas, float x, float y, float width, float height, const RuntimeWidget& widget, RuntimeColor fillColor, bool checkedOrSelected = false, bool active = false)\n";
    stream << "{\n";
    stream << "    const RuntimeVisualBaseState baseState = resolveVisualBaseState(widget, checkedOrSelected, active);\n";
    stream << "    const bool pressed = baseState == RuntimeVisualBaseState::Pressed;\n";
    stream << "    const RuntimeColor stateFill = visualStateFill(widget, fillColor, checkedOrSelected, active);\n";
    stream << "    canvas.setColor(canvasColor(stateFill));\n";
    stream << "    canvas.fill(x, y, width, height);\n";
    stream << "    drawBorder(canvas, x, y, width, height, widget.enabled ? widget.style.borderColor : blendColor(widget.style.borderColor, widget.style.disabledColor, 0.65f), widget.style.borderThickness);\n";
    stream << "    if (width < 3.0f || height < 3.0f) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    const RuntimeColor leading = pressed ? widget.style.shadowColor : widget.style.highlightColor;\n";
    stream << "    const RuntimeColor trailing = pressed ? widget.style.highlightColor : widget.style.shadowColor;\n";
    stream << "    canvas.setColor(canvasColor(leading));\n";
    stream << "    canvas.fill(x + 1.0f, y + 1.0f, std::max(0.0f, width - 2.0f), 1.0f);\n";
    stream << "    canvas.fill(x + 1.0f, y + 1.0f, 1.0f, std::max(0.0f, height - 2.0f));\n";
    stream << "    canvas.setColor(canvasColor(trailing));\n";
    stream << "    canvas.fill(x + 1.0f, y + height - 2.0f, std::max(0.0f, width - 2.0f), 1.0f);\n";
    stream << "    canvas.fill(x + width - 2.0f, y + 1.0f, 1.0f, std::max(0.0f, height - 2.0f));\n";
    stream << "    if (widget.interaction.focused && widget.enabled) {\n";
    stream << "        drawBorder(canvas, x - 1.0f, y - 1.0f, width + 2.0f, height + 2.0f, widget.style.focusColor);\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "void drawVisualRecessed(visage::Canvas& canvas, float x, float y, float width, float height, const RuntimeWidget& widget, bool focused)\n";
    stream << "{\n";
    stream << "    RuntimeColor fillColor = widget.style.recessedColor;\n";
    stream << "    if (widget.readOnly) {\n";
    stream << "        fillColor = blendColor(fillColor, widget.style.disabledColor, 0.28f);\n";
    stream << "    }\n";
    stream << "    if (!widget.enabled) {\n";
    stream << "        fillColor = blendColor(fillColor, widget.style.disabledColor, 0.55f);\n";
    stream << "    }\n";
    stream << "    canvas.setColor(canvasColor(fillColor));\n";
    stream << "    canvas.fill(x, y, width, height);\n";
    stream << "    drawBorder(canvas, x, y, width, height, widget.style.borderColor, widget.style.borderThickness);\n";
    stream << "    if (width >= 3.0f && height >= 3.0f) {\n";
    stream << "        canvas.setColor(canvasColor(widget.style.shadowColor));\n";
    stream << "        canvas.fill(x + 1.0f, y + 1.0f, std::max(0.0f, width - 2.0f), 1.0f);\n";
    stream << "        canvas.fill(x + 1.0f, y + 1.0f, 1.0f, std::max(0.0f, height - 2.0f));\n";
    stream << "        canvas.setColor(canvasColor(widget.style.highlightColor));\n";
    stream << "        canvas.fill(x + 1.0f, y + height - 2.0f, std::max(0.0f, width - 2.0f), 1.0f);\n";
    stream << "        canvas.fill(x + width - 2.0f, y + 1.0f, 1.0f, std::max(0.0f, height - 2.0f));\n";
    stream << "    }\n";
    stream << "    if (focused && widget.enabled) {\n";
    stream << "        drawBorder(canvas, x - 1.0f, y - 1.0f, width + 2.0f, height + 2.0f, widget.style.focusColor);\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "void fillCircleApprox(visage::Canvas& canvas, float centerX, float centerY, float radius, RuntimeColor color)\n";
    stream << "{\n";
    stream << "    if (radius <= 0.0f) {\n";
    stream << "        return;\n";
    stream << "    }\n\n";
    stream << "    canvas.setColor(canvasColor(color));\n";
    stream << "    const int radiusPixels = std::max(1, static_cast<int>(std::ceil(radius)));\n";
    stream << "    for (int offsetY = -radiusPixels; offsetY <= radiusPixels; ++offsetY) {\n";
    stream << "        const float dy = static_cast<float>(offsetY);\n";
    stream << "        const float halfWidth = std::sqrt(std::max(0.0f, radius * radius - dy * dy));\n";
    stream << "        canvas.fill(centerX - halfWidth, centerY + dy, halfWidth * 2.0f + 1.0f, 1.0f);\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "void fillRoundedRect(visage::Canvas& canvas, float x, float y, float width, float height, float radius, RuntimeColor color)\n";
    stream << "{\n";
    stream << "    if (width <= 0.0f || height <= 0.0f) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    if (radius <= 1.0f) {\n";
    stream << "        canvas.setColor(canvasColor(color));\n";
    stream << "        canvas.fill(x, y, width, height);\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    const float r = std::min(radius, std::min(width, height) * 0.5f);\n";
    stream << "    canvas.setColor(canvasColor(color));\n";
    stream << "    canvas.fill(x + r, y, std::max(0.0f, width - r * 2.0f), height);\n";
    stream << "    canvas.fill(x, y + r, r, std::max(0.0f, height - r * 2.0f));\n";
    stream << "    canvas.fill(x + width - r, y + r, r, std::max(0.0f, height - r * 2.0f));\n";
    stream << "    fillCircleApprox(canvas, x + r, y + r, r, color);\n";
    stream << "    fillCircleApprox(canvas, x + width - r, y + r, r, color);\n";
    stream << "    fillCircleApprox(canvas, x + r, y + height - r, r, color);\n";
    stream << "    fillCircleApprox(canvas, x + width - r, y + height - r, r, color);\n";
    stream << "}\n\n";
    stream << "void drawRoundedBox(visage::Canvas& canvas, float x, float y, float width, float height, RuntimeColor fillColor, RuntimeColor borderColor, float thickness, float radius)\n";
    stream << "{\n";
    stream << "    if (width <= 0.0f || height <= 0.0f) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    const float r = std::clamp(radius, 0.0f, std::min(width, height) * 0.5f);\n";
    stream << "    fillRoundedRect(canvas, x, y, width, height, r, fillColor);\n";
    stream << "    if (thickness <= 0.0f) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    if (r <= 1.0f) {\n";
    stream << "        drawBorder(canvas, x, y, width, height, borderColor, thickness);\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    canvas.setColor(canvasColor(borderColor));\n";
    stream << "    canvas.fill(x + r, y, std::max(0.0f, width - r * 2.0f), thickness);\n";
    stream << "    canvas.fill(x + r, y + height - thickness, std::max(0.0f, width - r * 2.0f), thickness);\n";
    stream << "    canvas.fill(x, y + r, thickness, std::max(0.0f, height - r * 2.0f));\n";
    stream << "    canvas.fill(x + width - thickness, y + r, thickness, std::max(0.0f, height - r * 2.0f));\n";
    stream << "    fillCircleApprox(canvas, x + r, y + r, r, borderColor);\n";
    stream << "    fillCircleApprox(canvas, x + width - r, y + r, r, borderColor);\n";
    stream << "    fillCircleApprox(canvas, x + r, y + height - r, r, borderColor);\n";
    stream << "    fillCircleApprox(canvas, x + width - r, y + height - r, r, borderColor);\n";
    stream << "    fillRoundedRect(canvas, x + thickness, y + thickness, std::max(0.0f, width - thickness * 2.0f), std::max(0.0f, height - thickness * 2.0f), std::max(0.0f, r - thickness), fillColor);\n";
    stream << "}\n\n";
    stream << "RuntimeColor parseColorOrDefault(std::string_view value, RuntimeColor fallback)\n";
    stream << "{\n";
    stream << "    if (value.empty() || value.front() != '#') {\n";
    stream << "        return fallback;\n";
    stream << "    }\n\n";
    stream << "    try {\n";
    stream << "        const std::string digits{ value.substr(1) };\n";
    stream << "        const std::uint32_t parsed = static_cast<std::uint32_t>(std::stoul(digits, nullptr, 16));\n";
    stream << "        if (digits.size() == 6) {\n";
    stream << "            return makeColor(\n";
    stream << "                static_cast<std::uint8_t>((parsed >> 16) & 0xffu),\n";
    stream << "                static_cast<std::uint8_t>((parsed >> 8) & 0xffu),\n";
    stream << "                static_cast<std::uint8_t>(parsed & 0xffu));\n";
    stream << "        }\n";
    stream << "        if (digits.size() == 8) {\n";
    stream << "            return makeColor(\n";
    stream << "                static_cast<std::uint8_t>((parsed >> 16) & 0xffu),\n";
    stream << "                static_cast<std::uint8_t>((parsed >> 8) & 0xffu),\n";
    stream << "                static_cast<std::uint8_t>(parsed & 0xffu),\n";
    stream << "                static_cast<std::uint8_t>((parsed >> 24) & 0xffu));\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    catch (...) {\n";
    stream << "    }\n\n";
    stream << "    return fallback;\n";
    stream << "}\n\n";
    stream << "std::string_view runtimeWidgetTypeName(RuntimeWidgetType type)\n";
    stream << "{\n";
    stream << "    switch (type) {\n";
    stream << "    case RuntimeWidgetType::Unknown: return \"Unknown\";\n";
    stream << "    case RuntimeWidgetType::Button: return \"Button\";\n";
    stream << "    case RuntimeWidgetType::TextBox: return \"TextBox\";\n";
    stream << "    case RuntimeWidgetType::ComboBox: return \"ComboBox\";\n";
    stream << "    case RuntimeWidgetType::ListBox: return \"ListBox\";\n";
    stream << "    case RuntimeWidgetType::TableGrid: return \"TableGrid\";\n";
    stream << "    case RuntimeWidgetType::CheckBox: return \"CheckBox\";\n";
    stream << "    case RuntimeWidgetType::RadioButton: return \"RadioButton\";\n";
    stream << "    case RuntimeWidgetType::Slider: return \"Slider\";\n";
    stream << "    case RuntimeWidgetType::ScrollBar: return \"ScrollBar\";\n";
    stream << "    case RuntimeWidgetType::ProgressBar: return \"ProgressBar\";\n";
    stream << "    case RuntimeWidgetType::StatusBar: return \"StatusBar\";\n";
    stream << "    case RuntimeWidgetType::ModalDialog: return \"ModalDialog\";\n";
    stream << "    case RuntimeWidgetType::Label: return \"Label\";\n";
    stream << "    case RuntimeWidgetType::Frame: return \"Frame\";\n";
    stream << "    case RuntimeWidgetType::GroupBox: return \"GroupBox\";\n";
    stream << "    case RuntimeWidgetType::Panel: return \"Panel\";\n";
    stream << "    case RuntimeWidgetType::Sizer: return \"Sizer\";\n";
    stream << "    case RuntimeWidgetType::TabControl: return \"TabControl\";\n";
    stream << "    case RuntimeWidgetType::MenuBar: return \"MenuBar\";\n";
    stream << "    case RuntimeWidgetType::ToolBar: return \"ToolBar\";\n";
    stream << "    case RuntimeWidgetType::Image: return \"Image\";\n";
    stream << "    case RuntimeWidgetType::Spacer: return \"Spacer\";\n";
    stream << "    case RuntimeWidgetType::ColorPicker: return \"ColorPicker\";\n";
    stream << "    }\n\n";
    stream << "    return \"Unknown\";\n";
    stream << "}\n\n";
    stream << "float normalizedRangeValue(const RuntimeWidget& widget)\n";
    stream << "{\n";
    stream << "    const float safeMaximum = widget.range.max <= widget.range.min ? widget.range.min + 1.0f : widget.range.max;\n";
    stream << "    return std::clamp((widget.range.value - widget.range.min) / (safeMaximum - widget.range.min), 0.0f, 1.0f);\n";
    stream << "}\n\n";
    stream << "float rangeValueForNormalized(const RuntimeWidget& widget, float normalized)\n";
    stream << "{\n";
    stream << "    const float safeMaximum = widget.range.max <= widget.range.min ? widget.range.min + 1.0f : widget.range.max;\n";
    stream << "    return widget.range.min + (safeMaximum - widget.range.min) * std::clamp(normalized, 0.0f, 1.0f);\n";
    stream << "}\n\n";
    stream << "std::string progressText(const RuntimeWidget& widget)\n";
    stream << "{\n";
    stream << "    if (!widget.text.showText) {\n";
    stream << "        return {};\n";
    stream << "    }\n";
    stream << "    if (!widget.text.value.empty()) {\n";
    stream << "        return widget.text.value;\n";
    stream << "    }\n";
    stream << "    return std::to_string(static_cast<int>(std::round(normalizedRangeValue(widget) * 100.0f))) + \"%\";\n";
    stream << "}\n\n";
    stream << "RuntimeRect scrollBarThumbRectForWidget(const RuntimeWidget& widget)\n";
    stream << "{\n";
    stream << "    const bool vertical = widget.range.orientation == RuntimeOrientation::Vertical;\n";
    stream << "    const float arrowSize = vertical ? std::min(widget.bounds.width, 20.0f) : std::min(widget.bounds.height, 20.0f);\n";
    stream << "    const float normalized = normalizedRangeValue(widget);\n";
    stream << "    const float safeMaximum = widget.range.max <= widget.range.min ? widget.range.min + 1.0f : widget.range.max;\n";
    stream << "    const float thumbFactor = std::clamp(widget.range.pageSize / (safeMaximum - widget.range.min + widget.range.pageSize), 0.18f, 0.55f);\n";
    stream << "    if (vertical) {\n";
    stream << "        const float trackTop = widget.bounds.y + arrowSize;\n";
    stream << "        const float trackHeight = std::max(0.0f, widget.bounds.height - arrowSize * 2.0f);\n";
    stream << "        const float thumbHeight = std::clamp(trackHeight * thumbFactor, 18.0f, std::max(18.0f, trackHeight));\n";
    stream << "        const float thumbY = trackTop + std::max(0.0f, trackHeight - thumbHeight) * normalized;\n";
    stream << "        return { widget.bounds.x + 4.0f, thumbY, std::max(0.0f, widget.bounds.width - 8.0f), thumbHeight };\n";
    stream << "    }\n";
    stream << "    const float trackLeft = widget.bounds.x + arrowSize;\n";
    stream << "    const float trackWidth = std::max(0.0f, widget.bounds.width - arrowSize * 2.0f);\n";
    stream << "    const float thumbWidth = std::clamp(trackWidth * thumbFactor, 18.0f, std::max(18.0f, trackWidth));\n";
    stream << "    const float thumbX = trackLeft + std::max(0.0f, trackWidth - thumbWidth) * normalized;\n";
    stream << "    return { thumbX, widget.bounds.y + 4.0f, thumbWidth, std::max(0.0f, widget.bounds.height - 8.0f) };\n";
    stream << "}\n\n";
    stream << "int sanitizeItemIndex(const RuntimeWidget& widget, int index)\n";
    stream << "{\n";
    stream << "    if (widget.items.empty()) {\n";
    stream << "        return -1;\n";
    stream << "    }\n";
    stream << "    return std::clamp(index, 0, static_cast<int>(widget.items.size()) - 1);\n";
    stream << "}\n\n";
    stream << "std::string selectedItemText(const RuntimeWidget& widget)\n";
    stream << "{\n";
    stream << "    const int safeIndex = sanitizeItemIndex(widget, widget.selectedIndex);\n";
    stream << "    if (safeIndex < 0) {\n";
    stream << "        return {};\n";
    stream << "    }\n";
    stream << "    return widget.items[static_cast<std::size_t>(safeIndex)];\n";
    stream << "}\n\n";
    stream << "std::string itemActionAt(const RuntimeWidget& widget, int index)\n";
    stream << "{\n";
    stream << "    if (index < 0 || index >= static_cast<int>(widget.itemActions.size())) {\n";
    stream << "        return {};\n";
    stream << "    }\n";
    stream << "    return widget.itemActions[static_cast<std::size_t>(index)];\n";
    stream << "}\n\n";
    stream << "std::string selectedItemAction(const RuntimeWidget& widget)\n";
    stream << "{\n";
    stream << "    return itemActionAt(widget, sanitizeItemIndex(widget, widget.selectedIndex));\n";
    stream << "}\n\n";
    stream << "float listBoxRowHeight(const RuntimeWidget& widget)\n";
    stream << "{\n";
    stream << "    return std::max(18.0f, std::max(8.0f, widget.style.fontSize) * 1.5f);\n";
    stream << "}\n\n";
    stream << "std::string tableGridCellText(const RuntimeWidget& widget, int row, int column)\n";
    stream << "{\n";
    stream << "    if (row < 0 || column < 0 || row >= static_cast<int>(widget.tableRows.size())) {\n";
    stream << "        return {};\n";
    stream << "    }\n";
    stream << "    const auto& cells = widget.tableRows[static_cast<std::size_t>(row)];\n";
    stream << "    if (column >= static_cast<int>(cells.size())) {\n";
    stream << "        return {};\n";
    stream << "    }\n";
    stream << "    return cells[static_cast<std::size_t>(column)];\n";
    stream << "}\n\n";
    stream << "std::vector<std::string> splitModalMessageLines(const std::string& text)\n";
    stream << "{\n";
    stream << "    std::vector<std::string> lines;\n";
    stream << "    std::istringstream stream(text);\n";
    stream << "    std::string line;\n";
    stream << "    while (std::getline(stream, line)) {\n";
    stream << "        lines.push_back(line);\n";
    stream << "    }\n";
    stream << "    if (lines.empty()) {\n";
    stream << "        lines.push_back(text);\n";
    stream << "    }\n";
    stream << "    return lines;\n";
    stream << "}\n\n";
    stream << "std::string lowerText(std::string value)\n";
    stream << "{\n";
    stream << "    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {\n";
    stream << "        return static_cast<char>(std::tolower(character));\n";
    stream << "    });\n";
    stream << "    return value;\n";
    stream << "}\n\n";
    stream << "bool isAcceptButtonLabel(std::string_view label)\n";
    stream << "{\n";
    stream << "    const std::string lowered = lowerText(std::string{ label });\n";
    stream << "    return lowered == \"ok\" || lowered == \"yes\" || lowered == \"apply\" || lowered == \"continue\";\n";
    stream << "}\n\n";
    stream << "bool isCancelButtonLabel(std::string_view label)\n";
    stream << "{\n";
    stream << "    const std::string lowered = lowerText(std::string{ label });\n";
    stream << "    return lowered == \"cancel\" || lowered == \"no\" || lowered == \"close\";\n";
    stream << "}\n\n";
    stream << "float measuredTextWidth(const visage::Font& font, const std::string& text)\n";
    stream << "{\n";
    stream << "    if (text.empty() || font.packedFont() == nullptr) return 0.0f;\n";
    stream << "    return font.stringWidth(visage::String::convertUtf8ToUtf32<std::u32string>(text));\n";
    stream << "}\n\n";
    stream << "std::string elideText(const visage::Font& font, const std::string& text, float availableWidth)\n";
    stream << "{\n";
    stream << "    if (text.empty() || availableWidth <= 0.0f || measuredTextWidth(font, text) <= availableWidth) return availableWidth <= 0.0f ? std::string{} : text;\n";
    stream << "    const std::u32string ellipsis = U\"\\u2026\";\n";
    stream << "    const float ellipsisWidth = font.stringWidth(ellipsis);\n";
    stream << "    if (ellipsisWidth > availableWidth) return {};\n";
    stream << "    std::u32string utf32 = visage::String::convertUtf8ToUtf32<std::u32string>(text);\n";
    stream << "    const int prefixLength = font.widthOverflowIndex(utf32.c_str(), static_cast<int>(utf32.size()), availableWidth - ellipsisWidth);\n";
    stream << "    utf32.resize(static_cast<std::size_t>(std::max(0, prefixLength)));\n";
    stream << "    while (!utf32.empty() && (utf32.back() == U' ' || utf32.back() == U'\\t')) utf32.pop_back();\n";
    stream << "    utf32 += ellipsis;\n";
    stream << "    return visage::String::convertUtf32ToUtf8(utf32);\n";
    stream << "}\n\n";
    stream << "std::vector<std::string> layoutRuntimeLines(const visage::Font& font, const std::string& text, float availableWidth, bool multiline, bool wordWrap)\n";
    stream << "{\n";
    stream << "    std::vector<std::string> logical;\n";
    stream << "    std::size_t lineStart = 0;\n";
    stream << "    while (lineStart <= text.size()) {\n";
    stream << "        const std::size_t lineEnd = text.find('\\n', lineStart);\n";
    stream << "        logical.push_back(text.substr(lineStart, (lineEnd == std::string::npos ? text.size() : lineEnd) - lineStart));\n";
    stream << "        if (lineEnd == std::string::npos || !multiline) break;\n";
    stream << "        lineStart = lineEnd + 1;\n";
    stream << "    }\n";
    stream << "    std::vector<std::string> lines;\n";
    stream << "    for (const auto& sourceLine : logical) {\n";
    stream << "        if (!multiline || !wordWrap || availableWidth <= 0.0f || measuredTextWidth(font, sourceLine) <= availableWidth) { lines.push_back(sourceLine); continue; }\n";
    stream << "        std::string remaining = sourceLine;\n";
    stream << "        while (!remaining.empty()) {\n";
    stream << "            while (!remaining.empty() && std::isspace(static_cast<unsigned char>(remaining.front())) != 0) remaining.erase(remaining.begin());\n";
    stream << "            if (remaining.empty()) break;\n";
    stream << "            if (measuredTextWidth(font, remaining) <= availableWidth) { lines.push_back(remaining); break; }\n";
    stream << "            std::size_t bestBreak = std::string::npos;\n";
    stream << "            std::size_t overflow = 1;\n";
    stream << "            for (std::size_t index = 1; index <= remaining.size(); ++index) {\n";
    stream << "                if (index < remaining.size() && std::isspace(static_cast<unsigned char>(remaining[index])) != 0) bestBreak = index;\n";
    stream << "                if (measuredTextWidth(font, remaining.substr(0, index)) > availableWidth) { overflow = index; break; }\n";
    stream << "            }\n";
    stream << "            const std::size_t split = bestBreak != std::string::npos && bestBreak > 0 ? bestBreak : std::max<std::size_t>(1, overflow - 1);\n";
    stream << "            std::string line = remaining.substr(0, split);\n";
    stream << "            while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back())) != 0) line.pop_back();\n";
    stream << "            lines.push_back(line);\n";
    stream << "            remaining.erase(0, split);\n";
    stream << "        }\n";
    stream << "        if (sourceLine.empty()) lines.emplace_back();\n";
    stream << "    }\n";
    stream << "    if (lines.empty()) lines.emplace_back();\n";
    stream << "    return lines;\n";
    stream << "}\n\n";
    stream << "void drawRuntimeText(visage::Canvas& canvas, const visage::Font& font, const RuntimeStyleState& style, const std::string& text, float x, float y, float width, float height, std::string defaultHorizontal = \"Left\", std::string defaultVertical = \"Top\", bool forceSingleLine = false)\n";
    stream << "{\n";
    stream << "    if (width <= 0.0f || height <= 0.0f) return;\n";
    stream << "    const bool multiline = !forceSingleLine && style.multiline;\n";
    stream << "    std::vector<std::string> lines = layoutRuntimeLines(font, text, width, multiline, !forceSingleLine && style.wordWrap);\n";
    stream << "    if (!multiline && style.overflowMode == \"Ellipsis\" && !lines.empty()) lines.front() = elideText(font, lines.front(), width);\n";
    stream << "    const float lineHeight = std::max(1.0f, font.packedFont() != nullptr ? font.lineHeight() : std::max(8.0f, style.fontSize) * 1.25f);\n";
    stream << "    const float totalHeight = lineHeight * static_cast<float>(lines.size());\n";
    stream << "    const std::string horizontal = style.horizontalTextAlignment == \"Default\" ? defaultHorizontal : style.horizontalTextAlignment;\n";
    stream << "    const std::string vertical = style.verticalTextAlignment == \"Default\" ? defaultVertical : style.verticalTextAlignment;\n";
    stream << "    float lineTop = y;\n";
    stream << "    if (vertical == \"Center\") lineTop = y + std::max(0.0f, (height - totalHeight) * 0.5f);\n";
    stream << "    else if (vertical == \"Bottom\") lineTop = y + std::max(0.0f, height - totalHeight);\n";
    stream << "    canvas.saveState();\n";
    stream << "    canvas.setClampBounds(x, y, width, height);\n";
    stream << "    for (const auto& line : lines) {\n";
    stream << "        const float lineWidth = measuredTextWidth(font, line);\n";
    stream << "        float lineX = x;\n";
    stream << "        if (horizontal == \"Center\") lineX = x + std::max(0.0f, (width - lineWidth) * 0.5f);\n";
    stream << "        else if (horizontal == \"Right\") lineX = x + std::max(0.0f, width - lineWidth);\n";
    stream << "        canvas.text(line, font, visage::Font::kTopLeft, lineX, lineTop, std::max(width, lineWidth), lineHeight);\n";
    stream << "        lineTop += lineHeight;\n";
    stream << "    }\n";
    stream << "    canvas.restoreState();\n";
    stream << "}\n\n";
    stream << "void drawRuntimeWidget(visage::Canvas& canvas, const visage::Font& font, bool drawText, const RuntimeWidget& sourceWidget)\n";
    stream << "{\n";
    stream << "    RuntimeWidget widget = sourceWidget;\n";
    stream << "    applyActiveStateAppearance(widget);\n";
    stream << "    const float x = kFormOffsetX + widget.bounds.x;\n";
    stream << "    const float y = kFormOffsetY + widget.bounds.y;\n";
    stream << "    const float width = widget.bounds.width;\n";
    stream << "    const float height = widget.bounds.height;\n\n";
    stream << "    switch (widget.type) {\n";
    stream << "    case RuntimeWidgetType::Unknown:\n";
    stream << "        break;\n";
    stream << "    case RuntimeWidgetType::Frame:\n";
    stream << "        drawVisualBevel(canvas, x, y, width, height, widget, widget.style.fillColor, false);\n";
    stream << "        canvas.setColor(canvasColor(widget.enabled ? widget.style.panelColor : blendColor(widget.style.panelColor, widget.style.disabledColor, 0.55f)));\n";
    stream << "        canvas.fill(x + 2.0f, y + 2.0f, std::max(0.0f, width - 4.0f), std::min(24.0f, std::max(0.0f, height - 4.0f)));\n";
    stream << "        if (drawText) {\n";
    stream << "            canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "            drawRuntimeText(canvas, font, widget.style, widget.text.value, x + 8.0f, y + 6.0f, std::max(0.0f, width - 16.0f), 20.0f, \"Left\", \"Top\", true);\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    case RuntimeWidgetType::GroupBox:\n";
    stream << "        drawVisualBevel(canvas, x, y + 10.0f, width, std::max(0.0f, height - 10.0f), widget, widget.style.fillColor, false);\n";
    stream << "        if (drawText) {\n";
    stream << "            canvas.setColor(canvasColor(widget.enabled ? widget.style.fillColor : blendColor(widget.style.fillColor, widget.style.disabledColor, 0.55f)));\n";
    stream << "            canvas.fill(x + 12.0f, y, std::max(0.0f, width - 24.0f), 20.0f);\n";
    stream << "            canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "            drawRuntimeText(canvas, font, widget.style, widget.text.value, x + 10.0f, y, std::max(0.0f, width - 20.0f), 20.0f, \"Left\", \"Top\", true);\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    case RuntimeWidgetType::Panel:\n";
    stream << "        if (widget.style.borderThickness > 0.0f) {\n";
    stream << "            drawVisualBevel(canvas, x, y, width, height, widget, blendColor(widget.style.fillColor, widget.style.panelColor, 0.08f), false);\n";
    stream << "        }\n";
    stream << "        else {\n";
    stream << "            canvas.setColor(canvasColor(visualStateFill(widget, widget.style.fillColor, false)));\n";
    stream << "            canvas.fill(x, y, width, height);\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    case RuntimeWidgetType::Sizer:\n";
    stream << "        // Layout-only sizers are intentionally invisible at runtime.\n";
    stream << "        break;\n";
    stream << "    case RuntimeWidgetType::TabControl: {\n";
    stream << "        drawVisualRecessed(canvas, x, y, width, height, widget, widget.interaction.focused);\n";
    stream << "        const std::size_t tabCount = std::max<std::size_t>(1, widget.items.size());\n";
    stream << "        const float tabWidth = width / static_cast<float>(tabCount);\n";
    stream << "        const int selectedTab = std::clamp(widget.selectedTab, 0, static_cast<int>(tabCount) - 1);\n";
    stream << "        for (std::size_t index = 0; index < tabCount; ++index) {\n";
    stream << "            const float tabX = x + tabWidth * static_cast<float>(index);\n";
    stream << "            const RuntimeColor tabFill = static_cast<int>(index) == selectedTab\n";
    stream << "                ? blendColor(widget.style.fillColor, widget.style.accentColor, 0.18f)\n";
    stream << "                : blendColor(widget.style.fillColor, widget.style.panelColor, 0.35f);\n";
    stream << "            drawVisualBevel(canvas, tabX, y, tabWidth, kTabHeaderHeight, widget, tabFill, static_cast<int>(index) == selectedTab, static_cast<int>(index) == selectedTab);\n";
    stream << "            if (drawText) {\n";
    stream << "                canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "                const std::string label = index < widget.items.size() ? widget.items[index] : std::string{ \"Tab\" };\n";
    stream << "                drawRuntimeText(canvas, font, widget.style, label, tabX, y, tabWidth, kTabHeaderHeight, \"Center\", \"Center\", true);\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        canvas.setColor(canvasColor(widget.enabled ? widget.style.fillColor : blendColor(widget.style.fillColor, widget.style.disabledColor, 0.55f)));\n";
    stream << "        canvas.fill(x + 2.0f, y + kTabHeaderHeight - 1.0f, std::max(0.0f, width - 4.0f), std::max(0.0f, height - kTabHeaderHeight - 1.0f));\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::MenuBar: {\n";
    stream << "        drawVisualBevel(canvas, x, y, width, height, widget, blendColor(widget.style.panelColor, widget.style.fillColor, 0.35f), widget.interaction.pressed);\n";
    stream << "        const int selectedIndex = sanitizeItemIndex(widget, widget.selectedIndex);\n";
    stream << "        const float itemHeight = std::max(0.0f, height - 8.0f);\n";
    stream << "        float itemLeft = x + 6.0f;\n";
    stream << "        for (std::size_t index = 0; index < widget.items.size() && itemLeft < x + width - 6.0f; ++index) {\n";
    stream << "            const std::string& item = widget.items[index];\n";
    stream << "            const float idealWidth = std::max(56.0f, static_cast<float>(item.size()) * std::max(8.0f, widget.style.fontSize) * 0.70f + 36.0f);\n";
    stream << "            const float itemWidth = std::min(idealWidth, std::max(0.0f, x + width - 6.0f - itemLeft));\n";
    stream << "            if (static_cast<int>(index) == selectedIndex) {\n";
    stream << "                canvas.setColor(canvasColor(blendColor(widget.style.accentColor, widget.style.fillColor, 0.32f)));\n";
    stream << "                canvas.fill(itemLeft, y + 4.0f, itemWidth, itemHeight);\n";
    stream << "            }\n";
    stream << "            if (drawText) {\n";
    stream << "                canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "                canvas.text(item, font, visage::Font::kCenter, itemLeft + 4.0f, y + 4.0f, std::max(0.0f, itemWidth - 8.0f), itemHeight);\n";
    stream << "            }\n";
    stream << "            itemLeft += itemWidth + 4.0f;\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::ToolBar: {\n";
    stream << "        drawRoundedBox(canvas, x, y, width, height, blendColor(widget.style.panelColor, widget.style.fillColor, 0.22f), widget.style.borderColor, widget.style.borderThickness, widget.style.cornerRadius);\n";
    stream << "        const int selectedIndex = sanitizeItemIndex(widget, widget.selectedIndex);\n";
    stream << "        const float itemHeight = std::max(0.0f, height - 10.0f);\n";
    stream << "        float itemLeft = x + 6.0f;\n";
    stream << "        for (std::size_t index = 0; index < widget.items.size() && itemLeft < x + width - 6.0f; ++index) {\n";
    stream << "            const std::string& item = widget.items[index];\n";
    stream << "            const float idealWidth = std::max(64.0f, static_cast<float>(item.size()) * std::max(8.0f, widget.style.fontSize) * 0.70f + 42.0f);\n";
    stream << "            const float itemWidth = std::min(idealWidth, std::max(0.0f, x + width - 6.0f - itemLeft));\n";
    stream << "            const RuntimeColor buttonFill = static_cast<int>(index) == selectedIndex\n";
    stream << "                ? blendColor(widget.style.accentColor, widget.style.fillColor, 0.34f)\n";
    stream << "                : widget.style.fillColor;\n";
    stream << "            canvas.setColor(canvasColor(buttonFill));\n";
    stream << "            canvas.fill(itemLeft, y + 5.0f, itemWidth, itemHeight);\n";
    stream << "            drawBorder(canvas, itemLeft, y + 5.0f, itemWidth, itemHeight, widget.style.borderColor, widget.style.borderThickness);\n";
    stream << "            if (drawText) {\n";
    stream << "                canvas.setColor(canvasColor(widget.style.textColor));\n";
    stream << "                canvas.text(item, font, visage::Font::kCenter, itemLeft + 4.0f, y + 5.0f, std::max(0.0f, itemWidth - 8.0f), itemHeight);\n";
    stream << "            }\n";
    stream << "            itemLeft += itemWidth + 6.0f;\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::Label:\n";
    stream << "        if (drawText) {\n";
    stream << "            canvas.setColor(canvasColor(widget.style.textColor));\n";
    stream << "            const float padding = std::clamp(widget.style.textPadding, 0.0f, width * 0.45f);\n";
    stream << "            drawRuntimeText(canvas, font, widget.style, widget.text.value, x + padding, y + padding, std::max(0.0f, width - padding * 2.0f), std::max(0.0f, height - padding * 2.0f));\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    case RuntimeWidgetType::Button:\n";
    stream << "    {\n";
    stream << "        const bool buttonPressed = widget.interaction.pressed || (widget.button.toggleMode && widget.toggle.checked);\n";
    stream << "        const RuntimeColor fillColor = buttonPressed ? widget.button.pressedFillColor : widget.style.fillColor;\n";
    stream << "        const std::string normalText = !widget.button.normalText.empty()\n";
    stream << "            ? widget.button.normalText\n";
    stream << "            : widget.text.value;\n";
    stream << "        const std::string& buttonText = buttonPressed && !widget.button.pressedText.empty() ? widget.button.pressedText : normalText;\n";
    stream << "        drawVisualBevel(canvas, x, y, width, height, widget, fillColor, buttonPressed);\n";
    stream << "        if (drawText) {\n";
    stream << "            const float padding = std::clamp(widget.style.textPadding, 0.0f, std::min(width, height) * 0.45f);\n";
    stream << "            canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "            drawRuntimeText(canvas, font, widget.style, buttonText, x + padding + (buttonPressed ? 1.0f : 0.0f), y + padding + (buttonPressed ? 1.0f : 0.0f), std::max(0.0f, width - padding * 2.0f), std::max(0.0f, height - padding * 2.0f), \"Center\", \"Center\");\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::TextBox:\n";
    stream << "        drawVisualRecessed(canvas, x, y, width, height, widget, widget.interaction.focused);\n";
    stream << "        if (drawText) {\n";
    stream << "            const float padding = std::clamp(widget.style.textPadding, 0.0f, width * 0.45f);\n";
    stream << "            canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "            drawRuntimeText(canvas, font, widget.style, widget.text.value, x + padding, y + 4.0f, std::max(0.0f, width - padding * 2.0f), std::max(0.0f, height - 8.0f), \"Left\", \"Center\");\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    case RuntimeWidgetType::ComboBox: {\n";
    stream << "        const float arrowWidth = std::min(26.0f, std::max(20.0f, width * 0.18f));\n";
    stream << "        drawVisualRecessed(canvas, x, y, width, height, widget, widget.interaction.focused);\n";
    stream << "        drawVisualBevel(canvas, x + width - arrowWidth, y, arrowWidth, height, widget, blendColor(widget.style.panelColor, widget.style.fillColor, 0.22f), widget.interaction.pressed);\n";
    stream << "        canvas.setColor(canvasColor(widget.enabled ? visualTextColor(widget) : widget.style.disabledColor));\n";
    stream << "        const float arrowCenterX = x + width - arrowWidth * 0.5f;\n";
    stream << "        const float arrowCenterY = y + height * 0.5f;\n";
    stream << "        canvas.fill(arrowCenterX - 4.0f, arrowCenterY - 1.0f, 8.0f, 1.0f);\n";
    stream << "        canvas.fill(arrowCenterX - 3.0f, arrowCenterY, 6.0f, 1.0f);\n";
    stream << "        canvas.fill(arrowCenterX - 2.0f, arrowCenterY + 1.0f, 4.0f, 1.0f);\n";
    stream << "        if (drawText) {\n";
    stream << "            canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "            const std::string display = selectedItemText(widget);\n";
    stream << "            const float padding = std::clamp(widget.style.textPadding, 0.0f, width * 0.35f);\n";
    stream << "            canvas.text(display, font, visage::Font::kTopLeft, x + padding, y + std::max(0.0f, (height - std::max(8.0f, widget.style.fontSize) * 1.6f) * 0.5f), std::max(0.0f, width - arrowWidth - padding * 1.5f), std::max(0.0f, height - 8.0f));\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::ListBox: {\n";
    stream << "        const float rowHeight = listBoxRowHeight(widget);\n";
    stream << "        const float listTop = y + 4.0f;\n";
    stream << "        const float visibleHeight = std::max(0.0f, height - 8.0f);\n";
    stream << "        const std::size_t visibleCount = std::max<std::size_t>(1, static_cast<std::size_t>(std::floor(visibleHeight / rowHeight)));\n";
    stream << "        drawVisualRecessed(canvas, x, y, width, height, widget, widget.interaction.focused);\n";
    stream << "        float rowTop = listTop;\n";
    stream << "        for (std::size_t index = 0; index < std::min(visibleCount, widget.items.size()); ++index) {\n";
    stream << "            const bool selected = static_cast<int>(index) == sanitizeItemIndex(widget, widget.selectedIndex);\n";
    stream << "            const RuntimeColor rowFill = !widget.enabled\n";
    stream << "                ? blendColor(widget.style.fillColor, widget.style.disabledColor, 0.55f)\n";
    stream << "                : selected ? blendColor(widget.style.accentColor, widget.style.fillColor, 0.32f)\n";
    stream << "                           : (index % 2 == 0 ? widget.style.fillColor : blendColor(widget.style.panelColor, widget.style.fillColor, 0.18f));\n";
    stream << "            canvas.setColor(canvasColor(rowFill));\n";
    stream << "            canvas.fill(x + 4.0f, rowTop, std::max(0.0f, width - 14.0f), rowHeight - 1.0f);\n";
    stream << "            if (drawText) {\n";
    stream << "                canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "                const float padding = std::clamp(widget.style.textPadding, 0.0f, width * 0.35f);\n";
    stream << "                canvas.text(widget.items[index], font, visage::Font::kTopLeft, x + padding, rowTop + std::max(2.0f, (rowHeight - std::max(8.0f, widget.style.fontSize) * 1.4f) * 0.5f), std::max(0.0f, width - padding - 12.0f), std::max(0.0f, rowHeight - 4.0f));\n";
    stream << "            }\n";
    stream << "            rowTop += rowHeight;\n";
    stream << "        }\n";
    stream << "        if (widget.items.size() > visibleCount) {\n";
    stream << "            canvas.setColor(canvasColor(widget.style.panelColor));\n";
    stream << "            canvas.fill(x + width - 8.0f, y + 4.0f, 4.0f, std::max(0.0f, height - 8.0f));\n";
    stream << "            canvas.setColor(canvasColor(widget.style.accentColor));\n";
    stream << "            canvas.fill(x + width - 8.0f, y + 10.0f, 4.0f, std::max(16.0f, height * 0.22f));\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::TableGrid: {\n";
    stream << "        const bool showHeader = widget.showHeader;\n";
    stream << "        const bool showGridLines = widget.showGridLines;\n";
    stream << "        const float headerHeight = showHeader ? std::max(18.0f, widget.headerHeight) : 0.0f;\n";
    stream << "        const float rowHeight = std::max(16.0f, widget.rowHeight);\n";
    stream << "        const float contentX = x + 4.0f;\n";
    stream << "        const float contentY = y + 4.0f;\n";
    stream << "        const float contentWidth = std::max(0.0f, width - 8.0f);\n";
    stream << "        const float contentHeight = std::max(0.0f, height - 8.0f);\n";
    stream << "        const std::size_t columnCount = std::max<std::size_t>(1, widget.tableColumns.empty() ? 1 : widget.tableColumns.size());\n";
    stream << "        const float columnWidth = contentWidth / static_cast<float>(columnCount);\n";
    stream << "        const float visibleRowsHeight = std::max(0.0f, contentHeight - headerHeight);\n";
    stream << "        const std::size_t visibleRowCount = std::max<std::size_t>(1, static_cast<std::size_t>(std::floor(visibleRowsHeight / rowHeight)));\n";
    stream << "        drawRoundedBox(canvas, x, y, width, height, widget.style.fillColor, widget.style.borderColor, widget.style.borderThickness, widget.style.cornerRadius);\n";
    stream << "        if (showHeader) {\n";
    stream << "            canvas.setColor(canvasColor(blendColor(widget.style.panelColor, widget.style.fillColor, 0.18f)));\n";
    stream << "            canvas.fill(contentX, contentY, contentWidth, std::min(headerHeight, contentHeight));\n";
    stream << "        }\n";
    stream << "        for (std::size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {\n";
    stream << "            const float columnX = contentX + static_cast<float>(columnIndex) * columnWidth;\n";
    stream << "            const bool selectedColumn = static_cast<int>(columnIndex) == widget.selectedColumn;\n";
    stream << "            if (showHeader && selectedColumn) {\n";
    stream << "                canvas.setColor(canvasColor(blendColor(widget.style.accentColor, widget.style.fillColor, 0.24f)));\n";
    stream << "                canvas.fill(columnX, contentY, std::max(0.0f, columnWidth - 1.0f), std::min(headerHeight, contentHeight));\n";
    stream << "            }\n";
    stream << "            if (showGridLines && columnIndex > 0) {\n";
    stream << "                canvas.setColor(canvasColor(blendColor(widget.style.borderColor, widget.style.fillColor, 0.35f)));\n";
    stream << "                canvas.fill(columnX, contentY, 1.0f, contentHeight);\n";
    stream << "            }\n";
    stream << "            if (showHeader && drawText && columnIndex < widget.tableColumns.size()) {\n";
    stream << "                const std::string& headerText = widget.tableColumns[columnIndex];\n";
    stream << "                canvas.setColor(canvasColor(widget.style.textColor));\n";
    stream << "                canvas.text(headerText, font, visage::Font::kTopLeft, columnX + 6.0f, contentY + std::max(3.0f, (headerHeight - std::max(8.0f, widget.style.fontSize) * 1.3f) * 0.35f), std::max(0.0f, columnWidth - 12.0f), std::max(0.0f, headerHeight - 6.0f));\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        float rowTop = contentY + headerHeight;\n";
    stream << "        for (std::size_t rowIndex = 0; rowIndex < std::min<std::size_t>(visibleRowCount, widget.tableRows.size()); ++rowIndex) {\n";
    stream << "            const bool selectedRow = static_cast<int>(rowIndex) == widget.selectedRow;\n";
    stream << "            for (std::size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {\n";
    stream << "                const float columnX = contentX + static_cast<float>(columnIndex) * columnWidth;\n";
    stream << "                const bool selectedCell = selectedRow && static_cast<int>(columnIndex) == widget.selectedColumn;\n";
    stream << "                const RuntimeColor rowFill = selectedCell\n";
    stream << "                    ? blendColor(widget.style.accentColor, widget.style.fillColor, 0.30f)\n";
    stream << "                    : (selectedRow\n";
    stream << "                        ? blendColor(widget.style.accentColor, widget.style.fillColor, 0.16f)\n";
    stream << "                        : (rowIndex % 2 == 0 ? widget.style.fillColor : blendColor(widget.style.panelColor, widget.style.fillColor, 0.14f)));\n";
    stream << "                canvas.setColor(canvasColor(rowFill));\n";
    stream << "                canvas.fill(columnX, rowTop, std::max(0.0f, columnWidth - 1.0f), std::max(0.0f, rowHeight - 1.0f));\n";
    stream << "                if (drawText) {\n";
    stream << "                    canvas.setColor(canvasColor(widget.style.textColor));\n";
    stream << "                    canvas.text(tableGridCellText(widget, static_cast<int>(rowIndex), static_cast<int>(columnIndex)), font, visage::Font::kTopLeft, columnX + 6.0f, rowTop + std::max(2.0f, (rowHeight - std::max(8.0f, widget.style.fontSize) * 1.3f) * 0.35f), std::max(0.0f, columnWidth - 12.0f), std::max(0.0f, rowHeight - 4.0f));\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "            if (showGridLines) {\n";
    stream << "                canvas.setColor(canvasColor(blendColor(widget.style.borderColor, widget.style.fillColor, 0.35f)));\n";
    stream << "                canvas.fill(contentX, rowTop + rowHeight - 1.0f, contentWidth, 1.0f);\n";
    stream << "            }\n";
    stream << "            rowTop += rowHeight;\n";
    stream << "        }\n";
    stream << "        if (widget.tableRows.size() > visibleRowCount) {\n";
    stream << "            canvas.setColor(canvasColor(widget.style.panelColor));\n";
    stream << "            canvas.fill(x + width - 8.0f, y + 4.0f, 4.0f, std::max(0.0f, height - 8.0f));\n";
    stream << "            canvas.setColor(canvasColor(widget.style.accentColor));\n";
    stream << "            canvas.fill(x + width - 8.0f, y + 10.0f, 4.0f, std::max(16.0f, height * 0.22f));\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::CheckBox:\n";
    stream << "        drawVisualBevel(canvas, x + 6.0f, y + (height - 18.0f) * 0.5f, 18.0f, 18.0f, widget, widget.style.fillColor, widget.toggle.checked);\n";
    stream << "        if (widget.toggle.checked) {\n";
    stream << "            canvas.setColor(canvasColor(widget.enabled ? widget.style.accentColor : blendColor(widget.style.accentColor, widget.style.disabledColor, 0.62f)));\n";
    stream << "            canvas.fill(x + 10.0f, y + (height - 10.0f) * 0.5f, 10.0f, 10.0f);\n";
    stream << "            canvas.setColor(canvasColor(widget.style.highlightColor));\n";
    stream << "            canvas.fill(x + 11.0f, y + (height - 10.0f) * 0.5f + 1.0f, 8.0f, 1.0f);\n";
    stream << "        }\n";
    stream << "        if (drawText) {\n";
    stream << "            canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "            drawRuntimeText(canvas, font, widget.style, widget.text.value, x + 30.0f, y + 4.0f, std::max(0.0f, width - 30.0f - widget.style.textPadding), std::max(0.0f, height - 8.0f), \"Left\", \"Center\");\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    case RuntimeWidgetType::RadioButton: {\n";
    stream << "        const float centerX = x + 15.0f;\n";
    stream << "        const float centerY = y + height * 0.5f;\n";
    stream << "        const RuntimeColor border = widget.enabled ? widget.style.borderColor : blendColor(widget.style.borderColor, widget.style.disabledColor, 0.65f);\n";
    stream << "        const RuntimeColor well = visualStateFill(widget, blendColor(widget.style.fillColor, makeColor(0x00, 0x00, 0x00), 0.12f), widget.toggle.selected);\n";
    stream << "        fillCircleApprox(canvas, centerX, centerY, 9.0f, border);\n";
    stream << "        fillCircleApprox(canvas, centerX, centerY, std::max(2.0f, 9.0f - std::max(2.0f, widget.style.borderThickness + 1.0f)), well);\n";
    stream << "        fillCircleApprox(canvas, centerX - 1.0f, centerY - 1.0f, 5.0f, widget.style.highlightColor);\n";
    stream << "        fillCircleApprox(canvas, centerX, centerY, 4.0f, well);\n";
    stream << "        if (widget.toggle.selected) {\n";
    stream << "            fillCircleApprox(canvas, centerX, centerY, 4.0f, widget.enabled ? widget.style.accentColor : blendColor(widget.style.accentColor, widget.style.disabledColor, 0.62f));\n";
    stream << "        }\n";
    stream << "        if (widget.interaction.focused && widget.enabled) {\n";
    stream << "            drawBorder(canvas, x - 1.0f, y - 1.0f, width + 2.0f, height + 2.0f, widget.style.accentColor);\n";
    stream << "        }\n";
    stream << "        if (drawText) {\n";
    stream << "            canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "            drawRuntimeText(canvas, font, widget.style, widget.text.value, x + 30.0f, y + 4.0f, std::max(0.0f, width - 30.0f - widget.style.textPadding), std::max(0.0f, height - 8.0f), \"Left\", \"Center\");\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::Slider: {\n";
    stream << "        const float normalized = normalizedRangeValue(widget);\n";
    stream << "        const float trackLeft = x + 8.0f;\n";
    stream << "        const float trackWidth = std::max(0.0f, width - 16.0f);\n";
    stream << "        const float handleX = std::clamp(trackLeft + trackWidth * normalized - 6.0f, trackLeft - 6.0f, trackLeft + trackWidth - 6.0f);\n";
    stream << "        drawVisualRecessed(canvas, trackLeft, std::floor(y + height * 0.5f - 3.0f), trackWidth, 6.0f, widget, false);\n";
    stream << "        drawVisualBevel(canvas, handleX, std::floor(y + height * 0.5f - 9.0f), 12.0f, 18.0f, widget, widget.style.accentColor, widget.interaction.pressed);\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::ScrollBar: {\n";
    stream << "        const bool vertical = widget.range.orientation == RuntimeOrientation::Vertical;\n";
    stream << "        const float arrowSize = vertical ? std::min(width, 20.0f) : std::min(height, 20.0f);\n";
    stream << "        const RuntimeRect thumb = scrollBarThumbRectForWidget(widget);\n";
    stream << "        drawVisualRecessed(canvas, x, y, width, height, widget, false);\n";
    stream << "        if (vertical) {\n";
    stream << "            drawVisualBevel(canvas, x, y, width, arrowSize, widget, widget.style.fillColor, false);\n";
    stream << "            drawVisualBevel(canvas, x, y + height - arrowSize, width, arrowSize, widget, widget.style.fillColor, false);\n";
    stream << "        }\n";
    stream << "        else {\n";
    stream << "            drawVisualBevel(canvas, x, y, arrowSize, height, widget, widget.style.fillColor, false);\n";
    stream << "            drawVisualBevel(canvas, x + width - arrowSize, y, arrowSize, height, widget, widget.style.fillColor, false);\n";
    stream << "        }\n";
    stream << "        canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "        if (vertical) {\n";
    stream << "            canvas.fill(x + width * 0.5f - 3.0f, y + 6.0f, 6.0f, 3.0f);\n";
    stream << "            canvas.fill(x + width * 0.5f - 3.0f, y + height - 9.0f, 6.0f, 3.0f);\n";
    stream << "        }\n";
    stream << "        else {\n";
    stream << "            canvas.fill(x + 6.0f, y + height * 0.5f - 3.0f, 3.0f, 6.0f);\n";
    stream << "            canvas.fill(x + width - 9.0f, y + height * 0.5f - 3.0f, 3.0f, 6.0f);\n";
    stream << "        }\n";
    stream << "        drawVisualBevel(canvas, kFormOffsetX + thumb.x, kFormOffsetY + thumb.y, thumb.width, thumb.height, widget, widget.style.accentColor, widget.interaction.pressed);\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::StatusBar: {\n";
    stream << "        drawVisualRecessed(canvas, x, y, width, height, widget, false);\n";
    stream << "        if (drawText && !widget.items.empty()) {\n";
    stream << "            const float fieldWidth = width / static_cast<float>(widget.items.size());\n";
    stream << "            const float fieldInset = std::min(10.0f, std::max(6.0f, height * 0.16f));\n";
    stream << "            for (std::size_t index = 0; index < widget.items.size(); ++index) {\n";
    stream << "                canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "                drawRuntimeText(canvas, font, widget.style, widget.items[index], x + fieldWidth * static_cast<float>(index) + fieldInset, y + 2.0f, std::max(0.0f, fieldWidth - fieldInset * 2.0f), std::max(0.0f, height - 4.0f), \"Left\", \"Center\");\n";
    stream << "                if (index + 1 < widget.items.size()) {\n";
    stream << "                    canvas.setColor(canvasColor(widget.style.borderColor));\n";
    stream << "                    canvas.fill(x + fieldWidth * static_cast<float>(index + 1) - 1.0f, y + fieldInset * 0.5f, 1.0f, std::max(0.0f, height - fieldInset));\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::ModalDialog:\n";
    stream << "        break;\n";
    stream << "    case RuntimeWidgetType::ProgressBar: {\n";
    stream << "        const float normalized = normalizedRangeValue(widget);\n";
    stream << "        drawVisualRecessed(canvas, x, y, width, height, widget, false);\n";
    stream << "        const float inset = std::min(3.0f, std::max(1.0f, widget.style.borderThickness + 1.0f));\n";
    stream << "        const RuntimeColor progressFill = widget.enabled ? widget.style.accentColor : blendColor(widget.style.accentColor, widget.style.disabledColor, 0.62f);\n";
    stream << "        canvas.setColor(canvasColor(progressFill));\n";
    stream << "        canvas.fill(x + inset, y + inset, std::max(0.0f, width - inset * 2.0f) * normalized, std::max(0.0f, height - inset * 2.0f));\n";
    stream << "        const std::string display = progressText(widget);\n";
    stream << "        if (drawText && !display.empty()) {\n";
    stream << "            canvas.setColor(canvasColor(normalized >= 0.5f && widget.enabled ? makeColor(0xF8, 0xFB, 0xFF) : visualTextColor(widget)));\n";
    stream << "            canvas.text(display, font, visage::Font::kCenter, x, y, width, height);\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::ColorPicker: {\n";
    stream << "        drawVisualBevel(canvas, x, y, width, height, widget, widget.style.fillColor, widget.interaction.pressed);\n";
    stream << "        if (widget.interaction.focused && widget.enabled) {\n";
    stream << "            drawBorder(canvas, x - 1.0f, y - 1.0f, width + 2.0f, height + 2.0f, widget.style.accentColor);\n";
    stream << "        }\n";
    stream << "        const RuntimeColor swatch = parseColorOrDefault(widget.colorValue, widget.style.accentColor);\n";
    stream << "        canvas.setColor(canvasColor(widget.enabled ? swatch : blendColor(swatch, widget.style.disabledColor, 0.58f)));\n";
    stream << "        canvas.fill(x + 6.0f, y + 6.0f, 22.0f, std::max(16.0f, height - 12.0f));\n";
    stream << "        drawBorder(canvas, x + 6.0f, y + 6.0f, 22.0f, std::max(16.0f, height - 12.0f), widget.style.borderColor, widget.style.borderThickness);\n";
    stream << "        if (drawText) {\n";
    stream << "            canvas.setColor(canvasColor(visualTextColor(widget)));\n";
    stream << "            const std::string label = widget.text.showText && !widget.text.value.empty() ? widget.text.value + \"  \" + widget.colorValue : widget.colorValue;\n";
    stream << "            canvas.text(label, font, visage::Font::kTopLeft, x + 36.0f, y + 6.0f, std::max(0.0f, width - 42.0f), std::max(0.0f, height - 8.0f));\n";
    stream << "        }\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::Image:\n";
    stream << "        drawRoundedBox(canvas, x, y, width, height, widget.style.fillColor, widget.style.borderColor, widget.style.borderThickness, widget.style.cornerRadius);\n";
    stream << "        break;\n";
    stream << "    case RuntimeWidgetType::Spacer:\n";
    stream << "        break;\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "} // namespace\n\n";
    stream << className << "::" << className << "()\n";
    stream << "{\n";
    stream << "    formBounds_ = RuntimeRect{ 0.0f, 0.0f, " << emitFloat(document.root.bounds.width) << ", " << emitFloat(document.root.bounds.height) << " };\n";
    stream << "    formTitle_ = " << emitStringLiteral(windowTitle) << ";\n";
    stream << "    formPanelColor_ = " << emitRuntimeColorLiteral(rootStyle.panelColor, "makeColor(0x2B, 0x31, 0x3D)") << ";\n";
    stream << "    formFillColor_ = " << emitRuntimeColorLiteral(rootStyle.fillColor, "makeColor(0x1F, 0x24, 0x2D)") << ";\n";
    stream << "    formTextColor_ = " << emitRuntimeColorLiteral(rootStyle.textColor, "makeColor(0xEE, 0xF2, 0xF8)") << ";\n";
    stream << "    formBorderColor_ = " << emitRuntimeColorLiteral(rootStyle.borderColor, "makeColor(0x97, 0xA3, 0xB7)") << ";\n";
    stream << "    formBorderThickness_ = " << emitFloat(rootStyle.borderThickness) << ";\n";
    stream << "    initializeRuntimeWidgets();\n";
    stream << "    applyRuntimeSizerLayouts();\n";
    stream << "    setTitle(" << emitStringLiteral(windowTitle) << ");\n";
    stream << "\n";
    stream << "    static constexpr std::array<const char*, 3> kFontCandidates = {\n";
    stream << "        \"C:/Windows/Fonts/segoeui.ttf\",\n";
    stream << "        \"C:/Windows/Fonts/tahoma.ttf\",\n";
    stream << "        \"C:/Windows/Fonts/arial.ttf\"\n";
    stream << "    };\n\n";
    stream << "    for (const char* fontPath : kFontCandidates) {\n";
    stream << "        std::ifstream fontFile(fontPath, std::ios::binary);\n";
    stream << "        if (!fontFile.good()) {\n";
    stream << "            continue;\n";
    stream << "        }\n\n";
    stream << "        labelFont_ = visage::Font(18.0f, std::string{ fontPath });\n";
    stream << "        if (canDrawText()) {\n";
    stream << "            break;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "void " << className << "::showWindow()\n";
    stream << "{\n";
    stream << "    show(visage::Dimension::logicalPixels(" << emitFloat(windowWidth) << "), visage::Dimension::logicalPixels(" << emitFloat(windowHeight) << "));\n";
    for (const auto& binding : bindings) {
        if (binding.widgetId == document.root.id && binding.eventKey == "onLoad" && binding.signature == HandlerSignature::Void) {
            stream << "    " << binding.handlerName << "(" << widgetEventLiteral(binding) << ");\n";
        }
    }
    stream << "    if (!modalState_.visible) {\n";
    stream << "        for (const auto& widget : runtimeWidgets_) {\n";
    stream << "            if (widget.type == RuntimeWidgetType::ModalDialog && widget.visibleAtStartup) {\n";
    stream << "                showModalDialog(widget.id);\n";
    stream << "                break;\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "void " << className << "::initializeRuntimeWidgets()\n";
    stream << "{\n";
    stream << "    runtimeWidgets_.clear();\n";
    for (const auto& runtimeWidget : runtimeWidgets) {
        emitRuntimeWidgetInitialization(stream, runtimeWidget, 1);
    }
    stream << "}\n\n";
    stream << "void " << className << "::applyRuntimeSizerLayouts()\n";
    stream << "{\n";
    stream << "    auto childrenOf = [this](const std::string& parentId) {\n";
    stream << "        std::vector<RuntimeWidget*> children;\n";
    stream << "        for (auto& widget : runtimeWidgets_) {\n";
    stream << "            if (widget.parentId == parentId) {\n";
    stream << "                children.push_back(&widget);\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        return children;\n";
    stream << "    };\n\n";
    stream << "    auto minimumSize = [&](const RuntimeWidget& widget, const auto& minimumSizeRef) -> std::pair<float, float> {\n";
    stream << "        if (widget.type == RuntimeWidgetType::Sizer) {\n";
    stream << "            const bool horizontal = widget.range.orientation == RuntimeOrientation::Horizontal;\n";
    stream << "            float mainTotal = 0.0f;\n";
    stream << "            float crossMax = 0.0f;\n";
    stream << "            int count = 0;\n";
    stream << "            for (auto* child : childrenOf(widget.id)) {\n";
    stream << "                if (!child->sizerItemShown) {\n";
    stream << "                    continue;\n";
    stream << "                }\n";
    stream << "                auto [childWidth, childHeight] = minimumSizeRef(*child, minimumSizeRef);\n";
    stream << "                childWidth = child->sizerItemMinimumWidth >= 0 ? std::max(childWidth, static_cast<float>(child->sizerItemMinimumWidth)) : childWidth;\n";
    stream << "                childHeight = child->sizerItemMinimumHeight >= 0 ? std::max(childHeight, static_cast<float>(child->sizerItemMinimumHeight)) : childHeight;\n";
    stream << "                const float left = (child->sizerItemBorderSides & 1) != 0 ? static_cast<float>(child->sizerItemBorder) : 0.0f;\n";
    stream << "                const float top = (child->sizerItemBorderSides & 2) != 0 ? static_cast<float>(child->sizerItemBorder) : 0.0f;\n";
    stream << "                const float right = (child->sizerItemBorderSides & 4) != 0 ? static_cast<float>(child->sizerItemBorder) : 0.0f;\n";
    stream << "                const float bottom = (child->sizerItemBorderSides & 8) != 0 ? static_cast<float>(child->sizerItemBorder) : 0.0f;\n";
    stream << "                const float main = horizontal ? childWidth + left + right : childHeight + top + bottom;\n";
    stream << "                const float cross = horizontal ? childHeight + top + bottom : childWidth + left + right;\n";
    stream << "                mainTotal += main;\n";
    stream << "                crossMax = std::max(crossMax, cross);\n";
    stream << "                ++count;\n";
    stream << "            }\n";
    stream << "            const float gapTotal = static_cast<float>(std::max(0, count - 1) * widget.sizerGap);\n";
    stream << "            if (horizontal) {\n";
    stream << "                return { static_cast<float>(widget.sizerPaddingLeft + widget.sizerPaddingRight) + mainTotal + gapTotal, static_cast<float>(widget.sizerPaddingTop + widget.sizerPaddingBottom) + crossMax };\n";
    stream << "            }\n";
    stream << "            return { static_cast<float>(widget.sizerPaddingLeft + widget.sizerPaddingRight) + crossMax, static_cast<float>(widget.sizerPaddingTop + widget.sizerPaddingBottom) + mainTotal + gapTotal };\n";
    stream << "        }\n";
    stream << "        if (widget.type == RuntimeWidgetType::Spacer) {\n";
    stream << "            const float size = widget.spacerStretch ? 0.0f : static_cast<float>(std::max(0, widget.spacerSize));\n";
    stream << "            return { size, size };\n";
    stream << "        }\n";
    stream << "        return { std::max(0.0f, widget.preferredWidth), std::max(0.0f, widget.preferredHeight) };\n";
    stream << "    };\n\n";
    stream << "    auto layoutSizer = [&](RuntimeWidget& sizer, const auto& layoutSizerRef) -> void {\n";
    stream << "        if (sizer.type != RuntimeWidgetType::Sizer) {\n";
    stream << "            return;\n";
    stream << "        }\n";
    stream << "        const bool horizontal = sizer.range.orientation == RuntimeOrientation::Horizontal;\n";
    stream << "        auto children = childrenOf(sizer.id);\n";
    stream << "        children.erase(std::remove_if(children.begin(), children.end(), [](const RuntimeWidget* child) { return !child->sizerItemShown; }), children.end());\n";
    stream << "        if (children.empty()) {\n";
    stream << "            return;\n";
    stream << "        }\n";
    stream << "        const RuntimeRect content{ sizer.bounds.x + static_cast<float>(sizer.sizerPaddingLeft), sizer.bounds.y + static_cast<float>(sizer.sizerPaddingTop), std::max(0.0f, sizer.bounds.width - static_cast<float>(sizer.sizerPaddingLeft + sizer.sizerPaddingRight)), std::max(0.0f, sizer.bounds.height - static_cast<float>(sizer.sizerPaddingTop + sizer.sizerPaddingBottom)) };\n";
    stream << "        const float availableMain = std::max(0.0f, (horizontal ? content.width : content.height) - static_cast<float>(std::max(0, static_cast<int>(children.size()) - 1) * sizer.sizerGap));\n";
    stream << "        const float availableCross = std::max(0.0f, horizontal ? content.height : content.width);\n";
    stream << "        struct RuntimeLayoutSlot { RuntimeWidget* child; float mainMinimum; float crossMinimum; float assignedMain; };\n";
    stream << "        std::vector<RuntimeLayoutSlot> slots;\n";
    stream << "        slots.reserve(children.size());\n";
    stream << "        float minimumMainTotal = 0.0f;\n";
    stream << "        int totalProportion = 0;\n";
    stream << "        for (auto* child : children) {\n";
    stream << "            auto [childWidth, childHeight] = minimumSize(*child, minimumSize);\n";
    stream << "            childWidth = child->sizerItemMinimumWidth >= 0 ? std::max(childWidth, static_cast<float>(child->sizerItemMinimumWidth)) : childWidth;\n";
    stream << "            childHeight = child->sizerItemMinimumHeight >= 0 ? std::max(childHeight, static_cast<float>(child->sizerItemMinimumHeight)) : childHeight;\n";
    stream << "            const float left = (child->sizerItemBorderSides & 1) != 0 ? static_cast<float>(child->sizerItemBorder) : 0.0f;\n";
    stream << "            const float top = (child->sizerItemBorderSides & 2) != 0 ? static_cast<float>(child->sizerItemBorder) : 0.0f;\n";
    stream << "            const float right = (child->sizerItemBorderSides & 4) != 0 ? static_cast<float>(child->sizerItemBorder) : 0.0f;\n";
    stream << "            const float bottom = (child->sizerItemBorderSides & 8) != 0 ? static_cast<float>(child->sizerItemBorder) : 0.0f;\n";
    stream << "            const float mainMinimum = horizontal ? childWidth + left + right : childHeight + top + bottom;\n";
    stream << "            const float crossMinimum = horizontal ? childHeight + top + bottom : childWidth + left + right;\n";
    stream << "            slots.push_back({ child, mainMinimum, crossMinimum, mainMinimum });\n";
    stream << "            minimumMainTotal += mainMinimum;\n";
    stream << "            totalProportion += std::max(0, child->sizerItemProportion);\n";
    stream << "        }\n";
    stream << "        const int integerExtra = std::max(0, static_cast<int>(std::floor(std::max(0.0f, availableMain - minimumMainTotal))));\n";
    stream << "        int assignedExtra = 0;\n";
    stream << "        if (totalProportion > 0) {\n";
    stream << "            for (auto& slot : slots) {\n";
    stream << "                if (slot.child->sizerItemProportion <= 0) {\n";
    stream << "                    continue;\n";
    stream << "                }\n";
    stream << "                const int share = integerExtra * slot.child->sizerItemProportion / totalProportion;\n";
    stream << "                slot.assignedMain += static_cast<float>(share);\n";
    stream << "                assignedExtra += share;\n";
    stream << "            }\n";
    stream << "            int remainder = integerExtra - assignedExtra;\n";
    stream << "            for (auto& slot : slots) {\n";
    stream << "                if (remainder <= 0) {\n";
    stream << "                    break;\n";
    stream << "                }\n";
    stream << "                if (slot.child->sizerItemProportion > 0) {\n";
    stream << "                    slot.assignedMain += 1.0f;\n";
    stream << "                    --remainder;\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        float cursor = horizontal ? content.x : content.y;\n";
    stream << "        for (auto& slot : slots) {\n";
    stream << "            RuntimeWidget& child = *slot.child;\n";
    stream << "            const float left = (child.sizerItemBorderSides & 1) != 0 ? static_cast<float>(child.sizerItemBorder) : 0.0f;\n";
    stream << "            const float top = (child.sizerItemBorderSides & 2) != 0 ? static_cast<float>(child.sizerItemBorder) : 0.0f;\n";
    stream << "            const float right = (child.sizerItemBorderSides & 4) != 0 ? static_cast<float>(child.sizerItemBorder) : 0.0f;\n";
    stream << "            const float bottom = (child.sizerItemBorderSides & 8) != 0 ? static_cast<float>(child.sizerItemBorder) : 0.0f;\n";
    stream << "            const float mainBorder = horizontal ? left + right : top + bottom;\n";
    stream << "            const float crossBorder = horizontal ? top + bottom : left + right;\n";
    stream << "            const float mainLength = std::max(0.0f, slot.assignedMain - mainBorder);\n";
    stream << "            const float crossLength = std::max(0.0f, availableCross - crossBorder);\n";
    stream << "            auto [minimumWidth, minimumHeight] = minimumSize(child, minimumSize);\n";
    stream << "            minimumWidth = child.sizerItemMinimumWidth >= 0 ? std::max(minimumWidth, static_cast<float>(child.sizerItemMinimumWidth)) : minimumWidth;\n";
    stream << "            minimumHeight = child.sizerItemMinimumHeight >= 0 ? std::max(minimumHeight, static_cast<float>(child.sizerItemMinimumHeight)) : minimumHeight;\n";
    stream << "            const float preferredCross = std::min(horizontal ? minimumHeight : minimumWidth, crossLength);\n";
    stream << "            float finalCrossStart = (horizontal ? content.y + top : content.x + left);\n";
    stream << "            float finalCrossLength = crossLength;\n";
    stream << "            if (!child.sizerItemExpand) {\n";
    stream << "                finalCrossLength = preferredCross;\n";
    stream << "                const float remainingCross = std::max(0.0f, crossLength - preferredCross);\n";
    stream << "                if (child.sizerItemAlignment == RuntimeSizerAlignment::Center) {\n";
    stream << "                    finalCrossStart += std::floor(remainingCross * 0.5f);\n";
    stream << "                }\n";
    stream << "                else if (child.sizerItemAlignment == RuntimeSizerAlignment::End) {\n";
    stream << "                    finalCrossStart += remainingCross;\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "            if (horizontal) {\n";
    stream << "                child.bounds = RuntimeRect{ cursor + left, finalCrossStart, mainLength, finalCrossLength };\n";
    stream << "            }\n";
    stream << "            else {\n";
    stream << "                child.bounds = RuntimeRect{ finalCrossStart, cursor + top, finalCrossLength, mainLength };\n";
    stream << "            }\n";
    stream << "            if (child.type == RuntimeWidgetType::Sizer) {\n";
    stream << "                layoutSizerRef(child, layoutSizerRef);\n";
    stream << "            }\n";
    stream << "            cursor += slot.assignedMain + static_cast<float>(sizer.sizerGap);\n";
    stream << "        }\n";
    stream << "    };\n\n";
    stream << "    for (auto& widget : runtimeWidgets_) {\n";
    stream << "        if (widget.type == RuntimeWidgetType::Sizer && (widget.parentId.empty() || findWidgetById(widget.parentId) == nullptr || findWidgetById(widget.parentId)->type != RuntimeWidgetType::Sizer)) {\n";
    stream << "            layoutSizer(widget, layoutSizer);\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "RuntimeWidget* " << className << "::findWidgetById(const std::string& id)\n";
    stream << "{\n";
    stream << "    for (auto& widget : runtimeWidgets_) {\n";
    stream << "        if (widget.id == id) {\n";
    stream << "            return &widget;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    return nullptr;\n";
    stream << "}\n\n";
    stream << "const RuntimeWidget* " << className << "::findWidgetById(const std::string& id) const\n";
    stream << "{\n";
    stream << "    for (const auto& widget : runtimeWidgets_) {\n";
    stream << "        if (widget.id == id) {\n";
    stream << "            return &widget;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    return nullptr;\n";
    stream << "}\n\n";
    stream << "RuntimeWidget* " << className << "::findWidgetByName(const std::string& name)\n";
    stream << "{\n";
    stream << "    for (auto& widget : runtimeWidgets_) {\n";
    stream << "        if (widget.name == name) {\n";
    stream << "            return &widget;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    return nullptr;\n";
    stream << "}\n\n";
    stream << "const RuntimeWidget* " << className << "::findWidgetByName(const std::string& name) const\n";
    stream << "{\n";
    stream << "    for (const auto& widget : runtimeWidgets_) {\n";
    stream << "        if (widget.name == name) {\n";
    stream << "            return &widget;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    return nullptr;\n";
    stream << "}\n\n";
    stream << "RuntimeWidget* " << className << "::findWidgetByIdOrName(const std::string& idOrName)\n";
    stream << "{\n";
    stream << "    if (RuntimeWidget* widget = findWidgetById(idOrName); widget != nullptr) {\n";
    stream << "        return widget;\n";
    stream << "    }\n";
    stream << "    return findWidgetByName(idOrName);\n";
    stream << "}\n\n";
    stream << "const RuntimeWidget* " << className << "::findWidgetByIdOrName(const std::string& idOrName) const\n";
    stream << "{\n";
    stream << "    if (const RuntimeWidget* widget = findWidgetById(idOrName); widget != nullptr) {\n";
    stream << "        return widget;\n";
    stream << "    }\n";
    stream << "    return findWidgetByName(idOrName);\n";
    stream << "}\n\n";
    stream << "RuntimeWidget* " << className << "::activeModalWidget()\n";
    stream << "{\n";
    stream << "    return modalState_.dialogId.empty() ? nullptr : findWidgetById(modalState_.dialogId);\n";
    stream << "}\n\n";
    stream << "const RuntimeWidget* " << className << "::activeModalWidget() const\n";
    stream << "{\n";
    stream << "    return modalState_.dialogId.empty() ? nullptr : findWidgetById(modalState_.dialogId);\n";
    stream << "}\n\n";
    stream << "bool " << className << "::isWidgetVisible(const RuntimeWidget& widget) const\n";
    stream << "{\n";
    stream << "    const RuntimeWidget* current = &widget;\n";
    stream << "    std::size_t guard = 0;\n";
    stream << "    while (!current->parentId.empty()) {\n";
    stream << "        if (guard++ > runtimeWidgets_.size()) {\n";
    stream << "            return false;\n";
    stream << "        }\n";
    stream << "        const RuntimeWidget* parent = findWidgetById(current->parentId);\n";
    stream << "        if (parent == nullptr) {\n";
    stream << "            return true;\n";
    stream << "        }\n";
    stream << "        if (parent->type == RuntimeWidgetType::TabControl) {\n";
    stream << "            const int tabCount = std::max(1, static_cast<int>(parent->items.size()));\n";
    stream << "            const int selectedTab = std::clamp(parent->selectedTab, 0, tabCount - 1);\n";
    stream << "            if (current->tabIndex != selectedTab) {\n";
    stream << "                return false;\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        current = parent;\n";
    stream << "    }\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "std::optional<int> " << className << "::hitTestTabHeader(const RuntimeWidget& widget, float x, float y) const\n";
    stream << "{\n";
    stream << "    if (widget.type != RuntimeWidgetType::TabControl) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    const std::size_t tabCount = std::max<std::size_t>(1, widget.items.size());\n";
    stream << "    if (x < widget.bounds.x || x > widget.bounds.x + widget.bounds.width\n";
    stream << "        || y < widget.bounds.y || y > widget.bounds.y + std::min(kTabHeaderHeight, widget.bounds.height)) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    const float tabWidth = widget.bounds.width / static_cast<float>(tabCount);\n";
    stream << "    const int tabIndex = std::clamp(static_cast<int>((x - widget.bounds.x) / std::max(1.0f, tabWidth)), 0, static_cast<int>(tabCount) - 1);\n";
    stream << "    return tabIndex;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::setText(const std::string& idOrName, const std::string& text)\n";
    stream << "{\n";
    stream << "    RuntimeWidget* widget = findWidgetByIdOrName(idOrName);\n";
    stream << "    if (widget == nullptr) {\n";
    stream << "        return false;\n";
    stream << "    }\n\n";
    stream << "    switch (widget->type) {\n";
    stream << "    case RuntimeWidgetType::Unknown:\n";
    stream << "    case RuntimeWidgetType::Slider:\n";
    stream << "    case RuntimeWidgetType::ScrollBar:\n";
    stream << "    case RuntimeWidgetType::Image:\n";
    stream << "    case RuntimeWidgetType::Sizer:\n";
    stream << "    case RuntimeWidgetType::Spacer:\n";
    stream << "        return false;\n";
    stream << "    case RuntimeWidgetType::TextBox:\n";
    stream << "        if (!updateTextBoxText(*widget, text, false)) {\n";
    stream << "            return false;\n";
    stream << "        }\n";
    stream << "        requestGeneratedUiRepaint();\n";
    stream << "        return true;\n";
    stream << "    case RuntimeWidgetType::StatusBar:\n";
    stream << "        if (widget->items.empty()) {\n";
    stream << "            widget->items.resize(1);\n";
    stream << "        }\n";
    stream << "        if (widget->items.front() == text) {\n";
    stream << "            return false;\n";
    stream << "        }\n";
    stream << "        widget->items.front() = text;\n";
    stream << "        requestGeneratedUiRepaint();\n";
    stream << "        return true;\n";
    stream << "    case RuntimeWidgetType::Label:\n";
    stream << "    case RuntimeWidgetType::Button:\n";
    stream << "    case RuntimeWidgetType::CheckBox:\n";
    stream << "    case RuntimeWidgetType::RadioButton:\n";
    stream << "    case RuntimeWidgetType::ModalDialog:\n";
    stream << "    case RuntimeWidgetType::ProgressBar:\n";
    stream << "    case RuntimeWidgetType::Frame:\n";
    stream << "    case RuntimeWidgetType::ColorPicker:\n";
    stream << "        if (widget->text.value == text) {\n";
    stream << "            return false;\n";
    stream << "        }\n";
    stream << "        widget->text.value = text;\n";
    stream << "        if (modalState_.visible && modalState_.dialogId == widget->id) {\n";
    stream << "            modalState_.message = text;\n";
    stream << "        }\n";
    stream << "        requestGeneratedUiRepaint();\n";
    stream << "        return true;\n";
    stream << "    }\n\n";
    stream << "    return false;\n";
    stream << "}\n\n";
    stream << "std::optional<std::string> " << className << "::getText(const std::string& idOrName) const\n";
    stream << "{\n";
    stream << "    const RuntimeWidget* widget = findWidgetByIdOrName(idOrName);\n";
    stream << "    if (widget == nullptr) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n\n";
    stream << "    switch (widget->type) {\n";
    stream << "    case RuntimeWidgetType::StatusBar:\n";
    stream << "        return widget->items.empty() ? std::optional<std::string>{ std::string{} } : std::optional<std::string>{ widget->items.front() };\n";
    stream << "    case RuntimeWidgetType::Label:\n";
    stream << "    case RuntimeWidgetType::Button:\n";
    stream << "    case RuntimeWidgetType::TextBox:\n";
    stream << "    case RuntimeWidgetType::CheckBox:\n";
    stream << "    case RuntimeWidgetType::RadioButton:\n";
    stream << "    case RuntimeWidgetType::ModalDialog:\n";
    stream << "    case RuntimeWidgetType::ProgressBar:\n";
    stream << "    case RuntimeWidgetType::Frame:\n";
    stream << "    case RuntimeWidgetType::ColorPicker:\n";
    stream << "        return widget->text.value;\n";
    stream << "    case RuntimeWidgetType::Unknown:\n";
    stream << "    case RuntimeWidgetType::Slider:\n";
    stream << "    case RuntimeWidgetType::ScrollBar:\n";
    stream << "    case RuntimeWidgetType::Image:\n";
    stream << "    case RuntimeWidgetType::Sizer:\n";
    stream << "    case RuntimeWidgetType::Spacer:\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n\n";
    stream << "    return std::nullopt;\n";
    stream << "}\n\n";
    stream << "std::string " << className << "::getTextOr(const std::string& idOrName, std::string fallback) const\n";
    stream << "{\n";
    stream << "    if (const auto value = getText(idOrName)) {\n";
    stream << "        return *value;\n";
    stream << "    }\n";
    stream << "    return fallback;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::setChecked(const std::string& idOrName, bool checked)\n";
    stream << "{\n";
    stream << "    RuntimeWidget* widget = findWidgetByIdOrName(idOrName);\n";
    stream << "    if (widget == nullptr || widget->type != RuntimeWidgetType::CheckBox || widget->toggle.checked == checked) {\n";
    stream << "        return false;\n";
    stream << "    }\n\n";
    stream << "    widget->toggle.checked = checked;\n";
    stream << "    requestGeneratedUiRepaint();\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "std::optional<bool> " << className << "::getChecked(const std::string& idOrName) const\n";
    stream << "{\n";
    stream << "    const RuntimeWidget* widget = findWidgetByIdOrName(idOrName);\n";
    stream << "    if (widget == nullptr || widget->type != RuntimeWidgetType::CheckBox) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    return widget->toggle.checked;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::getCheckedOr(const std::string& idOrName, bool fallback) const\n";
    stream << "{\n";
    stream << "    if (const auto value = getChecked(idOrName)) {\n";
    stream << "        return *value;\n";
    stream << "    }\n";
    stream << "    return fallback;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::setSelected(const std::string& idOrName, bool selected)\n";
    stream << "{\n";
    stream << "    RuntimeWidget* widget = findWidgetByIdOrName(idOrName);\n";
    stream << "    if (widget == nullptr || widget->type != RuntimeWidgetType::RadioButton) {\n";
    stream << "        return false;\n";
    stream << "    }\n\n";
    stream << "    bool changed = false;\n";
    stream << "    if (selected) {\n";
    stream << "        for (auto& candidate : runtimeWidgets_) {\n";
    stream << "            if (candidate.type == RuntimeWidgetType::RadioButton && candidate.toggle.group == widget->toggle.group) {\n";
    stream << "                const bool shouldBeSelected = candidate.id == widget->id;\n";
    stream << "                if (candidate.toggle.selected != shouldBeSelected) {\n";
    stream << "                    candidate.toggle.selected = shouldBeSelected;\n";
    stream << "                    changed = true;\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    else if (widget->toggle.selected) {\n";
    stream << "        widget->toggle.selected = false;\n";
    stream << "        changed = true;\n";
    stream << "    }\n\n";
    stream << "    if (changed) {\n";
    stream << "        requestGeneratedUiRepaint();\n";
    stream << "    }\n";
    stream << "    return changed;\n";
    stream << "}\n\n";
    stream << "std::optional<bool> " << className << "::getSelected(const std::string& idOrName) const\n";
    stream << "{\n";
    stream << "    const RuntimeWidget* widget = findWidgetByIdOrName(idOrName);\n";
    stream << "    if (widget == nullptr || widget->type != RuntimeWidgetType::RadioButton) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    return widget->toggle.selected;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::getSelectedOr(const std::string& idOrName, bool fallback) const\n";
    stream << "{\n";
    stream << "    if (const auto value = getSelected(idOrName)) {\n";
    stream << "        return *value;\n";
    stream << "    }\n";
    stream << "    return fallback;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::setValue(const std::string& idOrName, float value)\n";
    stream << "{\n";
    stream << "    RuntimeWidget* widget = findWidgetByIdOrName(idOrName);\n";
    stream << "    if (widget == nullptr) {\n";
    stream << "        return false;\n";
    stream << "    }\n\n";
    stream << "    switch (widget->type) {\n";
    stream << "    case RuntimeWidgetType::Slider:\n";
    stream << "    case RuntimeWidgetType::ScrollBar:\n";
    stream << "    case RuntimeWidgetType::ProgressBar:\n";
    stream << "        if (!setWidgetValue(*widget, value, false)) {\n";
    stream << "            return false;\n";
    stream << "        }\n";
    stream << "        requestGeneratedUiRepaint();\n";
    stream << "        return true;\n";
    stream << "    case RuntimeWidgetType::Unknown:\n";
    stream << "    case RuntimeWidgetType::Label:\n";
    stream << "    case RuntimeWidgetType::Button:\n";
    stream << "    case RuntimeWidgetType::TextBox:\n";
    stream << "    case RuntimeWidgetType::CheckBox:\n";
    stream << "    case RuntimeWidgetType::RadioButton:\n";
    stream << "    case RuntimeWidgetType::StatusBar:\n";
    stream << "    case RuntimeWidgetType::Frame:\n";
    stream << "    case RuntimeWidgetType::Image:\n";
    stream << "    case RuntimeWidgetType::Sizer:\n";
    stream << "    case RuntimeWidgetType::Spacer:\n";
    stream << "    case RuntimeWidgetType::ColorPicker:\n";
    stream << "        return false;\n";
    stream << "    }\n\n";
    stream << "    return false;\n";
    stream << "}\n\n";
    stream << "std::optional<float> " << className << "::getValue(const std::string& idOrName) const\n";
    stream << "{\n";
    stream << "    const RuntimeWidget* widget = findWidgetByIdOrName(idOrName);\n";
    stream << "    if (widget == nullptr) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n\n";
    stream << "    switch (widget->type) {\n";
    stream << "    case RuntimeWidgetType::Slider:\n";
    stream << "    case RuntimeWidgetType::ScrollBar:\n";
    stream << "    case RuntimeWidgetType::ProgressBar:\n";
    stream << "        return widget->range.value;\n";
    stream << "    case RuntimeWidgetType::Unknown:\n";
    stream << "    case RuntimeWidgetType::Label:\n";
    stream << "    case RuntimeWidgetType::Button:\n";
    stream << "    case RuntimeWidgetType::TextBox:\n";
    stream << "    case RuntimeWidgetType::CheckBox:\n";
    stream << "    case RuntimeWidgetType::RadioButton:\n";
    stream << "    case RuntimeWidgetType::StatusBar:\n";
    stream << "    case RuntimeWidgetType::Frame:\n";
    stream << "    case RuntimeWidgetType::Image:\n";
    stream << "    case RuntimeWidgetType::Sizer:\n";
    stream << "    case RuntimeWidgetType::Spacer:\n";
    stream << "    case RuntimeWidgetType::ColorPicker:\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n\n";
    stream << "    return std::nullopt;\n";
    stream << "}\n\n";
    stream << "float " << className << "::getValueOr(const std::string& idOrName, float fallback) const\n";
    stream << "{\n";
    stream << "    if (const auto value = getValue(idOrName)) {\n";
    stream << "        return *value;\n";
    stream << "    }\n";
    stream << "    return fallback;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::setProgressValue(const std::string& idOrName, float value)\n";
    stream << "{\n";
    stream << "    return setValue(idOrName, value);\n";
    stream << "}\n\n";
    stream << "bool " << className << "::setStatusBarField(const std::string& idOrName, int fieldIndex, const std::string& text)\n";
    stream << "{\n";
    stream << "    RuntimeWidget* widget = findWidgetByIdOrName(idOrName);\n";
    stream << "    if (widget == nullptr || widget->type != RuntimeWidgetType::StatusBar || fieldIndex < 0 || fieldIndex > 3) {\n";
    stream << "        return false;\n";
    stream << "    }\n\n";
    stream << "    const std::size_t index = static_cast<std::size_t>(fieldIndex);\n";
    stream << "    if (widget->items.size() <= index) {\n";
    stream << "        widget->items.resize(index + 1);\n";
    stream << "    }\n";
    stream << "    if (widget->items[index] == text) {\n";
    stream << "        return false;\n";
    stream << "    }\n\n";
    stream << "    widget->items[index] = text;\n";
    stream << "    requestGeneratedUiRepaint();\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::showMessageDialog(const std::string& title, const std::string& message)\n";
    stream << "{\n";
    stream << "    clearPressedState();\n";
    stream << "    if (draggingSlider_) {\n";
    stream << "        if (RuntimeWidget* slider = findWidgetById(draggingWidgetId_); slider != nullptr) {\n";
    stream << "            slider->interaction.pressed = false;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    draggingSlider_ = false;\n";
    stream << "    draggingScrollBar_ = false;\n";
    stream << "    draggingWidgetId_.clear();\n";
    stream << "    setFocusedWidget(std::string{});\n";
    stream << "    modalState_.visible = true;\n";
    stream << "    modalState_.dialogId.clear();\n";
    stream << "    modalState_.title = title.empty() ? std::string{ \"Message\" } : title;\n";
    stream << "    modalState_.message = message;\n";
    stream << "    modalState_.buttons = { \"OK\" };\n";
    stream << "    requestGeneratedUiRepaint();\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::showModalDialog(const std::string& idOrName)\n";
    stream << "{\n";
    stream << "    const RuntimeWidget* widget = findWidgetByIdOrName(idOrName);\n";
    stream << "    if (widget == nullptr || widget->type != RuntimeWidgetType::ModalDialog) {\n";
    stream << "        return false;\n";
    stream << "    }\n\n";
    stream << "    clearPressedState();\n";
    stream << "    draggingSlider_ = false;\n";
    stream << "    draggingScrollBar_ = false;\n";
    stream << "    draggingWidgetId_.clear();\n";
    stream << "    setFocusedWidget(std::string{});\n";
    stream << "    modalState_.visible = true;\n";
    stream << "    modalState_.dialogId = widget->id;\n";
    stream << "    modalState_.title = widget->dialogTitle;\n";
    stream << "    modalState_.message = widget->text.value;\n";
    stream << "    modalState_.buttons = widget->items.empty() ? std::vector<std::string>{ \"OK\" } : widget->items;\n";
    stream << "    requestGeneratedUiRepaint();\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "void " << className << "::closeModalDialog()\n";
    stream << "{\n";
    stream << "    if (!modalState_.visible) {\n";
    stream << "        return;\n";
    stream << "    }\n\n";
    stream << "    modalState_.visible = false;\n";
    stream << "    modalState_.dialogId.clear();\n";
    stream << "    modalState_.title.clear();\n";
    stream << "    modalState_.message.clear();\n";
    stream << "    modalState_.buttons.clear();\n";
    stream << "    clearPressedState();\n";
    stream << "    draggingSlider_ = false;\n";
    stream << "    draggingScrollBar_ = false;\n";
    stream << "    draggingWidgetId_.clear();\n";
    stream << "    requestGeneratedUiRepaint();\n";
    stream << "}\n\n";
    stream << "std::optional<std::string> " << className << "::activeModalDialogId() const\n";
    stream << "{\n";
    stream << "    if (!modalState_.visible || modalState_.dialogId.empty()) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    return modalState_.dialogId;\n";
    stream << "}\n\n";
    stream << "void " << className << "::requestGeneratedUiRepaint()\n";
    stream << "{\n";
    stream << "    redraw();\n";
    stream << "}\n\n";
    stream << "RuntimeWidget* " << className << "::focusedTextBox()\n";
    stream << "{\n";
    stream << "    RuntimeWidget* widget = findWidgetById(focusedWidgetId_);\n";
    stream << "    return widget != nullptr && widget->type == RuntimeWidgetType::TextBox ? widget : nullptr;\n";
    stream << "}\n\n";
    stream << "const RuntimeWidget* " << className << "::focusedTextBox() const\n";
    stream << "{\n";
    stream << "    const RuntimeWidget* widget = findWidgetById(focusedWidgetId_);\n";
    stream << "    return widget != nullptr && widget->type == RuntimeWidgetType::TextBox ? widget : nullptr;\n";
    stream << "}\n\n";
    stream << "void " << className << "::setFocusedWidget(const std::string& widgetId)\n";
    stream << "{\n";
    stream << "    focusedWidgetId_.clear();\n";
    stream << "    for (auto& widget : runtimeWidgets_) {\n";
    stream << "        widget.interaction.focused = false;\n";
    stream << "        if (widget.id == widgetId && (widget.type == RuntimeWidgetType::TextBox || widget.type == RuntimeWidgetType::ColorPicker)) {\n";
    stream << "            widget.interaction.focused = true;\n";
    stream << "            focusedWidgetId_ = widget.id;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "void " << className << "::clearPressedState()\n";
    stream << "{\n";
    stream << "    pressedWidgetId_.clear();\n";
    stream << "    for (auto& widget : runtimeWidgets_) {\n";
    stream << "        widget.interaction.pressed = false;\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "bool " << className << "::isInteractive(const RuntimeWidget& widget) const\n";
    stream << "{\n";
    stream << "    switch (widget.type) {\n";
    stream << "    case RuntimeWidgetType::Unknown:\n";
    stream << "        return false;\n";
    stream << "    case RuntimeWidgetType::Button:\n";
    stream << "    case RuntimeWidgetType::TextBox:\n";
    stream << "    case RuntimeWidgetType::ComboBox:\n";
    stream << "    case RuntimeWidgetType::ListBox:\n";
    stream << "    case RuntimeWidgetType::MenuBar:\n";
    stream << "    case RuntimeWidgetType::ToolBar:\n";
    stream << "    case RuntimeWidgetType::TableGrid:\n";
    stream << "    case RuntimeWidgetType::CheckBox:\n";
    stream << "    case RuntimeWidgetType::RadioButton:\n";
    stream << "    case RuntimeWidgetType::Slider:\n";
    stream << "    case RuntimeWidgetType::ScrollBar:\n";
    stream << "    case RuntimeWidgetType::ColorPicker:\n";
    stream << "    case RuntimeWidgetType::TabControl:\n";
    stream << "        return true;\n";
    stream << "    case RuntimeWidgetType::ProgressBar:\n";
    stream << "    case RuntimeWidgetType::StatusBar:\n";
    stream << "    case RuntimeWidgetType::Label:\n";
    stream << "    case RuntimeWidgetType::Frame:\n";
    stream << "    case RuntimeWidgetType::GroupBox:\n";
    stream << "    case RuntimeWidgetType::Panel:\n";
    stream << "    case RuntimeWidgetType::Sizer:\n";
    stream << "    case RuntimeWidgetType::Image:\n";
    stream << "    case RuntimeWidgetType::Spacer:\n";
    stream << "        return false;\n";
    stream << "    }\n";
    stream << "    return false;\n";
    stream << "}\n\n";
    stream << "std::optional<int> " << className << "::menuBarItemIndexAt(const RuntimeWidget& widget, float x, float y) const\n";
    stream << "{\n";
    stream << "    if (!widget.bounds.contains(x, y)) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    float itemOffset = 6.0f;\n";
    stream << "    for (std::size_t index = 0; index < widget.items.size(); ++index) {\n";
    stream << "        const float itemWidth = std::max(56.0f, static_cast<float>(widget.items[index].size()) * 11.2f + 36.0f);\n";
    stream << "        const RuntimeRect itemRect{ widget.bounds.x + itemOffset, widget.bounds.y + 4.0f, itemWidth, std::max(0.0f, widget.bounds.height - 8.0f) };\n";
    stream << "        if (itemRect.contains(x, y)) {\n";
    stream << "            return static_cast<int>(index);\n";
    stream << "        }\n";
    stream << "        itemOffset += itemWidth + 4.0f;\n";
    stream << "    }\n";
    stream << "    return std::nullopt;\n";
    stream << "}\n\n";
    stream << "std::optional<int> " << className << "::toolBarItemIndexAt(const RuntimeWidget& widget, float x, float y) const\n";
    stream << "{\n";
    stream << "    if (!widget.bounds.contains(x, y)) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    float itemOffset = 6.0f;\n";
    stream << "    for (std::size_t index = 0; index < widget.items.size(); ++index) {\n";
    stream << "        const float itemWidth = std::max(64.0f, static_cast<float>(widget.items[index].size()) * 11.2f + 42.0f);\n";
    stream << "        const RuntimeRect itemRect{ widget.bounds.x + itemOffset, widget.bounds.y + 5.0f, itemWidth, std::max(0.0f, widget.bounds.height - 10.0f) };\n";
    stream << "        if (itemRect.contains(x, y)) {\n";
    stream << "            return static_cast<int>(index);\n";
    stream << "        }\n";
    stream << "        itemOffset += itemWidth + 6.0f;\n";
    stream << "    }\n";
    stream << "    return std::nullopt;\n";
    stream << "}\n\n";
    stream << "WidgetEvent " << className << "::makeWidgetEvent(const RuntimeWidget& widget, int itemIndex) const\n";
    stream << "{\n";
    stream << "    const int safeItemIndex = itemIndex >= 0 && itemIndex < static_cast<int>(widget.items.size()) ? itemIndex : -1;\n";
    stream << "    const std::string_view itemLabel = safeItemIndex >= 0 ? std::string_view{ widget.items[static_cast<std::size_t>(safeItemIndex)] } : std::string_view{};\n";
    stream << "    const std::string_view itemAction = safeItemIndex >= 0 && safeItemIndex < static_cast<int>(widget.itemActions.size())\n";
    stream << "        ? std::string_view{ widget.itemActions[static_cast<std::size_t>(safeItemIndex)] }\n";
    stream << "        : std::string_view{};\n";
    stream << "    return WidgetEvent{ widget.id, widget.name, runtimeWidgetTypeName(widget.type), safeItemIndex, itemLabel, itemAction };\n";
    stream << "}\n\n";
    stream << "RuntimeRect " << className << "::activeModalDialogRect() const\n";
    stream << "{\n";
    stream << "    if (!modalState_.visible) {\n";
    stream << "        return {};\n";
    stream << "    }\n\n";
    stream << "    const float availableWidth = std::max(280.0f, std::min(formBounds_.width - 24.0f, 560.0f));\n";
    stream << "    const float lineCount = static_cast<float>(std::max<std::size_t>(1, splitModalMessageLines(modalState_.message).size()));\n";
    stream << "    const float dialogWidth = std::clamp(formBounds_.width * 0.58f, 280.0f, availableWidth);\n";
    stream << "    const float dialogHeight = std::clamp(132.0f + lineCount * 24.0f, 170.0f, std::max(170.0f, formBounds_.height - 24.0f));\n";
    stream << "    return RuntimeRect{\n";
    stream << "        kFormOffsetX + (formBounds_.width - dialogWidth) * 0.5f,\n";
    stream << "        kFormOffsetY + (formBounds_.height - dialogHeight) * 0.5f,\n";
    stream << "        dialogWidth,\n";
    stream << "        dialogHeight\n";
    stream << "    };\n";
    stream << "}\n\n";
    stream << "RuntimeRect " << className << "::activeModalButtonRect(std::size_t buttonIndex) const\n";
    stream << "{\n";
    stream << "    if (!modalState_.visible) {\n";
    stream << "        return {};\n";
    stream << "    }\n\n";
    stream << "    const std::size_t buttonCount = std::max<std::size_t>(1, modalState_.buttons.size());\n";
    stream << "    if (buttonIndex >= buttonCount) {\n";
    stream << "        return {};\n";
    stream << "    }\n\n";
    stream << "    const RuntimeRect dialog = activeModalDialogRect();\n";
    stream << "    const float spacing = 12.0f;\n";
    stream << "    const float availableWidth = std::max(96.0f, dialog.width - 48.0f - spacing * static_cast<float>(buttonCount - 1));\n";
    stream << "    const float buttonWidth = std::clamp(availableWidth / static_cast<float>(buttonCount), 96.0f, 150.0f);\n";
    stream << "    const float totalWidth = buttonWidth * static_cast<float>(buttonCount) + spacing * static_cast<float>(buttonCount - 1);\n";
    stream << "    const float buttonX = dialog.x + (dialog.width - totalWidth) * 0.5f + static_cast<float>(buttonIndex) * (buttonWidth + spacing);\n";
    stream << "    const float buttonY = dialog.y + dialog.height - 52.0f;\n";
    stream << "    return RuntimeRect{ buttonX, buttonY, buttonWidth, 32.0f };\n";
    stream << "}\n\n";
    stream << "void " << className << "::handleActiveModalButton(std::size_t buttonIndex)\n";
    stream << "{\n";
    stream << "    if (!modalState_.visible) {\n";
    stream << "        return;\n";
    stream << "    }\n\n";
    stream << "    const std::size_t buttonCount = std::max<std::size_t>(1, modalState_.buttons.size());\n";
    stream << "    if (buttonIndex >= buttonCount) {\n";
    stream << "        return;\n";
    stream << "    }\n\n";
    stream << "    const std::string label = modalState_.buttons.empty() ? std::string{ \"OK\" } : modalState_.buttons[buttonIndex];\n";
    stream << "    RuntimeWidget* widget = activeModalWidget();\n";
    stream << "    const bool cancelled = isCancelButtonLabel(label);\n";
    stream << "    const bool accepted = !cancelled && (isAcceptButtonLabel(label) || buttonIndex == 0);\n";
    stream << "    closeModalDialog();\n";
    stream << "    if (widget == nullptr) {\n";
    stream << "        return;\n";
    stream << "    }\n\n";
    stream << "    if (cancelled) {\n";
    stream << "        if (!widget->events.onCancelled.empty()) {\n";
    stream << "            emitVoidEvent(*widget, \"onCancelled\");\n";
    stream << "        }\n";
    stream << "        return;\n";
    stream << "    }\n\n";
    stream << "    if (accepted && !widget->events.onAccepted.empty()) {\n";
    stream << "        emitVoidEvent(*widget, \"onAccepted\");\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "void " << className << "::drawActiveModalDialog(visage::Canvas& canvas, bool drawText) const\n";
    stream << "{\n";
    stream << "    if (!modalState_.visible) {\n";
    stream << "        return;\n";
    stream << "    }\n\n";
    stream << "    const RuntimeWidget* widget = activeModalWidget();\n";
    stream << "    const RuntimeStyleState style = widget != nullptr ? widget->style : RuntimeStyleState{};\n";
    stream << "    const RuntimeRect dialog = activeModalDialogRect();\n";
    stream << "    const auto lines = splitModalMessageLines(modalState_.message);\n";
    stream << "    canvas.setColor(0x96000000);\n";
    stream << "    canvas.fill(0.0f, 0.0f, width(), height());\n";
    stream << "    canvas.setColor(canvasColor(style.fillColor));\n";
    stream << "    canvas.fill(dialog.x, dialog.y, dialog.width, dialog.height);\n";
    stream << "    canvas.setColor(canvasColor(style.panelColor));\n";
    stream << "    canvas.fill(dialog.x, dialog.y, dialog.width, 34.0f);\n";
    stream << "    drawBorder(canvas, dialog.x, dialog.y, dialog.width, dialog.height, style.borderColor, std::max(1.0f, style.borderThickness));\n";
    stream << "    canvas.setColor(canvasColor(style.highlightColor));\n";
    stream << "    canvas.fill(dialog.x + 1.0f, dialog.y + 1.0f, std::max(0.0f, dialog.width - 2.0f), 1.0f);\n";
    stream << "    canvas.fill(dialog.x + 1.0f, dialog.y + 1.0f, 1.0f, std::max(0.0f, dialog.height - 2.0f));\n";
    stream << "    canvas.setColor(canvasColor(style.shadowColor));\n";
    stream << "    canvas.fill(dialog.x + 1.0f, dialog.y + dialog.height - 2.0f, std::max(0.0f, dialog.width - 2.0f), 1.0f);\n";
    stream << "    canvas.fill(dialog.x + dialog.width - 2.0f, dialog.y + 1.0f, 1.0f, std::max(0.0f, dialog.height - 2.0f));\n";
    stream << "    if (drawText) {\n";
    stream << "        canvas.setColor(canvasColor(style.textColor));\n";
    stream << "        canvas.text(modalState_.title, labelFont_, visage::Font::kTopLeft, dialog.x + 12.0f, dialog.y + 6.0f, std::max(0.0f, dialog.width - 24.0f), 22.0f);\n";
    stream << "        float lineY = dialog.y + 46.0f;\n";
    stream << "        for (const auto& line : lines) {\n";
    stream << "            canvas.text(line, labelFont_, visage::Font::kTopLeft, dialog.x + 14.0f, lineY, std::max(0.0f, dialog.width - 28.0f), 22.0f);\n";
    stream << "            lineY += 22.0f;\n";
    stream << "            if (lineY > dialog.y + dialog.height - 72.0f) {\n";
    stream << "                break;\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "    }\n\n";
    stream << "    const std::size_t buttonCount = std::max<std::size_t>(1, modalState_.buttons.size());\n";
    stream << "    for (std::size_t index = 0; index < buttonCount; ++index) {\n";
    stream << "        const RuntimeRect button = activeModalButtonRect(index);\n";
    stream << "        const std::string label = modalState_.buttons.empty() ? std::string{ \"OK\" } : modalState_.buttons[index];\n";
    stream << "        const RuntimeColor fill = isCancelButtonLabel(label) ? style.fillColor : style.accentColor;\n";
    stream << "        if (widget != nullptr) {\n";
    stream << "            drawVisualBevel(canvas, button.x, button.y, button.width, button.height, *widget, fill, false);\n";
    stream << "        }\n";
    stream << "        else {\n";
    stream << "            canvas.setColor(canvasColor(fill));\n";
    stream << "            canvas.fill(button.x, button.y, button.width, button.height);\n";
    stream << "            drawBorder(canvas, button.x, button.y, button.width, button.height, style.borderColor, std::max(1.0f, style.borderThickness));\n";
    stream << "        }\n";
    stream << "        if (drawText) {\n";
    stream << "            canvas.setColor(canvasColor(style.textColor));\n";
    stream << "            canvas.text(label, labelFont_, visage::Font::kCenter, button.x, button.y, button.width, button.height);\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "RuntimeWidget* " << className << "::hitTest(float x, float y)\n";
    stream << "{\n";
    stream << "    for (auto iterator = runtimeWidgets_.rbegin(); iterator != runtimeWidgets_.rend(); ++iterator) {\n";
    stream << "        if (!isWidgetVisible(*iterator)) {\n";
    stream << "            continue;\n";
    stream << "        }\n";
    stream << "        if (isInteractive(*iterator) && iterator->bounds.contains(x, y)) {\n";
    stream << "            return &(*iterator);\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    return nullptr;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::setWidgetValue(RuntimeWidget& widget, float value, bool emitEvent)\n";
    stream << "{\n";
    stream << "    const float safeMaximum = widget.range.max <= widget.range.min ? widget.range.min + 1.0f : widget.range.max;\n";
    stream << "    const float clamped = std::clamp(value, widget.range.min, safeMaximum);\n";
    stream << "    if (std::fabs(clamped - widget.range.value) <= 0.001f) {\n";
    stream << "        return false;\n";
    stream << "    }\n";
    stream << "    widget.range.value = clamped;\n";
    stream << "    if (emitEvent && !widget.events.onChanged.empty()) {\n";
    stream << "        emitFloatEvent(widget, \"onChanged\", widget.range.value);\n";
    stream << "    }\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::setTableGridSelection(RuntimeWidget& widget, int row, int column, bool emitEvent)\n";
    stream << "{\n";
    stream << "    const int safeRow = widget.tableRows.empty() ? -1 : std::clamp(row, 0, static_cast<int>(widget.tableRows.size()) - 1);\n";
    stream << "    const int safeColumn = widget.tableColumns.empty() ? -1 : std::clamp(column, 0, static_cast<int>(widget.tableColumns.size()) - 1);\n";
    stream << "    if (widget.selectedRow == safeRow && widget.selectedColumn == safeColumn) {\n";
    stream << "        return false;\n";
    stream << "    }\n";
    stream << "    widget.selectedRow = safeRow;\n";
    stream << "    widget.selectedColumn = safeColumn;\n";
    stream << "    if (emitEvent && !widget.events.onSelectionChanged.empty()) {\n";
    stream << "        emitVoidEvent(widget, \"onSelectionChanged\");\n";
    stream << "    }\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::setItemSelection(RuntimeWidget& widget, int selectedIndex, bool emitEvent)\n";
    stream << "{\n";
    stream << "    const int safeIndex = sanitizeItemIndex(widget, selectedIndex);\n";
    stream << "    if (widget.selectedIndex == safeIndex) {\n";
    stream << "        return false;\n";
    stream << "    }\n";
    stream << "    widget.selectedIndex = safeIndex;\n";
    stream << "    if (widget.type == RuntimeWidgetType::ComboBox) {\n";
    stream << "        widget.text.value = selectedItemText(widget);\n";
    stream << "    }\n";
    stream << "    if (emitEvent && !widget.events.onChanged.empty()) {\n";
    stream << "        emitVoidEvent(widget, \"onChanged\");\n";
    stream << "    }\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::updateSliderFromPoint(RuntimeWidget& widget, float formX)\n";
    stream << "{\n";
    stream << "    const float trackLeft = widget.bounds.x + 8.0f;\n";
    stream << "    const float trackWidth = std::max(1.0f, widget.bounds.width - 16.0f);\n";
    stream << "    const float normalized = std::clamp((formX - trackLeft) / trackWidth, 0.0f, 1.0f);\n";
    stream << "    return setWidgetValue(widget, rangeValueForNormalized(widget, normalized), true);\n";
    stream << "}\n\n";
    stream << "RuntimeRect " << className << "::scrollBarThumbRect(const RuntimeWidget& widget) const\n";
    stream << "{\n";
    stream << "    return scrollBarThumbRectForWidget(widget);\n";
    stream << "}\n\n";
    stream << "std::optional<int> " << className << "::listBoxRowIndexAt(const RuntimeWidget& widget, float x, float y) const\n";
    stream << "{\n";
    stream << "    if (!widget.bounds.contains(x, y)) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    const float rowHeight = listBoxRowHeight(widget);\n";
    stream << "    const float listTop = widget.bounds.y + 4.0f;\n";
    stream << "    const float rowOffset = y - listTop;\n";
    stream << "    if (rowOffset < 0.0f) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    const int rowIndex = static_cast<int>(rowOffset / rowHeight);\n";
    stream << "    if (rowIndex < 0 || rowIndex >= static_cast<int>(widget.items.size())) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    return rowIndex;\n";
    stream << "}\n\n";
    stream << "std::optional<std::pair<int, int>> " << className << "::tableGridCellAt(const RuntimeWidget& widget, float x, float y) const\n";
    stream << "{\n";
    stream << "    if (!widget.bounds.contains(x, y) || widget.tableColumns.empty()) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    const float contentX = widget.bounds.x + 4.0f;\n";
    stream << "    const float contentY = widget.bounds.y + 4.0f;\n";
    stream << "    const float contentWidth = std::max(0.0f, widget.bounds.width - 8.0f);\n";
    stream << "    const float columnWidth = contentWidth / static_cast<float>(std::max<std::size_t>(1, widget.tableColumns.size()));\n";
    stream << "    const float headerHeight = widget.showHeader ? std::max(18.0f, widget.headerHeight) : 0.0f;\n";
    stream << "    const float rowHeight = std::max(16.0f, widget.rowHeight);\n";
    stream << "    if (y < contentY + headerHeight || columnWidth <= 0.0f) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    const int column = std::clamp(static_cast<int>((x - contentX) / columnWidth), 0, static_cast<int>(widget.tableColumns.size()) - 1);\n";
    stream << "    const int row = static_cast<int>((y - (contentY + headerHeight)) / rowHeight);\n";
    stream << "    if (row < 0 || row >= static_cast<int>(widget.tableRows.size())) {\n";
    stream << "        return std::nullopt;\n";
    stream << "    }\n";
    stream << "    return std::make_pair(row, column);\n";
    stream << "}\n\n";
    stream << "bool " << className << "::updateScrollBarFromPointer(RuntimeWidget& widget, float formX, float formY)\n";
    stream << "{\n";
    stream << "    const bool vertical = widget.range.orientation == RuntimeOrientation::Vertical;\n";
    stream << "    const float arrowSize = vertical ? std::min(widget.bounds.width, 20.0f) : std::min(widget.bounds.height, 20.0f);\n";
    stream << "    const RuntimeRect thumb = scrollBarThumbRect(widget);\n";
    stream << "    if (vertical) {\n";
    stream << "        const float trackTop = widget.bounds.y + arrowSize;\n";
    stream << "        const float trackHeight = std::max(0.0f, widget.bounds.height - arrowSize * 2.0f);\n";
    stream << "        if (trackHeight <= thumb.height) {\n";
    stream << "            return false;\n";
    stream << "        }\n";
    stream << "        const float thumbY = std::clamp(formY - dragPointerOffset_, trackTop, trackTop + trackHeight - thumb.height);\n";
    stream << "        const float normalized = (thumbY - trackTop) / std::max(1.0f, trackHeight - thumb.height);\n";
    stream << "        return setWidgetValue(widget, rangeValueForNormalized(widget, normalized), true);\n";
    stream << "    }\n";
    stream << "    const float trackLeft = widget.bounds.x + arrowSize;\n";
    stream << "    const float trackWidth = std::max(0.0f, widget.bounds.width - arrowSize * 2.0f);\n";
    stream << "    if (trackWidth <= thumb.width) {\n";
    stream << "        return false;\n";
    stream << "    }\n";
    stream << "    const float thumbX = std::clamp(formX - dragPointerOffset_, trackLeft, trackLeft + trackWidth - thumb.width);\n";
    stream << "    const float normalized = (thumbX - trackLeft) / std::max(1.0f, trackWidth - thumb.width);\n";
    stream << "    return setWidgetValue(widget, rangeValueForNormalized(widget, normalized), true);\n";
    stream << "}\n\n";
    stream << "bool " << className << "::updateTextBoxText(RuntimeWidget& widget, const std::string& text, bool emitEvent)\n";
    stream << "{\n";
    stream << "    if (widget.text.value == text) {\n";
    stream << "        return false;\n";
    stream << "    }\n";
    stream << "    widget.text.value = text;\n";
    stream << "    if (emitEvent && !widget.events.onTextChanged.empty()) {\n";
    stream << "        emitStringEvent(widget, \"onTextChanged\", widget.text.value);\n";
    stream << "    }\n";
    stream << "    return true;\n";
    stream << "}\n\n";
    stream << "void " << className << "::draw(visage::Canvas& canvas)\n";
    stream << "{\n";
    stream << "    canvas.setColor(0xff1B1D23);\n";
    stream << "    canvas.fill(0.0f, 0.0f, width(), height());\n\n";
    stream << "    const bool drawText = canDrawText();\n";
    stream << "    canvas.setColor(canvasColor(formFillColor_));\n";
    stream << "    canvas.fill(kFormOffsetX, kFormOffsetY, formBounds_.width, formBounds_.height);\n";
    stream << "    canvas.setColor(canvasColor(formPanelColor_));\n";
    stream << "    canvas.fill(kFormOffsetX, kFormOffsetY, formBounds_.width, std::min(kTitleBarHeight, formBounds_.height));\n";
    stream << "    drawBorder(canvas, kFormOffsetX, kFormOffsetY, formBounds_.width, formBounds_.height, formBorderColor_, formBorderThickness_);\n";
    stream << "    if (drawText) {\n";
    stream << "        canvas.setColor(canvasColor(formTextColor_));\n";
    stream << "        canvas.text(formTitle_, labelFont_, visage::Font::kTopLeft, kFormOffsetX + 10.0f, kFormOffsetY + 4.0f, std::max(0.0f, formBounds_.width - 20.0f), 22.0f);\n";
    stream << "    }\n";
    stream << "    applyRuntimeSizerLayouts();\n";
    stream << "    for (const auto& widget : runtimeWidgets_) {\n";
    stream << "        if (!isWidgetVisible(widget)) {\n";
    stream << "            continue;\n";
    stream << "        }\n";
    stream << "        drawRuntimeWidget(canvas, labelFont_, drawText, widget);\n";
    stream << "    }\n";
    stream << "    drawActiveModalDialog(canvas, drawText);\n";
    stream << "}\n\n";
    stream << "void " << className << "::mouseDown(const visage::MouseEvent& e)\n";
    stream << "{\n";
    stream << "    if (!e.isLeftButton()) {\n";
    stream << "        return;\n";
    stream << "    }\n\n";
    stream << "    requestKeyboardFocus();\n";
    stream << "    if (modalState_.visible) {\n";
    stream << "        for (std::size_t index = 0; index < std::max<std::size_t>(1, modalState_.buttons.size()); ++index) {\n";
    stream << "            if (activeModalButtonRect(index).contains(e.position.x, e.position.y)) {\n";
    stream << "                handleActiveModalButton(index);\n";
    stream << "                return;\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    const float formX = e.position.x - kFormOffsetX;\n";
    stream << "    const float formY = e.position.y - kFormOffsetY;\n";
    stream << "    if (!formBounds_.contains(formX, formY)) {\n";
    stream << "        const bool hadFocus = !focusedWidgetId_.empty();\n";
    stream << "        draggingSlider_ = false;\n";
    stream << "        draggingScrollBar_ = false;\n";
    stream << "        draggingWidgetId_.clear();\n";
    stream << "        clearPressedState();\n";
    stream << "        setFocusedWidget(std::string{});\n";
    stream << "        if (hadFocus) {\n";
    stream << "            redraw();\n";
    stream << "        }\n";
    stream << "        return;\n";
    stream << "    }\n\n";
    stream << "    RuntimeWidget* widget = hitTest(formX, formY);\n";
    stream << "    if (widget == nullptr) {\n";
    stream << "        clearPressedState();\n";
    stream << "        setFocusedWidget(std::string{});\n";
    stream << "        redraw();\n";
    stream << "        return;\n";
    stream << "    }\n\n";
    stream << "    if (!widget->enabled) {\n";
    stream << "        clearPressedState();\n";
    stream << "        setFocusedWidget(std::string{});\n";
    stream << "        redraw();\n";
    stream << "        return;\n";
    stream << "    }\n\n";
    stream << "    if (widget->type == RuntimeWidgetType::TextBox) {\n";
    stream << "        clearPressedState();\n";
    stream << "        draggingSlider_ = false;\n";
    stream << "        draggingScrollBar_ = false;\n";
    stream << "        draggingWidgetId_.clear();\n";
    stream << "        setFocusedWidget(widget->readOnly ? std::string{} : widget->id);\n";
    stream << "        redraw();\n";
    stream << "        return;\n";
    stream << "    }\n\n";
    stream << "    if (e.repeatClickCount() >= 2) {\n";
    stream << "        if (widget->type == RuntimeWidgetType::Button) {\n";
    stream << "            if (!widget->events.onDoubleClick.empty()) {\n";
    stream << "                emitVoidEvent(*widget, \"onDoubleClick\");\n";
    stream << "                redraw();\n";
    stream << "            }\n";
    stream << "            return;\n";
    stream << "        }\n";
    stream << "        if (widget->type == RuntimeWidgetType::ListBox) {\n";
    stream << "            if (!widget->events.onDoubleClick.empty()) {\n";
    stream << "                if (const auto rowIndex = listBoxRowIndexAt(*widget, formX, formY); rowIndex.has_value()) {\n";
    stream << "                    setItemSelection(*widget, *rowIndex, true);\n";
    stream << "                    emitVoidEvent(*widget, \"onDoubleClick\");\n";
    stream << "                    redraw();\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "            return;\n";
    stream << "        }\n";
    stream << "        if (widget->type == RuntimeWidgetType::TableGrid) {\n";
    stream << "            if (!widget->events.onCellDoubleClick.empty()) {\n";
    stream << "                if (const auto cell = tableGridCellAt(*widget, formX, formY); cell.has_value()) {\n";
    stream << "                    setTableGridSelection(*widget, cell->first, cell->second, true);\n";
    stream << "                    emitVoidEvent(*widget, \"onCellDoubleClick\");\n";
    stream << "                    redraw();\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "            return;\n";
    stream << "        }\n";
    stream << "    }\n\n";
    stream << "    setFocusedWidget(std::string{});\n";
    stream << "    switch (widget->type) {\n";
    stream << "    case RuntimeWidgetType::Unknown:\n";
    stream << "        return;\n";
    stream << "    case RuntimeWidgetType::ComboBox:\n";
    stream << "        clearPressedState();\n";
    stream << "        widget->interaction.pressed = true;\n";
    stream << "        pressedWidgetId_ = widget->id;\n";
    stream << "        if (!widget->items.empty() && setItemSelection(*widget, widget->selectedIndex + 1, true)) {\n";
    stream << "            redraw();\n";
    stream << "        }\n";
    stream << "        return;\n";
    stream << "    case RuntimeWidgetType::ListBox:\n";
    stream << "        clearPressedState();\n";
    stream << "        if (const auto rowIndex = listBoxRowIndexAt(*widget, formX, formY); rowIndex.has_value()) {\n";
    stream << "            if (setItemSelection(*widget, *rowIndex, true)) {\n";
    stream << "                redraw();\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        return;\n";
    stream << "    case RuntimeWidgetType::MenuBar:\n";
    stream << "        clearPressedState();\n";
    stream << "        if (const auto itemIndex = menuBarItemIndexAt(*widget, formX, formY); itemIndex.has_value()) {\n";
    stream << "            setItemSelection(*widget, *itemIndex, false);\n";
    stream << "            emitItemAction(*widget, *itemIndex);\n";
    stream << "            redraw();\n";
    stream << "        }\n";
    stream << "        return;\n";
    stream << "    case RuntimeWidgetType::ToolBar:\n";
    stream << "        clearPressedState();\n";
    stream << "        if (const auto itemIndex = toolBarItemIndexAt(*widget, formX, formY); itemIndex.has_value()) {\n";
    stream << "            setItemSelection(*widget, *itemIndex, false);\n";
    stream << "            emitItemAction(*widget, *itemIndex);\n";
    stream << "            redraw();\n";
    stream << "        }\n";
    stream << "        return;\n";
    stream << "    case RuntimeWidgetType::TableGrid:\n";
    stream << "        clearPressedState();\n";
    stream << "        if (const auto cell = tableGridCellAt(*widget, formX, formY); cell.has_value()) {\n";
    stream << "            if (setTableGridSelection(*widget, cell->first, cell->second, true)) {\n";
    stream << "                redraw();\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        return;\n";
    stream << "    case RuntimeWidgetType::TabControl:\n";
    stream << "        if (const auto tabIndex = hitTestTabHeader(*widget, formX, formY); tabIndex.has_value()) {\n";
    stream << "            if (widget->selectedTab != *tabIndex) {\n";
    stream << "                widget->selectedTab = *tabIndex;\n";
    stream << "                redraw();\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        return;\n";
    stream << "    case RuntimeWidgetType::Button:\n";
    stream << "    case RuntimeWidgetType::CheckBox:\n";
    stream << "    case RuntimeWidgetType::RadioButton:\n";
    stream << "        clearPressedState();\n";
    stream << "        widget->interaction.pressed = true;\n";
    stream << "        pressedWidgetId_ = widget->id;\n";
    stream << "        redraw();\n";
    stream << "        return;\n";
    stream << "    case RuntimeWidgetType::Slider:\n";
    stream << "        clearPressedState();\n";
    stream << "        widget->interaction.pressed = true;\n";
    stream << "        pressedWidgetId_ = widget->id;\n";
    stream << "        draggingSlider_ = true;\n";
    stream << "        draggingScrollBar_ = false;\n";
    stream << "        draggingWidgetId_ = widget->id;\n";
    stream << "        if (updateSliderFromPoint(*widget, formX)) {\n";
    stream << "            redraw();\n";
    stream << "        }\n";
    stream << "        return;\n";
    stream << "    case RuntimeWidgetType::ScrollBar: {\n";
    stream << "        clearPressedState();\n";
    stream << "        widget->interaction.pressed = true;\n";
    stream << "        pressedWidgetId_ = widget->id;\n";
    stream << "        draggingSlider_ = false;\n";
    stream << "        draggingWidgetId_ = widget->id;\n";
    stream << "        const RuntimeRect thumb = scrollBarThumbRect(*widget);\n";
    stream << "        const bool vertical = widget->range.orientation == RuntimeOrientation::Vertical;\n";
    stream << "        const float arrowSize = vertical ? std::min(widget->bounds.width, 20.0f) : std::min(widget->bounds.height, 20.0f);\n";
    stream << "        if (thumb.contains(formX, formY)) {\n";
    stream << "            draggingScrollBar_ = true;\n";
    stream << "            dragPointerOffset_ = vertical ? (formY - thumb.y) : (formX - thumb.x);\n";
    stream << "            redraw();\n";
    stream << "            return;\n";
    stream << "        }\n";
    stream << "        draggingScrollBar_ = false;\n";
    stream << "        const float step = std::max(1.0f, (std::max(widget->range.min + 1.0f, widget->range.max) - widget->range.min) / 20.0f);\n";
    stream << "        bool changed = false;\n";
    stream << "        if (vertical) {\n";
    stream << "            if (formY < widget->bounds.y + arrowSize) {\n";
    stream << "                changed = setWidgetValue(*widget, widget->range.value - step, true);\n";
    stream << "            }\n";
    stream << "            else if (formY > widget->bounds.y + widget->bounds.height - arrowSize) {\n";
    stream << "                changed = setWidgetValue(*widget, widget->range.value + step, true);\n";
    stream << "            }\n";
    stream << "            else if (formY < thumb.y) {\n";
    stream << "                changed = setWidgetValue(*widget, widget->range.value - std::max(1.0f, widget->range.pageSize), true);\n";
    stream << "            }\n";
    stream << "            else if (formY > thumb.y + thumb.height) {\n";
    stream << "                changed = setWidgetValue(*widget, widget->range.value + std::max(1.0f, widget->range.pageSize), true);\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        else {\n";
    stream << "            if (formX < widget->bounds.x + arrowSize) {\n";
    stream << "                changed = setWidgetValue(*widget, widget->range.value - step, true);\n";
    stream << "            }\n";
    stream << "            else if (formX > widget->bounds.x + widget->bounds.width - arrowSize) {\n";
    stream << "                changed = setWidgetValue(*widget, widget->range.value + step, true);\n";
    stream << "            }\n";
    stream << "            else if (formX < thumb.x) {\n";
    stream << "                changed = setWidgetValue(*widget, widget->range.value - std::max(1.0f, widget->range.pageSize), true);\n";
    stream << "            }\n";
    stream << "            else if (formX > thumb.x + thumb.width) {\n";
    stream << "                changed = setWidgetValue(*widget, widget->range.value + std::max(1.0f, widget->range.pageSize), true);\n";
    stream << "            }\n";
    stream << "        }\n";
    stream << "        if (changed) {\n";
    stream << "            redraw();\n";
    stream << "        }\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    case RuntimeWidgetType::ColorPicker:\n";
    stream << "        clearPressedState();\n";
    stream << "        setFocusedWidget(widget->id);\n";
    stream << "        widget->interaction.pressed = true;\n";
    stream << "        pressedWidgetId_ = widget->id;\n";
    stream << "        if (!widget->events.onChanged.empty()) {\n";
    stream << "            emitStringEvent(*widget, \"onChanged\", widget->colorValue);\n";
    stream << "        }\n";
    stream << "        redraw();\n";
    stream << "        return;\n";
    stream << "    case RuntimeWidgetType::ProgressBar:\n";
    stream << "    case RuntimeWidgetType::StatusBar:\n";
    stream << "    case RuntimeWidgetType::Label:\n";
    stream << "    case RuntimeWidgetType::Frame:\n";
    stream << "    case RuntimeWidgetType::Image:\n";
    stream << "    case RuntimeWidgetType::Sizer:\n";
    stream << "    case RuntimeWidgetType::Spacer:\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "void " << className << "::mouseMove(const visage::MouseEvent& e)\n";
    stream << "{\n";
    stream << "    if (modalState_.visible) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    RuntimeWidget* hoverWidget = formBounds_.contains(e.position.x - kFormOffsetX, e.position.y - kFormOffsetY)\n";
    stream << "        ? hitTest(e.position.x - kFormOffsetX, e.position.y - kFormOffsetY)\n";
    stream << "        : nullptr;\n";
    stream << "    bool hoverChanged = false;\n";
    stream << "    for (auto& widget : runtimeWidgets_) {\n";
    stream << "        const bool hovered = widget.enabled && hoverWidget != nullptr && hoverWidget->id == widget.id;\n";
    stream << "        if (widget.interaction.hovered != hovered) {\n";
    stream << "            widget.interaction.hovered = hovered;\n";
    stream << "            hoverChanged = true;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    if (hoverChanged) {\n";
    stream << "        redraw();\n";
    stream << "    }\n";
    stream << "    if (pressedWidgetId_.empty() || draggingSlider_ || draggingScrollBar_) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    RuntimeWidget* pressedWidget = findWidgetById(pressedWidgetId_);\n";
    stream << "    if (pressedWidget == nullptr) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    const bool shouldBePressed = hoverWidget != nullptr && hoverWidget->id == pressedWidgetId_;\n";
    stream << "    if (pressedWidget->interaction.pressed != shouldBePressed) {\n";
    stream << "        pressedWidget->interaction.pressed = shouldBePressed;\n";
    stream << "        redraw();\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "void " << className << "::mouseDrag(const visage::MouseEvent& e)\n";
    stream << "{\n";
    stream << "    if (modalState_.visible) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    const float formX = e.position.x - kFormOffsetX;\n";
    stream << "    const float formY = e.position.y - kFormOffsetY;\n";
    stream << "    if (draggingSlider_) {\n";
    stream << "        if (RuntimeWidget* widget = findWidgetById(draggingWidgetId_); widget != nullptr && updateSliderFromPoint(*widget, formX)) {\n";
    stream << "            redraw();\n";
    stream << "        }\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    if (draggingScrollBar_) {\n";
    stream << "        if (RuntimeWidget* widget = findWidgetById(draggingWidgetId_); widget != nullptr && updateScrollBarFromPointer(*widget, formX, formY)) {\n";
    stream << "            redraw();\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "void " << className << "::mouseUp(const visage::MouseEvent& e)\n";
    stream << "{\n";
    stream << "    if (!e.isLeftButton()) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    if (modalState_.visible) {\n";
    stream << "        draggingSlider_ = false;\n";
    stream << "        draggingScrollBar_ = false;\n";
    stream << "        draggingWidgetId_.clear();\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    if (draggingSlider_) {\n";
    stream << "        if (RuntimeWidget* slider = findWidgetById(draggingWidgetId_); slider != nullptr) {\n";
    stream << "            slider->interaction.pressed = false;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    draggingSlider_ = false;\n";
    stream << "    draggingScrollBar_ = false;\n";
    stream << "    draggingWidgetId_.clear();\n";
    stream << "    if (pressedWidgetId_.empty()) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    RuntimeWidget* pressedWidget = findWidgetById(pressedWidgetId_);\n";
    stream << "    RuntimeWidget* releaseWidget = formBounds_.contains(e.position.x - kFormOffsetX, e.position.y - kFormOffsetY)\n";
    stream << "        ? hitTest(e.position.x - kFormOffsetX, e.position.y - kFormOffsetY)\n";
    stream << "        : nullptr;\n";
    stream << "    const bool triggered = pressedWidget != nullptr && releaseWidget != nullptr && releaseWidget->id == pressedWidgetId_;\n";
    stream << "    if (pressedWidget != nullptr) {\n";
    stream << "        pressedWidget->interaction.pressed = false;\n";
    stream << "        switch (pressedWidget->type) {\n";
    stream << "        case RuntimeWidgetType::Unknown:\n";
    stream << "            break;\n";
    stream << "        case RuntimeWidgetType::Button:\n";
    stream << "            if (triggered) {\n";
    stream << "                if (pressedWidget->button.toggleMode) {\n";
    stream << "                    pressedWidget->toggle.checked = !pressedWidget->toggle.checked;\n";
    stream << "                }\n";
    stream << "                if (!pressedWidget->events.onRelease.empty()) {\n";
    stream << "                    emitVoidEvent(*pressedWidget, \"onRelease\");\n";
    stream << "                }\n";
    stream << "                if (!pressedWidget->events.onClick.empty()) {\n";
    stream << "                    emitVoidEvent(*pressedWidget, \"onClick\");\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "            break;\n";
    stream << "        case RuntimeWidgetType::CheckBox:\n";
    stream << "            if (triggered) {\n";
    stream << "                pressedWidget->toggle.checked = !pressedWidget->toggle.checked;\n";
    stream << "                if (!pressedWidget->events.onToggle.empty()) {\n";
    stream << "                    emitBoolEvent(*pressedWidget, \"onToggle\", pressedWidget->toggle.checked);\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "            break;\n";
    stream << "        case RuntimeWidgetType::RadioButton:\n";
    stream << "            if (triggered) {\n";
    stream << "                for (auto& widget : runtimeWidgets_) {\n";
    stream << "                    if (widget.type == RuntimeWidgetType::RadioButton && widget.toggle.group == pressedWidget->toggle.group) {\n";
    stream << "                        widget.toggle.selected = widget.id == pressedWidget->id;\n";
    stream << "                    }\n";
    stream << "                }\n";
    stream << "                if (!pressedWidget->events.onSelected.empty()) {\n";
    stream << "                    emitBoolEvent(*pressedWidget, \"onSelected\", true);\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "            break;\n";
    stream << "        case RuntimeWidgetType::TextBox:\n";
    stream << "        case RuntimeWidgetType::ComboBox:\n";
    stream << "        case RuntimeWidgetType::Slider:\n";
    stream << "        case RuntimeWidgetType::ScrollBar:\n";
    stream << "        case RuntimeWidgetType::TableGrid:\n";
    stream << "        case RuntimeWidgetType::ProgressBar:\n";
    stream << "        case RuntimeWidgetType::StatusBar:\n";
    stream << "        case RuntimeWidgetType::Label:\n";
    stream << "        case RuntimeWidgetType::Frame:\n";
    stream << "        case RuntimeWidgetType::Image:\n";
    stream << "        case RuntimeWidgetType::Sizer:\n";
    stream << "        case RuntimeWidgetType::Spacer:\n";
    stream << "        case RuntimeWidgetType::ColorPicker:\n";
    stream << "            break;\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    pressedWidgetId_.clear();\n";
    stream << "    redraw();\n";
    stream << "}\n\n";
    stream << "bool " << className << "::keyPress(const visage::KeyEvent& e)\n";
    stream << "{\n";
    stream << "    using KeyCode = visage::KeyCode;\n";
    stream << "    if (modalState_.visible) {\n";
    stream << "        if (e.keyCode() == KeyCode::Escape) {\n";
    stream << "            for (std::size_t index = 0; index < modalState_.buttons.size(); ++index) {\n";
    stream << "                if (isCancelButtonLabel(modalState_.buttons[index])) {\n";
    stream << "                    handleActiveModalButton(index);\n";
    stream << "                    return true;\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "            handleActiveModalButton(0);\n";
    stream << "            return true;\n";
    stream << "        }\n";
    stream << "        if (e.keyCode() == KeyCode::Return) {\n";
    stream << "            for (std::size_t index = 0; index < modalState_.buttons.size(); ++index) {\n";
    stream << "                if (isAcceptButtonLabel(modalState_.buttons[index])) {\n";
    stream << "                    handleActiveModalButton(index);\n";
    stream << "                    return true;\n";
    stream << "                }\n";
    stream << "            }\n";
    stream << "            handleActiveModalButton(0);\n";
    stream << "            return true;\n";
    stream << "        }\n";
    stream << "        return true;\n";
    stream << "    }\n";
    stream << "    RuntimeWidget* widget = focusedTextBox();\n";
    stream << "    if (widget == nullptr) {\n";
    stream << "        return false;\n";
    stream << "    }\n";
    stream << "    if (e.keyCode() == KeyCode::Backspace) {\n";
    stream << "        if (!widget->text.value.empty() && updateTextBoxText(*widget, widget->text.value.substr(0, widget->text.value.size() - 1), true)) {\n";
    stream << "            redraw();\n";
    stream << "        }\n";
    stream << "        return true;\n";
    stream << "    }\n";
    stream << "    if (e.keyCode() == KeyCode::Escape) {\n";
    stream << "        setFocusedWidget(std::string{});\n";
    stream << "        redraw();\n";
    stream << "        return true;\n";
    stream << "    }\n";
    stream << "    if (e.keyCode() == KeyCode::Return) {\n";
    stream << "        if (widget->style.multiline && updateTextBoxText(*widget, widget->text.value + \"\\n\", true)) {\n";
    stream << "            redraw();\n";
    stream << "        }\n";
    stream << "        return true;\n";
    stream << "    }\n";
    stream << "    return false;\n";
    stream << "}\n\n";
    stream << "bool " << className << "::receivesTextInput()\n";
    stream << "{\n";
    stream << "    return !modalState_.visible && focusedTextBox() != nullptr;\n";
    stream << "}\n\n";
    stream << "void " << className << "::textInput(const std::string& text)\n";
    stream << "{\n";
    stream << "    if (modalState_.visible) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    RuntimeWidget* widget = focusedTextBox();\n";
    stream << "    if (widget == nullptr || text.empty()) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    std::string appended;\n";
    stream << "    appended.reserve(text.size());\n";
    stream << "    for (char character : text) {\n";
    stream << "        if (character == '\\r' || character == '\\b') {\n";
    stream << "            continue;\n";
    stream << "        }\n";
    stream << "        if (character == '\\n') {\n";
    stream << "            if (widget->style.multiline) {\n";
    stream << "                appended.push_back(character);\n";
    stream << "            }\n";
    stream << "            continue;\n";
    stream << "        }\n";
    stream << "        if (!std::iscntrl(static_cast<unsigned char>(character))) {\n";
    stream << "            appended.push_back(character);\n";
    stream << "        }\n";
    stream << "    }\n";
    stream << "    if (!appended.empty() && updateTextBoxText(*widget, widget->text.value + appended, true)) {\n";
    stream << "        redraw();\n";
    stream << "    }\n";
    stream << "}\n\n";
    stream << "bool " << className << "::canDrawText() const\n";
    stream << "{\n";
    stream << "    return labelFont_.packedFont() != nullptr;\n";
    stream << "}\n";

    emitRuntimeEventDispatcher(stream,
        "\nvoid " + className + "::emitVoidEvent(const RuntimeWidget& widget, std::string_view eventKey)",
        HandlerSignature::Void,
        bindings);
    emitRuntimeEventDispatcher(stream,
        "\nvoid " + className + "::emitBoolEvent(const RuntimeWidget& widget, std::string_view eventKey, bool value)",
        HandlerSignature::Bool,
        bindings);
    emitRuntimeEventDispatcher(stream,
        "\nvoid " + className + "::emitFloatEvent(const RuntimeWidget& widget, std::string_view eventKey, float value)",
        HandlerSignature::Float,
        bindings);
    emitRuntimeEventDispatcher(stream,
        "\nvoid " + className + "::emitStringEvent(const RuntimeWidget& widget, std::string_view eventKey, const std::string& value)",
        HandlerSignature::String,
        bindings);

    stream << "\nvoid " << className << "::emitItemAction(const RuntimeWidget& widget, int itemIndex)\n";
    stream << "{\n";
    stream << "    const std::string handlerName = itemActionAt(widget, itemIndex);\n";
    stream << "    if (handlerName.empty()) {\n";
    stream << "        return;\n";
    stream << "    }\n";
    stream << "    const WidgetEvent event = makeWidgetEvent(widget, itemIndex);\n";
    {
        std::vector<std::string> emittedItemHandlers;
        bool emittedItemActionMatch = false;
        for (const auto& binding : bindings) {
            if (binding.signature != HandlerSignature::Void || binding.eventKey != "onItemAction") {
                continue;
            }

            if (std::find(emittedItemHandlers.begin(), emittedItemHandlers.end(), binding.handlerName) != emittedItemHandlers.end()) {
                continue;
            }
            emittedItemHandlers.push_back(binding.handlerName);
            emittedItemActionMatch = true;
            stream << "    if (handlerName == " << emitStringLiteral(binding.handlerName) << ") {\n";
            stream << "        " << binding.handlerName << "(event);\n";
            stream << "        return;\n";
            stream << "    }\n";
        }

        if (!emittedItemActionMatch) {
            stream << "    (void)widget;\n";
            stream << "    (void)itemIndex;\n";
            stream << "    return;\n";
        }
        else {
            stream << "    (void)widget;\n";
            stream << "    (void)itemIndex;\n";
            stream << "    return;\n";
        }
    }
    stream << "}\n";

    for (const auto& handler : handlers) {
        stream << "\n" << handlerDefinitionSignature(className, handler) << "\n";
        stream << "{\n";
        stream << "    // References: " << handlerReferenceList(handler) << "\n";
        stream << "    (void)event;\n";
        if (handler.signature == HandlerSignature::Bool) {
            stream << "    (void)value;\n";
        }
        else if (handler.signature == HandlerSignature::Float) {
            stream << "    (void)value;\n";
        }
        else if (handler.signature == HandlerSignature::String) {
            stream << "    (void)value;\n";
        }
        stream << "    // Default generated event hook.\n";
        stream << "}\n";
    }
    return stream.str();
}

std::string emitUserSubclassHeader(const visiform::model::ProjectDocument& document)
{
    const std::string baseClass = generatedBaseClassName(document);
    const std::string userClass = userSubclassName(document);
    std::vector<HandlerInfo> handlers;
    std::string ignoredError;
    collectHandlers(document, handlers, ignoredError);

    std::ostringstream stream;
    stream << kGeneratedFileHeader;
    stream << "#pragma once\n\n";
    stream << "#include \"" << baseClass << ".h\"\n\n";
    stream << "class " << userClass << " : public " << baseClass << " {\n";
    stream << "public:\n";
    stream << "    " << userClass << "();\n";
    stream << "    ~" << userClass << "() override = default;\n";
    if (!handlers.empty()) {
        stream << "\nprotected:\n";
        for (const auto& handler : handlers) {
            std::string declaration = handlerDeclaration(handler);
            if (!declaration.empty() && declaration.back() == ';') {
                declaration.pop_back();
            }
            stream << "    " << declaration << " override;\n";
        }
    }
    stream << "};\n";
    return stream.str();
}

std::string emitUserSubclassCpp(const visiform::model::ProjectDocument& document, const std::string& existingUserCpp)
{
    const std::string userClass = userSubclassName(document);
    std::vector<HandlerInfo> handlers;
    std::string ignoredError;
    collectHandlers(document, handlers, ignoredError);
    const PreservedUserCodeBlocks preservedUserCodeBlocks = extractPreservedUserCodeBlocks(existingUserCpp);

    std::ostringstream stream;
    stream << kGeneratedFileHeader;
    stream << "#include \"" << userClass << ".h\"\n\n";
    stream << userClass << "::" << userClass << "() = default;\n";

    for (const auto& handler : handlers) {
        stream << "\n" << handlerDefinitionSignature(userClass, handler) << "\n";
        stream << "{\n";
        stream << "    // References: " << handlerReferenceList(handler) << "\n";
        stream << "    // USER CODE BEGIN " << handler.handlerName << "\n";
        if (const auto iterator = preservedUserCodeBlocks.find(handler.handlerName); iterator != preservedUserCodeBlocks.end() && !iterator->second.empty()) {
            std::istringstream preservedStream(iterator->second);
            std::string preservedLine;
            while (std::getline(preservedStream, preservedLine)) {
                stream << "    " << preservedLine << "\n";
            }
        }
        else {
            stream << "    (void)event;\n";
            if (handler.signature == HandlerSignature::Bool) {
                stream << "    (void)value;\n";
            }
            else if (handler.signature == HandlerSignature::Float) {
                stream << "    (void)value;\n";
            }
            else if (handler.signature == HandlerSignature::String) {
                stream << "    (void)value;\n";
            }
            stream << "    " << handlerTodoLine(handler.bindings.front()) << "\n";
            for (const auto& exampleLine : handlerExampleLines(handler)) {
                stream << "    " << exampleLine << "\n";
            }
        }
        stream << "    // USER CODE END " << handler.handlerName << "\n";
        stream << "}\n";
    }
    return stream.str();
}

} // namespace

bool VisageCppEmitter::emitProjectSources(
    const model::ProjectDocument& document,
    const std::string& existingMainWindowCpp,
    EmittedSources& output,
    std::string& errorMessage) const
{
    errorMessage.clear();
    std::vector<HandlerInfo> handlers;
    if (!collectHandlers(document, handlers, errorMessage)) {
        return false;
    }

    output = {
        emitMainCpp(document),
        "MainWindow.h",
        emitGeneratedBaseHeader(document),
        "MainWindow.cpp",
        emitGeneratedBaseCpp(document),
        userSubclassName(document) + ".h",
        emitUserSubclassHeader(document),
        userSubclassName(document) + ".cpp",
        emitUserSubclassCpp(document, existingMainWindowCpp)
    };
    return true;
}

} // namespace visiform::generator
