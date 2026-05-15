#include "model/PropertyValue.h"

#include <sstream>
#include <type_traits>
#include <utility>

namespace visiform::model {

PropertyValue::PropertyValue(Value value)
    : value_(std::move(value))
{
}

bool PropertyValue::isEmpty() const
{
    return std::holds_alternative<std::monostate>(value_);
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
            } else if constexpr (std::is_same_v<CurrentType, std::string>) {
                return currentValue;
            } else {
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
