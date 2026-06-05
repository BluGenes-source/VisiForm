#include "validation/ProjectValidator.h"

#include "model/LookAndFeelRegistry.h"
#include "model/PropertyValue.h"
#include "model/WidgetItemUtils.h"
#include "model/WidgetRegistry.h"
#include "utils/CppIdentifier.h"
#include "utils/FileUtils.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
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

bool isKnownAnchorValue(std::string_view value)
{
    return value.empty() || model::anchorModeFromString(std::string{ value }).has_value();
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
        || key == "normalFillColor"
        || key == "pressedFillColor"
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
    if (property->isString()) {
        const std::string text = trim(property->asString());
        if (text.empty() || text == "<unset>") {
            return std::nullopt;
        }

        std::istringstream stream(text);
        double value = 0.0;
        char trailing = '\0';
        if (!(stream >> value)) {
            return std::nullopt;
        }
        if (stream >> trailing) {
            return std::nullopt;
        }
        return value;
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

std::string normalizedPathText(const std::filesystem::path& path)
{
    return utils::FileUtils::normalizeSeparators(path.lexically_normal().generic_string());
}

std::string imagePathProperty(const model::WidgetNode& widget)
{
    const std::string imagePath = trim(propertyString(widget, "imagePath"));
    if (!imagePath.empty()) {
        return imagePath;
    }

    return trim(propertyString(widget, "source"));
}

bool isKnownImageScaleMode(std::string_view value)
{
    return value.empty()
        || value == "Stretch"
        || value == "Fit"
        || value == "Fill"
        || value == "Center";
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

bool isKnownLayoutModeValue(std::string_view value)
{
    return value.empty() || model::layoutModeFromString(std::string{ value }).has_value();
}

bool hasHierarchyCycle(const std::unordered_map<std::string, const model::WidgetNode*>& widgetsById, const model::WidgetNode& widget)
{
    std::unordered_set<std::string> visitedIds;
    if (!widget.id.empty()) {
        visitedIds.insert(widget.id);
    }

    std::string currentParentId = trim(widget.parentId);
    while (!currentParentId.empty()) {
        if (!visitedIds.insert(currentParentId).second) {
            return true;
        }

        const auto iterator = widgetsById.find(currentParentId);
        if (iterator == widgetsById.end()) {
            return false;
        }

        currentParentId = trim(iterator->second->parentId);
    }

    return false;
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

    std::unordered_map<std::string, int> resourceIdCounts;
    std::unordered_map<std::string, std::string> exportPathSources;
    for (const auto& resource : document.resources) {
        if (!resource.id.empty()) {
            ++resourceIdCounts[resource.id];
        }
    }

    for (const auto& resource : document.resources) {
        if (resource.id.empty()) {
            addMessage(report, ValidationSeverity::Error,
                "RESOURCE_ID_EMPTY",
                "Project resource id must not be empty.",
                document.root.id,
                "resources.id");
        }
        else if (resourceIdCounts[resource.id] > 1) {
            addMessage(report, ValidationSeverity::Error,
                "RESOURCE_ID_DUPLICATE",
                "Project resource id is duplicated.",
                document.root.id,
                "resources.id");
        }

        if (trim(resource.displayName).empty()) {
            addMessage(report, ValidationSeverity::Warning,
                "RESOURCE_DISPLAY_NAME_EMPTY",
                "Project resource display name is empty.",
                document.root.id,
                resource.id);
        }

        if (trim(resource.sourcePath).empty()) {
            addMessage(report, ValidationSeverity::Error,
                "RESOURCE_SOURCE_EMPTY",
                "Project resource source path must not be empty.",
                document.root.id,
                resource.id);
        }
        else if (!std::filesystem::exists(std::filesystem::path{ resource.sourcePath })) {
            addMessage(report, ValidationSeverity::Error,
                "RESOURCE_SOURCE_MISSING",
                "Project resource source file does not exist.",
                document.root.id,
                resource.id);
        }

        if (trim(resource.exportRelativePath).empty()) {
            addMessage(report, ValidationSeverity::Error,
                "RESOURCE_EXPORT_PATH_EMPTY",
                "Project resource export path must not be empty.",
                document.root.id,
                resource.id);
        }
        else {
            const std::filesystem::path exportPath{ utils::FileUtils::sanitizeRelativeAssetPath(resource.exportRelativePath) };
            if (!utils::FileUtils::isRelativePathWithinDirectory(exportPath, "assets")) {
                addMessage(report, ValidationSeverity::Error,
                    "RESOURCE_EXPORT_PATH_INVALID",
                    "Project resource export path must stay inside assets/.",
                    document.root.id,
                    resource.id);
            }
            else {
                const std::string normalizedExportPath = normalizedPathText(exportPath);
                const std::string normalizedSourcePath = normalizedPathText(std::filesystem::path{ resource.sourcePath });
                const auto [iterator, inserted] = exportPathSources.emplace(normalizedExportPath, normalizedSourcePath);
                if (!inserted && iterator->second != normalizedSourcePath) {
                    addMessage(report, ValidationSeverity::Error,
                        "RESOURCE_EXPORT_PATH_DUPLICATE",
                        "Project resources cannot share an export path unless they use the same source file.",
                        document.root.id,
                        resource.id);
                }
            }
        }
    }

    std::vector<const model::WidgetNode*> widgets;
    widgets.reserve(32);
    collectWidgets(document.root, widgets);

    std::unordered_map<std::string, int> idCounts;
    std::unordered_map<std::string, int> nameCounts;
    std::unordered_map<std::string, const model::WidgetNode*> widgetsById;
    for (const auto* widget : widgets) {
        if (!widget->id.empty()) {
            ++idCounts[widget->id];
            widgetsById.emplace(widget->id, widget);
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

        const model::WidgetNode* actualParent = document.findParentOf(widget->id);
        const std::string storedParentId = trim(widget->parentId);
        if (document.isRootWidgetId(widget->id)) {
            if (!storedParentId.empty()) {
                addMessage(report, ValidationSeverity::Error,
                    "ROOT_PARENT_INVALID",
                    "Root form must not have a parentId.",
                    widget->id,
                    "parentId");
            }
        }
        else {
            if (storedParentId.empty()) {
                addMessage(report, ValidationSeverity::Error,
                    "WIDGET_PARENT_EMPTY",
                    "Non-root widgets must store a parentId.",
                    widget->id,
                    "parentId");
            }
            else if (storedParentId == widget->id) {
                addMessage(report, ValidationSeverity::Error,
                    "WIDGET_PARENT_SELF",
                    "Widget parentId must not reference the widget itself.",
                    widget->id,
                    "parentId");
            }
            else {
                const auto parentIterator = widgetsById.find(storedParentId);
                if (parentIterator == widgetsById.end()) {
                    addMessage(report, ValidationSeverity::Error,
                        "WIDGET_PARENT_MISSING",
                        "Widget parentId does not match any widget in the document.",
                        widget->id,
                        "parentId");
                }
                else if (actualParent != nullptr && actualParent->id != storedParentId) {
                    addMessage(report, ValidationSeverity::Error,
                        "WIDGET_PARENT_MISMATCH",
                        "Widget parentId does not match the actual parent-child hierarchy.",
                        widget->id,
                        "parentId");
                }
            }

            if (hasHierarchyCycle(widgetsById, *widget)) {
                addMessage(report, ValidationSeverity::Error,
                    "WIDGET_HIERARCHY_CYCLE",
                    "Widget parentId metadata contains a hierarchy cycle.",
                    widget->id,
                    "parentId");
            }
        }

        if (!definition->canContainChildren && !widget->children.empty()) {
            addMessage(report, ValidationSeverity::Error,
                "WIDGET_CHILDREN_NOT_ALLOWED",
                "This widget type cannot contain child widgets.",
                widget->id,
                "children");
        }

        if (actualParent != nullptr && !model::WidgetRegistry::instance().canContainChildren(actualParent->type)) {
            addMessage(report, ValidationSeverity::Error,
                "WIDGET_PARENT_NOT_CONTAINER",
                "Widget is attached to a parent type that cannot contain children.",
                widget->id,
                "parentId");
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
        else if (dock == "Fill" && actualParent != nullptr) {
            int fillSiblingCount = 0;
            for (const auto& sibling : actualParent->children) {
                if (trim(propertyString(sibling, "dock")) == "Fill") {
                    ++fillSiblingCount;
                }
            }

            if (fillSiblingCount > 1) {
                addMessage(report, ValidationSeverity::Warning,
                    "WIDGET_DOCK_FILL_CONFLICT",
                    "Multiple Fill-docked widgets in the same parent may overlap.",
                    widget->id,
                    "dock");
            }
        }

        const std::string anchor = trim(propertyString(*widget, "anchor"));
        if (!anchor.empty() && !isKnownAnchorValue(anchor)) {
            addMessage(report, ValidationSeverity::Error,
                "WIDGET_ANCHOR_INVALID",
                "Anchor value is not one of the supported presets.",
                widget->id,
                "anchor");
        }

        const std::string layoutMode = trim(propertyString(*widget, "layoutMode"));
        if (!layoutMode.empty() && !isKnownLayoutModeValue(layoutMode)) {
            addMessage(report, ValidationSeverity::Error,
                "WIDGET_LAYOUT_MODE_INVALID",
                "layoutMode must be empty, Absolute, Horizontal, Vertical, Grid, or TabPage.",
                widget->id,
                "layoutMode");
        }
        else if (!definition->canContainChildren && !layoutMode.empty()) {
            addMessage(report, ValidationSeverity::Warning,
                "WIDGET_LAYOUT_MODE_UNUSED",
                "layoutMode is stored on a widget type that does not contain children.",
                widget->id,
                "layoutMode");
        }

        if (const auto fontSize = propertyNumber(*widget, "fontSize"); fontSize.has_value() && (*fontSize < 8.0 || *fontSize > 72.0)) {
            addMessage(report, ValidationSeverity::Warning,
                "WIDGET_FONT_SIZE_RANGE",
                "fontSize is outside the supported 8-72 range and export will clamp it.",
                widget->id,
                "fontSize");
        }

        if (const auto borderThickness = propertyNumber(*widget, "borderThickness"); borderThickness.has_value() && (*borderThickness < 1.0 || *borderThickness > 25.0)) {
            addMessage(report, ValidationSeverity::Warning,
                "WIDGET_BORDER_THICKNESS_RANGE",
                "borderThickness is outside the supported 1-25 range and export will clamp it.",
                widget->id,
                "borderThickness");
        }
        if (const auto* borderThicknessProperty = widget->getProperty("borderThickness"); borderThicknessProperty != nullptr && borderThicknessProperty->isString()) {
            const std::string text = trim(borderThicknessProperty->asString());
            if (!text.empty() && text != "<unset>" && !propertyNumber(*widget, "borderThickness").has_value()) {
                addMessage(report, ValidationSeverity::Error,
                    "WIDGET_BORDER_THICKNESS_INVALID",
                    "borderThickness must be a numeric value from 1 to 25.",
                    widget->id,
                    "borderThickness");
            }
        }

        if (const auto cornerRadius = propertyNumber(*widget, "cornerRadius"); cornerRadius.has_value() && (*cornerRadius < 1.0 || *cornerRadius > 25.0)) {
            addMessage(report, ValidationSeverity::Warning,
                "WIDGET_CORNER_RADIUS_RANGE",
                "cornerRadius is outside the supported 1-25 range and export will clamp it.",
                widget->id,
                "cornerRadius");
        }
        if (const auto* cornerRadiusProperty = widget->getProperty("cornerRadius"); cornerRadiusProperty != nullptr && cornerRadiusProperty->isString()) {
            const std::string text = trim(cornerRadiusProperty->asString());
            if (!text.empty() && text != "<unset>" && !propertyNumber(*widget, "cornerRadius").has_value()) {
                addMessage(report, ValidationSeverity::Error,
                    "WIDGET_CORNER_RADIUS_INVALID",
                    "cornerRadius must be a numeric value from 1 to 25.",
                    widget->id,
                    "cornerRadius");
            }
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

            if (actualParent != &document.root) {
                addMessage(report, ValidationSeverity::Warning,
                    "STATUSBAR_PARENT_UNSUPPORTED",
                    "StatusBar bottom-dock behavior is only validated on the root form.",
                    widget->id,
                    "parentId");
            }

            if (dock.empty() || dock == "None") {
                addMessage(report, ValidationSeverity::Warning,
                    "STATUSBAR_DOCK_RECOMMENDED",
                    "StatusBar should use Bottom dock to preserve the expected root status strip behavior.",
                    widget->id,
                    "dock");
            }
            else if (dock != "Bottom") {
                addMessage(report, ValidationSeverity::Warning,
                    "STATUSBAR_DOCK_NON_BOTTOM",
                    "StatusBar is expected to use Bottom dock on the root form.",
                    widget->id,
                    "dock");
            }
        }

        if (model::supportsItemList(widget->type)) {
            const auto items = model::splitItems(propertyString(*widget, "items"));
            const std::string selectedIndexKey = std::string(model::selectedItemIndexPropertyKey(widget->type));
            const int selectedIndex = widget->getIntProperty(selectedIndexKey, items.empty() ? -1 : 0);
            const int safeSelectedIndex = model::sanitizeSelectedIndex(items, selectedIndex);
            if (selectedIndex != safeSelectedIndex) {
                std::string code = "WIDGET_SELECTED_INDEX_OUT_OF_RANGE";
                std::string message = items.empty()
                    ? "selectedIndex should be -1 when the item list is empty."
                    : "selectedIndex is outside the available item range and will be clamped.";
                if (widget->type == model::WidgetType::MenuBar) {
                    code = "MENUBAR_SELECTED_INDEX_OUT_OF_RANGE";
                    message = "MenuBar selectedMenuIndex is out of range.";
                }
                else if (widget->type == model::WidgetType::ToolBar) {
                    code = "TOOLBAR_SELECTED_INDEX_OUT_OF_RANGE";
                    message = "ToolBar selectedToolIndex is out of range.";
                }

                addMessage(report, ValidationSeverity::Warning,
                    code,
                    message,
                    widget->id,
                    selectedIndexKey);
            }

            if (widget->type == model::WidgetType::ComboBox && items.empty()) {
                addMessage(report, ValidationSeverity::Warning,
                    "COMBOBOX_ITEMS_EMPTY",
                    "ComboBox has no items, so no selection can be shown.",
                    widget->id,
                    "items");
            }
            else if (widget->type == model::WidgetType::MenuBar) {
                if (items.empty()) {
                    addMessage(report, ValidationSeverity::Warning,
                        "MENUBAR_ITEMS_EMPTY",
                        "MenuBar has no menu items.",
                        widget->id,
                        "items");
                }

                if (dock != "Top") {
                    addMessage(report, ValidationSeverity::Warning,
                        "MENUBAR_DOCK_TOP_RECOMMENDED",
                        "MenuBar is usually expected to use Dock = Top.",
                        widget->id,
                        "dock");
                }
            }
            else if (widget->type == model::WidgetType::ToolBar) {
                if (items.empty()) {
                    addMessage(report, ValidationSeverity::Warning,
                        "TOOLBAR_ITEMS_EMPTY",
                        "ToolBar has no tool items.",
                        widget->id,
                        "items");
                }

                if (dock != "Top") {
                    addMessage(report, ValidationSeverity::Warning,
                        "TOOLBAR_DOCK_TOP_RECOMMENDED",
                        "ToolBar is usually expected to use Dock = Top.",
                        widget->id,
                        "dock");
                }
            }

            if (model::supportsItemActions(widget->type)) {
                const auto rawActions = model::splitItemActions(propertyString(*widget, "itemActions"));
                const auto actions = model::getWidgetItemActions(*widget);
                if (rawActions.size() > items.size()) {
                    addMessage(report, ValidationSeverity::Warning,
                        widget->type == model::WidgetType::MenuBar
                            ? "MENUBAR_ITEM_ACTION_COUNT_MISMATCH"
                            : "TOOLBAR_ITEM_ACTION_COUNT_MISMATCH",
                        widget->type == model::WidgetType::MenuBar
                            ? "MenuBar itemActions has more entries than items. Extra action bindings will be ignored."
                            : "ToolBar itemActions has more entries than items. Extra action bindings will be ignored.",
                        widget->id,
                        "itemActions");
                }

                for (std::size_t index = 0; index < actions.size(); ++index) {
                    const std::string callbackName = trim(actions[index]);
                    if (callbackName.empty()) {
                        continue;
                    }

                    if (!utils::isValidCppIdentifier(callbackName)) {
                        addMessage(report, ValidationSeverity::Error,
                            widget->type == model::WidgetType::MenuBar
                                ? "MENUBAR_ITEM_ACTION_NAME_INVALID"
                                : "TOOLBAR_ITEM_ACTION_NAME_INVALID",
                            (widget->type == model::WidgetType::MenuBar
                                    ? std::string{ "Invalid MenuBar item action name at item index " }
                                    : std::string{ "Invalid ToolBar item action name at item index " })
                                + std::to_string(index) + ".",
                            widget->id,
                            "itemActions");
                        continue;
                    }

                    const auto [iterator, inserted] = callbackUsages.emplace(callbackName, CallbackUsage{ "void_event", widget->id, "itemActions" });
                    if (!inserted && iterator->second.signatureKind != "void_event") {
                        addMessage(report, ValidationSeverity::Error,
                            "CALLBACK_SIGNATURE_CONFLICT",
                            "Callback name is reused with incompatible event signature kinds.",
                            widget->id,
                            "itemActions");
                    }
                }
            }
        }

        if (model::supportsTableGrid(widget->type)) {
            const auto columns = model::splitTableColumns(propertyString(*widget, "columns"));
            const auto rowsData = model::splitTableRows(propertyString(*widget, "rows"));
            if (columns.empty()) {
                addMessage(report, ValidationSeverity::Warning,
                    "TABLEGRID_COLUMNS_EMPTY",
                    "Table/Grid has no columns.",
                    widget->id,
                    "columns");
            }
            else if (std::any_of(columns.begin(), columns.end(), [](const std::string& column) { return trim(column).empty(); })) {
                addMessage(report, ValidationSeverity::Warning,
                    "TABLEGRID_COLUMN_EMPTY",
                    "Table/Grid has an empty column name.",
                    widget->id,
                    "columns");
            }

            const int selectedRow = widget->getIntProperty("selectedRow", rowsData.empty() ? -1 : 0);
            const int selectedColumn = widget->getIntProperty("selectedColumn", columns.empty() ? -1 : 0);
            const auto safeSelection = model::clampSelectedCell(columns, rowsData, selectedRow, selectedColumn);
            if (selectedRow != safeSelection.row) {
                addMessage(report, ValidationSeverity::Warning,
                    "TABLEGRID_SELECTED_ROW_OUT_OF_RANGE",
                    "selectedRow is out of range.",
                    widget->id,
                    "selectedRow");
            }
            if (selectedColumn != safeSelection.column) {
                addMessage(report, ValidationSeverity::Warning,
                    "TABLEGRID_SELECTED_COLUMN_OUT_OF_RANGE",
                    "selectedColumn is out of range.",
                    widget->id,
                    "selectedColumn");
            }

            const double rowHeight = propertyNumber(*widget, "rowHeight").value_or(28.0);
            if (rowHeight <= 0.0) {
                addMessage(report, ValidationSeverity::Error,
                    "TABLEGRID_ROW_HEIGHT_INVALID",
                    "rowHeight must be greater than zero.",
                    widget->id,
                    "rowHeight");
            }

            const double headerHeight = propertyNumber(*widget, "headerHeight").value_or(30.0);
            if (headerHeight < 0.0) {
                addMessage(report, ValidationSeverity::Error,
                    "TABLEGRID_HEADER_HEIGHT_INVALID",
                    "headerHeight must be zero or greater.",
                    widget->id,
                    "headerHeight");
            }
        }

        if (model::supportsTreeNodes(widget->type)) {
            const std::string nodesText = propertyString(*widget, "nodes");
            const auto parseResult = model::parseTreeNodes(nodesText);
            if (!model::validateTreeNodeData(nodesText)) {
                addMessage(report, ValidationSeverity::Error,
                    "TREEVIEW_INDENTATION_INVALID",
                    "TreeView node indentation is invalid.",
                    widget->id,
                    "nodes");
            }

            std::set<std::string> nodePaths;
            for (const auto& node : parseResult.nodes) {
                nodePaths.insert(node.path);
            }

            const std::string selectedNodePath = trim(propertyString(*widget, "selectedNodePath"));
            if (!selectedNodePath.empty() && !nodePaths.contains(selectedNodePath)) {
                addMessage(report, ValidationSeverity::Warning,
                    "TREEVIEW_SELECTED_NODE_MISSING",
                    "TreeView selected node does not exist.",
                    widget->id,
                    "selectedNodePath");
            }

            for (const auto& expandedNodePath : model::splitTreeNodePaths(propertyString(*widget, "expandedNodePaths"))) {
                if (nodePaths.contains(expandedNodePath)) {
                    continue;
                }

                addMessage(report, ValidationSeverity::Warning,
                    "TREEVIEW_EXPANDED_NODE_MISSING",
                    "TreeView expanded node does not exist.",
                    widget->id,
                    "expandedNodePaths");
                break;
            }
        }

        if (widget->type == model::WidgetType::Image) {
            const std::string resourceId = trim(propertyString(*widget, "resourceId"));
            const std::string imagePath = imagePathProperty(*widget);
            const std::string scaleMode = trim(propertyString(*widget, "scaleMode"));

            if (!isKnownImageScaleMode(scaleMode)) {
                addMessage(report, ValidationSeverity::Error,
                    "IMAGE_SCALE_MODE_INVALID",
                    "Image scaleMode must be Stretch, Fit, Fill, or Center.",
                    widget->id,
                    "scaleMode");
            }

            if (!resourceId.empty()) {
                const auto* resource = document.findResourceById(resourceId);
                if (resource == nullptr) {
                    addMessage(report, ValidationSeverity::Error,
                        "IMAGE_RESOURCE_MISSING",
                        "Image widget resourceId does not match a project resource.",
                        widget->id,
                        "resourceId");
                }
                else if (resource->type != model::ProjectResourceType::Image) {
                    addMessage(report, ValidationSeverity::Error,
                        "IMAGE_RESOURCE_TYPE_INVALID",
                        "Image widget resourceId must point to an Image project resource.",
                        widget->id,
                        "resourceId");
                }
            }

            if (resourceId.empty() && imagePath.empty()) {
                addMessage(report, ValidationSeverity::Warning,
                    "IMAGE_SOURCE_EMPTY",
                    "Image widget has neither a managed resourceId nor a fallback imagePath.",
                    widget->id,
                    "resourceId");
            }
            else if (resourceId.empty() && !imagePath.empty() && !std::filesystem::exists(std::filesystem::path{ imagePath })) {
                addMessage(report, ValidationSeverity::Error,
                    "IMAGE_PATH_MISSING",
                    "Image widget fallback imagePath does not exist.",
                    widget->id,
                    "imagePath");
            }
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
                    "Invalid callback name for " + event.key + ".",
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
