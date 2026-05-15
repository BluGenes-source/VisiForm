#include "serialization/JsonProjectReader.h"

#include <nlohmann/json.hpp>

namespace visiform::serialization {

model::ProjectDocument JsonProjectReader::readFromString(const std::string& content) const
{
    const auto json = nlohmann::json::parse(content.empty() ? "{}" : content);

    model::ProjectDocument document{json.value("name", "Untitled Project")};

    if (!json.contains("forms") || !json["forms"].is_array()) {
        return document;
    }

    for (const auto& formJson : json["forms"]) {
        model::FormNode form{formJson.value("name", "Form")};

        if (formJson.contains("widgets") && formJson["widgets"].is_array()) {
            for (const auto& widgetJson : formJson["widgets"]) {
                form.addWidget(model::WidgetNode{
                    widgetJson.value("id", "widget"),
                    widgetJson.value("type", "Widget")
                });
            }
        }

        document.addForm(std::move(form));
    }

    return document;
}

} // namespace visiform::serialization
