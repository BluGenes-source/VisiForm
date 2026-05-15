#pragma once

#pragma once

#include "model/FormNode.h"

#include <string>

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

    void selectWidget(const std::string& id);
    void clearSelection();
    [[nodiscard]] bool hasSelection() const;
    void markDirty();
    void clearDirty();
};

} // namespace visiform::model
