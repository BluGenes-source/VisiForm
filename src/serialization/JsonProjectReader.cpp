#include "serialization/JsonProjectReader.h"

#include "serialization/JsonProjectReader.h"

#include "model/WidgetItemUtils.h"
#include "utils/FileUtils.h"

#include <nlohmann/json.hpp>

#include <cctype>
#include <sstream>
#include <vector>

namespace visiform::serialization {
namespace {

std::string sanitizeExecutableName(const std::string& value, const std::string& fallback)
{
    const std::string source = value.empty() ? fallback : value;
    std::string sanitized;
    sanitized.reserve(source.size());
    for (char character : source) {
        if (std::isalnum(static_cast<unsigned char>(character)) != 0 || character == '_' || character == '-') {
            sanitized.push_back(character);
        }
        else if (!std::isspace(static_cast<unsigned char>(character))) {
            sanitized.push_back('_');
        }
        else {
            sanitized.push_back('_');
        }
    }

    while (!sanitized.empty() && sanitized.front() == '_') {
        if (sanitized.size() == 1) {
            break;
        }
        if (std::isdigit(static_cast<unsigned char>(sanitized[1])) == 0) {
            break;
        }
        sanitized.erase(sanitized.begin());
    }

    if (sanitized.empty()) {
        sanitized = fallback;
    }
    if (std::isdigit(static_cast<unsigned char>(sanitized.front())) != 0) {
        sanitized.insert(sanitized.begin(), '_');
    }

    return sanitized;
}

bool requireObject(const nlohmann::json& json, const char* fieldName, std::string& errorMessage)
{
    if (!json.is_object()) {
        errorMessage = std::string(fieldName) + " must be a JSON object.";
        return false;
    }

    return true;
}

bool tryReadString(const nlohmann::json& json, const char* fieldName, std::string& value, std::string& errorMessage)
{
    const auto iterator = json.find(fieldName);
    if (iterator == json.end() || !iterator->is_string()) {
        errorMessage = std::string("Missing or invalid string field: ") + fieldName;
        return false;
    }

    value = iterator->get<std::string>();
    return true;
}

bool tryReadInt(const nlohmann::json& json, const char* fieldName, int& value, std::string& errorMessage)
{
    const auto iterator = json.find(fieldName);
    if (iterator == json.end() || !iterator->is_number_integer()) {
        errorMessage = std::string("Missing or invalid integer field: ") + fieldName;
        return false;
    }

    value = iterator->get<int>();
    return true;
}

bool tryReadFloat(const nlohmann::json& json, const char* fieldName, float& value, std::string& errorMessage)
{
    const auto iterator = json.find(fieldName);
    if (iterator == json.end() || !iterator->is_number()) {
        errorMessage = std::string("Missing or invalid numeric field: ") + fieldName;
        return false;
    }

    value = iterator->get<float>();
    return true;
}

bool parsePropertyValue(const nlohmann::json& jsonValue, model::PropertyValue& propertyValue, std::string& errorMessage)
{
    if (jsonValue.is_null()) {
        propertyValue = model::PropertyValue{};
        return true;
    }
    if (jsonValue.is_boolean()) {
        propertyValue = model::PropertyValue{ jsonValue.get<bool>() };
        return true;
    }
    if (jsonValue.is_number_integer()) {
        propertyValue = model::PropertyValue{ jsonValue.get<int>() };
        return true;
    }
    if (jsonValue.is_number_float()) {
        propertyValue = model::PropertyValue{ jsonValue.get<float>() };
        return true;
    }
    if (jsonValue.is_string()) {
        propertyValue = model::PropertyValue{ jsonValue.get<std::string>() };
        return true;
    }

    errorMessage = "Unsupported property value type. Expected null, boolean, integer, float, or string.";
    return false;
}

bool parseRect(const nlohmann::json& json, model::Rect& rect, std::string& errorMessage)
{
    if (!requireObject(json, "bounds", errorMessage)) {
        return false;
    }

    if (!tryReadFloat(json, "x", rect.x, errorMessage)
        || !tryReadFloat(json, "y", rect.y, errorMessage)
        || !tryReadFloat(json, "width", rect.width, errorMessage)
        || !tryReadFloat(json, "height", rect.height, errorMessage)) {
        return false;
    }

    if (!rect.isValid()) {
        errorMessage = "Widget bounds must have positive width and height.";
        return false;
    }

    return true;
}

bool parseProperties(const nlohmann::json& json, std::map<std::string, model::PropertyValue>& properties, std::string& errorMessage)
{
    properties.clear();
    if (json.is_null()) {
        return true;
    }

    if (!json.is_object()) {
        errorMessage = "Widget properties must be a JSON object.";
        return false;
    }

    for (const auto& [key, value] : json.items()) {
        if (key == "items" && value.is_array()) {
            std::vector<std::string> items;
            items.reserve(value.size());
            for (const auto& itemValue : value) {
                if (!itemValue.is_string()) {
                    errorMessage = "Invalid property 'items': every array element must be a string.";
                    return false;
                }

                items.push_back(itemValue.get<std::string>());
            }

            properties.insert_or_assign(key, model::PropertyValue{ model::joinItems(items) });
            continue;
        }

        model::PropertyValue propertyValue;
        if (!parsePropertyValue(value, propertyValue, errorMessage)) {
            errorMessage = "Invalid property '" + key + "': " + errorMessage;
            return false;
        }

        properties.insert_or_assign(key, std::move(propertyValue));
    }

    return true;
}

bool parseWidget(const nlohmann::json& json, model::WidgetNode& widget, std::string& errorMessage)
{
    if (!requireObject(json, "root", errorMessage)) {
        return false;
    }

    std::string typeString;
    if (!tryReadString(json, "id", widget.id, errorMessage)
        || !tryReadString(json, "name", widget.name, errorMessage)
        || !tryReadString(json, "type", typeString, errorMessage)) {
        return false;
    }

    const auto widgetType = model::widgetTypeFromString(typeString);
    if (!widgetType.has_value()) {
        errorMessage = "Unsupported widget type: " + typeString;
        return false;
    }
    widget.type = *widgetType;

    if (const auto iterator = json.find("parentId"); iterator != json.end()) {
        if (!iterator->is_string()) {
            errorMessage = "parentId must be a string when present.";
            return false;
        }
        widget.parentId = iterator->get<std::string>();
    }
    else {
        widget.parentId.clear();
    }

    if (const auto iterator = json.find("zOrder"); iterator != json.end()) {
        if (!iterator->is_number_integer()) {
            errorMessage = "zOrder must be an integer when present.";
            return false;
        }
        widget.zOrder = iterator->get<int>();
    }
    else {
        widget.zOrder = 0;
    }

    const auto boundsIterator = json.find("bounds");
    if (boundsIterator == json.end()) {
        errorMessage = "Missing required widget field: bounds";
        return false;
    }
    if (!parseRect(*boundsIterator, widget.bounds, errorMessage)) {
        return false;
    }

    const auto propertiesIterator = json.find("properties");
    if (propertiesIterator != json.end() && !parseProperties(*propertiesIterator, widget.properties, errorMessage)) {
        return false;
    }
    if (propertiesIterator == json.end()) {
        widget.properties.clear();
    }

    model::normalizeItemListProperties(widget);
    model::normalizeTableGridProperties(widget);
    model::normalizeTreeViewProperties(widget);

    const auto childrenIterator = json.find("children");
    widget.children.clear();
    if (childrenIterator != json.end()) {
        if (!childrenIterator->is_array()) {
            errorMessage = "children must be an array when present.";
            return false;
        }

        for (const auto& childJson : *childrenIterator) {
            model::WidgetNode child;
            if (!parseWidget(childJson, child, errorMessage)) {
                return false;
            }

            widget.children.push_back(std::move(child));
        }
    }

    return true;
}

bool parseWidgetArray(const nlohmann::json& json, const char* fieldName, std::vector<model::WidgetNode>& widgets, std::string& errorMessage)
{
    widgets.clear();

    const auto iterator = json.find(fieldName);
    if (iterator == json.end()) {
        return true;
    }
    if (!iterator->is_array()) {
        errorMessage = std::string(fieldName) + " must be an array when present.";
        return false;
    }

    for (const auto& widgetJson : *iterator) {
        model::WidgetNode widget;
        if (!parseWidget(widgetJson, widget, errorMessage)) {
            return false;
        }

        widgets.push_back(std::move(widget));
    }

    return true;
}

bool parseResource(const nlohmann::json& json, model::ProjectResource& resource, std::string& errorMessage)
{
    if (!requireObject(json, "resources", errorMessage)) {
        return false;
    }

    std::string typeString;
    if (!tryReadString(json, "id", resource.id, errorMessage)
        || !tryReadString(json, "type", typeString, errorMessage)
        || !tryReadString(json, "displayName", resource.displayName, errorMessage)
        || !tryReadString(json, "sourcePath", resource.sourcePath, errorMessage)
        || !tryReadString(json, "exportRelativePath", resource.exportRelativePath, errorMessage)) {
        return false;
    }

    const auto resourceType = model::projectResourceTypeFromString(typeString);
    if (!resourceType.has_value()) {
        errorMessage = "Unsupported resource type: " + typeString;
        return false;
    }

    resource.type = *resourceType;
    return true;
}

} // namespace

std::optional<model::ProjectDocument> JsonProjectReader::readFromString(const std::string& jsonText, std::string& errorMessage) const
{
    errorMessage.clear();

    try {
        const auto json = nlohmann::json::parse(jsonText.empty() ? "{}" : jsonText);
        if (!json.is_object()) {
            errorMessage = "Project file must contain a JSON object.";
            return std::nullopt;
        }

        model::ProjectDocument document;
        if (!tryReadInt(json, "schemaVersion", document.schemaVersion, errorMessage)) {
            return std::nullopt;
        }
        if (document.schemaVersion != 1) {
            errorMessage = "Unsupported schemaVersion: " + std::to_string(document.schemaVersion);
            return std::nullopt;
        }

        if (!tryReadString(json, "projectName", document.projectName, errorMessage)
            || !tryReadString(json, "mainFormClassName", document.mainFormClassName, errorMessage)) {
            return std::nullopt;
        }
        if (document.projectName.empty()) {
            document.projectName = "VisiFormProject";
        }
        if (const auto iterator = json.find("lookAndFeelId"); iterator != json.end()) {
            if (!iterator->is_string()) {
                errorMessage = "lookAndFeelId must be a string when present.";
                return std::nullopt;
            }
            document.lookAndFeelId = iterator->get<std::string>();
        }
        document.generatedBaseClassName = "MainWindow";
        document.userSubclassName = (document.mainFormClassName.empty() || document.mainFormClassName == "MainWindow")
            ? "AppMainWindow"
            : document.mainFormClassName;
        if (const auto iterator = json.find("userSubclassName"); iterator != json.end()) {
            if (!iterator->is_string()) {
                errorMessage = "userSubclassName must be a string when present.";
                return std::nullopt;
            }
            document.userSubclassName = iterator->get<std::string>();
        }
        if (const auto iterator = json.find("executableName"); iterator != json.end()) {
            if (!iterator->is_string()) {
                errorMessage = "executableName must be a string when present.";
                return std::nullopt;
            }
            document.executableName = iterator->get<std::string>();
        }
        document.executableName = sanitizeExecutableName(document.executableName, document.projectName);
        if (const auto iterator = json.find("windowTitle"); iterator != json.end()) {
            if (!iterator->is_string()) {
                errorMessage = "windowTitle must be a string when present.";
                return std::nullopt;
            }
            document.windowTitle = iterator->get<std::string>();
        }
        if (const auto iterator = json.find("generatedBaseClassName"); iterator != json.end()) {
            if (!iterator->is_string()) {
                errorMessage = "generatedBaseClassName must be a string when present.";
                return std::nullopt;
            }
            (void)iterator;
        }
        document.mainFormClassName = document.userSubclassName;

        document.resources.clear();
        if (const auto iterator = json.find("resources"); iterator != json.end()) {
            if (!iterator->is_array()) {
                errorMessage = "resources must be an array when present.";
                return std::nullopt;
            }

            for (const auto& resourceJson : *iterator) {
                model::ProjectResource resource;
                if (!parseResource(resourceJson, resource, errorMessage)) {
                    return std::nullopt;
                }

                document.resources.push_back(std::move(resource));
            }
        }

        std::vector<model::WidgetNode> legacyWidgets;
        if (!parseWidgetArray(json, "widgets", legacyWidgets, errorMessage)) {
            return std::nullopt;
        }

        const auto rootIterator = json.find("root");
        if (rootIterator != json.end()) {
            if (!parseWidget(*rootIterator, document.root, errorMessage)) {
                return std::nullopt;
            }
        }
        else if (!legacyWidgets.empty()) {
            document.root = model::ProjectDocument::createDefault().root;
            document.root.children.clear();
            document.root.setProperty("title", document.windowTitle.empty() ? document.projectName : document.windowTitle);
        }
        else {
            errorMessage = "Missing required field: root";
            return std::nullopt;
        }

        for (auto& widget : legacyWidgets) {
            document.root.appendChild(std::move(widget));
        }

        document.refreshHierarchyMetadata();
        document.applyDockLayout();

        if (document.windowTitle.empty()) {
            document.windowTitle = document.root.getStringProperty("title", document.projectName);
        }
        if (document.windowTitle.empty()) {
            document.windowTitle = document.projectName;
        }
        document.root.setProperty("title", document.windowTitle);

        const auto selectedWidgetIterator = json.find("selectedWidgetId");
        if (selectedWidgetIterator != json.end()) {
            if (!selectedWidgetIterator->is_string()) {
                errorMessage = "selectedWidgetId must be a string when present.";
                return std::nullopt;
            }

            document.selectedWidgetId = selectedWidgetIterator->get<std::string>();
        }

        if (document.selectedWidgetId.empty() || document.findWidgetById(document.selectedWidgetId) == nullptr) {
            document.selectedWidgetId = document.root.id;
        }

        document.clearDirty();
        return document;
    }
    catch (const nlohmann::json::exception& exception) {
        errorMessage = std::string("Failed to parse project JSON: ") + exception.what();
        return std::nullopt;
    }
}

std::optional<model::ProjectDocument> JsonProjectReader::readFromFile(const std::filesystem::path& path, std::string& errorMessage) const
{
    std::string jsonText;
    if (!utils::FileUtils::readTextFile(path, jsonText, errorMessage)) {
        return std::nullopt;
    }

    return readFromString(jsonText, errorMessage);
}

} // namespace visiform::serialization
