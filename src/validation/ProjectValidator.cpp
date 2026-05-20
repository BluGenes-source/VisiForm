#include "validation/ProjectValidator.h"

#include "model/LookAndFeelRegistry.h"
#include "model/PropertyValue.h"
#include "model/WidgetRegistry.h"
#include "utils/CppIdentifier.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace visiform::validation {
namespace {

constexpr const char* kDefaultProjectName = "VisiFormProject";
constexpr std::string_view kValidationSafeExecutableFallback = "VisiFormProject";

void addMessage(ValidationReport& report,
    ValidationSeverity severity,
    std::string code,
    std::string message,
    std::string widgetId = {},
    std::string propertyKey = {})
{
    report.messages.push_back(ValidationMessage{ severity, std::move(code), std::move(message), std::move(widgetId), std::move(propertyKey) });
}

std::string trim(std::string_view value)
{
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }

    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }

    return std::string{ value.substr(start, end - start) };
}

std::string toLower(std::string_view value)
{
    std::string lowered;
    lowered.reserve(value.size());
    for (char character : value) {
        lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
    }
    return lowered;
}

bool isDefaultProjectName(std::string_view value)
{
    const std::string normalized = toLower(trim(value));
    return normalized.empty()
        || normalized == toLower(kDefaultProjectName)
        || normalized == "untitled"
        || normalized == "untitledproject";
}

std::string sanitizeProjectNameForCMake(std::string_view value)
{
    const std::string source = trim(value).empty() ? std::string{ kDefaultProjectName } : std::string{ value };
    std::string sanitized;
    sanitized.reserve(source.size());
    for (char character : source) {
        if (std::isalnum(static_cast<unsigned char>(character)) != 0) {
            sanitized.push_back(character);
        }
        else if (character == '_' || character == '-' || std::isspace(static_cast<unsigned char>(character)) != 0) {
            sanitized.push_back('_');
        }
    }

    if (sanitized.empty()) {
        sanitized = kDefaultProjectName;
    }
    if (std::isdigit(static_cast<unsigned char>(sanitized.front())) != 0) {
        sanitized.insert(sanitized.begin(), '_');
    }

    return sanitized;
}

std::string sanitizeExecutableName(std::string_view value)
{
    const std::string source = trim(value).empty() ? std::string{ kValidationSafeExecutableFallback } : std::string{ value };
    std::string sanitized;
    sanitized.reserve(source.size());
    for (char character : source) {
        if (std::isalnum(static_cast<unsigned char>(character)) != 0 || character == '_' || character == '-') {
            sanitized.push_back(character);
        }
        else {
            sanitized.push_back('_');
        }
    }

    if (sanitized.empty()) {
        sanitized = std::string{ kValidationSafeExecutableFallback };
    }
    if (std::isdigit(static_cast<unsigned char>(sanitized.front())) != 0) {
        sanitized.insert(sanitized.begin(), '_');
    }

    return sanitized;
}

bool isSafeFileStem(std::string_view value)
{
    static constexpr std::string_view kInvalidCharacters = "<>:\"/\\|?*";
    return !value.empty() && value.find_first_of(kInvalidCharacters) == std::string_view::npos;
}

bool isValidColorValue(std::string_view value)
{
    if (value.empty()) {
        return true;
    }

    if ((value.size() != 7 && value.size() != 9) || value.front() != '#') {
        return false;
    }

    return std::all_of(value.begin() + 1, value.end(), [](unsigned char character) {
        return std::isxdigit(character) != 0;
    });
}

bool isColorPropertyKey(std::string_view key)
{
    return key == "fillColor"
        || key == "textColor"
        || key == "borderColor"
        || key == "accentColor"
        || key == "backgroundColor"
        || key == "panelColor"
        || key == "controlFillColor"
        || key == "controlTextColor"
        || key == "controlBorderColor"
        || key == "disabledColor";
}

std::optional<double> propertyNumber(const model::WidgetNode& widget, const std::string& key)
{
    const auto* property = widget.getProperty(key);
    if (property == nullptr) {
        return std::nullopt;
    }

    if (property->isInt()) {
        return static_cast<double>(property->asInt());
    }
    if (property->isFloat()) {
        return static_cast<double>(property->asFloat());
    }

    return std::nullopt;
}

std::string propertyString(const model::WidgetNode& widget, const std::string& key)
{
    if (const auto* property = widget.getProperty(key); property != nullptr && property->isString()) {
        return property->asString();
    }

    return {};
}

bool propertyBool(const model::WidgetNode& widget, const std::string& key, bool defaultValue = false)
{
    if (const auto* property = widget.getProperty(key); property != nullptr && property->isBool()) {
        return property->asBool(defaultValue);
    }

    return defaultValue;
}

std::string widgetLabel(const model::WidgetNode& widget)
{
    if (!widget.name.empty()) {
        return widget.name;
    }
    if (!widget.id.empty()) {
        return widget.id;
    }
    return widget.typeName();
}

void collectWidgets(const model::WidgetNode& widget, std::vector<const model::WidgetNode*>& widgets)
{
    widgets.push_back(&widget);
    for (const auto& child : widget.children) {
        collectWidgets(child, widgets);
    }
}

bool rangesOverlap(float startA, float endA, float startB, float endB)
{
    return std::max(startA, startB) < std::min(endA, endB);
}

bool overlapsBottomDockedStatusBar(const model::ProjectDocument& document, const model::WidgetNode& widget)
{
    if (document.isRootWidgetId(widget.id) || widget.type == model::WidgetType::StatusBar) {
        return false;
    }

    const auto* parent = document.findParentOf(widget.id);
    if (parent == nullptr || parent != &document.root) {
        return false;
    }

    for (const auto& candidate : document.root.children) {
        if (candidate.type != model::WidgetType::StatusBar) {
            continue;
        }

        const std::string dock = trim(propertyString(candidate, "dock"));
        if (!dock.empty() && dock != "Bottom") {
            continue;
        }
        if (candidate.bounds.height <= 0.0f) {
            continue;
        }

        const float reservedTop = document.root.bounds.height - candidate.bounds.height;
        if (widget.bounds.y + widget.bounds.height <= reservedTop) {
            continue;
        }

        if (rangesOverlap(widget.bounds.x, widget.bounds.x + widget.bounds.width,
                0.0f, document.root.bounds.width)) {
            return true;
        }
    }

    return false;
}

bool isKnownDockValue(std::string_view value)
{
    return value.empty()
        || value == "None"
        || value == "Bottom"
        || value == "Top"
        || value == "Left"
        || value == "Right"
        || value == "Fill";
}

std::string eventSignatureKind(const model::WidgetDefinition& definition, const std::string& eventKey)
{
    const auto iterator = std::find_if(definition.events.begin(), definition.events.end(), [&eventKey](const model::WidgetEventDefinition& eventDefinition) {
        return eventDefinition.key == eventKey;
    });
    return iterator == definition.events.end() ? std::string{} : iterator->handlerSignatureKind;
}

struct CallbackUsage {
    std::string signatureKind{};
    std::string widgetId{};
    std::string propertyKey{};
};

} // namespace

bool ValidationReport::hasErrors() const
{
    return errorCount() > 0;
}

bool ValidationReport::hasWarnings() const
{
    return warningCount() > 0;
}

int ValidationReport::errorCount() const
{
    return static_cast<int>(std::count_if(messages.begin(), messages.end(), [](const ValidationMessage& message) {
        return message.severity == ValidationSeverity::Error;
    }));
}

int ValidationReport::warningCount() const
{
    return static_cast<int>(std::count_if(messages.begin(), messages.end(), [](const ValidationMessage& message) {
        return message.severity == ValidationSeverity::Warning;
    }));
}

ValidationReport ProjectValidator::validate(const model::ProjectDocument& document, const utils::AppSettings& settings) const
{
    ValidationReport report;

    const std::string trimmedProjectName = trim(document.projectName);
    const std::string trimmedExecutableName = trim(document.executableName);
    const std::string trimmedUserSubclassName = trim(document.userSubclassName);
    const std::string sanitizedProjectName = sanitizeProjectNameForCMake(document.projectName);
    const std::string sanitizedExecutable = sanitizeExecutableName(document.executableName);

    if (trimmedExecutableName.empty()) {
        addMessage(report, ValidationSeverity::Error,
            "PROJECT_EXECUTABLE_EMPTY",
            "Executable name must not be empty.",
            document.root.id,
            "executableName");
    }
    else if (sanitizedExecutable != trimmedExecutableName) {
        addMessage(report, ValidationSeverity::Warning,
            "PROJECT_EXECUTABLE_SANITIZED",
            "Executable name will be sanitized for CMake and the generated executable.",
            document.root.id,
            "executableName");
    }

    if (trimmedProjectName.empty()) {
        addMessage(report, ValidationSeverity::Warning,
            "PROJECT_NAME_EMPTY",
            "Project name is empty and the generated project name will fall back to VisiFormProject.",
            document.root.id,
            "projectName");
    }
    else if (isDefaultProjectName(trimmedProjectName)) {
        addMessage(report, ValidationSeverity::Warning,
            "PROJECT_NAME_DEFAULT",
            "Project name still uses the default or untitled value.",
            document.root.id,
            "projectName");
    }

    if (!trimmedProjectName.empty() && sanitizedProjectName != trimmedProjectName) {
        addMessage(report, ValidationSeverity::Warning,
            "PROJECT_NAME_SANITIZED",
            "Project name will be sanitized before it is used in generated CMake project names.",
            document.root.id,
            "projectName");
    }

    if (trimmedUserSubclassName.empty() || !utils::isValidCppIdentifier(trimmedUserSubclassName)) {
        addMessage(report, ValidationSeverity::Error,
            "PROJECT_SUBCLASS_INVALID",
            "User subclass name must be a valid C++ identifier.",
            document.root.id,
            "userSubclassName");
    }
    else {
        if (trimmedUserSubclassName == "MainWindow") {
            addMessage(report, ValidationSeverity::Error,
                "PROJECT_SUBCLASS_RESERVED",
                "User subclass name must not be MainWindow because the generated base class keeps that name.",
                document.root.id,
                "userSubclassName");
        }
        if (!isSafeFileStem(trimmedUserSubclassName)) {
            addMessage(report, ValidationSeverity::Error,
                "PROJECT_SUBCLASS_FILENAME_INVALID",
                "User subclass name cannot be used safely as a generated source file name.",
                document.root.id,
                "userSubclassName");
        }
    }

    if (!settings.localVisageSourceDirectory.empty()
        && !std::filesystem::exists(settings.localVisageSourceDirectory / "CMakeLists.txt")) {
        addMessage(report, ValidationSeverity::Warning,
            "SETTINGS_LOCAL_VISAGE_MISSING_CMAKELISTS",
            "Local Visage source path does not contain CMakeLists.txt.",
            document.root.id,
            "localVisageSourceDirectory");
    }

    if (trim(settings.visageGitRepository).empty()) {
        addMessage(report, ValidationSeverity::Warning,
            "SETTINGS_VISAGE_REPOSITORY_EMPTY",
            "Visage Git repository is empty and FetchContent fallback metadata will be incomplete.",
            document.root.id,
            "visageGitRepository");
    }

    if (trim(settings.visageGitTag).empty()) {
        addMessage(report, ValidationSeverity::Warning,
            "SETTINGS_VISAGE_TAG_EMPTY",
            "Visage Git tag is empty and FetchContent fallback metadata will be incomplete.",
            document.root.id,
            "visageGitTag");
    }

    if (!document.lookAndFeelId.empty() && model::LookAndFeelRegistry::instance().findById(document.lookAndFeelId) == nullptr) {
        addMessage(report, ValidationSeverity::Warning,
            "PROJECT_LOOK_AND_FEEL_UNKNOWN",
            "Project look and feel id does not match a known preset and export will fall back to the default preset.",
            document.root.id,
            "lookAndFeelId");
    }

    std::vector<const model::WidgetNode*> widgets;
    widgets.reserve(32);
    collectWidgets(document.root, widgets);

    std::unordered_map<std::string, int> idCounts;
    std::unordered_map<std::string, int> nameCounts;
    for (const auto* widget : widgets) {
        if (!widget->id.empty()) {
            ++idCounts[widget->id];
        }
        if (!widget->name.empty()) {
            ++nameCounts[widget->name];
        }
    }

    std::map<std::string, std::vector<const model::WidgetNode*>> radioGroups;
    std::unordered_map<std::string, CallbackUsage> callbackUsages;

    for (const auto* widget : widgets) {
        const model::WidgetDefinition* definition = model::WidgetRegistry::instance().find(widget->type);
        if (definition == nullptr) {
            addMessage(report, ValidationSeverity::Error,
                "WIDGET_TYPE_INVALID",
                "Widget type is not registered for export.",
                widget->id,
                "type");
            continue;
        }

        if (widget->id.empty()) {
            addMessage(report, ValidationSeverity::Error,
                "WIDGET_ID_EMPTY",
                "Widget id must not be empty.",
                widget->id,
                "id");
        }
        else if (idCounts[widget->id] > 1) {
            addMessage(report, ValidationSeverity::Error,
                "WIDGET_ID_DUPLICATE",
                "Widget id is duplicated and generated lookup by id would be ambiguous.",
                widget->id,
                "id");
        }

        if (widget->name.empty()) {
            addMessage(report, ValidationSeverity::Warning,
                "WIDGET_NAME_EMPTY",
                "Widget name is empty and generated name-based helpers will be less useful.",
                widget->id,
                "name");
        }
        else {
            if (nameCounts[widget->name] > 1) {
                addMessage(report, ValidationSeverity::Warning,
                    "WIDGET_NAME_DUPLICATE",
                    "Widget name is duplicated and generated lookup by name may resolve the wrong widget.",
                    widget->id,
                    "name");
            }
            if (utils::sanitizeCppIdentifier(widget->name) != widget->name) {
                addMessage(report, ValidationSeverity::Warning,
                    "WIDGET_NAME_SANITIZED",
                    "Widget name will be sanitized before it is used in generated helper names.",
                    widget->id,
                    "name");
            }
        }

        if (widget->bounds.width < 0.0f) {
            addMessage(report, ValidationSeverity::Error,
                "WIDGET_WIDTH_NEGATIVE",
                "Widget width must not be negative.",
                widget->id,
                "width");
        }
        else if (widget->bounds.width == 0.0f) {
            addMessage(report, ValidationSeverity::Warning,
                "WIDGET_WIDTH_ZERO",
                "Widget width is zero and the widget may not be visible.",
                widget->id,
                "width");
        }

        if (widget->bounds.height < 0.0f) {
            addMessage(report, ValidationSeverity::Error,
                "WIDGET_HEIGHT_NEGATIVE",
                "Widget height must not be negative.",
                widget->id,
                "height");
        }
        else if (widget->bounds.height == 0.0f) {
            addMessage(report, ValidationSeverity::Warning,
                "WIDGET_HEIGHT_ZERO",
                "Widget height is zero and the widget may not be visible.",
                widget->id,
                "height");
        }

        if (!document.isRootWidgetId(widget->id)) {
            const bool outsideRoot = widget->bounds.x < document.root.bounds.x
                || widget->bounds.y < document.root.bounds.y
                || widget->bounds.x + widget->bounds.width > document.root.bounds.x + document.root.bounds.width
                || widget->bounds.y + widget->bounds.height > document.root.bounds.y + document.root.bounds.height;
            if (outsideRoot) {
                addMessage(report, ValidationSeverity::Warning,
                    "WIDGET_OUTSIDE_ROOT",
                    "Widget bounds extend outside the root form bounds.",
                    widget->id,
                    "bounds");
            }
            if (overlapsBottomDockedStatusBar(document, *widget)) {
                addMessage(report, ValidationSeverity::Warning,
                    "WIDGET_OVERLAPS_STATUSBAR",
                    "Widget overlaps the reserved area for a bottom-docked StatusBar.",
                    widget->id,
                    "bounds");
            }
        }

        for (const auto& [key, property] : widget->properties) {
            if (key == "lookAndFeelId") {
                const std::string value = property.isString() ? property.asString() : std::string{};
                if (!value.empty() && model::LookAndFeelRegistry::instance().findById(value) == nullptr) {
                    addMessage(report, ValidationSeverity::Warning,
                        "WIDGET_LOOK_AND_FEEL_UNKNOWN",
                        "Widget look and feel id does not match a known preset.",
                        widget->id,
                        key);
                }
            }

            if (isColorPropertyKey(key) || (widget->type == model::WidgetType::ColorPicker && key == "value")) {
                const std::string colorValue = property.isString() ? property.asString() : std::string{};
                if (!colorValue.empty() && !isValidColorValue(colorValue)) {
                    addMessage(report, ValidationSeverity::Error,
                        "WIDGET_COLOR_INVALID",
                        "Color value must be empty, #RRGGBB, or #AARRGGBB.",
                        widget->id,
                        key);
                }
            }
        }

        const std::string orientation = trim(propertyString(*widget, "orientation"));
        if (widget->type == model::WidgetType::ScrollBar && !orientation.empty()
            && orientation != "Horizontal" && orientation != "Vertical") {
            addMessage(report, ValidationSeverity::Error,
                "WIDGET_ORIENTATION_INVALID",
                "ScrollBar orientation must be Horizontal or Vertical.",
                widget->id,
                "orientation");
        }

        const std::string dock = trim(propertyString(*widget, "dock"));
        if (!dock.empty() && !isKnownDockValue(dock)) {
            addMessage(report, ValidationSeverity::Error,
                "WIDGET_DOCK_INVALID",
                "Dock value must be empty, None, Bottom, Top, Left, Right, or Fill.",
                widget->id,
                "dock");
        }

        if (const auto fontSize = propertyNumber(*widget, "fontSize"); fontSize.has_value() && (*fontSize < 8.0 || *fontSize > 72.0)) {
            addMessage(report, ValidationSeverity::Warning,
                "WIDGET_FONT_SIZE_RANGE",
                "fontSize is outside the supported 8-72 range and export will clamp it.",
                widget->id,
                "fontSize");
        }

        if (const auto borderThickness = propertyNumber(*widget, "borderThickness"); borderThickness.has_value() && (*borderThickness < 0.0 || *borderThickness > 20.0)) {
            addMessage(report, ValidationSeverity::Warning,
                "WIDGET_BORDER_THICKNESS_RANGE",
                "borderThickness is outside the supported 0-20 range and export will clamp it.",
                widget->id,
                "borderThickness");
        }

        if (const auto cornerRadius = propertyNumber(*widget, "cornerRadius"); cornerRadius.has_value() && (*cornerRadius < 0.0 || *cornerRadius > 50.0)) {
            addMessage(report, ValidationSeverity::Warning,
                "WIDGET_CORNER_RADIUS_RANGE",
                "cornerRadius is outside the supported 0-50 range and export will clamp it.",
                widget->id,
                "cornerRadius");
        }

        if (widget->type == model::WidgetType::Slider
            || widget->type == model::WidgetType::ScrollBar
            || widget->type == model::WidgetType::ProgressBar) {
            const double minimum = propertyNumber(*widget, "min").value_or(0.0);
            const double maximum = propertyNumber(*widget, "max").value_or(100.0);
            const double value = propertyNumber(*widget, "value").value_or(minimum);
            if (maximum <= minimum) {
                addMessage(report, ValidationSeverity::Error,
                    "WIDGET_RANGE_INVALID",
                    "Widget max value must be greater than min.",
                    widget->id,
                    "max");
            }
            else if (value < minimum || value > maximum) {
                addMessage(report, ValidationSeverity::Warning,
                    "WIDGET_VALUE_OUT_OF_RANGE",
                    "Widget value is outside the configured min/max range and export will clamp it.",
                    widget->id,
                    "value");
            }
        }

        if (widget->type == model::WidgetType::StatusBar) {
            const int fields = static_cast<int>(std::lround(propertyNumber(*widget, "fields").value_or(1.0)));
            if (fields < 1 || fields > 4) {
                addMessage(report, ValidationSeverity::Error,
                    "STATUSBAR_FIELDS_RANGE",
                    "StatusBar fields must stay in the supported 1-4 range.",
                    widget->id,
                    "fields");
            }
        }

        if (widget->type == model::WidgetType::Image && trim(propertyString(*widget, "source")).empty()) {
            addMessage(report, ValidationSeverity::Warning,
                "IMAGE_SOURCE_EMPTY",
                "Image widget source path is empty.",
                widget->id,
                "source");
        }

        if (widget->type == model::WidgetType::ColorPicker) {
            addMessage(report, ValidationSeverity::Warning,
                "COLORPICKER_RUNTIME_TODO",
                "ColorPicker export is currently static-only and runtime interaction is not generated yet.",
                widget->id,
                "value");
        }

        for (const auto& event : definition->events) {
            const std::string callbackName = trim(propertyString(*widget, event.key));
            if (callbackName.empty()) {
                continue;
            }

            if (!utils::isValidCppIdentifier(callbackName)) {
                addMessage(report, ValidationSeverity::Error,
                    "CALLBACK_NAME_INVALID",
                    "Callback name must be a valid C++ identifier.",
                    widget->id,
                    event.key);
                continue;
            }

            const auto [iterator, inserted] = callbackUsages.emplace(callbackName, CallbackUsage{ event.handlerSignatureKind, widget->id, event.key });
            if (!inserted && iterator->second.signatureKind != event.handlerSignatureKind) {
                addMessage(report, ValidationSeverity::Error,
                    "CALLBACK_SIGNATURE_CONFLICT",
                    "Callback name is reused with incompatible event signature kinds.",
                    widget->id,
                    event.key);
            }
        }

        if (widget->type == model::WidgetType::RadioButton) {
            const std::string group = trim(propertyString(*widget, "group"));
            if (group.empty()) {
                addMessage(report, ValidationSeverity::Warning,
                    "RADIO_GROUP_EMPTY",
                    "RadioButton group is empty.",
                    widget->id,
                    "group");
            }
            radioGroups[group].push_back(widget);
        }
    }

    for (const auto& [group, radios] : radioGroups) {
        if (radios.size() == 1) {
            addMessage(report, ValidationSeverity::Warning,
                "RADIO_GROUP_SINGLE",
                "RadioButton group contains only one RadioButton.",
                radios.front()->id,
                "group");
        }

        int selectedCount = 0;
        const model::WidgetNode* firstSelected = nullptr;
        for (const auto* radio : radios) {
            if (!propertyBool(*radio, "selected", false)) {
                continue;
            }
            ++selectedCount;
            if (firstSelected == nullptr) {
                firstSelected = radio;
            }
        }

        if (selectedCount == 0 && !radios.empty()) {
            addMessage(report, ValidationSeverity::Warning,
                "RADIO_GROUP_NONE_SELECTED",
                "RadioButton group has no selected button.",
                radios.front()->id,
                "selected");
        }
        else if (selectedCount > 1 && firstSelected != nullptr) {
            addMessage(report, ValidationSeverity::Error,
                "RADIO_GROUP_MULTI_SELECTED",
                "RadioButton group has more than one selected button.",
                firstSelected->id,
                "selected");
        }
    }

    return report;
}

} // namespace visiform::validation
