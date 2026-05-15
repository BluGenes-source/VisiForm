#pragma once

#include "commands/Command.h"

#include <memory>
#include <vector>

namespace visiform::commands {

// Placeholder undo/redo container for editor commands.
class UndoRedoStack {
public:
    void execute(const std::shared_ptr<Command>& command);
    void undo();
    void redo();

    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;

private:
    std::vector<std::shared_ptr<Command>> undoStack_{};
    std::vector<std::shared_ptr<Command>> redoStack_{};
};

} // namespace visiform::commands
