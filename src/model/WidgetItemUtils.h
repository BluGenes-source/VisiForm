#pragma once
#pragma once

#include "model/WidgetNode.h"

#include <string>
#include <string_view>
#include <vector>

namespace visiform::model {

[[nodiscard]] bool supportsItemList(WidgetType type);
[[nodiscard]] std::vector<std::string> splitItems(std::string_view text);
[[nodiscard]] std::string joinItems(const std::vector<std::string>& items);
[[nodiscard]] std::vector<std::string> getWidgetItems(const WidgetNode& widget);
void setWidgetItems(WidgetNode& widget, const std::vector<std::string>& items);
[[nodiscard]] int clampSelectedIndex(const std::vector<std::string>& items, int selectedIndex);
[[nodiscard]] int sanitizeSelectedIndex(const std::vector<std::string>& items, int selectedIndex);
[[nodiscard]] std::string getSelectedItemText(const std::vector<std::string>& items, int selectedIndex);
void normalizeItemListProperties(WidgetNode& widget);

} // namespace visiform::model
