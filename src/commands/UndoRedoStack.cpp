#include "commands/UndoRedoStack.h"

namespace visiform::commands {

void UndoRedoStack::execute(const std::shared_ptr<Command>& command)
{
    if (!command) {
        return;
    }

    command->execute();
    undoStack_.push_back(command);
    redoStack_.clear();
}

void UndoRedoStack::undo()
{
    if (!canUndo()) {
        return;
    }

    auto command = undoStack_.back();
    undoStack_.pop_back();
    command->undo();
    redoStack_.push_back(std::move(command));
}

void UndoRedoStack::redo()
{
    if (!canRedo()) {
        return;
    }

    auto command = redoStack_.back();
    redoStack_.pop_back();
    command->execute();
    undoStack_.push_back(std::move(command));
}

bool UndoRedoStack::canUndo() const
{
    return !undoStack_.empty();
}

bool UndoRedoStack::canRedo() const
{
    return !redoStack_.empty();
}

} // namespace visiform::commands
