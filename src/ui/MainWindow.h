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
#include "validation/ProjectValidator.h"

#include <filesystem>
#include <optional>
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
    void mouseMove(const visage::MouseEvent& e) override;
    void mouseDrag(const visage::MouseEvent& e) override;
    void mouseUp(const visage::MouseEvent& e) override;
    bool mouseWheel(const visage::MouseEvent& e) override;
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
    bool validateProject();
    // Export helpers
    [[nodiscard]] std::filesystem::path defaultExportPath() const;
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
        PanelBounds menuBar{};
        PanelBounds toolbar{};
        PanelBounds widgetPalette{};
        PanelBounds designerCanvas{};
        PanelBounds propertyInspector{};
        PanelBounds projectTree{};
        PanelBounds statusBar{};
        bool showProjectTree = false;
    };

    enum class CommandId {
        None,
        NewProject,
        OpenProject,
        OpenSample,
        SaveProject,
        SaveProjectAsDialog,
        ExportCode,
        ValidateProject,
        ShowValidationReport,
        ShowAboutDialog,
        ShowKeyboardShortcuts,
        ShowGeneratedCodeGuide,
        ShowProjectSettings,
        ShowExportDependencies,
        FitText,
        CopyWidgets,
        PasteWidgets,
        DeleteWidget,
        DuplicateWidget,
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
        ToggleSmartGuides,
        BringForward,
        SendBackward,
        ToggleGrid,
        ToggleSnap,
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
        std::vector<DesignerCanvas::SmartGuide> smartGuides{};
        bool smartGuideSnapUsed = false;
        bool changed = false;
    };

    struct ToolbarButton {
        CommandId command = CommandId::None;
        std::string label{};
        std::string hint{};
        PanelBounds bounds{};
        bool accent = false;
    };

    struct MenuItem {
        std::string id{};
        std::string label{};
        std::string shortcut{};
        CommandId command = CommandId::None;
        bool enabled = true;
        bool checked = false;
        bool separator = false;
        std::optional<model::WidgetType> widgetType{};
        std::optional<std::filesystem::path> filePath{};
    };

    struct Menu {
        std::string label{};
        std::vector<MenuItem> items{};
    };

    struct MenuBarButton {
        int menuIndex = -1;
        PanelBounds bounds{};
    };

    struct MenuItemHit {
        int menuIndex = -1;
        int itemIndex = -1;
        PanelBounds bounds{};
    };

    struct EditorModalButton {
        std::string id{};
        std::string text{};
    };

    struct EditorModalDialog {
        bool visible = false;
        std::string title{};
        std::string message{};
        std::vector<std::string> lines{};
        std::vector<EditorModalButton> buttons{};
        std::string result{};
    };

    void loadLabelFont();
    void updateLayout();
    [[nodiscard]] WindowLayout calculateLayout(float windowWidth, float windowHeight) const;
    void applyLayout(const WindowLayout& layout);
    void updateWindowTitle();
    void drawMenuBar(visage::Canvas& canvas) const;
    void drawToolbar(visage::Canvas& canvas) const;
    void drawStatusBar(visage::Canvas& canvas) const;
    [[nodiscard]] std::vector<Menu> menus() const;
    [[nodiscard]] std::vector<MenuBarButton> menuBarButtons() const;
    [[nodiscard]] std::optional<int> menuIndexAt(float x, float y) const;
    [[nodiscard]] std::optional<MenuItemHit> menuItemAt(float x, float y) const;
    [[nodiscard]] PanelBounds menuDropdownBounds(int menuIndex) const;
    bool handleMenuMouseDown(const visage::MouseEvent& e);
    bool activateMenuItem(const MenuItem& item);
    bool executeCommand(CommandId command);
    [[nodiscard]] bool isCommandEnabled(CommandId command) const;
    [[nodiscard]] bool isCommandChecked(CommandId command) const;
    [[nodiscard]] std::string commandShortcutText(CommandId command) const;
    [[nodiscard]] std::string commandHintText(CommandId command) const;
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
    void toggleSmartGuides();
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
    [[nodiscard]] std::optional<ToolbarButton> toolbarButtonAt(float x, float y) const;
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
    void updateHoverHint(float x, float y);
    void clearCanvasInteraction();
    [[nodiscard]] bool canDrawText() const;
    void showEditorMessageDialog(const std::string& title, const std::string& message);
    void showEditorValidationDialog(const validation::ValidationReport& report,
        const std::string& reportPathText = {},
        const std::string& reportWriteError = {});
    void closeEditorModalDialog(const std::string& result);
    [[nodiscard]] bool isEditorModalVisible() const;
    [[nodiscard]] PanelBounds editorModalDialogBounds() const;
    [[nodiscard]] std::vector<PanelBounds> editorModalButtonBounds() const;
    void drawEditorModalDialog(visage::Canvas& canvas) const;
    bool handleEditorModalMouseDown(const visage::MouseEvent& e);

    WindowLayout layout_{};
    model::ProjectDocument document_ = model::ProjectDocument::createDefault();
    std::filesystem::path currentProjectPath_{};
    std::string statusMessage_{};
    std::string hoverHint_{};
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
    // Export progress state
    bool exportInProgress_ = false;
    int exportProgressPercent_ = 0;
    std::string exportProgressText_{};
    bool suggestionAppliedThisClick_ = false;
    EditorModalDialog editorModal_{};

    // Apply a callback suggestion directly to the selected widget property
    bool applySelectedWidgetCallbackProperty(const std::string& propertyKey, const std::string& callbackName);
    std::vector<model::WidgetNode> clipboardWidgets_{};
    int pasteCount_ = 0;
    int openMenuIndex_ = -1;
};

} // namespace visiform::ui
