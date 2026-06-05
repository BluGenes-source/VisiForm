#pragma once
#pragma once

#include "model/WidgetNode.h"

#include <string>
#include <string_view>
#include <vector>

namespace visiform::model {

struct TreeNodeEntry {
    std::string text{};
    std::string path{};
    int depth = 0;
    int visualDepth = 0;
    int parentIndex = -1;
    bool hasChildren = false;
    bool expanded = false;
};

struct TreeNodeParseResult {
    std::vector<TreeNodeEntry> nodes{};
    bool indentationNormalized = false;
};

struct TableGridSelection {
    int row = -1;
    int column = -1;
};

struct TableGridParseResult {
    std::vector<std::string> columns{};
    std::vector<std::vector<std::string>> rows{};
};

struct WidgetItemActionBinding {
    std::string label{};
    std::string action{};
};

[[nodiscard]] bool supportsItemList(WidgetType type);
[[nodiscard]] bool supportsItemActions(WidgetType type);
[[nodiscard]] std::string_view selectedItemIndexPropertyKey(WidgetType type);
[[nodiscard]] std::vector<std::string> splitItems(std::string_view text);
[[nodiscard]] std::string joinItems(const std::vector<std::string>& items);
[[nodiscard]] std::vector<std::string> getWidgetItems(const WidgetNode& widget);
void setWidgetItems(WidgetNode& widget, const std::vector<std::string>& items);
[[nodiscard]] std::vector<std::string> splitItemActions(std::string_view text);
[[nodiscard]] std::string joinItemActions(const std::vector<std::string>& actions);
[[nodiscard]] std::vector<std::string> getWidgetItemActions(const WidgetNode& widget);
void setWidgetItemActions(WidgetNode& widget, const std::vector<std::string>& actions);
[[nodiscard]] std::vector<WidgetItemActionBinding> getWidgetItemActionBindings(const WidgetNode& widget);
void setWidgetItemActionBindings(WidgetNode& widget, const std::vector<WidgetItemActionBinding>& bindings);
[[nodiscard]] int clampSelectedIndex(const std::vector<std::string>& items, int selectedIndex);
[[nodiscard]] int sanitizeSelectedIndex(const std::vector<std::string>& items, int selectedIndex);
[[nodiscard]] std::string getSelectedItemText(const std::vector<std::string>& items, int selectedIndex);
[[nodiscard]] std::string getSelectedItemAction(const std::vector<std::string>& actions, int selectedIndex);
[[nodiscard]] std::string getSelectedItemAction(const WidgetNode& widget);
void normalizeItemListProperties(WidgetNode& widget);

[[nodiscard]] bool supportsTreeNodes(WidgetType type);
[[nodiscard]] bool supportsTableGrid(WidgetType type);
[[nodiscard]] std::vector<std::string> splitTableColumns(std::string_view text);
[[nodiscard]] std::string joinTableColumns(const std::vector<std::string>& columns);
[[nodiscard]] std::vector<std::vector<std::string>> splitTableRows(std::string_view text);
[[nodiscard]] std::string joinTableRows(const std::vector<std::vector<std::string>>& rows);
[[nodiscard]] TableGridParseResult normalizeTableData(std::string_view columnsText, std::string_view rowsText);
[[nodiscard]] TableGridSelection clampSelectedCell(const std::vector<std::string>& columns,
    const std::vector<std::vector<std::string>>& rows,
    int selectedRow,
    int selectedColumn);
[[nodiscard]] std::string getCellText(const std::vector<std::vector<std::string>>& rows, int row, int column);
void setCellText(std::vector<std::vector<std::string>>& rows, int row, int column, std::string_view text);
void normalizeTableGridProperties(WidgetNode& widget);
[[nodiscard]] TreeNodeParseResult parseTreeNodes(std::string_view text);
[[nodiscard]] std::string serializeTreeNodes(const std::vector<TreeNodeEntry>& nodes);
[[nodiscard]] std::string normalizeTreeIndentation(std::string_view text);
[[nodiscard]] std::vector<std::string> splitTreeNodePaths(std::string_view text);
[[nodiscard]] std::string joinTreeNodePaths(const std::vector<std::string>& paths);
[[nodiscard]] std::string normalizeExpandedTreeNodePaths(std::string_view nodesText, std::string_view expandedNodePathsText);
[[nodiscard]] std::vector<TreeNodeEntry> flattenVisibleTreeNodes(std::string_view nodesText, bool showRoot, std::string_view expandedNodePathsText);
[[nodiscard]] std::string getSelectedNodeText(std::string_view nodesText, std::string_view selectedNodePath);
[[nodiscard]] std::string clampSelectedTreeNode(std::string_view nodesText, std::string_view selectedNodePath, bool showRoot, std::string_view expandedNodePathsText);
[[nodiscard]] bool validateTreeNodeData(std::string_view text);
void normalizeTreeViewProperties(WidgetNode& widget);

} // namespace visiform::model
