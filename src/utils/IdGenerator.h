#pragma once

#include <cstdint>
#include <string>

namespace visiform::utils {

// Placeholder identifier generator for forms and widgets.
class IdGenerator {
public:
    [[nodiscard]] std::string next(const std::string& prefix);

private:
    std::uint64_t nextValue_{1};
};

} // namespace visiform::utils
