#pragma once

#include <string>

namespace visiform::ui {

// Placeholder inspector for future property editing.
class PropertyInspector {
public:
    [[nodiscard]] std::string description() const;
};

} // namespace visiform::ui
