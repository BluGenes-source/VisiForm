#pragma once

#include "model/LookAndFeelDefinition.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace visiform::model {

class ProjectDocument;
class WidgetNode;
enum class WidgetType;

class LookAndFeelRegistry {
public:
    [[nodiscard]] static LookAndFeelRegistry& instance();

    [[nodiscard]] const LookAndFeelDefinition* findById(const std::string& id) const;
    [[nodiscard]] const LookAndFeelDefinition& defaultDefinition() const;
    [[nodiscard]] const std::vector<LookAndFeelDefinition>& definitions() const;
    [[nodiscard]] bool isBuiltIn(const std::string& id) const;
    void setCustomDefinitions(std::vector<LookAndFeelDefinition> definitions);
    [[nodiscard]] ResolvedLookAndFeelStyle resolveProjectStyle(
        const std::string& lookAndFeelId,
        const LookAndFeelOverrides& overrides) const;
    [[nodiscard]] ResolvedLookAndFeelStyle resolve(
        const ProjectDocument& document,
        const WidgetNode& widget) const;
    [[nodiscard]] static bool supportsWidgetOverrides(WidgetType type);
    [[nodiscard]] static bool supportsWidgetOverride(WidgetType type, std::string_view key);
    [[nodiscard]] static std::vector<std::string_view> supportedWidgetOverrideKeys(WidgetType type);

private:
    LookAndFeelRegistry();

    std::size_t builtInCount_ = 0;
    std::vector<LookAndFeelDefinition> definitions_{};
};

} // namespace visiform::model
