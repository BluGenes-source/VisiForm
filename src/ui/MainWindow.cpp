#include "ui/MainWindow.h"

#include "ui/MainWindow.h"

#include "commands/Command.h"
#include "serialization/JsonProjectReader.h"
#include "serialization/JsonProjectWriter.h"
#include "utils/FileUtils.h"
#include "utils/NativeFileDialogs.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

namespace visiform::ui {
namespace {

constexpr auto kWindowTitle = "VisiForm - Visage Form Builder";
constexpr float kToolbarHeight = 42.0f;
constexpr float kStatusBarHeight = 28.0f;
constexpr float kLeftPanelWidth = 220.0f;
constexpr float kRightPanelWidth = 300.0f;
constexpr float kGap = 8.0f;
constexpr float kProjectTreeMinHeight = 160.0f;
constexpr float kProjectTreePreferredHeight = 180.0f;
constexpr float kPadding = 12.0f;
constexpr float kToolbarButtonWidth = 90.0f;
constexpr float kToolbarButtonHeight = 26.0f;
constexpr float kToolbarButtonSpacing = 6.0f;
constexpr float kNewWidgetStartX = 40.0f;
constexpr float kNewWidgetStartY = 40.0f;
constexpr float kNewWidgetSpacing = 12.0f;

std::string normalizedPathText(const std::filesystem::path& path)
{
    return utils::FileUtils::normalizeSeparators(path.string());
}

std::string defaultWidgetName(model::WidgetType type, const std::string& id)
{
    const auto underscore = id.find_last_of('_');
    const std::string suffix = underscore == std::string::npos ? std::string{} : id.substr(underscore + 1);

    switch (type) {
    case model::WidgetType::Label:
        return "label" + suffix;
    case model::WidgetType::Button:
        return "button" + suffix;
    case model::WidgetType::TextBox:
        return "textBox" + suffix;
    case model::WidgetType::CheckBox:
        return "checkBox" + suffix;
    case model::WidgetType::Slider:
        return "slider" + suffix;
    case model::WidgetType::Frame:
        return "frame" + suffix;
    case model::WidgetType::Image:
        return "image" + suffix;
    case model::WidgetType::Spacer:
        return "spacer" + suffix;
    case model::WidgetType::FormWindow:
        return "form" + suffix;
    }

    return id;
}

std::filesystem::path suggestedProjectPath(const model::ProjectDocument& document, const std::filesystem::path& currentProjectPath)
{
    if (!currentProjectPath.empty()) {
        return currentProjectPath;
    }

    const std::string projectName = document.projectName.empty() ? std::string{ "UntitledVisiFormProject" } : document.projectName;
    return std::filesystem::path{ projectName + std::string{ model::ProjectDocument::projectFileExtension() } };
}

} // namespace

MainWindow::MainWindow()
{
    setTitle(kWindowTitle);
    loadLabelFont();
    propertyEditor_.setTextFieldEntry();
    propertyEditor_.setMargin(8.0f, 0.0f);
    if (canDrawText()) {
        propertyEditor_.setFont(labelFont_);
    }
    propertyEditor_.setVisible(false);
    propertyEditor_.onEnterKey() = [this] {
        commitInspectorEdit();
    };
    propertyEditor_.onEscapeKey() = [this] {
        cancelInspectorEdit();
    };
    addChild(&propertyEditor_);
    loadRecentFiles();
    updateLayout();
}

bool MainWindow::newProject()
{
    cancelInspectorEdit();
    // TODO: Add an unsaved-changes prompt before replacing a dirty document.
    document_ = model::ProjectDocument::createDefault();
    currentProjectPath_.clear();
    undoRedo_.clear();
    document_.clearDirty();
    setOperationStatus("New project created");
    redraw();
    return true;
}

bool MainWindow::openProjectDialog()
{
    // TODO: Add an unsaved-changes prompt before opening another project.
    const auto selectedPath = utils::showOpenProjectDialog();
    if (!selectedPath.has_value()) {
        setOperationStatus("Open cancelled");
        redraw();
        return false;
    }

    return loadProjectFromPath(*selectedPath);
}

bool MainWindow::exportGeneratedCode()
{
    generator::CodeGenerator codeGenerator;
    std::string errorMessage;
    const std::filesystem::path outputPath = projectRootPath() / "Generated" / "ExportedVisageProject";
    if (!codeGenerator.generateProject(document_, outputPath, errorMessage)) {
        setOperationStatus("Code export failed: " + errorMessage);
        redraw();
        return false;
    }

    setOperationStatus("Code exported: Generated/ExportedVisageProject (with CMake presets)");
    redraw();
    return true;
}

bool MainWindow::saveProject()
{
    if (currentProjectPath_.empty() || isTemplateExamplePath(currentProjectPath_)) {
        return saveProjectAsDialog();
    }

    return saveProjectAs(currentProjectPath_);
}

bool MainWindow::saveProjectAsDialog()
{
    const std::filesystem::path suggestedPath = currentProjectPath_.empty() || isTemplateExamplePath(currentProjectPath_)
        ? projectRootPath() / "Generated" / suggestedProjectPath(document_, {})
        : currentProjectPath_;
    const auto selectedPath = utils::showSaveProjectDialog(suggestedPath);
    if (!selectedPath.has_value()) {
        setOperationStatus("Save cancelled");
        redraw();
        return false;
    }

    return saveProjectAs(*selectedPath);
}

bool MainWindow::saveProjectAs(const std::filesystem::path& path)
{
    serialization::JsonProjectWriter writer;
    std::string errorMessage;
    if (!writer.writeToFile(document_, path, errorMessage)) {
        setOperationStatus("Save failed: " + errorMessage);
        redraw();
        return false;
    }

    currentProjectPath_ = path;
    document_.clearDirty();
    addRecentFile(currentProjectPath_);
    setOperationStatus("Project saved: " + normalizedPathText(currentProjectPath_));
    redraw();
    return true;
}

bool MainWindow::loadProjectFromPath(const std::filesystem::path& path)
{
    cancelInspectorEdit();
    // TODO: Add an unsaved-changes prompt before loading another project.
    serialization::JsonProjectReader reader;
    std::string errorMessage;
    auto loadedDocument = reader.readFromFile(path, errorMessage);
    if (!loadedDocument.has_value()) {
        setOperationStatus("Load failed: " + errorMessage);
        redraw();
        return false;
    }

    document_ = std::move(*loadedDocument);
    if (!document_.hasSelection() || document_.selectedWidget() == nullptr) {
        document_.selectWidget(document_.root.id);
    }

    currentProjectPath_ = path;
    undoRedo_.clear();
    document_.clearDirty();
    addRecentFile(currentProjectPath_);
    setOperationStatus("Project loaded: " + normalizedPathText(currentProjectPath_));
    redraw();
    return true;
}

bool MainWindow::openSampleProject()
{
    return loadProjectFromPath(sampleProjectPath());
}

bool MainWindow::saveDebugProject()
{
    serialization::JsonProjectWriter writer;
    std::string errorMessage;
    const std::filesystem::path debugPath = defaultDebugSavePath();
    if (!writer.writeToFile(document_, debugPath, errorMessage)) {
        setOperationStatus("Save failed: " + errorMessage);
        redraw();
        return false;
    }

    document_.clearDirty();
    setOperationStatus("Project saved: " + normalizedPathText(debugPath));
    redraw();
    return true;
}

const std::string& MainWindow::statusMessage() const
{
    return statusMessage_;
}

void MainWindow::showWindow()
{
    show(visage::Dimension::logicalPixels(1200), visage::Dimension::logicalPixels(800));
}

void MainWindow::resized()
{
    updateLayout();
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::draw(visage::Canvas& canvas)
{
    updateLayout();

    canvas.setColor(0xff1b1d23);
    canvas.fill(0, 0, width(), height());

    if (width() <= 0.0f || height() <= 0.0f) {
        return;
    }

    drawToolbar(canvas);
    widgetPalette_.draw(canvas, labelFont_, canDrawText());
    designerCanvas_.draw(canvas, labelFont_, canDrawText(), document_);
    propertyInspector_.draw(canvas, labelFont_, canDrawText(), document_.selectedWidget());
    if (layout_.showProjectTree) {
        projectTree_.drawPanel(canvas, labelFont_, canDrawText(), document_);
    }
    drawStatusBar(canvas);
}

void MainWindow::mouseDown(const visage::MouseEvent& e)
{
    if (!e.isLeftButton()) {
        return;
    }

    requestKeyboardFocus();

    switch (toolbarActionAt(e.position.x, e.position.y)) {
    case ToolbarAction::NewProject:
        newProject();
        return;
    case ToolbarAction::OpenProject:
        openProjectDialog();
        return;
    case ToolbarAction::SaveProjectAsDialog:
        saveProjectAsDialog();
        return;
    case ToolbarAction::OpenSample:
        openSampleProject();
        return;
    case ToolbarAction::SaveProject:
        saveProject();
        return;
    case ToolbarAction::SaveProjectAsDebug:
        saveDebugProject();
        return;
    case ToolbarAction::ExportCode:
        exportGeneratedCode();
        return;
    case ToolbarAction::DuplicateWidget:
        duplicateSelectedWidget();
        return;
    case ToolbarAction::DeleteWidget:
        deleteSelectedWidget();
        return;
    case ToolbarAction::UndoAction:
        undo();
        return;
    case ToolbarAction::RedoAction:
        redo();
        return;
    case ToolbarAction::None:
        break;
    }

    if (const auto widgetType = widgetPalette_.hitTestWidgetType(e.position.x, e.position.y)) {
        cancelInspectorEdit();
        addWidgetFromPalette(*widgetType);
        return;
    }

    if (propertyInspector_.isEditing()) {
        // Clicking another row or leaving the inspector attempts to commit the current edit.
        // If validation fails, editing stays active and the click is consumed.
        if (!commitInspectorEdit()) {
            return;
        }
    }

    if (const auto row = propertyInspector_.hitTestRow(document_.selectedWidget(), e.position.x, e.position.y)) {
        if (row->editKind == PropertyInspector::PropertyEditKind::Bool) {
            const bool currentValue = document_.selectedWidget() != nullptr
                && document_.selectedWidget()->getBoolProperty(row->key, false);
            setSelectedWidgetProperty(row->key, !currentValue);
        }
        else if (row->editKind != PropertyInspector::PropertyEditKind::ReadOnly) {
            beginInspectorEdit(*row);
        }
        return;
    }

    if (layout_.showProjectTree) {
        if (const auto widgetId = projectTree_.hitTestWidgetId(document_, e.position.x, e.position.y)) {
            selectWidget(*widgetId);
            return;
        }
        if (const auto recentFileIndex = projectTree_.hitTestRecentFileIndex(document_, e.position.x, e.position.y)) {
            openRecentFile(recentFiles_.paths()[*recentFileIndex]);
            return;
        }
    }

    if (const auto widgetId = designerCanvas_.hitTestWidgetId(document_, e.position.x, e.position.y)) {
        const bool wasSelected = document_.selectedWidgetId == *widgetId;
        selectWidget(*widgetId);

        if (wasSelected && *widgetId != document_.root.id) {
            const auto interactionHit = designerCanvas_.hitTestInteraction(document_, e.position.x, e.position.y, document_.selectedWidgetId);
            const auto dragStart = designerCanvas_.toFormPoint(document_, e.position.x, e.position.y);
            auto* widget = document_.findWidgetById(*widgetId);
            if (interactionHit.has_value() && dragStart.has_value() && widget != nullptr) {
                canvasInteraction_.widgetId = *widgetId;
                canvasInteraction_.region = interactionHit->region;
                canvasInteraction_.originalBounds = widget->bounds;
                canvasInteraction_.dragStart = *dragStart;
                canvasInteraction_.changed = false;
                canvasInteraction_.mode = interactionHit->region == DesignerCanvas::HitRegion::Body
                    ? CanvasInteractionState::Mode::Move
                    : CanvasInteractionState::Mode::Resize;
            }
        }
        else {
            clearCanvasInteraction();
        }
    }
}

void MainWindow::mouseDrag(const visage::MouseEvent& e)
{
    if (canvasInteraction_.mode == CanvasInteractionState::Mode::None) {
        return;
    }

    auto* widget = document_.findWidgetById(canvasInteraction_.widgetId);
    const auto currentPoint = designerCanvas_.toFormPoint(document_, e.position.x, e.position.y);
    if (widget == nullptr || currentPoint == std::nullopt) {
        return;
    }

    model::Rect updatedBounds = canvasInteraction_.originalBounds;
    if (canvasInteraction_.mode == CanvasInteractionState::Mode::Move) {
        updatedBounds = designerCanvas_.moveBounds(canvasInteraction_.originalBounds, canvasInteraction_.dragStart, *currentPoint);
    }
    else if (canvasInteraction_.mode == CanvasInteractionState::Mode::Resize) {
        updatedBounds = designerCanvas_.resizeBounds(canvasInteraction_.originalBounds, canvasInteraction_.region,
            canvasInteraction_.dragStart, *currentPoint);
    }

    if (updatedBounds.x != widget->bounds.x || updatedBounds.y != widget->bounds.y
        || updatedBounds.width != widget->bounds.width || updatedBounds.height != widget->bounds.height) {
        widget->bounds = updatedBounds;
        canvasInteraction_.changed = true;
        redraw();
    }
}

void MainWindow::mouseUp(const visage::MouseEvent& e)
{
    if (!e.isLeftButton() || canvasInteraction_.mode == CanvasInteractionState::Mode::None) {
        return;
    }

    auto* widget = document_.findWidgetById(canvasInteraction_.widgetId);
    if (canvasInteraction_.changed && widget != nullptr) {
        const model::Rect finalBounds = widget->bounds;
        widget->bounds = canvasInteraction_.originalBounds;
        if (canvasInteraction_.mode == CanvasInteractionState::Mode::Move) {
            undoRedo_.executeCommand(std::make_unique<commands::MoveWidgetCommand>(
                document_, widget->id, canvasInteraction_.originalBounds, finalBounds));
        }
        else {
            undoRedo_.executeCommand(std::make_unique<commands::ResizeWidgetCommand>(
                document_, widget->id, canvasInteraction_.originalBounds, finalBounds));
        }

        document_.markDirty();
        const std::string displayName = widget->name.empty() ? widget->id : widget->name;
        if (canvasInteraction_.mode == CanvasInteractionState::Mode::Move) {
            setOperationStatus("Moved widget: " + displayName + " (" + widget->id + ")");
        }
        else {
            setOperationStatus("Resized widget: " + displayName + " (" + widget->id + ")");
        }
    }

    clearCanvasInteraction();
    redraw();
}

bool MainWindow::keyPress(const visage::KeyEvent& e)
{
    using KeyCode = visage::KeyCode;
    if (propertyInspector_.isEditing()) {
        return false;
    }

    if (e.keyCode() == KeyCode::Delete) {
        setOperationStatus("Delete shortcut received");
        deleteSelectedWidget();
        return true;
    }

    if (!e.isCtrlDown()) {
        return false;
    }

    if (e.isShiftDown() && e.keyCode() == KeyCode::S) {
        return saveProjectAsDialog();
    }

    if (e.keyCode() == KeyCode::N) {
        return newProject();
    }
    if (e.keyCode() == KeyCode::O) {
        return openProjectDialog();
    }
    if (e.keyCode() == KeyCode::S) {
        return saveProject();
    }
    if (e.keyCode() == KeyCode::D) {
        setOperationStatus("Duplicate shortcut received");
        duplicateSelectedWidget();
        return true;
    }
    if (e.keyCode() == KeyCode::Z) {
        if (e.isShiftDown()) {
            redo();
        }
        else {
            undo();
        }
        return true;
    }
    if (e.keyCode() == KeyCode::Y) {
        redo();
        return true;
    }

    return false;
}

bool MainWindow::receivesTextInput()
{
    return false;
}

void MainWindow::textInput(const std::string& text)
{
    (void)text;
}

void MainWindow::addWidgetFromPalette(model::WidgetType type)
{
    if (document_.root.type != model::WidgetType::FormWindow) {
        setOperationStatus("Add widget failed: root form is invalid");
        redraw();
        return;
    }

    model::WidgetNode widget = createDefaultWidget(type);
    const std::string addedId = widget.id;
    undoRedo_.executeCommand(std::make_unique<commands::AddWidgetCommand>(document_, document_.root.id, std::move(widget), addedId));
    document_.markDirty();
    setOperationStatus("Added widget: " + addedId);
    redraw();
}

void MainWindow::deleteSelectedWidget()
{
    cancelInspectorEdit();
    const auto* selectedWidget = document_.selectedWidget();
    if (selectedWidget == nullptr) {
        setOperationStatus("No widget selected");
        redraw();
        return;
    }
    if (document_.isRootWidgetId(selectedWidget->id)) {
        setOperationStatus("Cannot delete root form");
        redraw();
        return;
    }

    const std::string widgetId = selectedWidget->id;
    const std::string displayName = selectedWidget->name.empty() ? selectedWidget->id : selectedWidget->name;
    if (!document_.removeWidgetById(widgetId)) {
        setOperationStatus("Delete failed: " + widgetId);
        redraw();
        return;
    }

    // TODO: Reconnect delete to `DeleteWidgetCommand` after the direct flow is verified stable.
    undoRedo_.clear();
    document_.selectWidget(document_.root.id);
    document_.markDirty();
    setOperationStatus("Deleted widget: " + displayName + " (" + widgetId + ")");
    redraw();
}

void MainWindow::duplicateSelectedWidget()
{
    cancelInspectorEdit();
    const auto* selectedWidget = document_.selectedWidget();
    if (selectedWidget == nullptr) {
        setOperationStatus("No widget selected");
        redraw();
        return;
    }
    if (document_.isRootWidgetId(selectedWidget->id)) {
        setOperationStatus("Cannot duplicate root form");
        redraw();
        return;
    }

    const std::string selectedId = selectedWidget->id;

    auto* duplicate = document_.duplicateWidgetById(selectedId, idGenerator_);
    if (duplicate == nullptr) {
        setOperationStatus("Duplicate failed: " + selectedId);
        redraw();
        return;
    }

    const std::string duplicateId = duplicate->id;
    const std::string displayName = duplicate->name.empty() ? duplicate->id : duplicate->name;

    // TODO: Reconnect duplicate to `AddWidgetCommand` or a dedicated duplicate command after the direct flow is verified stable.
    undoRedo_.clear();
    document_.selectWidget(duplicateId);
    document_.markDirty();
    setOperationStatus("Duplicated widget: " + displayName + " (" + duplicateId + ")");
    redraw();
}

void MainWindow::undo()
{
    if (!undoRedo_.canUndo()) {
        return;
    }

    const std::string description = undoRedo_.undoDescription();
    undoRedo_.undo();
    document_.markDirty();
    setOperationStatus("Undo: " + description);
    redraw();
}

void MainWindow::redo()
{
    if (!undoRedo_.canRedo()) {
        return;
    }

    const std::string description = undoRedo_.redoDescription();
    undoRedo_.redo();
    document_.markDirty();
    setOperationStatus("Redo: " + description);
    redraw();
}

bool MainWindow::canUndo() const
{
    return undoRedo_.canUndo();
}

bool MainWindow::canRedo() const
{
    return undoRedo_.canRedo();
}

model::WidgetNode MainWindow::createDefaultWidget(model::WidgetType type)
{
    const std::string id = idGenerator_.next(type, document_);
    model::WidgetNode widget{ id, defaultWidgetName(type, id), type, nextDefaultWidgetBounds(type) };

    switch (type) {
    case model::WidgetType::Label:
        widget.setProperty("text", "Label");
        break;
    case model::WidgetType::Button:
        widget.setProperty("text", "Button");
        break;
    case model::WidgetType::TextBox:
        widget.setProperty("text", "");
        break;
    case model::WidgetType::CheckBox:
        widget.setProperty("text", "CheckBox");
        widget.setProperty("checked", false);
        break;
    case model::WidgetType::Slider:
        widget.setProperty("min", 0);
        widget.setProperty("max", 100);
        widget.setProperty("value", 50);
        break;
    case model::WidgetType::Frame:
        widget.setProperty("title", "Frame");
        break;
    case model::WidgetType::Image:
        widget.setProperty("source", "");
        break;
    case model::WidgetType::Spacer:
        break;
    case model::WidgetType::FormWindow:
        widget.setProperty("title", "FormWindow");
        break;
    }

    return widget;
}

model::Rect MainWindow::nextDefaultWidgetBounds(model::WidgetType type) const
{
    float width = 160.0f;
    float height = 40.0f;

    switch (type) {
    case model::WidgetType::Label:
        width = 160.0f;
        height = 28.0f;
        break;
    case model::WidgetType::Button:
        width = 160.0f;
        height = 40.0f;
        break;
    case model::WidgetType::TextBox:
        width = 180.0f;
        height = 32.0f;
        break;
    case model::WidgetType::CheckBox:
        width = 180.0f;
        height = 28.0f;
        break;
    case model::WidgetType::Slider:
        width = 180.0f;
        height = 32.0f;
        break;
    case model::WidgetType::Frame:
        width = 220.0f;
        height = 140.0f;
        break;
    case model::WidgetType::Image:
        width = 160.0f;
        height = 100.0f;
        break;
    case model::WidgetType::Spacer:
        width = 160.0f;
        height = 40.0f;
        break;
    case model::WidgetType::FormWindow:
        width = 220.0f;
        height = 160.0f;
        break;
    }

    float nextY = kNewWidgetStartY;
    for (const auto& child : document_.root.children) {
        nextY = std::max(nextY, child.bounds.y + child.bounds.height + kNewWidgetSpacing);
    }

    const float maxY = std::max(kNewWidgetStartY, document_.root.bounds.height - height - kNewWidgetStartY);
    if (nextY > maxY) {
        nextY = kNewWidgetStartY;
    }

    return { kNewWidgetStartX, nextY, width, height };
}

bool MainWindow::setSelectedWidgetName(const std::string& name)
{
    auto* widget = document_.selectedWidget();
    if (widget == nullptr) {
        setOperationStatus("No widget selected");
        redraw();
        return false;
    }

    const std::string trimmedName = trimWhitespace(name);
    if (trimmedName.empty()) {
        setOperationStatus("Invalid value for name");
        redraw();
        return false;
    }

    widget->name = trimmedName;
    document_.markDirty();
    setOperationStatus("Widget renamed: " + trimmedName);
    updatePropertyEditorBounds();
    redraw();
    return true;
}

bool MainWindow::setSelectedWidgetBounds(float x, float y, float width, float height)
{
    auto* widget = document_.selectedWidget();
    if (widget == nullptr) {
        setOperationStatus("No widget selected");
        redraw();
        return false;
    }

    if (x < 0.0f || y < 0.0f || width < 20.0f || height < 20.0f) {
        setOperationStatus("Invalid bounds for selected widget");
        redraw();
        return false;
    }

    widget->bounds = { x, y, width, height };
    document_.markDirty();
    updatePropertyEditorBounds();
    redraw();
    return true;
}

bool MainWindow::setSelectedWidgetProperty(const std::string& key, model::PropertyValue value)
{
    auto* widget = document_.selectedWidget();
    if (widget == nullptr) {
        setOperationStatus("No widget selected");
        redraw();
        return false;
    }

    widget->setProperty(key, std::move(value));
    document_.markDirty();
    setOperationStatus("Property changed: " + key);
    updatePropertyEditorBounds();
    redraw();
    return true;
}

bool MainWindow::setSelectedWidgetPropertyFromString(const std::string& key, const std::string& valueText)
{
    auto* widget = document_.selectedWidget();
    if (widget == nullptr) {
        setOperationStatus("No widget selected");
        redraw();
        return false;
    }

    const std::string trimmedValue = trimWhitespace(valueText);
    auto parseFloat = [](const std::string& text, float& output) -> bool {
        try {
            std::size_t parsedCharacters = 0;
            output = std::stof(text, &parsedCharacters);
            return parsedCharacters == text.size();
        }
        catch (...) {
            return false;
        }
    };
    auto parseInt = [](const std::string& text, int& output) -> bool {
        try {
            std::size_t parsedCharacters = 0;
            output = std::stoi(text, &parsedCharacters);
            return parsedCharacters == text.size();
        }
        catch (...) {
            return false;
        }
    };
    auto parseBool = [](const std::string& text, bool& output) -> bool {
        std::string normalized = text;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(),
            [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
        if (normalized == "true" || normalized == "1" || normalized == "yes" || normalized == "on") {
            output = true;
            return true;
        }
        if (normalized == "false" || normalized == "0" || normalized == "no" || normalized == "off") {
            output = false;
            return true;
        }

        return false;
    };

    if (key == "name") {
        return setSelectedWidgetName(trimmedValue);
    }

    if (key == "x" || key == "y" || key == "width" || key == "height") {
        float numericValue = 0.0f;
        if (!parseFloat(trimmedValue, numericValue)) {
            setOperationStatus("Invalid value for " + key);
            redraw();
            return false;
        }

        float x = widget->bounds.x;
        float y = widget->bounds.y;
        float width = widget->bounds.width;
        float height = widget->bounds.height;
        if (key == "x") {
            x = numericValue;
        }
        else if (key == "y") {
            y = numericValue;
        }
        else if (key == "width") {
            width = numericValue;
        }
        else {
            height = numericValue;
        }

        if (!setSelectedWidgetBounds(x, y, width, height)) {
            setOperationStatus("Invalid value for " + key);
            redraw();
            return false;
        }

        setOperationStatus("Property changed: " + key);
        redraw();
        return true;
    }

    if (const auto* existingProperty = widget->getProperty(key)) {
        if (existingProperty->isBool()) {
            bool parsedValue = false;
            if (!parseBool(trimmedValue, parsedValue)) {
                setOperationStatus("Invalid value for " + key);
                redraw();
                return false;
            }

            return setSelectedWidgetProperty(key, parsedValue);
        }
        if (existingProperty->isInt()) {
            int parsedValue = 0;
            if (!parseInt(trimmedValue, parsedValue)) {
                setOperationStatus("Invalid value for " + key);
                redraw();
                return false;
            }

            return setSelectedWidgetProperty(key, parsedValue);
        }
        if (existingProperty->isFloat()) {
            float parsedValue = 0.0f;
            if (!parseFloat(trimmedValue, parsedValue)) {
                setOperationStatus("Invalid value for " + key);
                redraw();
                return false;
            }

            return setSelectedWidgetProperty(key, parsedValue);
        }
    }

    return setSelectedWidgetProperty(key, trimmedValue);
}

void MainWindow::loadLabelFont()
{
    static constexpr std::array<const char*, 3> kFontCandidates = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "C:/Windows/Fonts/arial.ttf"
    };

    for (const char* fontPath : kFontCandidates) {
        std::ifstream fontFile(fontPath, std::ios::binary);
        if (!fontFile.good()) {
            continue;
        }

        labelFont_ = visage::Font(18.0f, std::string{ fontPath });
        if (canDrawText()) {
            return;
        }
    }
}

void MainWindow::updateLayout()
{
    applyLayout(calculateLayout(width(), height()));
}

MainWindow::WindowLayout MainWindow::calculateLayout(float windowWidth, float windowHeight) const
{
    WindowLayout layout;
    if (windowWidth <= 0.0f || windowHeight <= 0.0f) {
        return layout;
    }

    layout.toolbar = { 0.0f, 0.0f, windowWidth, kToolbarHeight };
    layout.statusBar = { 0.0f, std::max(0.0f, windowHeight - kStatusBarHeight), windowWidth, kStatusBarHeight };

    const float contentTop = layout.toolbar.height + kGap;
    const float contentBottom = std::max(contentTop, layout.statusBar.y - kGap);
    const float contentHeight = std::max(0.0f, contentBottom - contentTop);

    const float leftWidth = std::min(kLeftPanelWidth, std::max(140.0f, windowWidth * 0.2f));
    const float rightWidth = std::min(kRightPanelWidth, std::max(220.0f, windowWidth * 0.24f));
    const float leftX = kGap;
    const float rightX = std::max(leftX + leftWidth + kGap, windowWidth - rightWidth - kGap);

    float projectTreeHeight = 0.0f;
    if (contentHeight >= 420.0f) {
        projectTreeHeight = std::min(kProjectTreePreferredHeight, contentHeight * 0.28f);
        projectTreeHeight = std::max(projectTreeHeight, kProjectTreeMinHeight);
    }

    const bool showProjectTree = projectTreeHeight > 0.0f && contentHeight > projectTreeHeight + 120.0f;
    const float paletteHeight = showProjectTree ? contentHeight - projectTreeHeight - kGap : contentHeight;

    layout.widgetPalette = { leftX, contentTop, leftWidth, std::max(0.0f, paletteHeight) };
    layout.showProjectTree = showProjectTree;
    if (showProjectTree) {
        layout.projectTree = { leftX, contentTop + paletteHeight + kGap, leftWidth, projectTreeHeight };
    }

    layout.propertyInspector = { rightX, contentTop, rightWidth, contentHeight };

    const float canvasX = layout.widgetPalette.x + layout.widgetPalette.width + kGap;
    const float canvasRight = layout.propertyInspector.x - kGap;
    layout.designerCanvas = {
        canvasX,
        contentTop,
        std::max(0.0f, canvasRight - canvasX),
        contentHeight
    };

    return layout;
}

void MainWindow::applyLayout(const WindowLayout& layout)
{
    layout_ = layout;
    widgetPalette_.setBounds(layout_.widgetPalette.x, layout_.widgetPalette.y,
        layout_.widgetPalette.width, layout_.widgetPalette.height);
    designerCanvas_.setBounds(layout_.designerCanvas.x, layout_.designerCanvas.y,
        layout_.designerCanvas.width, layout_.designerCanvas.height);
    propertyInspector_.setBounds(layout_.propertyInspector.x, layout_.propertyInspector.y,
        layout_.propertyInspector.width, layout_.propertyInspector.height);
    projectTree_.setBounds(layout_.projectTree.x, layout_.projectTree.y,
        layout_.projectTree.width, layout_.projectTree.height);
    updatePropertyEditorBounds();
}

void MainWindow::drawToolbar(visage::Canvas& canvas) const
{
    if (!layout_.toolbar.isVisible()) {
        return;
    }

    canvas.setColor(0xff2a2f39);
    canvas.fill(layout_.toolbar.x, layout_.toolbar.y, layout_.toolbar.width, layout_.toolbar.height);

    canvas.setColor(0xff14161b);
    canvas.fill(layout_.toolbar.x, layout_.toolbar.y + layout_.toolbar.height - 1.0f, layout_.toolbar.width, 1.0f);

    if (!canDrawText()) {
        return;
    }

    for (const auto& button : toolbarButtons()) {
        canvas.setColor(button.accent ? 0xff355382 : 0xff39414e);
        canvas.fill(button.bounds.x, button.bounds.y, button.bounds.width, button.bounds.height);
        canvas.setColor(0xff14161b);
        canvas.fill(button.bounds.x, button.bounds.y + button.bounds.height - 1.0f, button.bounds.width, 1.0f);
        canvas.setColor(0xfff3f5f8);
        canvas.text(button.label, labelFont_, visage::Font::kCenter,
            button.bounds.x, button.bounds.y, button.bounds.width, button.bounds.height);
    }
}

void MainWindow::drawStatusBar(visage::Canvas& canvas) const
{
    if (!layout_.statusBar.isVisible()) {
        return;
    }

    canvas.setColor(0xff2a2f39);
    canvas.fill(layout_.statusBar.x, layout_.statusBar.y, layout_.statusBar.width, layout_.statusBar.height);

    canvas.setColor(0xff14161b);
    canvas.fill(layout_.statusBar.x, layout_.statusBar.y, layout_.statusBar.width, 1.0f);

    if (!canDrawText()) {
        return;
    }

    canvas.setColor(0xfff2f4f8);
    canvas.text(statusText(), labelFont_, visage::Font::kTopLeft,
        layout_.statusBar.x + kPadding, layout_.statusBar.y + 4.0f,
        layout_.statusBar.width - kPadding * 2.0f, layout_.statusBar.height - 6.0f);
}

void MainWindow::selectWidget(const std::string& widgetId)
{
    statusMessage_.clear();
    cancelInspectorEdit();
    document_.selectWidget(widgetId);
    redraw();
}

std::string MainWindow::statusText() const
{
    if (!statusMessage_.empty()) {
        return statusMessage_;
    }

    const auto* selectedWidget = document_.selectedWidget();
    if (selectedWidget == nullptr) {
        return "Status: Ready";
    }

    const std::string displayName = selectedWidget->name.empty() ? selectedWidget->id : selectedWidget->name;
    return "Selected: " + displayName + " (" + selectedWidget->id + ")";
}

void MainWindow::setOperationStatus(std::string message)
{
    statusMessage_ = std::move(message);
}

MainWindow::ToolbarAction MainWindow::toolbarActionAt(float x, float y) const
{
    for (const auto& button : toolbarButtons()) {
        if (x >= button.bounds.x && x <= button.bounds.x + button.bounds.width
            && y >= button.bounds.y && y <= button.bounds.y + button.bounds.height) {
            return button.action;
        }
    }

    return ToolbarAction::None;
}

std::vector<MainWindow::ToolbarButton> MainWindow::toolbarButtons() const
{
    std::vector<ToolbarButton> buttons;
    if (!layout_.toolbar.isVisible()) {
        return buttons;
    }

    const float top = layout_.toolbar.y + 8.0f;
    float left = layout_.toolbar.x + kPadding;
    const auto addButton = [&](ToolbarAction action, std::string label, bool accent = false) {
        buttons.push_back(ToolbarButton{ action, std::move(label), { left, top, kToolbarButtonWidth, kToolbarButtonHeight }, accent });
        left += kToolbarButtonWidth + kToolbarButtonSpacing;
    };

    addButton(ToolbarAction::NewProject, "New");
    addButton(ToolbarAction::OpenProject, "Open");
    addButton(ToolbarAction::SaveProject, "Save");
    addButton(ToolbarAction::SaveProjectAsDialog, "Save As", true);
    addButton(ToolbarAction::OpenSample, "Sample");
    addButton(ToolbarAction::SaveProjectAsDebug, "Debug Save");
    addButton(ToolbarAction::ExportCode, "Export");
    addButton(ToolbarAction::DeleteWidget, "Delete");
    addButton(ToolbarAction::DuplicateWidget, "Duplicate");
    addButton(ToolbarAction::UndoAction, "Undo");
    addButton(ToolbarAction::RedoAction, "Redo");

    return buttons;
}

bool MainWindow::isTemplateExamplePath(const std::filesystem::path& path) const
{
    if (path.empty()) {
        return false;
    }

    const std::filesystem::path root = projectRootPath();
    const std::filesystem::path examplesRoot = (root / "templates" / "examples").lexically_normal();
    const std::filesystem::path absolutePath = (path.is_absolute() ? path : root / path).lexically_normal();
    const std::filesystem::path relativePath = absolutePath.lexically_relative(examplesRoot);
    if (relativePath.empty()) {
        return false;
    }

    const std::string relativeText = relativePath.generic_string();
    return relativeText == "." || !relativeText.starts_with("..");
}

std::filesystem::path MainWindow::projectRootPath() const
{
    std::filesystem::path current = std::filesystem::current_path();
    while (!current.empty()) {
        if (std::filesystem::exists(current / "CMakeLists.txt")) {
            return current;
        }

        const auto parent = current.parent_path();
        if (parent == current) {
            break;
        }

        current = parent;
    }

    return std::filesystem::current_path();
}

std::filesystem::path MainWindow::sampleProjectPath() const
{
    return projectRootPath() / "templates" / "examples" / "BasicWindow.vfb.json";
}

std::filesystem::path MainWindow::defaultDebugSavePath() const
{
    return projectRootPath() / "Generated" / "debug_saved_project.vfb.json";
}

void MainWindow::addRecentFile(const std::filesystem::path& path)
{
    std::string errorMessage;
    if (!recentFiles_.addPath(path, errorMessage)) {
        return;
    }
    projectTree_.setRecentFiles(recentFiles_.paths());
}

void MainWindow::removeRecentFile(const std::filesystem::path& path)
{
    std::string errorMessage;
    if (!recentFiles_.removePath(path, errorMessage)) {
        return;
    }
    projectTree_.setRecentFiles(recentFiles_.paths());
}

bool MainWindow::openRecentFile(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path)) {
        removeRecentFile(path);
        setOperationStatus("Recent file is missing: " + normalizedPathText(path));
        redraw();
        return false;
    }

    return loadProjectFromPath(path);
}

void MainWindow::loadRecentFiles()
{
    std::string errorMessage;
    if (!recentFiles_.load(errorMessage)) {
        return;
    }
    projectTree_.setRecentFiles(recentFiles_.paths());
}

std::string MainWindow::trimWhitespace(const std::string& value)
{
    const auto first = std::find_if_not(value.begin(), value.end(),
        [](unsigned char character) { return std::isspace(character) != 0; });
    if (first == value.end()) {
        return {};
    }

    const auto last = std::find_if_not(value.rbegin(), value.rend(),
        [](unsigned char character) { return std::isspace(character) != 0; }).base();
    return std::string(first, last);
}

bool MainWindow::beginInspectorEdit(const PropertyInspector::PropertyRow& row)
{
    if (row.editKind == PropertyInspector::PropertyEditKind::ReadOnly
        || row.editKind == PropertyInspector::PropertyEditKind::Bool) {
        return false;
    }

    if (!propertyInspector_.beginEditing(document_.selectedWidget(), row.key)) {
        return false;
    }

    propertyEditor_.setText(row.displayValue);
    updatePropertyEditorBounds();
    propertyEditor_.setVisible(true);
    propertyEditor_.selectAll();
    propertyEditor_.requestKeyboardFocus();
    redraw();
    return true;
}

bool MainWindow::commitInspectorEdit()
{
    if (!propertyInspector_.isEditing()) {
        return true;
    }

    const auto pendingEdit = propertyInspector_.buildPendingEdit(propertyEditor_.text().toUtf8());
    if (!pendingEdit.has_value()) {
        cancelInspectorEdit();
        return true;
    }

    if (!setSelectedWidgetPropertyFromString(pendingEdit->key, pendingEdit->valueText)) {
        propertyEditor_.requestKeyboardFocus();
        return false;
    }

    propertyInspector_.clearEditing();
    propertyEditor_.setVisible(false);
    requestKeyboardFocus();
    redraw();
    return true;
}

void MainWindow::cancelInspectorEdit()
{
    propertyInspector_.cancelEditing();
    propertyEditor_.setVisible(false);
    requestKeyboardFocus();
    redraw();
}

void MainWindow::updatePropertyEditorBounds()
{
    if (!propertyInspector_.isEditing()) {
        propertyEditor_.setVisible(false);
        return;
    }

    const auto bounds = propertyInspector_.activeEditorBounds(document_.selectedWidget());
    if (!bounds.has_value()) {
        propertyEditor_.setVisible(false);
        return;
    }

    propertyEditor_.setBounds(bounds->x, bounds->y, bounds->width, bounds->height);
}

void MainWindow::clearCanvasInteraction()
{
    canvasInteraction_ = {};
}

bool MainWindow::canDrawText() const
{
    return labelFont_.packedFont() != nullptr;
}

} // namespace visiform::ui
