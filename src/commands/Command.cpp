#include "commands/Command.h"

#include "commands/Command.h"

#include <utility>

namespace visiform::commands {

AddWidgetCommand::AddWidgetCommand(model::ProjectDocument& document, std::string parentId, model::WidgetNode widget, std::string selectionAfterExecute)
    : document_(document)
    , parentId_(std::move(parentId))
    , widget_(std::move(widget))
    , selectionAfterExecute_(std::move(selectionAfterExecute))
{
    if (selectionAfterExecute_.empty()) {
        selectionAfterExecute_ = widget_.id;
    }
}

void AddWidgetCommand::execute()
{
    selectionBeforeExecute_ = document_.selectedWidgetId;
    document_.addChildToParent(parentId_, widget_);
    document_.selectWidget(selectionAfterExecute_);
}

void AddWidgetCommand::undo()
{
    document_.removeWidgetById(widget_.id);
    document_.selectWidget(selectionBeforeExecute_);
}

std::string AddWidgetCommand::description() const
{
    return "Add widget";
}

DeleteWidgetCommand::DeleteWidgetCommand(model::ProjectDocument& document, std::string widgetId)
    : document_(document)
    , widgetId_(std::move(widgetId))
{
}

void DeleteWidgetCommand::execute()
{
    selectionBeforeExecute_ = document_.selectedWidgetId;
    const auto* parent = document_.findParentOf(widgetId_);
    const auto* widget = document_.findWidgetById(widgetId_);
    if (parent == nullptr || widget == nullptr) {
        return;
    }

    parentId_ = parent->id;
    removedWidget_ = *widget;
    document_.removeWidgetById(widgetId_);
    document_.selectWidget(document_.root.id);
}

void DeleteWidgetCommand::undo()
{
    if (parentId_.empty()) {
        return;
    }

    document_.addChildToParent(parentId_, removedWidget_);
    document_.selectWidget(selectionBeforeExecute_);
}

std::string DeleteWidgetCommand::description() const
{
    return "Delete widget";
}

MoveWidgetCommand::MoveWidgetCommand(model::ProjectDocument& document, std::string widgetId, model::Rect beforeBounds, model::Rect afterBounds)
    : document_(document)
    , widgetId_(std::move(widgetId))
    , beforeBounds_(beforeBounds)
    , afterBounds_(afterBounds)
{
}

void MoveWidgetCommand::execute()
{
    if (auto* widget = document_.findWidgetById(widgetId_)) {
        widget->bounds = afterBounds_;
        document_.selectWidget(widgetId_);
    }
}

void MoveWidgetCommand::undo()
{
    if (auto* widget = document_.findWidgetById(widgetId_)) {
        widget->bounds = beforeBounds_;
        document_.selectWidget(widgetId_);
    }
}

std::string MoveWidgetCommand::description() const
{
    return "Move widget";
}

ResizeWidgetCommand::ResizeWidgetCommand(model::ProjectDocument& document, std::string widgetId, model::Rect beforeBounds, model::Rect afterBounds)
    : document_(document)
    , widgetId_(std::move(widgetId))
    , beforeBounds_(beforeBounds)
    , afterBounds_(afterBounds)
{
}

void ResizeWidgetCommand::execute()
{
    if (auto* widget = document_.findWidgetById(widgetId_)) {
        widget->bounds = afterBounds_;
        document_.selectWidget(widgetId_);
    }
}

void ResizeWidgetCommand::undo()
{
    if (auto* widget = document_.findWidgetById(widgetId_)) {
        widget->bounds = beforeBounds_;
        document_.selectWidget(widgetId_);
    }
}

std::string ResizeWidgetCommand::description() const
{
    return "Resize widget";
}

DocumentStateCommand::DocumentStateCommand(model::ProjectDocument& document,
    std::string description,
    model::ProjectDocument beforeDocument,
    model::ProjectDocument afterDocument)
    : document_(document)
    , description_(std::move(description))
    , beforeDocument_(std::move(beforeDocument))
    , afterDocument_(std::move(afterDocument))
{
}

void DocumentStateCommand::execute()
{
    document_ = afterDocument_;
}

void DocumentStateCommand::undo()
{
    document_ = beforeDocument_;
}

std::string DocumentStateCommand::description() const
{
    return description_;
}

} // namespace visiform::commands
