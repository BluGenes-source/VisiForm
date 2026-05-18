#include "serialization/JsonProjectWriter.h"

#include "serialization/JsonProjectWriter.h"

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

nlohmann::json propertiesToJson(const std::map<std::string, model::PropertyValue>& properties)
{
    nlohmann::json json = nlohmann::json::object();
    for (const auto& [key, value] : properties) {
        json[key] = propertyValueToJson(value);
    }

    return json;
}

nlohmann::json widgetToJson(const model::WidgetNode& widget)
{
    nlohmann::json json;
    json["id"] = widget.id;
    json["name"] = widget.name;
    json["type"] = model::toString(widget.type);
    json["bounds"] = rectToJson(widget.bounds);
    json["properties"] = propertiesToJson(widget.properties);
    json["children"] = nlohmann::json::array();

    for (const auto& child : widget.children) {
        json["children"].push_back(widgetToJson(child));
    }

    return json;
}

} // namespace

std::string JsonProjectWriter::writeToString(const model::ProjectDocument& document) const
{
    nlohmann::json json;
    json["schemaVersion"] = document.schemaVersion;
    json["projectName"] = document.projectName;
    const std::string userSubclassName = document.userSubclassName.empty() ? document.mainFormClassName : document.userSubclassName;
    json["mainFormClassName"] = userSubclassName;
    json["generatedBaseClassName"] = "MainWindow";
    json["userSubclassName"] = userSubclassName;
    json["lookAndFeelId"] = document.lookAndFeelId.empty() ? std::string{"VisiFormDark"} : document.lookAndFeelId;
    json["selectedWidgetId"] = document.selectedWidgetId;
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
