#include "model/WidgetItemUtils.h"

#include "model/WidgetItemUtils.h"

#include <algorithm>
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

} // namespace

bool supportsItemList(WidgetType type)
{
    return type == WidgetType::ComboBox || type == WidgetType::ListBox;
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

void normalizeItemListProperties(WidgetNode& widget)
{
    if (!supportsItemList(widget.type)) {
        return;
    }

    const std::vector<std::string> items = getWidgetItems(widget);
    const int defaultIndex = items.empty() ? -1 : 0;
    const int selectedIndex = clampSelectedIndex(items, widget.getIntProperty("selectedIndex", defaultIndex));
    widget.setProperty("items", joinItems(items));
    widget.setProperty("selectedIndex", selectedIndex);

    if (widget.type == WidgetType::ComboBox) {
        widget.setProperty("text", getSelectedItemText(items, selectedIndex));
    }
}

} // namespace visiform::model
