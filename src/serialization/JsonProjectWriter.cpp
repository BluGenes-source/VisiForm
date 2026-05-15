#include "serialization/JsonProjectWriter.h"

#include "serialization/JsonProjectWriter.h"

#include <nlohmann/json.hpp>

namespace visiform::serialization {

std::string JsonProjectWriter::writeToString(const model::ProjectDocument& document) const
{
    nlohmann::json json;
    json["projectName"] = document.projectName;
    json["mainFormClassName"] = document.mainFormClassName;
    json["schemaVersion"] = document.schemaVersion;
    json["projectFileExtension"] = model::ProjectDocument::projectFileExtension();
    json["root"] = {
        {"id", document.root.id},
        {"name", document.root.name},
        {"type", document.root.typeName()}
    };
    json["selectedWidgetId"] = document.selectedWidgetId;

    return json.dump(2);
}

} // namespace visiform::serialization
