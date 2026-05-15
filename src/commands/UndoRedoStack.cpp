#include "commands/UndoRedoStack.h"

#include "commands/UndoRedoStack.h"

namespace visiform::commands {

void UndoRedoStack::executeCommand(std::unique_ptr<Command> command)
{
    if (!command) {
        return;
    }

    command->execute();
    undoStack_.push_back(std::move(command));
    redoStack_.clear();
}

void UndoRedoStack::undo()
{
    if (!canUndo()) {
        return;
    }

    auto command = std::move(undoStack_.back());
    undoStack_.pop_back();
    command->undo();
    redoStack_.push_back(std::move(command));
}

void UndoRedoStack::redo()
{
    if (!canRedo()) {
        return;
    }

    auto command = std::move(redoStack_.back());
    redoStack_.pop_back();
    command->execute();
    undoStack_.push_back(std::move(command));
}

void UndoRedoStack::clear()
{
    undoStack_.clear();
    redoStack_.clear();
}

bool UndoRedoStack::canUndo() const
{
    return !undoStack_.empty();
}

bool UndoRedoStack::canRedo() const
{
    return !redoStack_.empty();
}

std::string UndoRedoStack::undoDescription() const
{
    return canUndo() ? undoStack_.back()->description() : std::string{};
}

std::string UndoRedoStack::redoDescription() const
{
    return canRedo() ? redoStack_.back()->description() : std::string{};
}

} // namespace visiform::commands
