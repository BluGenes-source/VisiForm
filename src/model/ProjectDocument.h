#pragma once

#pragma once

#include "model/FormNode.h"

#include <string>

namespace visiform::utils {
class IdGenerator;
}

namespace visiform::model {

class ProjectDocument {
public:
    int schemaVersion = 1;
    std::string projectName{};
    std::string mainFormClassName{};
    WidgetNode root{};
    std::string selectedWidgetId{};
    bool dirty = false;

    static constexpr const char* projectFileExtension()
    {
        return ".vfb.json";
    }

    [[nodiscard]] static ProjectDocument createDefault();

    [[nodiscard]] const std::string& name() const;
    [[nodiscard]] WidgetNode* selectedWidget();
    [[nodiscard]] const WidgetNode* selectedWidget() const;

    [[nodiscard]] WidgetNode* findWidgetById(const std::string& id);
    [[nodiscard]] const WidgetNode* findWidgetById(const std::string& id) const;
    [[nodiscard]] WidgetNode* findParentOf(const std::string& childId);
    [[nodiscard]] const WidgetNode* findParentOf(const std::string& childId) const;
    bool removeWidgetById(const std::string& id);
    [[nodiscard]] bool isRootWidgetId(const std::string& id) const;
    bool addChildToRoot(WidgetNode widget);
    bool addChildToParent(const std::string& parentId, WidgetNode widget);
    [[nodiscard]] WidgetNode* duplicateWidgetById(const std::string& id, utils::IdGenerator& idGenerator);

    void selectWidget(const std::string& id);
    void clearSelection();
    [[nodiscard]] bool hasSelection() const;
    void markDirty();
    void clearDirty();
};

} // namespace visiform::model
