#pragma once

#include <optional>
#include <string>

namespace visiform::model {

enum class ProjectResourceType {
    Image,
    Font,
    Icon,
    Theme,
    Other
};

[[nodiscard]] inline std::string toString(ProjectResourceType type)
{
    switch (type) {
    case ProjectResourceType::Image:
        return "Image";
    case ProjectResourceType::Font:
        return "Font";
    case ProjectResourceType::Icon:
        return "Icon";
    case ProjectResourceType::Theme:
        return "Theme";
    case ProjectResourceType::Other:
        return "Other";
    }

    return "Other";
}

[[nodiscard]] inline std::optional<ProjectResourceType> projectResourceTypeFromString(const std::string& value)
{
    if (value == "Image") {
        return ProjectResourceType::Image;
    }
    if (value == "Font") {
        return ProjectResourceType::Font;
    }
    if (value == "Icon") {
        return ProjectResourceType::Icon;
    }
    if (value == "Theme") {
        return ProjectResourceType::Theme;
    }
    if (value == "Other") {
        return ProjectResourceType::Other;
    }

    return std::nullopt;
}

struct ProjectResource {
    std::string id{};
    ProjectResourceType type = ProjectResourceType::Other;
    std::string displayName{};
    std::string sourcePath{};
    std::string exportRelativePath{};
};

} // namespace visiform::model
