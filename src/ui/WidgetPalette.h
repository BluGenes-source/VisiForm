#pragma once

#include <string>

namespace visiform::ui {

// Placeholder widget palette for future drag-and-drop tools.
class WidgetPalette {
public:
    [[nodiscard]] std::string description() const;
};

} // namespace visiform::ui
