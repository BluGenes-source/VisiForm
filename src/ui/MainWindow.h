#pragma once

#pragma once

#include "model/ProjectDocument.h"
#include "ui/DesignerCanvas.h"
#include "ui/ProjectTree.h"
#include "ui/PropertyInspector.h"
#include "ui/WidgetPalette.h"
#include "utils/IdGenerator.h"

#include <filesystem>
#include <string>

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
    bool keyPress(const visage::KeyEvent& e) override;

    bool newProject();
    bool saveProject();
    bool saveProjectAs(const std::filesystem::path& path);
    bool loadProjectFromPath(const std::filesystem::path& path);
    [[nodiscard]] const std::string& statusMessage() const;

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

    enum class ToolbarAction {
        None,
        NewProject,
        OpenSample,
        SaveProject,
        SaveProjectAsDebug
    };

    void loadLabelFont();
    void updateLayout();
    [[nodiscard]] WindowLayout calculateLayout(float windowWidth, float windowHeight) const;
    void applyLayout(const WindowLayout& layout);
    void drawToolbar(visage::Canvas& canvas) const;
    void drawStatusBar(visage::Canvas& canvas) const;
    void addWidgetFromPalette(model::WidgetType type);
    [[nodiscard]] model::WidgetNode createDefaultWidget(model::WidgetType type);
    [[nodiscard]] model::Rect nextDefaultWidgetBounds(model::WidgetType type) const;
    void selectWidget(const std::string& widgetId);
    [[nodiscard]] std::string statusText() const;
    void setOperationStatus(std::string message);
    [[nodiscard]] ToolbarAction toolbarActionAt(float x, float y) const;
    [[nodiscard]] bool isTemplateExamplePath(const std::filesystem::path& path) const;
    [[nodiscard]] std::filesystem::path projectRootPath() const;
    [[nodiscard]] std::filesystem::path sampleProjectPath() const;
    [[nodiscard]] std::filesystem::path defaultDebugSavePath() const;
    [[nodiscard]] bool canDrawText() const;

    WindowLayout layout_{};
    model::ProjectDocument document_ = model::ProjectDocument::createDefault();
    std::filesystem::path currentProjectPath_{};
    std::string statusMessage_{};
    utils::IdGenerator idGenerator_{};
    WidgetPalette widgetPalette_{};
    DesignerCanvas designerCanvas_{};
    PropertyInspector propertyInspector_{};
    ProjectTree projectTree_{};
    visage::Font labelFont_{};
};

} // namespace visiform::ui
