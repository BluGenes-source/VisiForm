#include "serialization/JsonProjectWriter.h"

#include "serialization/JsonProjectWriter.h"

#include "model/WidgetItemUtils.h"
#include "utils/FileUtils.h"

#include <nlohmann/json.hpp>

#include <type_traits>
#include <utility>

namespace visiform::serialization {
namespace {

nlohmann::json rectToJson(const model::Rect& rect)
{
    return {
        { "x", rect.x },
        { "y", rect.y },
        { "width", rect.width },
        { "height", rect.height }
    };
}

nlohmann::json propertyValueToJson(const model::PropertyValue& propertyValue)
{
    return std::visit(
        []<typename T>(const T& value) -> nlohmann::json {
            using ValueType = std::decay_t<T>;

            if constexpr (std::is_same_v<ValueType, std::monostate>) {
                return nullptr;
            }
            else {
                return value;
            }
        },
        propertyValue.value());
}

nlohmann::json propertiesToJson(const model::WidgetNode& widget)
{
    nlohmann::json json = nlohmann::json::object();
    for (const auto& [key, value] : widget.properties) {
        if (key == "items" && model::supportsItemList(widget.type)) {
            json[key] = nlohmann::json::array();
            for (const auto& item : model::splitItems(widget.getStringProperty("items", {}))) {
                json[key].push_back(item);
            }
            continue;
        }

        if (key == "itemActions" && model::supportsItemActions(widget.type)) {
            json[key] = nlohmann::json::array();
            for (const auto& action : model::splitItemActions(widget.getStringProperty("itemActions", {}))) {
                json[key].push_back(action);
            }
            continue;
        }

        json[key] = propertyValueToJson(value);
    }

    return json;
}

nlohmann::json widgetAppearanceOverridesToJson(const model::WidgetLookAndFeelOverrides& overrides)
{
    nlohmann::json json = nlohmann::json::object();
    const auto addString = [&json](const char* key, const std::optional<std::string>& value) {
        if (value.has_value()) {
            json[key] = *value;
        }
    };
    const auto addFloat = [&json](const char* key, const std::optional<float>& value) {
        if (value.has_value()) {
            json[key] = *value;
        }
    };

    addString("controlSurfaceColor", overrides.controlSurfaceColor);
    addString("textColor", overrides.textColor);
    addString("borderColor", overrides.borderColor);
    addString("accentColor", overrides.accentColor);
    addString("focusOutlineColor", overrides.focusOutlineColor);
    addString("highlightEdgeColor", overrides.highlightEdgeColor);
    addString("shadowEdgeColor", overrides.shadowEdgeColor);
    addFloat("borderThickness", overrides.borderThickness);
    addFloat("cornerRadius", overrides.cornerRadius);
    addFloat("controlPadding", overrides.controlPadding);
    return json;
}

nlohmann::json widgetStateAppearanceOverridesToJson(
    const model::WidgetStateLookAndFeelOverrideMap& stateOverrides)
{
    nlohmann::json states = nlohmann::json::object();
    for (const auto& [state, overrides] : stateOverrides) {
        if (state == model::WidgetAppearanceState::Normal || overrides.empty()) {
            continue;
        }
        nlohmann::json json = nlohmann::json::object();
        const auto addString = [&json](const char* key, const std::optional<std::string>& value) {
            if (value.has_value()) {
                json[key] = *value;
            }
        };
        addString("controlSurfaceColor", overrides.controlSurfaceColor);
        addString("textColor", overrides.textColor);
        addString("borderColor", overrides.borderColor);
        addString("accentColor", overrides.accentColor);
        addString("focusOutlineColor", overrides.focusOutlineColor);
        addString("highlightEdgeColor", overrides.highlightEdgeColor);
        addString("shadowEdgeColor", overrides.shadowEdgeColor);
        states[std::string{ model::toString(state) }] = std::move(json);
    }
    return states;
}

nlohmann::json widgetToJson(const model::WidgetNode& widget, const std::string& parentId = {}, int zOrder = 0)
{
    nlohmann::json json;
    json["id"] = widget.id;
    json["name"] = widget.name;
    json["type"] = model::toString(widget.type);
    json["bounds"] = rectToJson(widget.bounds);
    json["parentId"] = parentId;
    json["zOrder"] = zOrder;
    json["properties"] = propertiesToJson(widget);
    const auto stateOverrides = widgetStateAppearanceOverridesToJson(widget.stateAppearanceOverrides);
    if (!widget.appearanceOverrides.empty() || !stateOverrides.empty()) {
        json["appearanceOverrides"] = widgetAppearanceOverridesToJson(widget.appearanceOverrides);
        if (!stateOverrides.empty()) {
            json["appearanceOverrides"]["states"] = stateOverrides;
        }
    }
    json["children"] = nlohmann::json::array();

    for (std::size_t index = 0; index < widget.children.size(); ++index) {
        json["children"].push_back(widgetToJson(widget.children[index], widget.id, static_cast<int>(index)));
    }

    return json;
}

nlohmann::json resourceToJson(const model::ProjectResource& resource)
{
    return {
        { "id", resource.id },
        { "type", model::toString(resource.type) },
        { "displayName", resource.displayName },
        { "sourcePath", resource.sourcePath },
        { "exportRelativePath", resource.exportRelativePath }
    };
}

nlohmann::json lookAndFeelOverridesToJson(const model::LookAndFeelOverrides& overrides)
{
    nlohmann::json json = nlohmann::json::object();
    const auto addString = [&json](const char* key, const std::optional<std::string>& value) {
        if (value.has_value()) {
            json[key] = *value;
        }
    };
    const auto addFloat = [&json](const char* key, const std::optional<float>& value) {
        if (value.has_value()) {
            json[key] = *value;
        }
    };

    addString("applicationSurfaceColor", overrides.applicationSurfaceColor);
    addString("controlSurfaceColor", overrides.controlSurfaceColor);
    addString("recessedSurfaceColor", overrides.recessedSurfaceColor);
    addString("primaryTextColor", overrides.primaryTextColor);
    addString("disabledTextColor", overrides.disabledTextColor);
    addString("borderColor", overrides.borderColor);
    addString("focusOutlineColor", overrides.focusOutlineColor);
    addString("accentColor", overrides.accentColor);
    addString("highlightEdgeColor", overrides.highlightEdgeColor);
    addString("shadowEdgeColor", overrides.shadowEdgeColor);
    addFloat("borderThickness", overrides.borderThickness);
    addFloat("cornerRadius", overrides.cornerRadius);
    addFloat("controlPadding", overrides.controlPadding);
    addFloat("splitterHighlightThickness", overrides.splitterHighlightThickness);
    addFloat("splitterShadowThickness", overrides.splitterShadowThickness);
    return json;
}

} // namespace

std::string JsonProjectWriter::writeToString(const model::ProjectDocument& document) const
{
    nlohmann::json json;
    json["schemaVersion"] = document.schemaVersion;
    json["projectName"] = document.projectName;
    json["executableName"] = document.executableName.empty() ? document.projectName : document.executableName;
    const std::string userSubclassName = document.userSubclassName.empty() ? document.mainFormClassName : document.userSubclassName;
    json["mainFormClassName"] = userSubclassName;
    json["generatedBaseClassName"] = "MainWindow";
    json["userSubclassName"] = userSubclassName;
    json["windowTitle"] = document.windowTitle.empty() ? document.projectName : document.windowTitle;
    json["lookAndFeelId"] = document.lookAndFeelId.empty() ? std::string{"VisiFormDark"} : document.lookAndFeelId;
    if (!document.lookAndFeelOverrides.empty()) {
        json["lookAndFeelOverrides"] = lookAndFeelOverridesToJson(document.lookAndFeelOverrides);
    }
    json["selectedWidgetId"] = document.selectedWidgetId;
    json["resources"] = nlohmann::json::array();
    for (const auto& resource : document.resources) {
        json["resources"].push_back(resourceToJson(resource));
    }
    json["root"] = widgetToJson(document.root);

    return json.dump(2);
}

bool JsonProjectWriter::writeToFile(const model::ProjectDocument& document, const std::filesystem::path& path, std::string& errorMessage) const
{
    errorMessage.clear();

    if (!utils::FileUtils::hasProjectExtension(path)) {
        errorMessage = "Project file must use the .vfb.json extension.";
        return false;
    }

    return utils::FileUtils::writeTextFile(path, writeToString(document), errorMessage);
}

} // namespace visiform::serialization
