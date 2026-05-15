#pragma once

#pragma once

#include <string>
#include <variant>

namespace visiform::model {

class PropertyValue {
public:
    using Value = std::variant<std::monostate, bool, int, float, std::string>;

    PropertyValue() = default;
    PropertyValue(bool value);
    PropertyValue(int value);
    PropertyValue(float value);
    PropertyValue(std::string value);
    PropertyValue(const char* value);
    explicit PropertyValue(Value value);

    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] bool isBool() const;
    [[nodiscard]] bool isInt() const;
    [[nodiscard]] bool isFloat() const;
    [[nodiscard]] bool isString() const;

    [[nodiscard]] bool asBool(bool defaultValue = false) const;
    [[nodiscard]] int asInt(int defaultValue = 0) const;
    [[nodiscard]] float asFloat(float defaultValue = 0.0f) const;
    [[nodiscard]] std::string asString(const std::string& defaultValue = {}) const;

    [[nodiscard]] std::string toDisplayString() const;
    [[nodiscard]] const Value& value() const;

private:
    Value value_{};
};

} // namespace visiform::model
