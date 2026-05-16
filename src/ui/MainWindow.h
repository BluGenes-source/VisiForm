#pragma once

#pragma once

#include "commands/UndoRedoStack.h"
#include "generator/CodeGenerator.h"
#include "model/ProjectDocument.h"
#include "ui/DesignerCanvas.h"
#include "ui/ProjectTree.h"
#include "ui/PropertyInspector.h"
#include "ui/WidgetMetrics.h"
#include "ui/WidgetPalette.h"
#include "utils/AppSettings.h"
#include "utils/IdGenerator.h"

#include <filesystem>
#include <string>
#include <vector>

#include <visage/app.h>
#include <visage/graphics.h>
#include <visage/widgets.h>

namespace visiform::ui {

class MainWindow : public visage::ApplicationWindow {
public:
    MainWindow();
    ~MainWindow() override = default;

    void showWindow();
    void draw(visage::Canvas& canvas) override;
    void resized() override;
    void mouseDown(const visage::MouseEvent& e) override;
    void mouseDrag(const visage::MouseEvent& e) override;
    void mouseUp(const visage::MouseEvent& e) override;
    bool keyPress(const visage::KeyEvent& e) override;
    bool receivesTextInput() override;
    void textInput(const std::string& text) override;

    bool newProject();
    bool openProjectDialog();
    bool saveProject();
    bool saveProjectAsDialog();
    bool saveProjectAs(const std::filesystem::path& path);
    bool loadProjectFromPath(const std::filesystem::path& path);
    bool exportGeneratedCode();
    void deleteSelectedWidget();
    void duplicateSelectedWidget();
    void undo();
    void redo();
    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;
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
        OpenProject,
        SaveProject,
        SaveProjectAsDialog,
        OpenSample,
        SaveProjectAsDebug,
        ExportCode,
        FitText,
        CopyWidgets,
        PasteWidgets,
        ToggleMultiSelect,
        AlignLeft,
        AlignTop,
        AlignRight,
        AlignBottom,
        CenterHorizontally,
        CenterVertically,
        SameWidth,
        SameHeight,
        DistributeHorizontally,
        DistributeVertically,
        BringForward,
        SendBackward,
        ToggleGrid,
        ToggleSnap,
        DuplicateWidget,
        DeleteWidget,
        UndoAction,
        RedoAction
    };

    enum class UnsavedChangesResult {
        Save,
        DontSave,
        Cancel
    };

    struct CanvasInteractionState {
        enum class Mode {
            None,
            Move,
            Resize,
            MarqueeSelect
        };

        struct SelectionBoundsSnapshot {
            std::string widgetId{};
            model::Rect originalBounds{};
        };

        Mode mode = Mode::None;
        DesignerCanvas::HitRegion region = DesignerCanvas::HitRegion::None;
        std::string widgetId{};
        model::Rect originalBounds{};
        DesignerCanvas::FormPoint dragStart{};
        DesignerCanvas::FormPoint currentPoint{};
        std::vector<SelectionBoundsSnapshot> selectionBounds{};
        bool changed = false;
    };

    struct ToolbarButton {
        ToolbarAction action = ToolbarAction::None;
        std::string label{};
        PanelBounds bounds{};
        bool accent = false;
    };

    void loadLabelFont();
    void updateLayout();
    [[nodiscard]] WindowLayout calculateLayout(float windowWidth, float windowHeight) const;
    void applyLayout(const WindowLayout& layout);
    void updateWindowTitle();
    void drawToolbar(visage::Canvas& canvas) const;
    void drawStatusBar(visage::Canvas& canvas) const;
    bool openSampleProject();
    bool saveDebugProject();
    UnsavedChangesResult promptForUnsavedChanges();
    bool confirmSaveIfDirty();
    void loadAppSettings();
    void saveAppSettings();
    void applyCanvasSettings();
    void fitSelectedWidgetToText();
    void copySelectedWidgets();
    void pasteWidgets();
    void toggleMultiSelectMode();
    [[nodiscard]] bool isMultiSelectModeEnabled() const;
    void handleWidgetClicked(const std::string& widgetId, bool additiveSelection);
    void alignSelectedLeft();
    void alignSelectedTop();
    void alignSelectedRight();
    void alignSelectedBottom();
    void centerSelectedHorizontally();
    void centerSelectedVertically();
    void makeSelectedSameWidth();
    void makeSelectedSameHeight();
    void distributeSelectedHorizontally();
    void distributeSelectedVertically();
    void nudgeSelectedWidgets(float dx, float dy);
    void bringSelectedForward();
    void sendSelectedBackward();
    void toggleGrid();
    void toggleSnapToGrid();
    bool normalizeWidgetBoundsForEditor();
    bool enforceMinimumBoundsRecursive(model::WidgetNode& widget);
    void addWidgetFromPalette(model::WidgetType type);
    [[nodiscard]] model::WidgetNode createDefaultWidget(model::WidgetType type);
    [[nodiscard]] model::Rect nextDefaultWidgetBounds(model::WidgetType type) const;
    [[nodiscard]] bool autoSizeWidgetForTextProperty(model::WidgetNode& widget, const std::string& key, const std::string& valueText);
    bool setSelectedWidgetName(const std::string& name);
    bool setSelectedWidgetBounds(float x, float y, float width, float height);
    bool setSelectedWidgetProperty(const std::string& key, model::PropertyValue value);
    bool setSelectedWidgetPropertyFromString(const std::string& key, const std::string& valueText);
    void selectWidget(const std::string& widgetId);
    [[nodiscard]] std::string statusText() const;
    void setOperationStatus(std::string message);
    [[nodiscard]] std::vector<ToolbarButton> toolbarButtons() const;
    [[nodiscard]] ToolbarAction toolbarActionAt(float x, float y) const;
    [[nodiscard]] bool isTemplateExamplePath(const std::filesystem::path& path) const;
    [[nodiscard]] std::filesystem::path projectRootPath() const;
    [[nodiscard]] std::filesystem::path sampleProjectPath() const;
    [[nodiscard]] std::filesystem::path defaultDebugSavePath() const;
    void addRecentFile(const std::filesystem::path& path);
    void removeRecentFile(const std::filesystem::path& path);
    bool openRecentFile(const std::filesystem::path& path);
    [[nodiscard]] static std::string trimWhitespace(const std::string& value);
    bool beginInspectorEdit(const PropertyInspector::PropertyRow& row);
    bool commitInspectorEdit();
    void cancelInspectorEdit();
    void updatePropertyEditorBounds();
    void clearCanvasInteraction();
    [[nodiscard]] bool canDrawText() const;

    WindowLayout layout_{};
    model::ProjectDocument document_ = model::ProjectDocument::createDefault();
    std::filesystem::path currentProjectPath_{};
    std::string statusMessage_{};
    CanvasInteractionState canvasInteraction_{};
    commands::UndoRedoStack undoRedo_{};
    utils::IdGenerator idGenerator_{};
    utils::AppSettings settings_{};
    WidgetPalette widgetPalette_{};
    DesignerCanvas designerCanvas_{};
    PropertyInspector propertyInspector_{};
    ProjectTree projectTree_{};
    visage::TextEditor propertyEditor_{ "propertyEditor" };
    visage::Font labelFont_{};
    bool autoSizeTextWidgets_ = true;
    bool multiSelectMode_ = false;
    std::vector<model::WidgetNode> clipboardWidgets_{};
    int pasteCount_ = 0;
};

} // namespace visiform::ui
