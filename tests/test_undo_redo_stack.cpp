#include "commands/UndoRedoStack.h"

#include <catch2/catch_test_macros.hpp>

#include <utility>

namespace {

class TestCommand final : public visiform::commands::Command {
public:
    explicit TestCommand(int& value, int delta, std::string description = "test")
        : value_(value)
        , delta_(delta)
        , description_(std::move(description))
    {
    }

    void execute() override
    {
        value_ += delta_;
    }

    void undo() override
    {
        value_ -= delta_;
    }

    [[nodiscard]] std::string description() const override
    {
        return description_;
    }

private:
    int& value_;
    int delta_{};
    std::string description_{};
};

} // namespace

TEST_CASE("UndoRedoStack executes, undoes, and redoes commands")
{
    visiform::commands::UndoRedoStack stack;
    int value = 0;

    stack.executeCommand(std::make_unique<TestCommand>(value, 3, "Add 3"));
    REQUIRE(value == 3);
    REQUIRE(stack.canUndo());
    REQUIRE_FALSE(stack.canRedo());
    REQUIRE(stack.undoDescription() == "Add 3");

    stack.undo();
    REQUIRE(value == 0);
    REQUIRE_FALSE(stack.canUndo());
    REQUIRE(stack.canRedo());
    REQUIRE(stack.redoDescription() == "Add 3");

    stack.redo();
    REQUIRE(value == 3);
    REQUIRE(stack.canUndo());
    REQUIRE_FALSE(stack.canRedo());
}

TEST_CASE("UndoRedoStack clears redo history when a new command executes")
{
    visiform::commands::UndoRedoStack stack;
    int value = 0;

    stack.executeCommand(std::make_unique<TestCommand>(value, 1, "Add 1"));
    stack.undo();
    REQUIRE(stack.canRedo());

    stack.executeCommand(std::make_unique<TestCommand>(value, 2, "Add 2"));
    REQUIRE(value == 2);
    REQUIRE_FALSE(stack.canRedo());
    REQUIRE(stack.canUndo());
    REQUIRE(stack.undoDescription() == "Add 2");
}
