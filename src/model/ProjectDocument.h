#pragma once

#include "model/FormNode.h"

#include <string>
#include <vector>

namespace visiform::model {

// Placeholder project document for `.vfb.json` projects.
class ProjectDocument {
public:
    explicit ProjectDocument(std::string name = {});

    static constexpr const char* projectFileExtension()
    {
        return ".vfb.json";
    }

    void addForm(FormNode form);

    [[nodiscard]] const std::string& name() const;
    [[nodiscard]] const std::vector<FormNode>& forms() const;

private:
    std::string name_{};
    std::vector<FormNode> forms_{};
};

} // namespace visiform::model
