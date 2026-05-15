#include "model/ProjectDocument.h"

#include "model/ProjectDocument.h"

namespace visiform::model {

ProjectDocument ProjectDocument::createDefault()
{
    ProjectDocument document;
    document.projectName = "UntitledVisiFormProject";
    document.mainFormClassName = "MainWindow";
    document.root = WidgetNode{
        "form_main",
        "MainWindow",
        WidgetType::FormWindow,
        Rect{ 0.0f, 0.0f, 900.0f, 600.0f }
    };
    document.root.setProperty("title", "MainWindow");
    document.root.setProperty("backgroundColor", "#202026");

    WidgetNode helloButton{
        "button_hello",
        "helloButton",
        WidgetType::Button,
        Rect{ 40.0f, 40.0f, 160.0f, 40.0f }
    };
    helloButton.setProperty("text", "Click Me");
    document.root.children.push_back(std::move(helloButton));
    document.selectedWidgetId = "button_hello";

    return document;
}

const std::string& ProjectDocument::name() const
{
    return projectName;
}

WidgetNode* ProjectDocument::selectedWidget()
{
    if (!hasSelection()) {
        return nullptr;
    }

    return findWidgetById(selectedWidgetId);
}

const WidgetNode* ProjectDocument::selectedWidget() const
{
    if (!hasSelection()) {
        return nullptr;
    }

    return findWidgetById(selectedWidgetId);
}

WidgetNode* ProjectDocument::findWidgetById(const std::string& id)
{
    return root.findById(id);
}

const WidgetNode* ProjectDocument::findWidgetById(const std::string& id) const
{
    return root.findById(id);
}

void ProjectDocument::selectWidget(const std::string& id)
{
    if (id.empty()) {
        clearSelection();
        return;
    }

    selectedWidgetId = findWidgetById(id) != nullptr ? id : std::string{};
}

void ProjectDocument::clearSelection()
{
    selectedWidgetId.clear();
}

bool ProjectDocument::hasSelection() const
{
    return !selectedWidgetId.empty();
}

void ProjectDocument::markDirty()
{
    dirty = true;
}

void ProjectDocument::clearDirty()
{
    dirty = false;
}

} // namespace visiform::model
