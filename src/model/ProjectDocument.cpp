#include "model/ProjectDocument.h"

#include "model/ProjectDocument.h"

#include "model/LayoutEngine.h"
#include "model/WidgetRegistry.h"
#include "utils/IdGenerator.h"

#include <functional>
#include <algorithm>
#include <map>
#include <set>
#include <type_traits>

namespace visiform::model {
namespace {

void syncTabPageBoundsRecursive(WidgetNode& widget)
{
    if (widget.type == WidgetType::TabControl) {
        const Rect pageBounds = LayoutEngine::clientBoundsForParent(widget);
        for (auto& child : widget.children) {
            if (child.type == WidgetType::TabPage) {
                child.bounds = pageBounds;
            }
        }
    }

    for (auto& child : widget.children) {
        syncTabPageBoundsRecursive(child);
    }
}

void collectWidgetsReferencingResource(const WidgetNode& widget, const std::string& resourceId, std::vector<std::string>& widgetIds)
{
    if (!resourceId.empty() && widget.getStringProperty("resourceId", {}) == resourceId) {
        widgetIds.push_back(widget.id);
    }

    for (const auto& child : widget.children) {
        collectWidgetsReferencingResource(child, resourceId, widgetIds);
    }
}

bool detachWidgetRecursive(WidgetNode& parent, const std::string& searchId, WidgetNode& detachedWidget)
{
    const auto iterator = std::find_if(parent.children.begin(), parent.children.end(),
        [&](const WidgetNode& child) { return child.id == searchId; });
    if (iterator != parent.children.end()) {
        detachedWidget = std::move(*iterator);
        parent.children.erase(iterator);
        parent.syncHierarchyMetadata(parent.parentId);
        return true;
    }

    for (auto& child : parent.children) {
        if (detachWidgetRecursive(child, searchId, detachedWidget)) {
            return true;
        }
    }

    return false;
}

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

template <typename DocumentType>
auto findNearestAncestorOfType(DocumentType& document, const std::string& widgetId, WidgetType type)
{
    using WidgetPointer = decltype(document.findWidgetById(widgetId));

    if (widgetId.empty()) {
        return WidgetPointer{ nullptr };
    }

    auto* current = document.findWidgetById(widgetId);
    while (current != nullptr) {
        if (current->type == type) {
            return current;
        }

        current = document.findParentOf(current->id);
    }

    return WidgetPointer{ nullptr };
}

template <typename DocumentType>
auto findOwningTabControl(DocumentType& document, const std::string& widgetId)
{
    using WidgetPointer = decltype(document.findWidgetById(widgetId));

    auto* tabPage = findNearestAncestorOfType(document, widgetId, WidgetType::TabPage);
    if (tabPage == nullptr) {
        auto* widget = document.findWidgetById(widgetId);
        if (widget != nullptr && widget->type == WidgetType::TabControl) {
            return widget;
        }
        return WidgetPointer{ nullptr };
    }

    auto* parent = document.findParentOf(tabPage->id);
    if (parent != nullptr && parent->type == WidgetType::TabControl) {
        return parent;
    }

    return WidgetPointer{ nullptr };
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
    case WidgetType::ComboBox:
        return "comboBox" + suffix;
    case WidgetType::ListBox:
        return "listBox" + suffix;
    case WidgetType::CheckBox:
        return "checkBox" + suffix;
    case WidgetType::RadioButton:
        return "radioButton" + suffix;
    case WidgetType::Slider:
        return "slider" + suffix;
    case WidgetType::ScrollBar:
        return "scrollBar" + suffix;
    case WidgetType::Frame:
        return "frame" + suffix;
    case WidgetType::GroupBox:
        return "groupBox" + suffix;
    case WidgetType::Panel:
        return "panel" + suffix;
    case WidgetType::TabControl:
        return "tabControl" + suffix;
    case WidgetType::TabPage:
        return "tabPage" + suffix;
    case WidgetType::Image:
        return "image" + suffix;
    case WidgetType::Spacer:
        return "spacer" + suffix;
    case WidgetType::FormWindow:
        return "form" + suffix;
    case WidgetType::StatusBar:
        return "statusBar" + suffix;
    case WidgetType::ProgressBar:
        return "progressBar" + suffix;
    case WidgetType::ModalDialog:
        return "modalDialog" + suffix;
    case WidgetType::ColorPicker:
        return "colorPicker" + suffix;
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
    document.projectName = "VisiFormProject";
    document.executableName = "VisiFormProject";
    document.mainFormClassName = "AppMainWindow";
    document.generatedBaseClassName = "MainWindow";
    document.userSubclassName = "AppMainWindow";
    document.windowTitle = document.projectName;
    document.lookAndFeelId = "VisiFormDark";
    document.root = WidgetRegistry::instance().createDefaultWidget(WidgetType::FormWindow, "form_main");
    document.root.name = "MainWindow";
    document.root.bounds = Rect{ 0.0f, 0.0f, 900.0f, 600.0f };
    document.root.setProperty("title", document.windowTitle);

    WidgetNode helloButton = WidgetRegistry::instance().createDefaultWidget(WidgetType::Button, "button_hello");
    helloButton.name = "helloButton";
    helloButton.bounds = Rect{ 40.0f, 40.0f, 160.0f, 40.0f };
    helloButton.setProperty("text", "Click Me");
    document.root.appendChild(std::move(helloButton));
    document.applyDockLayout();
    document.setSelection("button_hello");

    return document;
}

const std::string& ProjectDocument::name() const
{
    return projectName;
}

namespace {

void collectRadioButtonsByGroup(WidgetNode& widget, std::map<std::string, std::vector<WidgetNode*>>& groups)
{
    if (widget.type == WidgetType::RadioButton) {
        groups[widget.getStringProperty("group", "default")].push_back(&widget);
    }

    for (auto& child : widget.children) {
        collectRadioButtonsByGroup(child, groups);
    }
}

} // namespace

WidgetNode* ProjectDocument::selectedWidget()
{
    if (!hasSelection()) {
        return nullptr;
    }

    return findWidgetById(selectedWidgetId);
}

std::vector<WidgetNode*> ProjectDocument::selectedWidgets()
{
    std::vector<WidgetNode*> widgets;
    widgets.reserve(selectedWidgetIds_.size());
    for (const auto& id : selectedWidgetIds_) {
        if (auto* widget = findWidgetById(id)) {
            widgets.push_back(widget);
        }
    }
    return widgets;
}

std::vector<const WidgetNode*> ProjectDocument::selectedWidgets() const
{
    std::vector<const WidgetNode*> widgets;
    widgets.reserve(selectedWidgetIds_.size());
    for (const auto& id : selectedWidgetIds_) {
        if (const auto* widget = findWidgetById(id)) {
            widgets.push_back(widget);
        }
    }
    return widgets;
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

ProjectResource* ProjectDocument::findResourceById(const std::string& id)
{
    const auto iterator = std::find_if(resources.begin(), resources.end(), [&id](const ProjectResource& resource) {
        return resource.id == id;
    });
    return iterator == resources.end() ? nullptr : &*iterator;
}

const WidgetNode* ProjectDocument::findWidgetById(const std::string& id) const
{
    return root.findById(id);
}

const ProjectResource* ProjectDocument::findResourceById(const std::string& id) const
{
    const auto iterator = std::find_if(resources.begin(), resources.end(), [&id](const ProjectResource& resource) {
        return resource.id == id;
    });
    return iterator == resources.end() ? nullptr : &*iterator;
}

WidgetNode* ProjectDocument::findParentOf(const std::string& childId)
{
    if (childId.empty() || childId == root.id) {
        return nullptr;
    }

    return root.findParentOf(childId);
}

WidgetNode* ProjectDocument::findTabPageFor(const std::string& widgetId)
{
    return const_cast<WidgetNode*>(std::as_const(*this).findTabPageFor(widgetId));
}

const WidgetNode* ProjectDocument::findTabPageFor(const std::string& widgetId) const
{
    return findNearestAncestorOfType(*this, widgetId, WidgetType::TabPage);
}

WidgetNode* ProjectDocument::findTabControlFor(const std::string& widgetId)
{
    return const_cast<WidgetNode*>(std::as_const(*this).findTabControlFor(widgetId));
}

const WidgetNode* ProjectDocument::findTabControlFor(const std::string& widgetId) const
{
    return findOwningTabControl(*this, widgetId);
}

WidgetNode* ProjectDocument::selectedTabPageFor(const std::string& tabControlId)
{
    return const_cast<WidgetNode*>(std::as_const(*this).selectedTabPageFor(tabControlId));
}

const WidgetNode* ProjectDocument::selectedTabPageFor(const std::string& tabControlId) const
{
    const WidgetNode* tabControl = findWidgetById(tabControlId);
    if (tabControl == nullptr || tabControl->type != WidgetType::TabControl) {
        return nullptr;
    }

    return tabControl->tabPageAt(tabControl->selectedTabIndex());
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

    const bool reordered = reorderWithinParent(&parent->children, id, true);
    if (reordered) {
        refreshHierarchyMetadata();
    }
    return reordered;
}

bool ProjectDocument::sendWidgetBackward(const std::string& id)
{
    WidgetNode* parent = findParentOf(id);
    if (parent == nullptr) {
        return false;
    }

    const bool reordered = reorderWithinParent(&parent->children, id, false);
    if (reordered) {
        refreshHierarchyMetadata();
    }
    return reordered;
}

bool ProjectDocument::removeWidgetById(const std::string& id)
{
    if (id.empty() || id == root.id) {
        return false;
    }

    const bool removed = root.removeWidgetById(id);
    if (removed) {
        applyDockLayout();
    }
    return removed;
}

bool ProjectDocument::isRootWidgetId(const std::string& id) const
{
    return !id.empty() && id == root.id;
}

std::vector<std::string> ProjectDocument::widgetIdsReferencingResource(const std::string& resourceId) const
{
    std::vector<std::string> widgetIds;
    if (resourceId.empty()) {
        return widgetIds;
    }

    collectWidgetsReferencingResource(root, resourceId, widgetIds);
    return widgetIds;
}

bool ProjectDocument::isResourceReferenced(const std::string& resourceId) const
{
    return !widgetIdsReferencingResource(resourceId).empty();
}

bool ProjectDocument::removeResourceById(const std::string& id)
{
    if (id.empty()) {
        return false;
    }

    const auto iterator = std::find_if(resources.begin(), resources.end(), [&id](const ProjectResource& resource) {
        return resource.id == id;
    });
    if (iterator == resources.end()) {
        return false;
    }

    resources.erase(iterator);
    return true;
}

bool ProjectDocument::addChildToRoot(WidgetNode widget)
{
    root.appendChild(std::move(widget));
    applyDockLayout();
    return true;
}

bool ProjectDocument::addChildToParent(const std::string& parentId, WidgetNode widget)
{
    if (parentId.empty()) {
        return false;
    }
    if (parentId == root.id) {
        if (!WidgetRegistry::instance().canContainChild(root.type, widget.type)) {
            return false;
        }
        return addChildToRoot(std::move(widget));
    }

    if (auto* parent = findWidgetById(parentId)) {
        if (!WidgetRegistry::instance().canContainChild(parent->type, widget.type)) {
            return false;
        }
        parent->appendChild(std::move(widget));
        applyDockLayout();
        return true;
    }

    return false;
}

bool ProjectDocument::canReparentWidget(const std::string& widgetId, const std::string& newParentId) const
{
    if (widgetId.empty() || newParentId.empty() || widgetId == newParentId || isRootWidgetId(widgetId)) {
        return false;
    }

    const WidgetNode* widget = findWidgetById(widgetId);
    const WidgetNode* newParent = findWidgetById(newParentId);
    if (widget == nullptr || newParent == nullptr) {
        return false;
    }
    if (!WidgetRegistry::instance().canContainChild(newParent->type, widget->type)) {
        return false;
    }

    return widget->findById(newParentId) == nullptr;
}

bool ProjectDocument::reparentWidget(const std::string& widgetId, const std::string& newParentId, Rect newBounds)
{
    if (!canReparentWidget(widgetId, newParentId)) {
        return false;
    }

    WidgetNode detachedWidget;
    if (!detachWidgetRecursive(root, widgetId, detachedWidget)) {
        return false;
    }

    detachedWidget.bounds = newBounds;
    const bool added = addChildToParent(newParentId, std::move(detachedWidget));
    if (!added) {
        return false;
    }

    refreshHierarchyMetadata();
    return true;
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

void ProjectDocument::refreshHierarchyMetadata()
{
    root.syncHierarchyMetadata();
    syncTabPageBoundsRecursive(root);
    root.syncHierarchyMetadata();
}

void ProjectDocument::applyDockLayout()
{
    refreshHierarchyMetadata();
    LayoutEngine::applyDockLayout(root);
    refreshHierarchyMetadata();
}

void ProjectDocument::applyLayoutFromPrevious(const ProjectDocument& previousDocument)
{
    refreshHierarchyMetadata();
    LayoutEngine::applyLayoutFromPrevious(root, previousDocument.root);
    refreshHierarchyMetadata();
}

void ProjectDocument::selectWidget(const std::string& id)
{
    setSelection(id);
}

void ProjectDocument::setSelection(const std::string& id)
{
    if (id.empty()) {
        clearSelection();
        return;
    }

    if (findWidgetById(id) == nullptr) {
        clearSelection();
        return;
    }

    selectedWidgetIds_.clear();
    selectedWidgetIds_.push_back(id);
    syncPrimarySelection();
    syncTabSelectionForWidget(selectedWidgetId);
}

void ProjectDocument::addToSelection(const std::string& id)
{
    if (id.empty() || findWidgetById(id) == nullptr) {
        return;
    }

    if (isRootWidgetId(id)) {
        setSelection(id);
        return;
    }

    selectedWidgetIds_.erase(std::remove(selectedWidgetIds_.begin(), selectedWidgetIds_.end(), root.id), selectedWidgetIds_.end());
    selectedWidgetIds_.erase(std::remove(selectedWidgetIds_.begin(), selectedWidgetIds_.end(), id), selectedWidgetIds_.end());
    selectedWidgetIds_.push_back(id);
    syncPrimarySelection();
    syncTabSelectionForWidget(selectedWidgetId);
}

void ProjectDocument::removeFromSelection(const std::string& id)
{
    if (id.empty()) {
        return;
    }

    selectedWidgetIds_.erase(std::remove(selectedWidgetIds_.begin(), selectedWidgetIds_.end(), id), selectedWidgetIds_.end());
    syncPrimarySelection();
    syncTabSelectionForWidget(selectedWidgetId);
}

void ProjectDocument::toggleSelection(const std::string& id)
{
    if (id.empty() || findWidgetById(id) == nullptr) {
        return;
    }

    if (isRootWidgetId(id)) {
        setSelection(id);
        return;
    }

    if (isSelected(id)) {
        removeFromSelection(id);
        return;
    }

    addToSelection(id);
}

bool ProjectDocument::isSelected(const std::string& id) const
{
    return std::find(selectedWidgetIds_.begin(), selectedWidgetIds_.end(), id) != selectedWidgetIds_.end();
}

bool ProjectDocument::isPrimarySelected(const std::string& id) const
{
    return !id.empty() && selectedWidgetId == id;
}

bool ProjectDocument::isSecondarySelected(const std::string& id) const
{
    return isSelected(id) && !isPrimarySelected(id);
}

bool ProjectDocument::selectRadioButtonInGroup(const std::string& id)
{
    WidgetNode* selectedRadio = findWidgetById(id);
    if (selectedRadio == nullptr || selectedRadio->type != WidgetType::RadioButton) {
        return false;
    }

    const std::string group = selectedRadio->getStringProperty("group", "default");
    bool changed = false;
    std::map<std::string, std::vector<WidgetNode*>> groups;
    collectRadioButtonsByGroup(root, groups);
    if (const auto iterator = groups.find(group); iterator != groups.end()) {
        for (auto* radio : iterator->second) {
            const bool shouldBeSelected = radio->id == id;
            if (radio->getBoolProperty("selected", false) != shouldBeSelected) {
                radio->setProperty("selected", shouldBeSelected);
                changed = true;
            }
        }
    }

    return changed;
}

bool ProjectDocument::normalizeRadioGroups()
{
    std::map<std::string, std::vector<WidgetNode*>> groups;
    collectRadioButtonsByGroup(root, groups);

    bool changed = false;
    for (auto& [groupName, radios] : groups) {
        (void)groupName;
        bool foundSelected = false;
        for (auto* radio : radios) {
            const bool isSelected = radio->getBoolProperty("selected", false);
            if (isSelected && !foundSelected) {
                foundSelected = true;
                continue;
            }
            if (isSelected) {
                radio->setProperty("selected", false);
                changed = true;
            }
        }
    }

    return changed;
}

void ProjectDocument::clearSelection()
{
    selectedWidgetIds_.clear();
    selectedWidgetId.clear();
}

bool ProjectDocument::hasSelection() const
{
    return !selectedWidgetIds_.empty();
}

bool ProjectDocument::hasMultiSelection() const
{
    return selectedWidgetIds_.size() > 1;
}

const std::vector<std::string>& ProjectDocument::selectedWidgetIds() const
{
    return selectedWidgetIds_;
}

void ProjectDocument::markDirty()
{
    dirty = true;
}

void ProjectDocument::clearDirty()
{
    dirty = false;
}

void ProjectDocument::syncPrimarySelection()
{
    selectedWidgetIds_.erase(std::remove_if(selectedWidgetIds_.begin(), selectedWidgetIds_.end(),
        [this](const std::string& id) { return findWidgetById(id) == nullptr; }), selectedWidgetIds_.end());

    if (selectedWidgetIds_.empty()) {
        selectedWidgetId.clear();
        return;
    }

    if (selectedWidgetIds_.size() > 1) {
        selectedWidgetIds_.erase(std::remove(selectedWidgetIds_.begin(), selectedWidgetIds_.end(), root.id), selectedWidgetIds_.end());
        if (selectedWidgetIds_.empty()) {
            selectedWidgetIds_.push_back(root.id);
        }
    }

    selectedWidgetId = selectedWidgetIds_.back();
}

void ProjectDocument::syncTabSelectionForWidget(const std::string& id)
{
    if (id.empty()) {
        return;
    }

    WidgetNode* tabPage = findTabPageFor(id);
    if (tabPage == nullptr) {
        return;
    }

    WidgetNode* tabControl = findParentOf(tabPage->id);
    if (tabControl == nullptr || tabControl->type != WidgetType::TabControl) {
        return;
    }

    int tabIndex = 0;
    for (const auto& child : tabControl->children) {
        if (child.type != WidgetType::TabPage) {
            continue;
        }
        if (child.id == tabPage->id) {
            tabControl->setSelectedTabIndex(tabIndex);
            return;
        }
        ++tabIndex;
    }
}

} // namespace visiform::model
