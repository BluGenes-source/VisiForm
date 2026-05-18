#pragma once

#include "model/LookAndFeelDefinition.h"

#include <string>
#include <vector>

namespace visiform::model {

class LookAndFeelRegistry {
public:
    [[nodiscard]] static const LookAndFeelRegistry& instance();

    [[nodiscard]] const LookAndFeelDefinition* findById(const std::string& id) const;
    [[nodiscard]] const LookAndFeelDefinition& defaultDefinition() const;
    [[nodiscard]] const std::vector<LookAndFeelDefinition>& definitions() const;

private:
    LookAndFeelRegistry();

    std::vector<LookAndFeelDefinition> definitions_{};
};

} // namespace visiform::model
