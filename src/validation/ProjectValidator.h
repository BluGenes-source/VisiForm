#pragma once

#include "model/ProjectDocument.h"
#include "utils/AppSettings.h"

#include <string>
#include <vector>

namespace visiform::validation {

enum class ValidationSeverity {
    Info,
    Warning,
    Error
};

struct ValidationMessage {
    ValidationSeverity severity = ValidationSeverity::Info;
    std::string code{};
    std::string message{};
    std::string widgetId{};
    std::string propertyKey{};
};

struct ValidationReport {
    std::vector<ValidationMessage> messages{};

    [[nodiscard]] bool hasErrors() const;
    [[nodiscard]] bool hasWarnings() const;
    [[nodiscard]] int errorCount() const;
    [[nodiscard]] int warningCount() const;
};

class ProjectValidator {
public:
    [[nodiscard]] ValidationReport validate(
        const model::ProjectDocument& document,
        const utils::AppSettings& settings) const;
};

} // namespace visiform::validation
