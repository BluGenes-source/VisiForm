#pragma once

#include <string>

namespace visiform::utils {

[[nodiscard]] bool isValidCppIdentifier(const std::string& text);
[[nodiscard]] std::string sanitizeCppIdentifier(const std::string& text);

} // namespace visiform::utils
