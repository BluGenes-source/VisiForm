#include "model/PropertyValue.h"

#include "model/PropertyValue.h"

#include <sstream>
#include <type_traits>
#include <utility>

namespace visiform::model {

PropertyValue::PropertyValue(bool value)
    : value_(value)
{
}

PropertyValue::PropertyValue(int value)
    : value_(value)
{
}

PropertyValue::PropertyValue(float value)
    : value_(value)
{
}

PropertyValue::PropertyValue(std::string value)
    : value_(std::move(value))
{
}

PropertyValue::PropertyValue(const char* value)
    : value_(value != nullptr ? Value{ std::string{ value } } : Value{})
{
}

PropertyValue::PropertyValue(Value value)
    : value_(std::move(value))
{
}

bool PropertyValue::isEmpty() const
{
    return std::holds_alternative<std::monostate>(value_);
}

bool PropertyValue::isBool() const
{
    return std::holds_alternative<bool>(value_);
}

bool PropertyValue::isInt() const
{
    return std::holds_alternative<int>(value_);
}

bool PropertyValue::isFloat() const
{
    return std::holds_alternative<float>(value_);
}

bool PropertyValue::isString() const
{
    return std::holds_alternative<std::string>(value_);
}

bool PropertyValue::asBool(bool defaultValue) const
{
    if (const auto* storedValue = std::get_if<bool>(&value_)) {
        return *storedValue;
    }

    return defaultValue;
}

int PropertyValue::asInt(int defaultValue) const
{
    if (const auto* storedValue = std::get_if<int>(&value_)) {
        return *storedValue;
    }

    return defaultValue;
}

float PropertyValue::asFloat(float defaultValue) const
{
    if (const auto* storedValue = std::get_if<float>(&value_)) {
        return *storedValue;
    }

    return defaultValue;
}

std::string PropertyValue::asString(const std::string& defaultValue) const
{
    if (const auto* storedValue = std::get_if<std::string>(&value_)) {
        return *storedValue;
    }

    return defaultValue;
}

std::string PropertyValue::toDisplayString() const
{
    return std::visit(
        []<typename T>(const T& currentValue) -> std::string {
            using CurrentType = std::decay_t<T>;

            if constexpr (std::is_same_v<CurrentType, std::monostate>) {
                return "<unset>";
            } else if constexpr (std::is_same_v<CurrentType, bool>) {
                return currentValue ? "true" : "false";
            }
            else if constexpr (std::is_same_v<CurrentType, std::string>) {
                return currentValue;
            }
            else {
                std::ostringstream stream;
                stream << currentValue;
                return stream.str();
            }
        },
        value_);
}

const PropertyValue::Value& PropertyValue::value() const
{
    return value_;
}

} // namespace visiform::model
