#pragma once

#include "ui/DesignerCanvas.h"
#include "ui/ProjectTree.h"
#include "ui/PropertyInspector.h"
#include "ui/WidgetPalette.h"

#include <string>

namespace visiform::ui {

// Placeholder main window composition for the future editor shell.
class MainWindow {
public:
    MainWindow() = default;

    [[nodiscard]] std::string title() const;
    [[nodiscard]] const DesignerCanvas& designerCanvas() const;
    [[nodiscard]] const WidgetPalette& widgetPalette() const;
    [[nodiscard]] const PropertyInspector& propertyInspector() const;
    [[nodiscard]] const ProjectTree& projectTree() const;

private:
    DesignerCanvas designerCanvas_{};
    WidgetPalette widgetPalette_{};
    PropertyInspector propertyInspector_{};
    ProjectTree projectTree_{};
};

} // namespace visiform::ui
