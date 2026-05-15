#include "ui/MainWindow.h"

namespace visiform::ui {

std::string MainWindow::title() const
{
    return "VisiForm - Form Builder";
}

const DesignerCanvas& MainWindow::designerCanvas() const
{
    return designerCanvas_;
}

const WidgetPalette& MainWindow::widgetPalette() const
{
    return widgetPalette_;
}

const PropertyInspector& MainWindow::propertyInspector() const
{
    return propertyInspector_;
}

const ProjectTree& MainWindow::projectTree() const
{
    return projectTree_;
}

} // namespace visiform::ui
