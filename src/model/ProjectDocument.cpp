#include "model/ProjectDocument.h"

namespace visiform::model {

ProjectDocument::ProjectDocument(std::string name)
    : name_(std::move(name))
{
}

void ProjectDocument::addForm(FormNode form)
{
    forms_.push_back(std::move(form));
}

const std::string& ProjectDocument::name() const
{
    return name_;
}

const std::vector<FormNode>& ProjectDocument::forms() const
{
    return forms_;
}

} // namespace visiform::model
