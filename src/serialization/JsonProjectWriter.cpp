#include "serialization/JsonProjectWriter.h"

#include <nlohmann/json.hpp>

namespace visiform::serialization {

std::string JsonProjectWriter::writeToString(const model::ProjectDocument& document) const
{
    nlohmann::json json;
    json["name"] = document.name();
    json["projectFileExtension"] = model::ProjectDocument::projectFileExtension();
    json["forms"] = nlohmann::json::array();

    for (const auto& form : document.forms()) {
        nlohmann::json formJson;
        formJson["name"] = form.name();
        formJson["widgets"] = nlohmann::json::array();

        for (const auto& widget : form.widgets()) {
            formJson["widgets"].push_back({
                {"id", widget.id()},
                {"type", widget.typeName()}
            });
        }

        json["forms"].push_back(std::move(formJson));
    }

    return json.dump(2);
}

} // namespace visiform::serialization
