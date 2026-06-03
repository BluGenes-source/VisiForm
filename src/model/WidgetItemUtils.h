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

[[nodiscard]] bool supportsItemList(WidgetType type);
[[nodiscard]] std::vector<std::string> splitItems(std::string_view text);
[[nodiscard]] std::string joinItems(const std::vector<std::string>& items);
[[nodiscard]] std::vector<std::string> getWidgetItems(const WidgetNode& widget);
void setWidgetItems(WidgetNode& widget, const std::vector<std::string>& items);
[[nodiscard]] int clampSelectedIndex(const std::vector<std::string>& items, int selectedIndex);
[[nodiscard]] int sanitizeSelectedIndex(const std::vector<std::string>& items, int selectedIndex);
[[nodiscard]] std::string getSelectedItemText(const std::vector<std::string>& items, int selectedIndex);
void normalizeItemListProperties(WidgetNode& widget);

[[nodiscard]] bool supportsTreeNodes(WidgetType type);
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
