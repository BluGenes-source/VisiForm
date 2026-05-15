#include "serialization/JsonProjectReader.h"

#include "serialization/JsonProjectReader.h"

#include <nlohmann/json.hpp>

namespace visiform::serialization {

model::ProjectDocument JsonProjectReader::readFromString(const std::string& content) const
{
    const auto json = nlohmann::json::parse(content.empty() ? "{}" : content);

    model::ProjectDocument document = model::ProjectDocument::createDefault();
    document.projectName = json.value("projectName", document.projectName);
    document.mainFormClassName = json.value("mainFormClassName", document.mainFormClassName);

    return document;
}

} // namespace visiform::serialization
