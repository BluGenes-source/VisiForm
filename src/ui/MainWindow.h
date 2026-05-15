#pragma once

#pragma once

#include "model/ProjectDocument.h"
#include "ui/DesignerCanvas.h"
#include "ui/ProjectTree.h"
#include "ui/PropertyInspector.h"
#include "ui/WidgetPalette.h"

#include <visage/app.h>
#include <visage/graphics.h>

namespace visiform::ui {

class MainWindow : public visage::ApplicationWindow {
public:
    MainWindow();
    ~MainWindow() override = default;

    void showWindow();
    void draw(visage::Canvas& canvas) override;
    void resized() override;
    void mouseDown(const visage::MouseEvent& e) override;

private:
    struct PanelBounds {
        float x;
        float y;
        float width;
        float height;

        [[nodiscard]] bool isVisible() const
        {
            return width > 0.0f && height > 0.0f;
        }
    };

    struct WindowLayout {
        PanelBounds toolbar{};
        PanelBounds widgetPalette{};
        PanelBounds designerCanvas{};
        PanelBounds propertyInspector{};
        PanelBounds projectTree{};
        PanelBounds statusBar{};
        bool showProjectTree = false;
    };

    void loadLabelFont();
    void updateLayout();
    [[nodiscard]] WindowLayout calculateLayout(float windowWidth, float windowHeight) const;
    void applyLayout(const WindowLayout& layout);
    void drawToolbar(visage::Canvas& canvas) const;
    void drawStatusBar(visage::Canvas& canvas) const;
    void selectWidget(const std::string& widgetId);
    [[nodiscard]] std::string statusText() const;
    [[nodiscard]] bool canDrawText() const;

    WindowLayout layout_{};
    model::ProjectDocument document_ = model::ProjectDocument::createDefault();
    WidgetPalette widgetPalette_{};
    DesignerCanvas designerCanvas_{};
    PropertyInspector propertyInspector_{};
    ProjectTree projectTree_{};
    visage::Font labelFont_{};
};

} // namespace visiform::ui
