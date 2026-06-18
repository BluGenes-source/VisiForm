#pragma once

#include "commands/UndoRedoStack.h"
#include "generator/CodeGenerator.h"
#include "model/ProjectDocument.h"
#include "ui/DesignerCanvas.h"
#include "ui/ProjectTree.h"
#include "ui/PropertyInspector.h"
#include "ui/Splitter.h"
#include "ui/WidgetMetrics.h"
#include "ui/WidgetPalette.h"
#include "ui/resources/ImageResourceCache.h"
#include "ui/editors/DropdownControl.h"
#include "ui/editors/TextEditControl.h"
#include "utils/AppSettings.h"
#include "utils/IdGenerator.h"
#include "utils/UiTimer.h"
#include "validation/ProjectValidator.h"

#include <filesystem>
#include <functional>
#include <map>
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
    void dpiChanged() override;
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
        PanelBounds projectTreeCanvasRegion{};
        PanelBounds canvasInspectorRegion{};
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
        ShowResourceManager,
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
        std::string originalParentId{};
        std::string dropTargetWidgetId{};
        std::string marqueeScopeWidgetId{};
        model::Rect originalBounds{};
        DesignerCanvas::FormPoint dragStart{};
        DesignerCanvas::FormPoint currentPoint{};
        std::optional<model::ProjectDocument> sizerItemResizeBeforeDocument{};
        std::optional<model::ProjectDocument> layoutResizeBeforeDocument{};
        int originalSizerItemPreferredWidth = -1;
        int originalSizerItemPreferredHeight = -1;
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

    enum class EditorModalMode {
        Message,
        NewProjectWizard,
        ProjectSettings,
        ResourceManager,
        KeyboardShortcuts,
        ItemListEditor,
        TableGridEditor,
        TreeNodeEditor
    };

    struct ItemListEditorDialogState {
        bool visible = false;
        std::string widgetId{};
        model::WidgetType widgetType = model::WidgetType::FormWindow;
        std::vector<std::string> items{};
        std::vector<std::string> actions{};
        bool supportsActions = false;
        int selectedItemIndex = -1;
        std::string originalItemsText{};
        std::string originalActionsText{};
        int originalSelectedIndex = -1;
        int previewScrollOffset = 0;
    };

    struct TableGridEditorDialogState {
        bool visible = false;
        std::string widgetId{};
        std::string originalColumnsText{};
        std::string originalRowsText{};
        int originalSelectedRow = -1;
        int originalSelectedColumn = -1;
        std::vector<std::string> columns{};
        std::vector<std::vector<std::string>> rows{};
        int selectedRow = -1;
        int selectedColumn = -1;
    };

    struct TableGridEditorActionButton {
        std::string id{};
        std::string text{};
        PanelBounds bounds{};
        bool enabled = true;
    };

    struct TableGridEditorCellHit {
        int row = -1;
        int column = -1;
        bool header = false;
    };

    struct TreeEditorNode {
        int id = -1;
        std::string title{};
        int parentId = -1;
        std::vector<int> childIds{};
        bool expanded = true;
    };

    struct TreeNodeEditorDialogState {
        bool visible = false;
        std::string widgetId{};
        std::string originalNodesText{};
        std::string originalSelectedNodePath{};
        std::string originalExpandedNodePaths{};
        std::vector<TreeEditorNode> nodes{};
        std::vector<int> rootNodeIds{};
        int nextNodeId = 1;
        int selectedNodeId = -1;
        int previewScrollOffset = 0;
    };

    struct TreeNodeEditorRow {
        int nodeId = -1;
        int depth = 0;
        bool hasChildren = false;
        std::string path{};
    };

    struct TreeNodeEditorActionButton {
        std::string id{};
        std::string text{};
        PanelBounds bounds{};
        bool enabled = true;
    };

    struct EditorModalField {
        std::string key{};
        std::string label{};
        std::string value{};
        PropertyInspector::PropertyEditKind editKind = PropertyInspector::PropertyEditKind::ReadOnly;
        std::vector<PropertyInspector::PropertyChoice> choices{};

        EditorModalField() = default;

        EditorModalField(std::string fieldKey, std::string fieldLabel, std::string fieldValue, PropertyInspector::PropertyEditKind fieldEditKind)
            : key(std::move(fieldKey))
            , label(std::move(fieldLabel))
            , value(std::move(fieldValue))
            , editKind(fieldEditKind)
        {
        }

        EditorModalField(std::string fieldKey,
            std::string fieldLabel,
            std::string fieldValue,
            PropertyInspector::PropertyEditKind fieldEditKind,
            std::vector<PropertyInspector::PropertyChoice> fieldChoices)
            : key(std::move(fieldKey))
            , label(std::move(fieldLabel))
            , value(std::move(fieldValue))
            , editKind(fieldEditKind)
            , choices(std::move(fieldChoices))
        {
        }
    };

    struct EditorModalFieldHit {
        EditorModalField field{};
        PanelBounds bounds{};
    };

    struct EditorModalEditState {
        bool active = false;
        std::string key{};
        PropertyInspector::PropertyEditKind editKind = PropertyInspector::PropertyEditKind::ReadOnly;
    };

    struct NewProjectWizardState {
        bool visible = false;
        int step = 0;
        std::string projectName = "My VisiForm App";
        std::string executableName = "MyVisiFormApp";
        std::string userSubclassName = "AppMainWindow";
        std::string windowTitle = "My VisiForm App";
        int formWidth = 900;
        int formHeight = 600;
        std::string lookAndFeelId = "VisiFormDark";
        std::string templateId = "blank";
    };

    struct ProjectSettingsDialogState {
        bool visible = false;
        std::string projectName{};
        std::string executableName{};
        std::string userSubclassName{};
        std::string windowTitle{};
        std::string lookAndFeelId = "VisiFormDark";
        std::string localVisageSourceDirectory{};
        std::string visageGitRepository{};
        std::string visageGitTag{};
    };

    struct ResourceManagerDialogState {
        bool visible = false;
        std::string selectedResourceId{};
        bool confirmReferencedRemoval = false;
        std::string previewSourcePath{};
        bool previewImageAvailable = false;
        int previewImageWidth = 0;
        int previewImageHeight = 0;
        std::string previewStatus{};
    };

    struct KeyboardShortcutDialogState {
        bool visible = false;
        std::string selectedCommandId{};
        std::map<std::string, std::string> pendingShortcuts{};
    };

    struct EditorModalDialog {
        bool visible = false;
        EditorModalMode mode = EditorModalMode::Message;
        std::string title{};
        std::string message{};
        std::vector<std::string> lines{};
        std::vector<EditorModalButton> buttons{};
        std::string result{};
        std::string statusText{};
        float preferredWidth = 0.0f;
        float preferredHeight = 0.0f;
    };

    void loadLabelFont();
    void updateTextEditMetricsFont();
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
    [[nodiscard]] static std::string_view commandRegistryId(CommandId command);
    [[nodiscard]] static CommandId commandFromRegistryId(std::string_view registryId);
    [[nodiscard]] std::string commandShortcutText(CommandId command) const;
    [[nodiscard]] std::string commandHintText(CommandId command) const;
    bool reportUnavailableCommand(CommandId command);
    bool openSampleProject();
    bool saveDebugProject();
    UnsavedChangesResult promptForUnsavedChanges();
    bool confirmSaveIfDirty();
    void loadAppSettings();
    void saveAppSettings();
    void applyCanvasSettings();
    void updateEditorCursor(float x, float y);
    [[nodiscard]] bool hasSelectedNonRootWidgets(std::size_t minimumCount) const;
    [[nodiscard]] model::WidgetNode* selectedNonRootWidget();
    bool requireSelectedNonRootWidgets(std::size_t minimumCount, std::vector<model::WidgetNode*>& selectedWidgets);
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
    [[nodiscard]] std::string resolveDropParentId(const std::string& movingWidgetId, float x, float y) const;
    void addWidgetFromPalette(model::WidgetType type);
    [[nodiscard]] model::WidgetNode createDefaultWidget(model::WidgetType type, const std::string& parentId);
    [[nodiscard]] model::Rect nextDefaultWidgetBounds(model::WidgetType type, const std::string& parentId) const;
    [[nodiscard]] bool autoSizeWidgetForTextProperty(model::WidgetNode& widget, const std::string& key, const std::string& valueText);
    bool setSelectedWidgetName(const std::string& name);
    bool setSelectedWidgetBounds(float x, float y, float width, float height);
    bool setSelectedWidgetProperty(const std::string& key, model::PropertyValue value);
    bool setSelectedWidgetPropertyFromString(const std::string& key, const std::string& valueText);
    bool selectGroupBoxChildFromInspector(const std::string& childId);
    bool selectTabPageFromInspector(const std::string& tabPageId);
    bool addExistingWidgetToSelectedGroupBox(const std::string& childId);
    bool removeSelectedGroupBoxChildToRoot(const std::string& childId);
    bool addTabPageToSelectedTabControl();
    bool removeSelectedTabPageFromSelectedTabControl();
    bool openSelectedWidgetItemEditor();
    bool openSelectedTableGridEditor();
    bool openSelectedTreeNodeEditor();
    bool applyItemListEditor();
    bool applyTableGridEditor();
    bool applyTreeNodeEditor();
    void setItemListEditorSelectedIndex(int index);
    void refreshTableGridEditorState();
    void selectTableGridEditorCell(int row, int column, bool preserveRow = false);
    bool activateTableGridEditorAction(const std::string& actionId);
    bool applyUndoableDocumentChange(const std::string& description, const std::function<bool()>& applyChange);
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
    [[nodiscard]] std::string inspectorPropertyLabel(const std::string& key) const;
    [[nodiscard]] std::string editorModalFieldLabel(const std::string& key) const;
    bool beginInspectorEdit(const PropertyInspector::PropertyRow& row);
    bool commitInspectorEdit();
    void cancelInspectorEdit();
    void cancelPopupEditors();
    void updatePropertyEditorBounds();
    void openInspectorDropdown(const PropertyInspector::PropertyRow& row);
    bool handleInspectorEventAction(const PropertyInspector::PendingEventAction& action);
    void openInspectorEventDropdown(const PropertyInspector::PendingEventAction& action);
    [[nodiscard]] std::string proposedEventHandlerName(const model::WidgetNode& widget,
        const std::string& eventKey,
        const std::string& signatureKind) const;
    [[nodiscard]] std::optional<std::string> assignedHandlerSignatureKind(const std::string& handlerName) const;
    bool applyInspectorDropdownSelection(const std::string& key, const std::string& value, const std::string& label);
    bool applyPendingInspectorInteractionEdit();
    void handleTextEditPendingAction();
    void updateTextEditCaretTimer();
    void stopTextEditCaretTimer();
    void handleTextEditCaretTimerTick();
    void handleDropdownSelection();
    void updateHoverHint(float x, float y);
    void clearCanvasInteraction();
    [[nodiscard]] bool canDrawText() const;
    bool openNewProjectWizard();
    bool openProjectSettingsDialog();
    bool openResourceManagerDialog();
    bool openKeyboardShortcutsDialog();
    void resetNewProjectWizard();
    void populateProjectSettingsDialog();
    void populateResourceManagerDialog();
    void populateKeyboardShortcutsDialog();
    void refreshResourceManagerPreview();
    bool addResourceFromDialog(model::ProjectResourceType resourceType);
    bool removeSelectedResourceFromManager();
    void showEditorMessageDialog(const std::string& title, const std::string& message);
    void showEditorValidationDialog(const validation::ValidationReport& report,
        const std::string& reportPathText = {},
        const std::string& reportWriteError = {});
    void closeEditorModalDialog(const std::string& result);
    bool activateEditorModalButton(const std::string& buttonId);
    [[nodiscard]] bool isEditorModalVisible() const;
    [[nodiscard]] PanelBounds editorModalDialogBounds() const;
    [[nodiscard]] PanelBounds editorModalBodyBounds() const;
    [[nodiscard]] PanelBounds resourceManagerDetailBounds() const;
    [[nodiscard]] PanelBounds resourceManagerPreviewBounds() const;
    [[nodiscard]] PanelBounds itemListEditorPreviewBounds() const;
    [[nodiscard]] PanelBounds itemListEditorFormBounds() const;
    [[nodiscard]] PanelBounds tableGridEditorGridBounds() const;
    [[nodiscard]] PanelBounds tableGridEditorFormBounds() const;
    [[nodiscard]] PanelBounds treeNodeEditorTextBounds() const;
    [[nodiscard]] PanelBounds treeNodeEditorFormBounds() const;
    [[nodiscard]] PanelBounds editorModalStatusBounds() const;
    [[nodiscard]] std::vector<EditorModalField> editorModalFields() const;
    [[nodiscard]] std::vector<EditorModalFieldHit> editorModalFieldHits() const;
    [[nodiscard]] std::optional<EditorModalFieldHit> editorModalFieldAt(float x, float y) const;
    [[nodiscard]] std::optional<int> itemListEditorPreviewIndexAt(float x, float y) const;
    [[nodiscard]] std::optional<TableGridEditorCellHit> tableGridEditorCellAt(float x, float y) const;
    [[nodiscard]] std::optional<int> treeNodeEditorRowAt(float x, float y) const;
    [[nodiscard]] std::string editorModalFieldValue(const std::string& key) const;
    void setEditorModalFieldValue(const std::string& key, const std::string& valueText);
    bool beginEditorModalFieldEdit(const EditorModalField& field);
    bool commitEditorModalFieldEdit();
    void cancelEditorModalFieldEdit();
    void updateEditorModalEditorBounds();
    void openEditorModalDropdown(const EditorModalField& field);
    bool applyEditorModalDropdownSelection(const std::string& key, const std::string& value, const std::string& label);
    [[nodiscard]] std::vector<TableGridEditorActionButton> tableGridEditorActionButtons() const;
    [[nodiscard]] std::vector<TreeNodeEditorRow> visibleTreeNodeEditorRows() const;
    [[nodiscard]] std::vector<TreeNodeEditorActionButton> treeNodeEditorActionButtons() const;
    [[nodiscard]] TreeEditorNode* findTreeEditorNode(int nodeId);
    [[nodiscard]] const TreeEditorNode* findTreeEditorNode(int nodeId) const;
    [[nodiscard]] std::vector<int>* treeNodeEditorSiblingList(int parentId);
    [[nodiscard]] const std::vector<int>* treeNodeEditorSiblingList(int parentId) const;
    [[nodiscard]] std::string treeNodeEditorNodePath(int nodeId) const;
    void refreshTreeNodeEditorState();
    void selectTreeEditorNode(int nodeId);
    bool renameSelectedTreeEditorNode(const std::string& valueText);
    bool activateTreeNodeEditorAction(const std::string& actionId);
    [[nodiscard]] std::string keyboardShortcutDialogEffectiveText(const std::string& commandId) const;
    [[nodiscard]] std::string validateKeyboardShortcutDialog() const;
    bool applyKeyboardShortcutsDialog();
    void resetSelectedKeyboardShortcut();
    [[nodiscard]] std::optional<editors::DropdownControl::Bounds> activeDropdownViewportBounds() const;
    [[nodiscard]] std::vector<editors::DropdownControl::Item> dropdownItemsFromChoices(const std::vector<PropertyInspector::PropertyChoice>& choices) const;
    [[nodiscard]] std::string validateNewProjectWizard() const;
    [[nodiscard]] std::string validateProjectSettingsDialog() const;
    bool applyNewProjectWizard();
    bool applyProjectSettingsDialog();
    [[nodiscard]] model::ProjectDocument createDocumentFromWizard();
    void applyWizardTemplate(model::ProjectDocument& document, const std::string& templateId);
    [[nodiscard]] std::vector<PanelBounds> editorModalButtonBounds() const;
    void drawEditorModalDialog(visage::Canvas& canvas) const;
    bool handleEditorModalMouseDown(const visage::MouseEvent& e);
    void applyNativeWindowIcon();

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
    Splitter projectTreeCanvasSplitter_{};
    Splitter canvasInspectorSplitter_{};
    DesignerCanvas designerCanvas_{};
    PropertyInspector propertyInspector_{};
    ProjectTree projectTree_{};
    editors::TextEditControl textEditControl_{};
    editors::DropdownControl dropdownControl_{};
    utils::UiTimer textEditCaretTimer_{};
    visage::Font labelFont_{};
    resources::ImageResourceCache imageResourceCache_{};
    bool projectTreeWidthInitialized_ = false;
    bool autoSizeTextWidgets_ = true;
    bool multiSelectMode_ = false;
    // Export progress state
    bool exportInProgress_ = false;
    int exportProgressPercent_ = 0;
    std::string exportProgressText_{};
    EditorModalDialog editorModal_{};
    EditorModalEditState editorModalEdit_{};
    NewProjectWizardState newProjectWizard_{};
    ProjectSettingsDialogState projectSettingsDialog_{};
    ResourceManagerDialogState resourceManagerDialog_{};
    KeyboardShortcutDialogState keyboardShortcutDialog_{};
    ItemListEditorDialogState itemListEditorDialog_{};
    TableGridEditorDialogState tableGridEditorDialog_{};
    TreeNodeEditorDialogState treeNodeEditorDialog_{};

    // Apply a callback suggestion directly to the selected widget property
    bool applySelectedWidgetCallbackProperty(const std::string& propertyKey, const std::string& callbackName);
    std::vector<model::WidgetNode> clipboardWidgets_{};
    int pasteCount_ = 0;
    int openMenuIndex_ = -1;
};

} // namespace visiform::ui
