#include "model/WidgetNode.h"

#include "model/WidgetNode.h"

#include <algorithm>
#include <utility>

namespace visiform::model {
namespace {

const std::string& widgetTypeName(WidgetType type)
{
    static const std::string formWindow = "FormWindow";
    static const std::string frame = "Frame";
    static const std::string groupBox = "GroupBox";
    static const std::string panel = "Panel";
    static const std::string tabControl = "TabControl";
    static const std::string tabPage = "TabPage";
    static const std::string label = "Label";
    static const std::string button = "Button";
    static const std::string textBox = "TextBox";
    static const std::string checkBox = "CheckBox";
    static const std::string radioButton = "RadioButton";
    static const std::string slider = "Slider";
    static const std::string scrollBar = "ScrollBar";
    static const std::string statusBar = "StatusBar";
    static const std::string progressBar = "ProgressBar";
    static const std::string modalDialog = "ModalDialog";
    static const std::string colorPicker = "ColorPicker";
    static const std::string image = "Image";
    static const std::string spacer = "Spacer";

    switch (type) {
    case WidgetType::FormWindow:
        return formWindow;
    case WidgetType::Frame:
        return frame;
    case WidgetType::GroupBox:
        return groupBox;
    case WidgetType::Panel:
        return panel;
    case WidgetType::TabControl:
        return tabControl;
    case WidgetType::TabPage:
        return tabPage;
    case WidgetType::Label:
        return label;
    case WidgetType::Button:
        return button;
    case WidgetType::TextBox:
        return textBox;
    case WidgetType::CheckBox:
        return checkBox;
    case WidgetType::RadioButton:
        return radioButton;
    case WidgetType::Slider:
        return slider;
    case WidgetType::ScrollBar:
        return scrollBar;
    case WidgetType::StatusBar:
        return statusBar;
    case WidgetType::ProgressBar:
        return progressBar;
    case WidgetType::ModalDialog:
        return modalDialog;
    case WidgetType::ColorPicker:
        return colorPicker;
    case WidgetType::Image:
        return image;
    case WidgetType::Spacer:
        return spacer;
    }

    return frame;
}

const std::string& dockModeName(DockMode mode)
{
    static const std::string none = "None";
    static const std::string top = "Top";
    static const std::string bottom = "Bottom";
    static const std::string left = "Left";
    static const std::string right = "Right";
    static const std::string fill = "Fill";

    switch (mode) {
    case DockMode::None:
        return none;
    case DockMode::Top:
        return top;
    case DockMode::Bottom:
        return bottom;
    case DockMode::Left:
        return left;
    case DockMode::Right:
        return right;
    case DockMode::Fill:
        return fill;
    }

    return none;
}

const std::string& layoutModeName(LayoutMode mode)
{
    static const std::string absolute = "Absolute";
    static const std::string horizontal = "Horizontal";
    static const std::string vertical = "Vertical";
    static const std::string grid = "Grid";
    static const std::string tabPage = "TabPage";

    switch (mode) {
    case LayoutMode::Absolute:
        return absolute;
    case LayoutMode::Horizontal:
        return horizontal;
    case LayoutMode::Vertical:
        return vertical;
    case LayoutMode::Grid:
        return grid;
    case LayoutMode::TabPage:
        return tabPage;
    }

    return absolute;
}

void normalizeChildMetadata(WidgetNode& parent)
{
    for (std::size_t index = 0; index < parent.children.size(); ++index) {
        auto& child = parent.children[index];
        child.zOrder = static_cast<int>(index);
        child.syncHierarchyMetadata(parent.id);
    }
}

} // namespace

std::string toString(WidgetType type)
{
    return widgetTypeName(type);
}

std::optional<WidgetType> widgetTypeFromString(const std::string& value)
{
    if (value == "FormWindow") {
        return WidgetType::FormWindow;
    }
    if (value == "Frame") {
        return WidgetType::Frame;
    }
    if (value == "GroupBox") {
        return WidgetType::GroupBox;
    }
    if (value == "Panel") {
        return WidgetType::Panel;
    }
    if (value == "TabControl") {
        return WidgetType::TabControl;
    }
    if (value == "TabPage") {
        return WidgetType::TabPage;
    }
    if (value == "Label") {
        return WidgetType::Label;
    }
    if (value == "Button") {
        return WidgetType::Button;
    }
    if (value == "TextBox") {
        return WidgetType::TextBox;
    }
    if (value == "CheckBox") {
        return WidgetType::CheckBox;
    }
    if (value == "RadioButton") {
        return WidgetType::RadioButton;
    }
    if (value == "Slider") {
        return WidgetType::Slider;
    }
    if (value == "ScrollBar") {
        return WidgetType::ScrollBar;
    }
    if (value == "StatusBar") {
        return WidgetType::StatusBar;
    }
    if (value == "ProgressBar") {
        return WidgetType::ProgressBar;
    }
    if (value == "ModalDialog") {
        return WidgetType::ModalDialog;
    }
    if (value == "ColorPicker") {
        return WidgetType::ColorPicker;
    }
    if (value == "Image") {
        return WidgetType::Image;
    }
    if (value == "Spacer") {
        return WidgetType::Spacer;
    }

    return std::nullopt;
}

std::string toString(DockMode mode)
{
    return dockModeName(mode);
}

std::optional<DockMode> dockModeFromString(const std::string& value)
{
    if (value == "None") {
        return DockMode::None;
    }
    if (value == "Top") {
        return DockMode::Top;
    }
    if (value == "Bottom") {
        return DockMode::Bottom;
    }
    if (value == "Left") {
        return DockMode::Left;
    }
    if (value == "Right") {
        return DockMode::Right;
    }
    if (value == "Fill") {
        return DockMode::Fill;
    }

    return std::nullopt;
}

std::string toString(LayoutMode mode)
{
    return layoutModeName(mode);
}

std::optional<LayoutMode> layoutModeFromString(const std::string& value)
{
    if (value == "Absolute") {
        return LayoutMode::Absolute;
    }
    if (value == "Horizontal") {
        return LayoutMode::Horizontal;
    }
    if (value == "Vertical") {
        return LayoutMode::Vertical;
    }
    if (value == "Grid") {
        return LayoutMode::Grid;
    }
    if (value == "TabPage") {
        return LayoutMode::TabPage;
    }

    return std::nullopt;
}

bool Rect::contains(float px, float py) const
{
    return px >= x && py >= y && px <= x + width && py <= y + height;
}

bool Rect::isValid() const
{
    return width > 0.0f && height > 0.0f;
}

WidgetNode::WidgetNode(std::string idValue, std::string nameValue, WidgetType widgetType, Rect widgetBounds)
    : id(std::move(idValue))
    , name(std::move(nameValue))
    , type(widgetType)
    , bounds(widgetBounds)
{
}

const std::string& WidgetNode::typeName() const
{
    return widgetTypeName(type);
}

DockMode WidgetNode::dockMode() const
{
    return dockModeFromString(getStringProperty("dock", "None")).value_or(DockMode::None);
}

LayoutMode WidgetNode::layoutMode() const
{
    return layoutModeFromString(getStringProperty("layoutMode", "Absolute")).value_or(LayoutMode::Absolute);
}

int WidgetNode::selectedTabIndex() const
{
    if (type != WidgetType::TabControl) {
        return 0;
    }

    const int tabCount = static_cast<int>(tabPageCount());
    if (tabCount <= 0) {
        return 0;
    }

    return std::clamp(getIntProperty("selectedTabIndex", getIntProperty("selectedTab", 0)), 0, tabCount - 1);
}

void WidgetNode::setSelectedTabIndex(int index)
{
    const int clampedIndex = std::max(0, index);
    setProperty("selectedTabIndex", clampedIndex);
    setProperty("selectedTab", clampedIndex);
}

std::size_t WidgetNode::tabPageCount() const
{
    return static_cast<std::size_t>(std::count_if(children.begin(), children.end(), [](const WidgetNode& child) {
        return child.type == WidgetType::TabPage;
    }));
}

WidgetNode* WidgetNode::tabPageAt(int index)
{
    return const_cast<WidgetNode*>(std::as_const(*this).tabPageAt(index));
}

const WidgetNode* WidgetNode::tabPageAt(int index) const
{
    if (index < 0) {
        return nullptr;
    }

    int currentIndex = 0;
    for (const auto& child : children) {
        if (child.type != WidgetType::TabPage) {
            continue;
        }

        if (currentIndex == index) {
            return &child;
        }

        ++currentIndex;
    }

    return nullptr;
}

std::string WidgetNode::tabTitle() const
{
    if (type != WidgetType::TabPage) {
        return {};
    }

    const std::string fallback = name.empty() ? std::string{ "Tab" } : name;
    return getStringProperty("title", fallback);
}

PropertyValue* WidgetNode::getProperty(const std::string& key)
{
    const auto iterator = properties.find(key);
    if (iterator == properties.end()) {
        return nullptr;
    }

    return &iterator->second;
}

const PropertyValue* WidgetNode::getProperty(const std::string& key) const
{
    const auto iterator = properties.find(key);
    if (iterator == properties.end()) {
        return nullptr;
    }

    return &iterator->second;
}

void WidgetNode::setProperty(const std::string& key, PropertyValue value)
{
    properties.insert_or_assign(key, std::move(value));
}

std::string WidgetNode::getStringProperty(const std::string& key, const std::string& defaultValue) const
{
    if (const auto* property = getProperty(key)) {
        return property->asString(defaultValue);
    }

    return defaultValue;
}

float WidgetNode::getFloatProperty(const std::string& key, float defaultValue) const
{
    if (const auto* property = getProperty(key)) {
        return property->asFloat(defaultValue);
    }

    return defaultValue;
}

int WidgetNode::getIntProperty(const std::string& key, int defaultValue) const
{
    if (const auto* property = getProperty(key)) {
        return property->asInt(defaultValue);
    }

    return defaultValue;
}

bool WidgetNode::getBoolProperty(const std::string& key, bool defaultValue) const
{
    if (const auto* property = getProperty(key)) {
        return property->asBool(defaultValue);
    }

    return defaultValue;
}

WidgetNode* WidgetNode::findById(const std::string& searchId)
{
    return const_cast<WidgetNode*>(std::as_const(*this).findById(searchId));
}

const WidgetNode* WidgetNode::findById(const std::string& searchId) const
{
    if (id == searchId) {
        return this;
    }

    for (const auto& child : children) {
        if (const auto* match = child.findById(searchId)) {
            return match;
        }
    }

    return nullptr;
}

WidgetNode* WidgetNode::findParentOf(const std::string& childId)
{
    return const_cast<WidgetNode*>(std::as_const(*this).findParentOf(childId));
}

const WidgetNode* WidgetNode::findParentOf(const std::string& childId) const
{
    for (const auto& child : children) {
        if (child.id == childId) {
            return this;
        }
        if (const auto* match = child.findParentOf(childId)) {
            return match;
        }
    }

    return nullptr;
}

bool WidgetNode::removeWidgetById(const std::string& searchId)
{
    const auto iterator = std::find_if(children.begin(), children.end(),
        [&](const WidgetNode& child) { return child.id == searchId; });
    if (iterator != children.end()) {
        children.erase(iterator);
        normalizeChildMetadata(*this);
        return true;
    }

    for (auto& child : children) {
        if (child.removeWidgetById(searchId)) {
            return true;
        }
    }

    return false;
}

bool WidgetNode::addChildToParent(const std::string& parentId, WidgetNode widget)
{
    if (id == parentId) {
        appendChild(std::move(widget));
        return true;
    }

    for (auto& child : children) {
        if (child.addChildToParent(parentId, std::move(widget))) {
            return true;
        }
    }

    return false;
}

WidgetNode* WidgetNode::hitTest(float x, float y)
{
    return const_cast<WidgetNode*>(std::as_const(*this).hitTest(x, y));
}

const WidgetNode* WidgetNode::hitTest(float x, float y) const
{
    for (auto iterator = children.rbegin(); iterator != children.rend(); ++iterator) {
        if (const auto* match = iterator->hitTest(x, y)) {
            return match;
        }
    }

    if (bounds.contains(x, y)) {
        return this;
    }

    return nullptr;
}

void WidgetNode::syncHierarchyMetadata(const std::string& resolvedParentId)
{
    parentId = resolvedParentId;
    normalizeChildMetadata(*this);
}

void WidgetNode::appendChild(WidgetNode child)
{
    child.zOrder = static_cast<int>(children.size());
    child.syncHierarchyMetadata(id);
    children.push_back(std::move(child));
}

} // namespace visiform::model
