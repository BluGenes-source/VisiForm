#include "model/WidgetRegistry.h"
#include "model/WidgetRegistry.h"

#include "model/BoxSizerLayout.h"
#include "model/WidgetItemUtils.h"

#include <algorithm>
#include <cassert>
#include <set>
#include <utility>

namespace visiform::model {
namespace {

void setPaletteMetadata(WidgetDefinition& definition, bool visible, int order, std::string group)
{
    definition.paletteVisible = visible;
    definition.paletteOrder = order;
    definition.paletteGroup = std::move(group);
}

bool hasConsistentPaletteDefinitions(const std::vector<WidgetDefinition>& definitions)
{
    std::set<std::string> displayNames;
    std::set<int> paletteOrders;
    for (const auto& definition : definitions) {
        if (!definition.paletteVisible) {
            continue;
        }

        if (definition.displayName.empty() || definition.defaultNamePrefix.empty()) {
            return false;
        }

        if (!displayNames.insert(definition.displayName).second) {
            return false;
        }

        if (!paletteOrders.insert(definition.paletteOrder).second) {
            return false;
        }
    }

    return true;
}

std::vector<WidgetPropertyDefinition> commonTextProperties()
{
    return {
        { "hint", "hint", PropertyValue{}, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
}

std::vector<WidgetPropertyDefinition> commonStyleProperties(bool includeLookAndFeelId = true)
{
    std::vector<WidgetPropertyDefinition> properties;
    if (includeLookAndFeelId) {
        properties.push_back({ "lookAndFeelId", "Look and Feel", "", PropertyEditKind::Text, true, "Optional widget look and feel override. Empty means inherit from the project." });
    }
    properties.push_back({ "fillColor", "Fill Color", "", PropertyEditKind::Color, true, "Optional fill color override. Empty means inherit." });
    properties.push_back({ "textColor", "Text Color", "", PropertyEditKind::Color, true, "Optional text color override. Empty means inherit." });
    properties.push_back({ "borderColor", "Border Color", "", PropertyEditKind::Color, true, "Optional border color override. Empty means inherit." });
    properties.push_back({ "accentColor", "Accent Color", "", PropertyEditKind::Color, true, "Optional accent color override. Empty means inherit." });
    properties.push_back({ "borderThickness", "Border Thickness", 1.0f, PropertyEditKind::Slider, true, "Border thickness override.", {}, 0.0f, 25.0f, 1.0f });
    properties.push_back({ "cornerRadius", "Corner Radius", 1.0f, PropertyEditKind::Slider, true, "Corner radius override.", {}, 0.0f, 25.0f, 1.0f });
    properties.push_back({ "fontSize", "Font Size", PropertyValue{}, PropertyEditKind::Float, true, "Optional font size override. Empty means inherit." });
    return properties;
}

std::vector<WidgetPropertyDefinition> commonFontProperties()
{
    return {
        { "fontFamily", "Font Family", "Default", PropertyEditKind::Text, true, "Font family name. \"Default\" uses the editor fallback font." },
        { "fontBold", "Bold", false, PropertyEditKind::Bool, true, "Use bold text style when supported by the preview font." },
        { "fontItalic", "Italic", false, PropertyEditKind::Bool, true, "Use italic text style when supported by the preview font." }
    };
}

void appendProperties(std::vector<WidgetPropertyDefinition>& target, const std::vector<WidgetPropertyDefinition>& source)
{
    target.insert(target.end(), source.begin(), source.end());
}

std::vector<WidgetPropertyDefinition> commonChildLayoutProperties()
{
    return {
        { "dock", "Dock", "None", PropertyEditKind::Text, true, "Attaches the widget to an edge of its parent.", { "None", "Top", "Bottom", "Left", "Right", "Fill" } },
        { "anchor", "Anchor", "Top Left", PropertyEditKind::Text, true, "Controls how the widget moves or resizes when its parent resizes.",
            { "Top Left", "Top Right", "Bottom Left", "Bottom Right", "Stretch Width Top", "Stretch Width Bottom", "Stretch Height Left", "Stretch Height Right", "Fill", "None" } },
        { "tabIndex", "Tab Index", 0, PropertyEditKind::Integer, true, "Tab page index used when this widget is inside a tab control." }
    };
}

std::vector<WidgetPropertyDefinition> containerLayoutProperties(LayoutMode layoutMode = LayoutMode::Absolute)
{
    return {
        { "layoutMode", "Layout Mode", toString(layoutMode), PropertyEditKind::Text, true, "Child layout mode used by this container.", { "Absolute", "Horizontal", "Vertical", "Grid", "TabPage" } }
    };
}

WidgetDefinition makeFormWindowDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::FormWindow;
    definition.typeName = "FormWindow";
    definition.displayName = "Form Window";
    setPaletteMetadata(definition, false, -1, "Root");
    definition.defaultNamePrefix = "form";
    definition.defaultHint = "Main form window.";
    definition.canContainChildren = true;
    definition.allowsDrop = true;
    definition.drawsChildrenInside = true;
    definition.defaultChildLayoutMode = LayoutMode::Absolute;
    definition.size = { 900.0f, 600.0f, 300.0f, 200.0f };
    definition.properties = {
        { "title", "title", "MainWindow", PropertyEditKind::Text, true, "Window title text." },
        { "backgroundColor", "backgroundColor", "", PropertyEditKind::Color, true, "Form background color override. Empty means inherit from the look and feel." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, containerLayoutProperties(LayoutMode::Absolute));
    appendProperties(definition.properties, commonStyleProperties(false));
    definition.events = {
        { "onLoad", "onLoad", "void_event", "Called when the form loads." },
        { "onClose", "onClose", "void_event", "Called when the form closes." }
    };
    return definition;
}

WidgetDefinition makeTableGridDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::TableGrid;
    definition.typeName = "TableGrid";
    definition.displayName = "Table / Grid";
    setPaletteMetadata(definition, true, 15, "Data");
    definition.defaultNamePrefix = "tableGrid";
    definition.defaultHint = "Displays editable rows and columns of data.";
    definition.size = { 360.0f, 220.0f, 180.0f, 120.0f };
    definition.properties = {
        { "columns", "Columns", "Name\nType\nValue", PropertyEditKind::Text, true, "Newline-delimited table column names." },
        { "rows", "Rows", "Row 1\tText\tHello\nRow 2\tNumber\t100\nRow 3\tBool\ttrue", PropertyEditKind::Text, true, "Newline-delimited table rows with tab-delimited cells." },
        { "selectedRow", "Selected Row", 0, PropertyEditKind::Integer, true, "Zero-based selected row index. Use -1 when there are no rows." },
        { "selectedColumn", "Selected Column", 0, PropertyEditKind::Integer, true, "Zero-based selected column index. Use -1 when there are no columns." },
        { "showHeader", "Show Header", true, PropertyEditKind::Bool, true, "Draw the header row in the designer and generated runtime." },
        { "showGridLines", "Show Grid Lines", true, PropertyEditKind::Bool, true, "Draw table row and column grid lines." },
        { "rowHeight", "Row Height", 28, PropertyEditKind::Integer, true, "Height of each table data row." },
        { "headerHeight", "Header Height", 30, PropertyEditKind::Integer, true, "Height of the table header row." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onSelectionChanged", "On Selection Changed", "void_event", "Called when the selected cell changes." },
        { "onCellDoubleClick", "On Cell Double Click", "void_event", "Called when a table cell is double-clicked." }
    };
    return definition;
}

WidgetDefinition makeTreeViewDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::TreeView;
    definition.typeName = "TreeView";
    definition.displayName = "Tree View";
    setPaletteMetadata(definition, true, 14, "Data");
    definition.defaultNamePrefix = "treeView";
    definition.defaultHint = "Displays a hierarchical list of expandable tree nodes.";
    definition.size = { 240.0f, 180.0f, 140.0f, 100.0f };
    definition.properties = {
        { "nodes", "Nodes", "Root\n  Child 1\n  Child 2\n    Grandchild 1", PropertyEditKind::Text, true, "Indented tree-node text using two spaces per level." },
        { "selectedNodePath", "Selected Node", "Root/Child 1", PropertyEditKind::Text, true, "Selected node path within the tree." },
        { "expandedNodePaths", "Expanded Nodes", "Root,Root/Child 2", PropertyEditKind::Text, true, "Comma-separated list of expanded node paths." },
        { "showRoot", "Show Root", true, PropertyEditKind::Bool, true, "Show the root node in the tree preview and generated runtime." },
        { "showLines", "Show Lines", true, PropertyEditKind::Bool, true, "Draw connecting guide lines between visible tree nodes." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onChanged", "On Changed", "void_event", "Called when the selected node changes." },
        { "onDoubleClick", "On Double Click", "void_event", "Called when a node is double-clicked." }
    };
    return definition;
}

WidgetDefinition makeStatusBarDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::StatusBar;
    definition.typeName = "StatusBar";
    definition.displayName = "Status Bar";
    setPaletteMetadata(definition, true, 6, "Menu/Toolbar");
    definition.defaultNamePrefix = "statusBar";
    definition.defaultHint = "Displays status messages in one or more fields.";
    definition.size = { 600.0f, 50.0f, 200.0f, 44.0f };
    definition.properties = {
        { "fields", "Sections", 3, PropertyEditKind::Integer, true, "Number of status sections (1-4)." },
        { "text0", "Section 1", "Ready", PropertyEditKind::Text, true, "Text for section 1." },
        { "text1", "Section 2", "This", PropertyEditKind::Text, true, "Text for section 2." },
        { "text2", "Section 3", "Cool", PropertyEditKind::Text, true, "Text for section 3." },
        { "text3", "Section 4", "", PropertyEditKind::Text, true, "Text for section 4." },
        { "fieldWidths", "Section Widths", "1,1,1", PropertyEditKind::Text, true, "Relative section widths e.g. \"1,2,1\"." },
        { "fillWidth", "Fill Width", true, PropertyEditKind::Bool, true, "Stretch the status bar to the root form width when docked." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeProgressBarDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::ProgressBar;
    definition.typeName = "ProgressBar";
    definition.displayName = "Progress Bar";
    setPaletteMetadata(definition, true, 18, "Forms");
    definition.defaultNamePrefix = "progressBar";
    definition.defaultHint = "Displays task progress.";
    definition.size = { 240.0f, 32.0f, 100.0f, 24.0f };
    definition.properties = {
        { "min", "min", 0, PropertyEditKind::Integer, true, "Minimum value." },
        { "max", "max", 100, PropertyEditKind::Integer, true, "Maximum value." },
        { "value", "value", 25, PropertyEditKind::Integer, true, "Current value." },
        { "showText", "showText", true, PropertyEditKind::Bool, true, "Show text overlay." },
        { "text", "text", "", PropertyEditKind::Text, true, "Optional text to display." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeFrameDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Frame;
    definition.typeName = "Frame";
    definition.displayName = "Frame";
    setPaletteMetadata(definition, true, 0, "Containers");
    definition.defaultNamePrefix = "frame";
    definition.defaultHint = "Groups related controls visually.";
    definition.canContainChildren = true;
    definition.allowsDrop = true;
    definition.clipsChildren = true;
    definition.drawsChildrenInside = true;
    definition.defaultChildLayoutMode = LayoutMode::Absolute;
    definition.size = { 300.0f, 180.0f, 180.0f, 120.0f };
    definition.properties = {
        { "title", "title", "Frame", PropertyEditKind::Text, true, "Frame caption text." },
        { "backgroundColor", "backgroundColor", "", PropertyEditKind::Color, true, "Frame background color override. Empty means inherit from the look and feel." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, containerLayoutProperties(LayoutMode::Absolute));
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeGroupBoxDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::GroupBox;
    definition.typeName = "GroupBox";
    definition.displayName = "Group Box";
    setPaletteMetadata(definition, true, 1, "Containers");
    definition.defaultNamePrefix = "groupBox";
    definition.defaultHint = "Titled container for related controls.";
    definition.canContainChildren = true;
    definition.allowsDrop = true;
    definition.clipsChildren = true;
    definition.drawsChildrenInside = true;
    definition.defaultChildLayoutMode = LayoutMode::Absolute;
    definition.size = { 240.0f, 160.0f, 180.0f, 120.0f };
    definition.properties = {
        { "title", "Title", "Group", PropertyEditKind::Text, true, "Group box caption text." },
        { "backgroundColor", "Background Color", "", PropertyEditKind::Color, true, "Optional group box background override." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, containerLayoutProperties(LayoutMode::Absolute));
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makePanelDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Panel;
    definition.typeName = "Panel";
    definition.displayName = "Panel";
    setPaletteMetadata(definition, true, 2, "Containers");
    definition.defaultNamePrefix = "panel";
    definition.defaultHint = "Generic container for child controls.";
    definition.canContainChildren = true;
    definition.allowsDrop = true;
    definition.clipsChildren = true;
    definition.drawsChildrenInside = true;
    definition.defaultChildLayoutMode = LayoutMode::Absolute;
    definition.size = { 300.0f, 200.0f, 180.0f, 120.0f };
    definition.properties = {
        { "backgroundColor", "Background Color", "", PropertyEditKind::Color, true, "Optional panel background override." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, containerLayoutProperties(LayoutMode::Absolute));
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeSizerDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Sizer;
    definition.typeName = "Sizer";
    definition.displayName = "Sizer";
    setPaletteMetadata(definition, true, 23, "Layout");
    definition.defaultNamePrefix = "sizer";
    definition.defaultHint = "Arranges child widgets horizontally or vertically.";
    definition.canContainChildren = true;
    definition.allowsDrop = true;
    definition.clipsChildren = true;
    definition.drawsChildrenInside = true;
    definition.defaultChildLayoutMode = LayoutMode::Vertical;
    definition.size = { 320.0f, 220.0f, 120.0f, 80.0f };
    definition.properties = {
        { "orientation", "Orientation", "Vertical", PropertyEditKind::Text, true, "Sizer direction.", { "Vertical", "Horizontal" } },
        { "padding", "Padding", 8.0f, PropertyEditKind::Slider, true, "Inset around child widgets.", {}, 0.0f, 64.0f, 1.0f },
        { "paddingLeft", "Padding Left", 8, PropertyEditKind::Integer, true, "Left inset around child widgets." },
        { "paddingTop", "Padding Top", 8, PropertyEditKind::Integer, true, "Top inset around child widgets." },
        { "paddingRight", "Padding Right", 8, PropertyEditKind::Integer, true, "Right inset around child widgets." },
        { "paddingBottom", "Padding Bottom", 8, PropertyEditKind::Integer, true, "Bottom inset around child widgets." },
        { "gap", "Gap", 8.0f, PropertyEditKind::Slider, true, "Spacing between child widgets.", {}, 0.0f, 64.0f, 1.0f },
        { "backgroundColor", "Background Color", "", PropertyEditKind::Color, true, "Optional sizer background override." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeTabControlDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::TabControl;
    definition.typeName = "TabControl";
    definition.displayName = "Tab Control";
    setPaletteMetadata(definition, true, 3, "Containers");
    definition.defaultNamePrefix = "tabControl";
    definition.defaultHint = "Container that owns tab pages.";
    definition.canContainChildren = true;
    definition.allowsDrop = true;
    definition.clipsChildren = true;
    definition.drawsChildrenInside = true;
    definition.defaultChildLayoutMode = LayoutMode::TabPage;
    definition.size = { 420.0f, 280.0f, 220.0f, 160.0f };
    definition.properties = {
        { "selectedTabIndex", "Selected Tab", 0, PropertyEditKind::Integer, true, "Selected tab page index used by the designer preview." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, containerLayoutProperties(LayoutMode::TabPage));
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeTabPageDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::TabPage;
    definition.typeName = "TabPage";
    definition.displayName = "Tab Page";
    setPaletteMetadata(definition, false, -1, "Containers");
    definition.defaultNamePrefix = "tabPage";
    definition.defaultHint = "Logical tab page owned by a tab control.";
    definition.canContainChildren = true;
    definition.allowsDrop = true;
    definition.clipsChildren = true;
    definition.drawsChildrenInside = true;
    definition.defaultChildLayoutMode = LayoutMode::Absolute;
    definition.size = { 380.0f, 220.0f, 180.0f, 120.0f };
    definition.properties = {
        { "title", "Title", "Tab", PropertyEditKind::Text, true, "Displayed tab page title." },
        { "backgroundColor", "Background Color", "", PropertyEditKind::Color, true, "Optional tab page background override." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, containerLayoutProperties(LayoutMode::Absolute));
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeMenuBarDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::MenuBar;
    definition.typeName = "MenuBar";
    definition.displayName = "Menu Bar";
    setPaletteMetadata(definition, true, 4, "Menu/Toolbar");
    definition.defaultNamePrefix = "menuBar";
    definition.defaultHint = "Displays a top-level application menu.";
    definition.size = { 900.0f, 32.0f, 220.0f, 28.0f };
    definition.properties = {
        { "items", "Items", "File\nEdit\nView\nProject\nExport\nHelp", PropertyEditKind::Text, true, "Newline-delimited top-level menu item labels." },
        { "itemActions", "Action Bindings", "", PropertyEditKind::Text, true, "Newline-delimited callback names aligned to the MenuBar items by index." },
        { "selectedMenuIndex", "Selected Menu Index", 0, PropertyEditKind::Integer, true, "Zero-based selected menu item index. Use -1 when the menu has no items." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeToolBarDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::ToolBar;
    definition.typeName = "ToolBar";
    definition.displayName = "Tool Bar";
    setPaletteMetadata(definition, true, 5, "Menu/Toolbar");
    definition.defaultNamePrefix = "toolBar";
    definition.defaultHint = "Displays command buttons in a toolbar.";
    definition.size = { 900.0f, 40.0f, 240.0f, 32.0f };
    definition.properties = {
        { "items", "Items", "New\nOpen\nSave\nExport\nValidate", PropertyEditKind::Text, true, "Newline-delimited toolbar item labels." },
        { "itemActions", "Action Bindings", "", PropertyEditKind::Text, true, "Newline-delimited callback names aligned to the ToolBar items by index." },
        { "selectedToolIndex", "Selected Tool Index", 0, PropertyEditKind::Integer, true, "Zero-based selected tool item index. Use -1 when the toolbar has no items." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeLabelDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Label;
    definition.typeName = "Label";
    definition.displayName = "Label";
    setPaletteMetadata(definition, true, 7, "Common");
    definition.defaultNamePrefix = "label";
    definition.defaultHint = "Displays static text.";
    definition.size = { 260.0f, 64.0f, 140.0f, 58.0f };
    definition.properties = {
        { "text", "text", "Label", PropertyEditKind::Text, true, "Displayed label text." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    return definition;
}

WidgetDefinition makeButtonDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Button;
    definition.typeName = "Button";
    definition.displayName = "Button";
    setPaletteMetadata(definition, true, 8, "Common");
    definition.defaultNamePrefix = "button";
    definition.defaultHint = "Runs an action when clicked.";
    definition.size = { 260.0f, 56.0f, 140.0f, 52.0f };
    definition.properties = {
        { "text", "Text", "Button", PropertyEditKind::Text, true, "Default/general label. Used when no state-specific label is provided." },
        { "normalText", "Normal Text", "Button", PropertyEditKind::Text, true, "Optional override for the normal (unpressed) label. If empty, the value of 'Text' is used." },
        { "pressedText", "Pressed Text", "", PropertyEditKind::Text, true, "Optional override shown while the button is pressed or toggled on. If empty, uses 'Normal Text' or 'Text'." },
        { "toggleMode", "Toggle Mode", false, PropertyEditKind::Bool, true, "Keeps the button in a checked state after click." },
        { "checked", "Checked", false, PropertyEditKind::Bool, true, "Initial checked state used when toggle mode is enabled." },
        { "normalFillColor", "Normal Fill Color", "", PropertyEditKind::Color, true, "Optional fill color used for the normal button state." },
        { "pressedFillColor", "Pressed Fill Color", "", PropertyEditKind::Color, true, "Optional fill color used for the pressed or checked button state." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onClick", "On Click", "void_event", "Called when the button click is completed." },
        { "onRelease", "On Release", "void_event", "Called when the left mouse button is released over the button." },
        { "onDoubleClick", "On Double Click", "void_event", "Called when the button is double-clicked." }
    };
    return definition;
}

WidgetDefinition makeTextBoxDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::TextBox;
    definition.typeName = "TextBox";
    definition.displayName = "Text Box";
    setPaletteMetadata(definition, true, 9, "Common");
    definition.defaultNamePrefix = "textBox";
    definition.defaultHint = "Allows text entry.";
    definition.size = { 260.0f, 48.0f, 160.0f, 44.0f };
    definition.properties = {
        { "text", "text", "", PropertyEditKind::Text, true, "Text box contents." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onTextChanged", "onTextChanged", "string_event", "Called when the text changes." }
    };
    return definition;
}

WidgetDefinition makeComboBoxDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::ComboBox;
    definition.typeName = "ComboBox";
    definition.displayName = "Combo Box";
    setPaletteMetadata(definition, true, 12, "Data");
    definition.defaultNamePrefix = "comboBox";
    definition.defaultHint = "Selects one item from a dropdown list.";
    definition.size = { 180.0f, 32.0f, 120.0f, 28.0f };
    definition.properties = {
        { "items", "Items", "Apple\nOrange\nBanana", PropertyEditKind::Text, true, "Newline-delimited list of selectable items." },
        { "selectedIndex", "Selected Index", 0, PropertyEditKind::Integer, true, "Zero-based selected item index. Use -1 when the list is empty." },
        { "text", "Selected Item", "Apple", PropertyEditKind::ReadOnly, false, "Selected item text derived from items and selectedIndex." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onChanged", "On Changed", "void_event", "Called when the selected item changes." }
    };
    return definition;
}

WidgetDefinition makeListBoxDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::ListBox;
    definition.typeName = "ListBox";
    definition.displayName = "List Box";
    setPaletteMetadata(definition, true, 13, "Data");
    definition.defaultNamePrefix = "listBox";
    definition.defaultHint = "Displays a selectable list of items.";
    definition.size = { 220.0f, 140.0f, 140.0f, 80.0f };
    definition.properties = {
        { "items", "Items", "Item 1\nItem 2\nItem 3", PropertyEditKind::Text, true, "Newline-delimited list of selectable items." },
        { "selectedIndex", "Selected Index", 0, PropertyEditKind::Integer, true, "Zero-based selected item index. Use -1 when the list is empty." },
        { "multiSelect", "Multi Select", false, PropertyEditKind::Bool, true, "Allows multiple selection when runtime support is enabled." },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onChanged", "On Changed", "void_event", "Called when the selected item changes." },
        { "onDoubleClick", "On Double Click", "void_event", "Called when an item is double-clicked." }
    };
    return definition;
}

WidgetDefinition makeCheckBoxDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::CheckBox;
    definition.typeName = "CheckBox";
    definition.displayName = "Check Box";
    setPaletteMetadata(definition, true, 10, "Common");
    definition.defaultNamePrefix = "checkBox";
    definition.defaultHint = "Toggles an option on or off.";
    definition.size = { 300.0f, 68.0f, 200.0f, 62.0f };
    definition.properties = {
        { "text", "text", "CheckBox", PropertyEditKind::Text, true, "Check box label text." },
        { "checked", "checked", false, PropertyEditKind::Bool, true, "Initial checked state." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onToggle", "onToggle", "bool_event", "Called when the check box toggles." }
    };
    return definition;
}

WidgetDefinition makeRadioButtonDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::RadioButton;
    definition.typeName = "RadioButton";
    definition.displayName = "Radio Button";
    setPaletteMetadata(definition, true, 11, "Common");
    definition.defaultNamePrefix = "radioButton";
    definition.defaultHint = "Selects one option from a group.";
    definition.size = { 280.0f, 52.0f, 180.0f, 48.0f };
    definition.properties = {
        { "text", "text", "Radio Button", PropertyEditKind::Text, true, "Radio button label text." },
        { "selected", "selected", false, PropertyEditKind::Bool, true, "Initial selected state." },
        { "group", "group", "default", PropertyEditKind::Text, true, "Logical radio group name." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onSelected", "onSelected", "bool_event", "Called when the radio button is selected." }
    };
    return definition;
}

WidgetDefinition makeSliderDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Slider;
    definition.typeName = "Slider";
    definition.displayName = "Slider";
    setPaletteMetadata(definition, true, 16, "Forms");
    definition.defaultNamePrefix = "slider";
    definition.defaultHint = "Adjusts a numeric value.";
    definition.size = { 240.0f, 44.0f, 120.0f, 40.0f };
    definition.properties = {
        { "min", "min", 0, PropertyEditKind::Integer, true, "Minimum slider value." },
        { "max", "max", 100, PropertyEditKind::Integer, true, "Maximum slider value." },
        { "value", "value", 50, PropertyEditKind::Integer, true, "Current slider value." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    definition.events = {
        { "onChanged", "onChanged", "float_event", "Called when the slider value changes." }
    };
    return definition;
}

WidgetDefinition makeScrollBarDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::ScrollBar;
    definition.typeName = "ScrollBar";
    definition.displayName = "Scroll Bar";
    setPaletteMetadata(definition, true, 17, "Forms");
    definition.defaultNamePrefix = "scrollBar";
    definition.defaultHint = "Scrolls through a range of values.";
    definition.size = { 240.0f, 36.0f, 100.0f, 28.0f };
    definition.properties = {
        { "orientation", "orientation", "Horizontal", PropertyEditKind::Text, true, "Scroll bar orientation: Horizontal or Vertical.", { "Horizontal", "Vertical" } },
        { "min", "min", 0, PropertyEditKind::Integer, true, "Minimum scroll value." },
        { "max", "max", 100, PropertyEditKind::Integer, true, "Maximum scroll value." },
        { "value", "value", 0, PropertyEditKind::Integer, true, "Current scroll value." },
        { "pageSize", "pageSize", 10, PropertyEditKind::Integer, true, "Visible page size for the thumb." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    definition.events = {
        { "onChanged", "onChanged", "float_event", "Called when the scroll value changes." }
    };
    return definition;
}

WidgetDefinition makeImageDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Image;
    definition.typeName = "Image";
    definition.displayName = "Image";
    setPaletteMetadata(definition, true, 19, "Additional");
    definition.defaultNamePrefix = "image";
    definition.defaultHint = "Displays or reserves space for an image.";
    definition.size = { 200.0f, 140.0f, 100.0f, 80.0f };
    definition.properties = {
        { "resourceId", "Resource", "", PropertyEditKind::Text, true, "Selects a managed image resource from the project Resource Manager." },
        { "imagePath", "Image Path", "", PropertyEditKind::FilePath, true, "Optional direct image file path used when no managed resource is selected." },
        { "scaleMode", "Scale Mode", "Fit", PropertyEditKind::Text, true, "Controls how the image is fitted inside the widget bounds.", { "Stretch", "Fit", "Fill", "Center" } },
        { "hint", "Hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    return definition;
}

WidgetDefinition makeColorPickerDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::ColorPicker;
    definition.typeName = "ColorPicker";
    definition.displayName = "Color Picker";
    setPaletteMetadata(definition, true, 22, "Forms");
    definition.defaultNamePrefix = "colorPicker";
    definition.defaultHint = "Selects a color value.";
    definition.size = { 220.0f, 40.0f, 140.0f, 34.0f };
    definition.properties = {
        { "value", "value", "#2D7DFF", PropertyEditKind::Color, true, "Selected color value." },
        { "text", "text", "Color", PropertyEditKind::Text, true, "Optional color picker label text." },
        { "showText", "showText", true, PropertyEditKind::Bool, true, "Show the label text beside the swatch." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onChanged", "onChanged", "string_event", "Called when the selected color changes." }
    };
    return definition;
}

WidgetDefinition makeModalDialogDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::ModalDialog;
    definition.typeName = "ModalDialog";
    definition.displayName = "Modal Dialog";
    setPaletteMetadata(definition, true, 21, "Additional");
    definition.defaultNamePrefix = "modalDialog";
    definition.defaultHint = "Displays a modal dialog.";
    definition.size = { 420.0f, 240.0f, 260.0f, 160.0f };
    definition.properties = {
        { "title", "title", "Dialog", PropertyEditKind::Text, true, "Dialog title text." },
        { "message", "message", "Message text", PropertyEditKind::Text, true, "Dialog message text." },
        { "buttons", "buttons", "OK", PropertyEditKind::Text, true, "Comma-separated dialog buttons such as OK or OK,Cancel." },
        { "modal", "modal", true, PropertyEditKind::Bool, true, "Keep the dialog modal at runtime." },
        { "visibleAtStartup", "visibleAtStartup", false, PropertyEditKind::Bool, true, "Show this dialog when the generated window first opens." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    appendProperties(definition.properties, commonFontProperties());
    definition.events = {
        { "onAccepted", "onAccepted", "void_event", "Called when an accept-style dialog button is clicked." },
        { "onCancelled", "onCancelled", "void_event", "Called when a cancel-style dialog button is clicked." }
    };
    return definition;
}

WidgetDefinition makeSpacerDefinition()
{
    WidgetDefinition definition;
    definition.type = WidgetType::Spacer;
    definition.typeName = "Spacer";
    definition.displayName = "Spacer";
    setPaletteMetadata(definition, true, 20, "Layout");
    definition.defaultNamePrefix = "spacer";
    definition.defaultHint = "Adds spacing between widgets.";
    definition.size = { 180.0f, 50.0f, 40.0f, 30.0f };
    definition.properties = {
        { "spacer.kind", "Spacer Kind", "Fixed", PropertyEditKind::Text, true, "Spacer behavior inside a Sizer.", { "Fixed", "Stretch" } },
        { "spacer.size", "Fixed Size", 24, PropertyEditKind::Integer, true, "Fixed spacer size along the parent Sizer main axis." },
        { "hint", "hint", definition.defaultHint, PropertyEditKind::Text, true, "Editor help text shown in VisiForm." }
    };
    appendProperties(definition.properties, commonChildLayoutProperties());
    appendProperties(definition.properties, commonStyleProperties());
    return definition;
}

std::string defaultNameFromId(const WidgetDefinition& definition, const std::string& id)
{
    const auto underscore = id.find_last_of('_');
    const std::string suffix = underscore == std::string::npos ? std::string{} : id.substr(underscore + 1);
    return definition.defaultNamePrefix + suffix;
}

} // namespace

WidgetRegistry::WidgetRegistry()
    : definitions_{
        makeFormWindowDefinition(),
        makeFrameDefinition(),
        makeGroupBoxDefinition(),
        makePanelDefinition(),
        makeSizerDefinition(),
        makeTabControlDefinition(),
        makeTabPageDefinition(),
        makeMenuBarDefinition(),
        makeToolBarDefinition(),
        makeLabelDefinition(),
        makeButtonDefinition(),
        makeTextBoxDefinition(),
        makeComboBoxDefinition(),
        makeListBoxDefinition(),
        makeTableGridDefinition(),
        makeTreeViewDefinition(),
        makeCheckBoxDefinition(),
        makeRadioButtonDefinition(),
        makeSliderDefinition(),
        makeScrollBarDefinition(),
        makeStatusBarDefinition(),
        makeProgressBarDefinition(),
        makeModalDialogDefinition(),
        makeColorPickerDefinition(),
        makeImageDefinition(),
        makeSpacerDefinition() }
{
    assert(hasConsistentPaletteDefinitions(definitions_));
}

const WidgetRegistry& WidgetRegistry::instance()
{
    static const WidgetRegistry registry;
    return registry;
}

const WidgetDefinition* WidgetRegistry::find(WidgetType type) const
{
    const auto iterator = std::find_if(definitions_.begin(), definitions_.end(),
        [type](const WidgetDefinition& definition) { return definition.type == type; });
    return iterator == definitions_.end() ? nullptr : &*iterator;
}

const WidgetDefinition* WidgetRegistry::findByTypeName(const std::string& typeName) const
{
    const auto iterator = std::find_if(definitions_.begin(), definitions_.end(),
        [&typeName](const WidgetDefinition& definition) { return definition.typeName == typeName; });
    return iterator == definitions_.end() ? nullptr : &*iterator;
}

const std::vector<WidgetDefinition>& WidgetRegistry::definitions() const
{
    return definitions_;
}

std::vector<const WidgetDefinition*> WidgetRegistry::paletteDefinitions() const
{
    std::vector<const WidgetDefinition*> paletteDefinitions;
    paletteDefinitions.reserve(definitions_.size());
    for (const auto& definition : definitions_) {
        if (!definition.paletteVisible) {
            continue;
        }

        paletteDefinitions.push_back(&definition);
    }

    std::stable_sort(paletteDefinitions.begin(), paletteDefinitions.end(), [](const WidgetDefinition* left, const WidgetDefinition* right) {
        return left->paletteOrder == right->paletteOrder
            ? left->displayName < right->displayName
            : left->paletteOrder < right->paletteOrder;
    });

    return paletteDefinitions;
}

WidgetNode WidgetRegistry::createDefaultWidget(WidgetType type, const std::string& id) const
{
    if (const WidgetDefinition* definition = find(type)) {
        WidgetNode widget{ id, defaultNameFromId(*definition, id), type,
            Rect{ 0.0f, 0.0f, definition->size.defaultWidth, definition->size.defaultHeight } };
        for (const auto& property : definition->properties) {
            widget.setProperty(property.key, property.defaultValue);
        }
        for (const auto& event : definition->events) {
            if (widget.getProperty(event.key) == nullptr) {
                widget.setProperty(event.key, "");
            }
        }
        normalizeItemListProperties(widget);
        normalizeTableGridProperties(widget);
        normalizeTreeViewProperties(widget);
        normalizeBoxSizerProperties(widget);
        widget.syncHierarchyMetadata();
        return widget;
    }

    return WidgetNode{ id, id, type };
}

bool WidgetRegistry::canContainChildren(WidgetType type) const
{
    const WidgetDefinition* definition = find(type);
    return definition != nullptr && definition->canContainChildren;
}

bool WidgetRegistry::canContainChild(WidgetType parentType, WidgetType childType) const
{
    if (!canContainChildren(parentType)) {
        return false;
    }

    if (parentType == WidgetType::TabControl) {
        return childType == WidgetType::TabPage;
    }

    if (parentType == WidgetType::TabPage) {
        return childType != WidgetType::FormWindow && childType != WidgetType::TabPage;
    }

    return childType != WidgetType::TabPage;
}

} // namespace visiform::model
