#pragma once

#include <string>

namespace visiform::commands {

// Placeholder command interface for undoable actions.
class Command {
public:
    virtual ~Command() = default;

    virtual void execute() = 0;
    virtual void undo() = 0;
    [[nodiscard]] virtual std::string name() const = 0;
};

} // namespace visiform::commands
