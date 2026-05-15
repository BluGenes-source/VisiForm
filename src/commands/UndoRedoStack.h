#pragma once

#pragma once

#include "commands/Command.h"

#include <memory>
#include <string>
#include <vector>

namespace visiform::commands {

class UndoRedoStack {
public:
    void executeCommand(std::unique_ptr<Command> command);
    void undo();
    void redo();
    void clear();

    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;
    [[nodiscard]] std::string undoDescription() const;
    [[nodiscard]] std::string redoDescription() const;

private:
    std::vector<std::unique_ptr<Command>> undoStack_{};
    std::vector<std::unique_ptr<Command>> redoStack_{};
};

} // namespace visiform::commands
