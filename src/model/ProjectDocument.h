#pragma once

#pragma once

#include "model/FormNode.h"

#include <string>
#include <vector>

namespace visiform::utils {
class IdGenerator;
}

namespace visiform::model {

class ProjectDocument {
public:
    int schemaVersion = 1;
    std::string projectName{};
    std::string mainFormClassName{};
    std::string generatedBaseClassName{};
    std::string userSubclassName{};
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
    [[nodiscard]] std::vector<WidgetNode*> selectedWidgets();
    [[nodiscard]] std::vector<const WidgetNode*> selectedWidgets() const;

    [[nodiscard]] WidgetNode* findWidgetById(const std::string& id);
    [[nodiscard]] const WidgetNode* findWidgetById(const std::string& id) const;
    [[nodiscard]] WidgetNode* findParentOf(const std::string& childId);
    [[nodiscard]] const WidgetNode* findParentOf(const std::string& childId) const;
    [[nodiscard]] WidgetNode* previousSiblingOf(const std::string& id);
    [[nodiscard]] const WidgetNode* previousSiblingOf(const std::string& id) const;
    bool bringWidgetForward(const std::string& id);
    bool sendWidgetBackward(const std::string& id);
    bool removeWidgetById(const std::string& id);
    [[nodiscard]] bool isRootWidgetId(const std::string& id) const;
    bool addChildToRoot(WidgetNode widget);
    bool addChildToParent(const std::string& parentId, WidgetNode widget);
    [[nodiscard]] WidgetNode* duplicateWidgetById(const std::string& id, utils::IdGenerator& idGenerator);

    void selectWidget(const std::string& id);
    void setSelection(const std::string& id);
    void addToSelection(const std::string& id);
    void removeFromSelection(const std::string& id);
    void toggleSelection(const std::string& id);
    [[nodiscard]] bool isSelected(const std::string& id) const;
    [[nodiscard]] bool isPrimarySelected(const std::string& id) const;
    [[nodiscard]] bool isSecondarySelected(const std::string& id) const;
    bool selectRadioButtonInGroup(const std::string& id);
    bool normalizeRadioGroups();
    void clearSelection();
    [[nodiscard]] bool hasSelection() const;
    [[nodiscard]] bool hasMultiSelection() const;
    [[nodiscard]] const std::vector<std::string>& selectedWidgetIds() const;
    void markDirty();
    void clearDirty();

private:
    void syncPrimarySelection();

    std::vector<std::string> selectedWidgetIds_{};
};

} // namespace visiform::model
