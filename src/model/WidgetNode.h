#pragma once

#pragma once

#include "model/LookAndFeelDefinition.h"
#include "model/PropertyValue.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace visiform::utils {
class IdGenerator;
}

namespace visiform::model {

enum class WidgetType {
    FormWindow,
    Frame,
    GroupBox,
    Panel,
    Sizer,
    TabControl,
    TabPage,
    MenuBar,
    ToolBar,
    Label,
    Button,
    TextBox,
    ComboBox,
    ListBox,
    TableGrid,
    TreeView,
    CheckBox,
    RadioButton,
    Slider,
    ScrollBar,
    StatusBar,
    ProgressBar,
    ModalDialog,
    ColorPicker,
    Image,
    Spacer
};

[[nodiscard]] std::string toString(WidgetType type);
[[nodiscard]] std::optional<WidgetType> widgetTypeFromString(const std::string& value);

enum class DockMode {
    None,
    Top,
    Bottom,
    Left,
    Right,
    Fill
};

[[nodiscard]] std::string toString(DockMode mode);
[[nodiscard]] std::optional<DockMode> dockModeFromString(const std::string& value);

enum class AnchorMode {
    None,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    StretchWidthTop,
    StretchWidthBottom,
    StretchHeightLeft,
    StretchHeightRight,
    Fill
};

[[nodiscard]] std::string toString(AnchorMode mode);
[[nodiscard]] std::optional<AnchorMode> anchorModeFromString(const std::string& value);

enum class LayoutMode {
    Absolute,
    Horizontal,
    Vertical,
    Grid,
    TabPage
};

[[nodiscard]] std::string toString(LayoutMode mode);
[[nodiscard]] std::optional<LayoutMode> layoutModeFromString(const std::string& value);

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 100.0f;
    float height = 40.0f;

    [[nodiscard]] bool contains(float px, float py) const;
    [[nodiscard]] bool isValid() const;
};

class WidgetNode {
public:
    WidgetNode() = default;
    WidgetNode(std::string id, std::string name, WidgetType type, Rect bounds = {});

    std::string id{};
    std::string name{};
    WidgetType type = WidgetType::Frame;
    Rect bounds{};
    std::string parentId{};
    int zOrder = 0;
    std::map<std::string, PropertyValue> properties{};
    WidgetLookAndFeelOverrides appearanceOverrides{};
    std::vector<WidgetNode> children{};

    [[nodiscard]] const std::string& typeName() const;
    [[nodiscard]] DockMode dockMode() const;
    [[nodiscard]] AnchorMode anchorMode() const;
    [[nodiscard]] LayoutMode layoutMode() const;
    [[nodiscard]] int selectedTabIndex() const;
    void setSelectedTabIndex(int index);
    [[nodiscard]] std::size_t tabPageCount() const;
    [[nodiscard]] WidgetNode* tabPageAt(int index);
    [[nodiscard]] const WidgetNode* tabPageAt(int index) const;
    [[nodiscard]] std::string tabTitle() const;

    [[nodiscard]] PropertyValue* getProperty(const std::string& key);
    [[nodiscard]] const PropertyValue* getProperty(const std::string& key) const;
    void setProperty(const std::string& key, PropertyValue value);

    [[nodiscard]] std::string getStringProperty(const std::string& key, const std::string& defaultValue) const;
    [[nodiscard]] float getFloatProperty(const std::string& key, float defaultValue) const;
    [[nodiscard]] int getIntProperty(const std::string& key, int defaultValue) const;
    [[nodiscard]] bool getBoolProperty(const std::string& key, bool defaultValue) const;

    [[nodiscard]] WidgetNode* findById(const std::string& searchId);
    [[nodiscard]] const WidgetNode* findById(const std::string& searchId) const;
    [[nodiscard]] WidgetNode* findParentOf(const std::string& childId);
    [[nodiscard]] const WidgetNode* findParentOf(const std::string& childId) const;
    bool removeWidgetById(const std::string& searchId);
    bool addChildToParent(const std::string& parentId, WidgetNode widget);

    [[nodiscard]] WidgetNode* hitTest(float x, float y);
    [[nodiscard]] const WidgetNode* hitTest(float x, float y) const;

    void syncHierarchyMetadata(const std::string& resolvedParentId = {});
    void appendChild(WidgetNode child);

private:
};

} // namespace visiform::model
