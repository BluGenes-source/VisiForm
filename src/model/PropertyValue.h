#pragma once

#include <string>
#include <variant>

namespace visiform::model {

// Placeholder property container for widget metadata.
class PropertyValue {
public:
    using Value = std::variant<std::monostate, bool, int, double, std::string>;

    PropertyValue() = default;
    explicit PropertyValue(Value value);

    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] std::string toDisplayString() const;
    [[nodiscard]] const Value& value() const;

private:
    Value value_{};
};

} // namespace visiform::model
