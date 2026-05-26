#pragma once

#pragma once

#include "model/ProjectDocument.h"

#include <string>

namespace visiform::commands {

class Command {
public:
    virtual ~Command() = default;

    virtual void execute() = 0;
    virtual void undo() = 0;
    [[nodiscard]] virtual std::string description() const = 0;
};

class AddWidgetCommand final : public Command {
public:
    AddWidgetCommand(model::ProjectDocument& document, std::string parentId, model::WidgetNode widget, std::string selectionAfterExecute = {});

    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;

private:
    model::ProjectDocument& document_;
    std::string parentId_{};
    model::WidgetNode widget_{};
    std::string selectionBeforeExecute_{};
    std::string selectionAfterExecute_{};
};

class DeleteWidgetCommand final : public Command {
public:
    DeleteWidgetCommand(model::ProjectDocument& document, std::string widgetId);

    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;

private:
    model::ProjectDocument& document_;
    std::string widgetId_{};
    std::string parentId_{};
    model::WidgetNode removedWidget_{};
    std::string selectionBeforeExecute_{};
};

class MoveWidgetCommand final : public Command {
public:
    MoveWidgetCommand(model::ProjectDocument& document, std::string widgetId, model::Rect beforeBounds, model::Rect afterBounds);

    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;

private:
    model::ProjectDocument& document_;
    std::string widgetId_{};
    model::Rect beforeBounds_{};
    model::Rect afterBounds_{};
};

class MoveWidgetsCommand final : public Command {
public:
    struct WidgetBoundsChange {
        std::string widgetId{};
        model::Rect beforeBounds{};
        model::Rect afterBounds{};
    };

    MoveWidgetsCommand(model::ProjectDocument& document,
        std::vector<WidgetBoundsChange> boundsChanges,
        std::vector<std::string> selectedWidgetIds);

    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;

private:
    model::ProjectDocument& document_;
    std::vector<WidgetBoundsChange> boundsChanges_{};
    std::vector<std::string> selectedWidgetIds_{};
};

class ResizeWidgetCommand final : public Command {
public:
    ResizeWidgetCommand(model::ProjectDocument& document, std::string widgetId, model::Rect beforeBounds, model::Rect afterBounds);

    void execute() override;
    void undo() override;
    [[nodiscard]] std::string description() const override;

private:
    model::ProjectDocument& document_;
    std::string widgetId_{};
    model::Rect beforeBounds_{};
    model::Rect afterBounds_{};
};

} // namespace visiform::commands
