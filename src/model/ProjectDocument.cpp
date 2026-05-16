#include "model/ProjectDocument.h"

#include "model/ProjectDocument.h"

#include "utils/IdGenerator.h"

#include <functional>
#include <set>
#include <type_traits>

namespace visiform::model {
namespace {

template <typename ParentType>
using SiblingPointer = std::conditional_t<
    std::is_const_v<ParentType>,
    const typename std::remove_const_t<ParentType>::value_type*,
    typename ParentType::value_type*>;

template <typename ParentType>
SiblingPointer<ParentType> findPreviousSiblingInParent(ParentType* parent, const std::string& id)
{
    if (parent == nullptr) {
        return nullptr;
    }

    for (std::size_t index = 0; index < parent->size(); ++index) {
        if ((*parent)[index].id == id) {
            if (index == 0) {
                return nullptr;
            }

            return &(*parent)[index - 1];
        }
    }

    return nullptr;
}

template <typename ParentType>
bool reorderWithinParent(ParentType* parent, const std::string& id, bool forward)
{
    if (parent == nullptr) {
        return false;
    }

    for (std::size_t index = 0; index < parent->size(); ++index) {
        if ((*parent)[index].id != id) {
            continue;
        }

        // Z-order convention:
        // - children[0] is backmost
        // - children.back() is frontmost
        // Toolbar actions use Front/Back semantics, so move the selected widget
        // all the way to the end or beginning of the child vector.
        if (forward) {
            if (index + 1 >= parent->size()) {
                return false;
            }
            auto widget = std::move((*parent)[index]);
            parent->erase(parent->begin() + static_cast<std::ptrdiff_t>(index));
            parent->push_back(std::move(widget));
            return true;
        }

        if (index == 0) {
            return false;
        }
        auto widget = std::move((*parent)[index]);
        parent->erase(parent->begin() + static_cast<std::ptrdiff_t>(index));
        parent->insert(parent->begin(), std::move(widget));
        return true;
    }

    return false;
}

std::string duplicateNameFor(const WidgetNode& widget, const std::string& id)
{
    const auto underscore = id.find_last_of('_');
    const std::string suffix = underscore == std::string::npos ? std::string{} : id.substr(underscore + 1);
    if (!widget.name.empty()) {
        return widget.name + suffix;
    }

    switch (widget.type) {
    case WidgetType::Label:
        return "label" + suffix;
    case WidgetType::Button:
        return "button" + suffix;
    case WidgetType::TextBox:
        return "textBox" + suffix;
    case WidgetType::CheckBox:
        return "checkBox" + suffix;
    case WidgetType::Slider:
        return "slider" + suffix;
    case WidgetType::Frame:
        return "frame" + suffix;
    case WidgetType::Image:
        return "image" + suffix;
    case WidgetType::Spacer:
        return "spacer" + suffix;
    case WidgetType::FormWindow:
        return "form" + suffix;
    }

    return id;
}

void assignDuplicateIds(WidgetNode& widget, ProjectDocument& document, utils::IdGenerator& idGenerator, std::set<std::string>& generatedIds)
{
    std::string newId;
    do {
        newId = idGenerator.next(widget.type, document);
    } while (generatedIds.contains(newId));

    generatedIds.insert(newId);
    widget.id = newId;
    widget.name = duplicateNameFor(widget, newId);

    for (auto& child : widget.children) {
        assignDuplicateIds(child, document, idGenerator, generatedIds);
    }
}

} // namespace

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
    document.root.setProperty("onLoad", "");
    document.root.setProperty("onClose", "");

    WidgetNode helloButton{
        "button_hello",
        "helloButton",
        WidgetType::Button,
        Rect{ 40.0f, 40.0f, 160.0f, 40.0f }
    };
    helloButton.setProperty("text", "Click Me");
    helloButton.setProperty("onClick", "");
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

WidgetNode* ProjectDocument::findParentOf(const std::string& childId)
{
    if (childId.empty() || childId == root.id) {
        return nullptr;
    }

    return root.findParentOf(childId);
}

const WidgetNode* ProjectDocument::findParentOf(const std::string& childId) const
{
    if (childId.empty() || childId == root.id) {
        return nullptr;
    }

    return root.findParentOf(childId);
}

WidgetNode* ProjectDocument::previousSiblingOf(const std::string& id)
{
    WidgetNode* parent = findParentOf(id);
    if (parent == nullptr) {
        return nullptr;
    }

    return findPreviousSiblingInParent(&parent->children, id);
}

const WidgetNode* ProjectDocument::previousSiblingOf(const std::string& id) const
{
    const WidgetNode* parent = findParentOf(id);
    if (parent == nullptr) {
        return nullptr;
    }

    return findPreviousSiblingInParent(&parent->children, id);
}

bool ProjectDocument::bringWidgetForward(const std::string& id)
{
    WidgetNode* parent = findParentOf(id);
    if (parent == nullptr) {
        return false;
    }

    return reorderWithinParent(&parent->children, id, true);
}

bool ProjectDocument::sendWidgetBackward(const std::string& id)
{
    WidgetNode* parent = findParentOf(id);
    if (parent == nullptr) {
        return false;
    }

    return reorderWithinParent(&parent->children, id, false);
}

bool ProjectDocument::removeWidgetById(const std::string& id)
{
    if (id.empty() || id == root.id) {
        return false;
    }

    return root.removeWidgetById(id);
}

bool ProjectDocument::isRootWidgetId(const std::string& id) const
{
    return !id.empty() && id == root.id;
}

bool ProjectDocument::addChildToRoot(WidgetNode widget)
{
    root.children.push_back(std::move(widget));
    return true;
}

bool ProjectDocument::addChildToParent(const std::string& parentId, WidgetNode widget)
{
    if (parentId.empty()) {
        return false;
    }
    if (parentId == root.id) {
        return addChildToRoot(std::move(widget));
    }

    if (auto* parent = findWidgetById(parentId)) {
        parent->children.push_back(std::move(widget));
        return true;
    }

    return false;
}

WidgetNode* ProjectDocument::duplicateWidgetById(const std::string& id, utils::IdGenerator& idGenerator)
{
    if (id.empty() || id == root.id) {
        return nullptr;
    }

    const WidgetNode* source = findWidgetById(id);
    const WidgetNode* parent = findParentOf(id);
    if (source == nullptr || parent == nullptr) {
        return nullptr;
    }

    WidgetNode duplicate = *source;
    std::set<std::string> generatedIds;
    assignDuplicateIds(duplicate, *this, idGenerator, generatedIds);
    duplicate.bounds.x += 20.0f;
    duplicate.bounds.y += 20.0f;

    const std::string duplicatedId = duplicate.id;
    if (!addChildToParent(parent->id, std::move(duplicate))) {
        return nullptr;
    }

    return findWidgetById(duplicatedId);
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
