#pragma once

#include "model/LookAndFeelDefinition.h"

#include <string>
#include <vector>

namespace visiform::model {

class ProjectDocument;
class WidgetNode;

class LookAndFeelRegistry {
public:
    [[nodiscard]] static const LookAndFeelRegistry& instance();

    [[nodiscard]] const LookAndFeelDefinition* findById(const std::string& id) const;
    [[nodiscard]] const LookAndFeelDefinition& defaultDefinition() const;
    [[nodiscard]] const std::vector<LookAndFeelDefinition>& definitions() const;
    [[nodiscard]] ResolvedLookAndFeelStyle resolve(
        const ProjectDocument& document,
        const WidgetNode& widget) const;

private:
    LookAndFeelRegistry();

    std::vector<LookAndFeelDefinition> definitions_{};
};

} // namespace visiform::model
