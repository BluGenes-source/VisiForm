#include "model/WidgetItemUtils.h"

#include "model/WidgetItemUtils.h"

#include <algorithm>
#include <map>
#include <set>
#include <cctype>
#include <sstream>

namespace visiform::model {
namespace {

std::string trimItemText(std::string_view text)
{
    std::size_t start = 0;
    std::size_t end = text.size();
    while (start < end && std::isspace(static_cast<unsigned char>(text[start])) != 0) {
        ++start;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
        --end;
    }

    return std::string{ text.substr(start, end - start) };
}

std::vector<std::string> normalizeItems(const std::vector<std::string>& items)
{
    std::vector<std::string> normalizedItems;
    normalizedItems.reserve(items.size());
    for (const auto& item : items) {
        const std::string trimmedItem = trimItemText(item);
        if (!trimmedItem.empty()) {
            normalizedItems.push_back(trimmedItem);
        }
    }
    return normalizedItems;
}

std::vector<std::string> normalizeItemActions(const std::vector<std::string>& actions, std::size_t itemCount)
{
    std::vector<std::string> normalizedActions;
    normalizedActions.resize(itemCount);
    for (std::size_t index = 0; index < itemCount; ++index) {
        if (index < actions.size()) {
            normalizedActions[index] = trimItemText(actions[index]);
        }
    }
    return normalizedActions;
}

std::string trimText(std::string_view text)
{
    std::size_t start = 0;
    std::size_t end = text.size();
    while (start < end && std::isspace(static_cast<unsigned char>(text[start])) != 0) {
        ++start;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1])) != 0) {
        --end;
    }

    return std::string{ text.substr(start, end - start) };
}

std::vector<std::string> splitLines(std::string_view text)
{
    std::vector<std::string> lines;
    if (text.empty()) {
        return lines;
    }

    std::istringstream stream(std::string{ text });
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(std::move(line));
    }

    return lines;
}

std::string joinPathParts(const std::vector<std::string>& parts)
{
    std::string path;
    for (std::size_t index = 0; index < parts.size(); ++index) {
        if (index > 0) {
            path += '/';
        }
        path += parts[index];
    }
    return path;
}

std::set<std::string> expandedPathSet(std::string_view text)
{
    const auto paths = splitTreeNodePaths(text);
    return std::set<std::string>(paths.begin(), paths.end());
}

} // namespace

bool supportsItemList(WidgetType type)
{
    return type == WidgetType::ComboBox
        || type == WidgetType::ListBox
        || type == WidgetType::MenuBar
        || type == WidgetType::ToolBar;
}

bool supportsItemActions(WidgetType type)
{
    return type == WidgetType::MenuBar
        || type == WidgetType::ToolBar;
}

std::string_view selectedItemIndexPropertyKey(WidgetType type)
{
    switch (type) {
    case WidgetType::ComboBox:
    case WidgetType::ListBox:
        return "selectedIndex";
    case WidgetType::MenuBar:
        return "selectedMenuIndex";
    case WidgetType::ToolBar:
        return "selectedToolIndex";
    default:
        return {};
    }
}

bool supportsTableGrid(WidgetType type)
{
    return type == WidgetType::TableGrid;
}

std::vector<std::string> splitItems(std::string_view text)
{
    if (text.empty()) {
        return {};
    }

    std::vector<std::string> items;
    std::istringstream stream(std::string{ text });
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const std::string trimmedItem = trimItemText(line);
        if (!trimmedItem.empty()) {
            items.push_back(trimmedItem);
        }
    }

    return items;
}

std::string joinItems(const std::vector<std::string>& items)
{
    const std::vector<std::string> normalizedItems = normalizeItems(items);
    std::string text;
    for (std::size_t index = 0; index < normalizedItems.size(); ++index) {
        if (index > 0) {
            text += '\n';
        }
        text += normalizedItems[index];
    }
    return text;
}

std::vector<std::string> getWidgetItems(const WidgetNode& widget)
{
    if (!supportsItemList(widget.type)) {
        return {};
    }

    return splitItems(widget.getStringProperty("items", {}));
}

void setWidgetItems(WidgetNode& widget, const std::vector<std::string>& items)
{
    if (!supportsItemList(widget.type)) {
        return;
    }

    widget.setProperty("items", joinItems(items));
    normalizeItemListProperties(widget);
}

std::vector<std::string> splitItemActions(std::string_view text)
{
    std::vector<std::string> actions;
    for (const auto& rawLine : splitLines(text)) {
        actions.push_back(trimText(rawLine));
    }
    return actions;
}

std::string joinItemActions(const std::vector<std::string>& actions)
{
    std::string text;
    for (std::size_t index = 0; index < actions.size(); ++index) {
        if (index > 0) {
            text += '\n';
        }
        text += trimText(actions[index]);
    }
    return text;
}

std::vector<std::string> getWidgetItemActions(const WidgetNode& widget)
{
    if (!supportsItemActions(widget.type)) {
        return {};
    }

    const auto items = getWidgetItems(widget);
    return normalizeItemActions(splitItemActions(widget.getStringProperty("itemActions", {})), items.size());
}

void setWidgetItemActions(WidgetNode& widget, const std::vector<std::string>& actions)
{
    if (!supportsItemActions(widget.type)) {
        return;
    }

    const auto items = getWidgetItems(widget);
    widget.setProperty("itemActions", joinItemActions(normalizeItemActions(actions, items.size())));
    normalizeItemListProperties(widget);
}

std::vector<WidgetItemActionBinding> getWidgetItemActionBindings(const WidgetNode& widget)
{
    const auto items = getWidgetItems(widget);
    const auto actions = getWidgetItemActions(widget);
    std::vector<WidgetItemActionBinding> bindings;
    bindings.reserve(items.size());
    for (std::size_t index = 0; index < items.size(); ++index) {
        bindings.push_back({
            items[index],
            index < actions.size() ? actions[index] : std::string{}
        });
    }
    return bindings;
}

void setWidgetItemActionBindings(WidgetNode& widget, const std::vector<WidgetItemActionBinding>& bindings)
{
    if (!supportsItemList(widget.type)) {
        return;
    }

    std::vector<std::string> items;
    items.reserve(bindings.size());
    std::vector<std::string> actions;
    actions.reserve(bindings.size());
    for (const auto& binding : bindings) {
        items.push_back(binding.label);
        actions.push_back(binding.action);
    }

    widget.setProperty("items", joinItems(items));
    if (supportsItemActions(widget.type)) {
        widget.setProperty("itemActions", joinItemActions(actions));
    }
    normalizeItemListProperties(widget);
}

int clampSelectedIndex(const std::vector<std::string>& items, int selectedIndex)
{
    if (items.empty()) {
        return -1;
    }

    return std::clamp(selectedIndex, 0, static_cast<int>(items.size()) - 1);
}

int sanitizeSelectedIndex(const std::vector<std::string>& items, int selectedIndex)
{
    return clampSelectedIndex(items, selectedIndex);
}

std::string getSelectedItemText(const std::vector<std::string>& items, int selectedIndex)
{
    const int safeIndex = sanitizeSelectedIndex(items, selectedIndex);
    if (safeIndex < 0) {
        return {};
    }

    return items[static_cast<std::size_t>(safeIndex)];
}

std::string getSelectedItemAction(const std::vector<std::string>& actions, int selectedIndex)
{
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(actions.size())) {
        return {};
    }

    return actions[static_cast<std::size_t>(selectedIndex)];
}

std::string getSelectedItemAction(const WidgetNode& widget)
{
    if (!supportsItemActions(widget.type)) {
        return {};
    }

    const auto actions = getWidgetItemActions(widget);
    const std::string selectedIndexKey = std::string(selectedItemIndexPropertyKey(widget.type));
    return getSelectedItemAction(actions, widget.getIntProperty(selectedIndexKey, actions.empty() ? -1 : 0));
}

void normalizeItemListProperties(WidgetNode& widget)
{
    if (!supportsItemList(widget.type)) {
        return;
    }

    const std::vector<std::string> items = getWidgetItems(widget);
    const std::string_view selectedIndexKey = selectedItemIndexPropertyKey(widget.type);
    const int defaultIndex = items.empty() ? -1 : 0;
    const int selectedIndex = clampSelectedIndex(items, widget.getIntProperty(std::string(selectedIndexKey), defaultIndex));
    widget.setProperty("items", joinItems(items));
    if (!selectedIndexKey.empty()) {
        widget.setProperty(std::string(selectedIndexKey), selectedIndex);
    }

    if (supportsItemActions(widget.type)) {
        const auto itemActions = normalizeItemActions(splitItemActions(widget.getStringProperty("itemActions", {})), items.size());
        widget.setProperty("itemActions", joinItemActions(itemActions));
    }

    if (widget.type == WidgetType::ComboBox) {
        widget.setProperty("text", getSelectedItemText(items, selectedIndex));
    }
}

bool supportsTreeNodes(WidgetType type)
{
    return type == WidgetType::TreeView;
}

std::vector<std::string> splitTableColumns(std::string_view text)
{
    std::vector<std::string> columns;
    for (const auto& rawLine : splitLines(text)) {
        columns.push_back(trimText(rawLine));
    }
    return columns;
}

std::string joinTableColumns(const std::vector<std::string>& columns)
{
    std::string text;
    for (std::size_t index = 0; index < columns.size(); ++index) {
        if (index > 0) {
            text += '\n';
        }
        text += trimText(columns[index]);
    }
    return text;
}

std::vector<std::vector<std::string>> splitTableRows(std::string_view text)
{
    std::vector<std::vector<std::string>> rows;
    for (const auto& rawLine : splitLines(text)) {
        std::vector<std::string> row;
        std::size_t start = 0;
        while (start <= rawLine.size()) {
            const std::size_t separator = rawLine.find('\t', start);
            const std::size_t end = separator == std::string::npos ? rawLine.size() : separator;
            row.push_back(trimText(std::string_view{ rawLine }.substr(start, end - start)));
            if (separator == std::string::npos) {
                break;
            }
            start = separator + 1;
        }
        rows.push_back(std::move(row));
    }
    return rows;
}

std::string joinTableRows(const std::vector<std::vector<std::string>>& rows)
{
    std::string text;
    for (std::size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex) {
        if (rowIndex > 0) {
            text += '\n';
        }

        const auto& row = rows[rowIndex];
        for (std::size_t columnIndex = 0; columnIndex < row.size(); ++columnIndex) {
            if (columnIndex > 0) {
                text += '\t';
            }
            text += trimText(row[columnIndex]);
        }
    }
    return text;
}

TableGridParseResult normalizeTableData(std::string_view columnsText, std::string_view rowsText)
{
    TableGridParseResult result;
    result.columns = splitTableColumns(columnsText);
    result.rows = splitTableRows(rowsText);
    return result;
}

TableGridSelection clampSelectedCell(const std::vector<std::string>& columns,
    const std::vector<std::vector<std::string>>& rows,
    int selectedRow,
    int selectedColumn)
{
    TableGridSelection selection;
    selection.row = rows.empty() ? -1 : std::clamp(selectedRow, 0, static_cast<int>(rows.size()) - 1);
    selection.column = columns.empty() ? -1 : std::clamp(selectedColumn, 0, static_cast<int>(columns.size()) - 1);
    return selection;
}

std::string getCellText(const std::vector<std::vector<std::string>>& rows, int row, int column)
{
    if (row < 0 || column < 0 || row >= static_cast<int>(rows.size())) {
        return {};
    }

    const auto& cells = rows[static_cast<std::size_t>(row)];
    if (column >= static_cast<int>(cells.size())) {
        return {};
    }

    return cells[static_cast<std::size_t>(column)];
}

void setCellText(std::vector<std::vector<std::string>>& rows, int row, int column, std::string_view text)
{
    if (row < 0 || column < 0 || row >= static_cast<int>(rows.size())) {
        return;
    }

    auto& cells = rows[static_cast<std::size_t>(row)];
    if (column >= static_cast<int>(cells.size())) {
        cells.resize(static_cast<std::size_t>(column + 1));
    }

    cells[static_cast<std::size_t>(column)] = trimText(text);
}

void normalizeTableGridProperties(WidgetNode& widget)
{
    if (!supportsTableGrid(widget.type)) {
        return;
    }

    const TableGridParseResult data = normalizeTableData(
        widget.getStringProperty("columns", {}),
        widget.getStringProperty("rows", {}));
    const TableGridSelection selection = clampSelectedCell(
        data.columns,
        data.rows,
        widget.getIntProperty("selectedRow", data.rows.empty() ? -1 : 0),
        widget.getIntProperty("selectedColumn", data.columns.empty() ? -1 : 0));

    widget.setProperty("columns", joinTableColumns(data.columns));
    widget.setProperty("rows", joinTableRows(data.rows));
    widget.setProperty("selectedRow", selection.row);
    widget.setProperty("selectedColumn", selection.column);
}

TreeNodeParseResult parseTreeNodes(std::string_view text)
{
    TreeNodeParseResult result;
    std::vector<std::string> pathParts;
    for (const auto& rawLine : splitLines(text)) {
        std::size_t leadingSpaces = 0;
        while (leadingSpaces < rawLine.size() && rawLine[leadingSpaces] == ' ') {
            ++leadingSpaces;
        }

        const std::string nodeText = trimText(rawLine.substr(leadingSpaces));
        if (nodeText.empty()) {
            continue;
        }

        const int rawDepth = static_cast<int>(leadingSpaces / 2);
        const int safeDepth = std::min(rawDepth, static_cast<int>(pathParts.size()));
        if ((leadingSpaces % 2) != 0 || safeDepth != rawDepth) {
            result.indentationNormalized = true;
        }

        pathParts.resize(static_cast<std::size_t>(safeDepth));
        pathParts.push_back(nodeText);

        TreeNodeEntry node;
        node.text = nodeText;
        node.depth = safeDepth;
        node.visualDepth = safeDepth;
        node.parentIndex = safeDepth > 0 ? static_cast<int>(result.nodes.size()) - 1 : -1;
        for (int index = static_cast<int>(result.nodes.size()) - 1; index >= 0; --index) {
            if (result.nodes[static_cast<std::size_t>(index)].depth == safeDepth - 1) {
                node.parentIndex = index;
                break;
            }
        }
        node.path = joinPathParts(pathParts);
        result.nodes.push_back(std::move(node));
    }

    for (std::size_t index = 0; index + 1 < result.nodes.size(); ++index) {
        if (result.nodes[index + 1].depth > result.nodes[index].depth) {
            result.nodes[index].hasChildren = true;
        }
    }

    return result;
}

std::string serializeTreeNodes(const std::vector<TreeNodeEntry>& nodes)
{
    std::string text;
    for (std::size_t index = 0; index < nodes.size(); ++index) {
        if (index > 0) {
            text += '\n';
        }
        text.append(static_cast<std::size_t>(std::max(0, nodes[index].depth)) * 2, ' ');
        text += trimText(nodes[index].text);
    }
    return text;
}

std::string normalizeTreeIndentation(std::string_view text)
{
    return serializeTreeNodes(parseTreeNodes(text).nodes);
}

std::vector<std::string> splitTreeNodePaths(std::string_view text)
{
    std::vector<std::string> paths;
    std::istringstream stream(std::string{ text });
    std::string item;
    while (std::getline(stream, item, ',')) {
        const std::string trimmedItem = trimText(item);
        if (!trimmedItem.empty()) {
            paths.push_back(trimmedItem);
        }
    }
    return paths;
}

std::string joinTreeNodePaths(const std::vector<std::string>& paths)
{
    std::string text;
    for (std::size_t index = 0; index < paths.size(); ++index) {
        if (index > 0) {
            text += ',';
        }
        text += trimText(paths[index]);
    }
    return text;
}

std::string normalizeExpandedTreeNodePaths(std::string_view nodesText, std::string_view expandedNodePathsText)
{
    const TreeNodeParseResult parseResult = parseTreeNodes(nodesText);
    std::map<std::string, bool> expandablePaths;
    for (const auto& node : parseResult.nodes) {
        expandablePaths.insert_or_assign(node.path, node.hasChildren);
    }

    std::vector<std::string> normalizedPaths;
    std::set<std::string> seenPaths;
    for (const auto& path : splitTreeNodePaths(expandedNodePathsText)) {
        const auto iterator = expandablePaths.find(path);
        if (iterator == expandablePaths.end() || !iterator->second || seenPaths.contains(path)) {
            continue;
        }
        normalizedPaths.push_back(path);
        seenPaths.insert(path);
    }

    return joinTreeNodePaths(normalizedPaths);
}

std::vector<TreeNodeEntry> flattenVisibleTreeNodes(std::string_view nodesText, bool showRoot, std::string_view expandedNodePathsText)
{
    const TreeNodeParseResult parseResult = parseTreeNodes(nodesText);
    const std::set<std::string> expandedPaths = expandedPathSet(expandedNodePathsText);

    std::vector<TreeNodeEntry> visibleNodes;
    visibleNodes.reserve(parseResult.nodes.size());
    std::vector<bool> visibility(parseResult.nodes.size(), false);

    for (std::size_t index = 0; index < parseResult.nodes.size(); ++index) {
        TreeNodeEntry node = parseResult.nodes[index];
        const bool expanded = expandedPaths.contains(node.path);
        node.expanded = expanded;

        bool isVisible = false;
        if (node.parentIndex < 0) {
            isVisible = showRoot;
        }
        else {
            const auto& parent = parseResult.nodes[static_cast<std::size_t>(node.parentIndex)];
            const bool parentVisible = visibility[static_cast<std::size_t>(node.parentIndex)];
            const bool hiddenRootParent = !showRoot && parent.depth == 0;
            isVisible = (parentVisible && expandedPaths.contains(parent.path)) || hiddenRootParent;
        }

        visibility[index] = isVisible;
        if (!isVisible) {
            continue;
        }

        node.visualDepth = showRoot ? node.depth : std::max(0, node.depth - 1);
        visibleNodes.push_back(std::move(node));
    }

    return visibleNodes;
}

std::string getSelectedNodeText(std::string_view nodesText, std::string_view selectedNodePath)
{
    const std::string normalizedSelectedPath = trimText(selectedNodePath);
    if (normalizedSelectedPath.empty()) {
        return {};
    }

    const TreeNodeParseResult parseResult = parseTreeNodes(nodesText);
    const auto iterator = std::find_if(parseResult.nodes.begin(), parseResult.nodes.end(), [&normalizedSelectedPath](const TreeNodeEntry& node) {
        return node.path == normalizedSelectedPath;
    });
    return iterator == parseResult.nodes.end() ? std::string{} : iterator->text;
}

std::string clampSelectedTreeNode(std::string_view nodesText, std::string_view selectedNodePath, bool showRoot, std::string_view expandedNodePathsText)
{
    const std::string normalizedSelectedPath = trimText(selectedNodePath);
    const auto visibleNodes = flattenVisibleTreeNodes(nodesText, showRoot, expandedNodePathsText);
    const auto visibleIterator = std::find_if(visibleNodes.begin(), visibleNodes.end(), [&normalizedSelectedPath](const TreeNodeEntry& node) {
        return node.path == normalizedSelectedPath;
    });
    if (visibleIterator != visibleNodes.end()) {
        return visibleIterator->path;
    }

    const TreeNodeParseResult parseResult = parseTreeNodes(nodesText);
    const auto anyIterator = std::find_if(parseResult.nodes.begin(), parseResult.nodes.end(), [&normalizedSelectedPath](const TreeNodeEntry& node) {
        return node.path == normalizedSelectedPath;
    });
    if (anyIterator != parseResult.nodes.end() && !showRoot) {
        return visibleNodes.empty() ? anyIterator->path : visibleNodes.front().path;
    }

    if (!visibleNodes.empty()) {
        return visibleNodes.front().path;
    }
    return parseResult.nodes.empty() ? std::string{} : parseResult.nodes.front().path;
}

bool validateTreeNodeData(std::string_view text)
{
    return !parseTreeNodes(text).indentationNormalized;
}

void normalizeTreeViewProperties(WidgetNode& widget)
{
    if (!supportsTreeNodes(widget.type)) {
        return;
    }

    const std::string normalizedNodes = normalizeTreeIndentation(widget.getStringProperty("nodes", {}));
    const std::string normalizedExpandedPaths = normalizeExpandedTreeNodePaths(
        normalizedNodes,
        widget.getStringProperty("expandedNodePaths", {}));
    const bool showRoot = widget.getBoolProperty("showRoot", true);
    const std::string selectedNodePath = clampSelectedTreeNode(
        normalizedNodes,
        widget.getStringProperty("selectedNodePath", {}),
        showRoot,
        normalizedExpandedPaths);

    widget.setProperty("nodes", normalizedNodes);
    widget.setProperty("expandedNodePaths", normalizedExpandedPaths);
    widget.setProperty("selectedNodePath", selectedNodePath);
}

} // namespace visiform::model
