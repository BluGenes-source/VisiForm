#include "model/LookAndFeelRegistry.h"

#include <algorithm>

namespace visiform::model {

LookAndFeelRegistry::LookAndFeelRegistry()
    : definitions_{
        { "VisiFormDark", "VisiForm Dark", "#1F242D", "#2B313D", "#EEF2F8", "#97A3B7", "#2D7FF9", "#6C7788", 1.0f, 4.0f, 16.0f },
        { "VisiFormLight", "VisiForm Light", "#F2F4F8", "#FFFFFF", "#1B2533", "#B8C2D0", "#2D7FF9", "#A2ACBA", 1.0f, 4.0f, 16.0f },
        { "ImGuiDark", "ImGui Dark", "#1E1E1E", "#2B2B2B", "#E6E6E6", "#4A4F57", "#3D84F7", "#6A6F78", 1.0f, 6.0f, 15.0f },
        { "FlatClassic", "Flat Classic", "#E7EAEE", "#F7F7F7", "#20262F", "#9CA8B5", "#4C86D9", "#B8C0C8", 1.0f, 0.0f, 16.0f } }
{
}

const LookAndFeelRegistry& LookAndFeelRegistry::instance()
{
    static const LookAndFeelRegistry registry;
    return registry;
}

const LookAndFeelDefinition* LookAndFeelRegistry::findById(const std::string& id) const
{
    const auto iterator = std::find_if(definitions_.begin(), definitions_.end(),
        [&id](const LookAndFeelDefinition& definition) { return definition.id == id; });
    return iterator == definitions_.end() ? nullptr : &*iterator;
}

const LookAndFeelDefinition& LookAndFeelRegistry::defaultDefinition() const
{
    return definitions_.front();
}

const std::vector<LookAndFeelDefinition>& LookAndFeelRegistry::definitions() const
{
    return definitions_;
}

} // namespace visiform::model
