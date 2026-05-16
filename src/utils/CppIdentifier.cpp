#include "utils/CppIdentifier.h"

#include <cctype>

namespace visiform::utils {

bool isValidCppIdentifier(const std::string& text)
{
    if (text.empty()) {
        return false;
    }

    const auto isIdentifierStart = [](unsigned char character) {
        return std::isalpha(character) != 0 || character == '_';
    };
    const auto isIdentifierContinue = [](unsigned char character) {
        return std::isalnum(character) != 0 || character == '_';
    };

    if (!isIdentifierStart(static_cast<unsigned char>(text.front()))) {
        return false;
    }

    for (char character : text) {
        if (!isIdentifierContinue(static_cast<unsigned char>(character))) {
            return false;
        }
    }

    return true;
}

std::string sanitizeCppIdentifier(const std::string& text)
{
    std::string sanitized;
    sanitized.reserve(text.size());
    for (char character : text) {
        if (std::isalnum(static_cast<unsigned char>(character)) != 0 || character == '_') {
            sanitized.push_back(character);
        }
        else {
            sanitized.push_back('_');
        }
    }

    if (sanitized.empty()) {
        return "GeneratedIdentifier";
    }
    if (std::isdigit(static_cast<unsigned char>(sanitized.front())) != 0) {
        sanitized.insert(sanitized.begin(), '_');
    }

    return sanitized;
}

} // namespace visiform::utils
