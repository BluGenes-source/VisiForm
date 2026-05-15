#include "ui/MainWindow.h"

#include "ui/MainWindow.h"

#include "serialization/JsonProjectReader.h"
#include "serialization/JsonProjectWriter.h"
#include "utils/FileUtils.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
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
constexpr float kToolbarButtonWidth = 120.0f;
constexpr float kToolbarButtonHeight = 26.0f;

std::string normalizedPathText(const std::filesystem::path& path)
{
    return utils::FileUtils::normalizeSeparators(path.string());
}

} // namespace

MainWindow::MainWindow()
{
    setTitle(kWindowTitle);
    loadLabelFont();
    updateLayout();
}

bool MainWindow::newProject()
{
    document_ = model::ProjectDocument::createDefault();
    currentProjectPath_.clear();
    document_.clearDirty();
    setOperationStatus("New project created");
    redraw();
    return true;
}

bool MainWindow::saveProject()
{
    const std::filesystem::path savePath = currentProjectPath_.empty() ? defaultDebugSavePath() : currentProjectPath_;
    return saveProjectAs(savePath);
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
    setOperationStatus("Project saved: " + normalizedPathText(currentProjectPath_));
    redraw();
    return true;
}

bool MainWindow::loadProjectFromPath(const std::filesystem::path& path)
{
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
    document_.clearDirty();
    setOperationStatus("Project loaded: " + normalizedPathText(currentProjectPath_));
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
        projectTree_.draw(canvas, labelFont_, canDrawText(), document_);
    }
    drawStatusBar(canvas);
}

void MainWindow::mouseDown(const visage::MouseEvent& e)
{
    if (!e.isLeftButton()) {
        return;
    }

    switch (toolbarActionAt(e.position.x, e.position.y)) {
    case ToolbarAction::NewProject:
        newProject();
        return;
    case ToolbarAction::OpenSample:
        loadProjectFromPath(sampleProjectPath());
        return;
    case ToolbarAction::SaveProject:
        saveProject();
        return;
    case ToolbarAction::SaveProjectAsDebug:
        saveProjectAs(defaultDebugSavePath());
        return;
    case ToolbarAction::None:
        break;
    }

    if (layout_.showProjectTree) {
        if (const auto widgetId = projectTree_.hitTestWidgetId(document_, e.position.x, e.position.y)) {
            selectWidget(*widgetId);
            return;
        }
    }

    if (const auto widgetId = designerCanvas_.hitTestWidgetId(document_, e.position.x, e.position.y)) {
        selectWidget(*widgetId);
    }
}

bool MainWindow::keyPress(const visage::KeyEvent& e)
{
    if (!e.isCtrlDown()) {
        return false;
    }

    using KeyCode = visage::KeyCode;
    if (e.isShiftDown() && e.keyCode() == KeyCode::S) {
        return saveProjectAs(defaultDebugSavePath());
    }

    if (e.keyCode() == KeyCode::N) {
        return newProject();
    }
    if (e.keyCode() == KeyCode::O) {
        return loadProjectFromPath(sampleProjectPath());
    }
    if (e.keyCode() == KeyCode::S) {
        return saveProject();
    }

    return false;
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

    const auto drawToolbarButton = [&](float x, const char* label, bool accent) {
        const float y = layout_.toolbar.y + 8.0f;
        canvas.setColor(accent ? 0xff355382 : 0xff39414e);
        canvas.fill(x, y, kToolbarButtonWidth, kToolbarButtonHeight);
        canvas.setColor(0xff14161b);
        canvas.fill(x, y + kToolbarButtonHeight - 1.0f, kToolbarButtonWidth, 1.0f);
        canvas.setColor(0xfff3f5f8);
        canvas.text(label, labelFont_, visage::Font::kCenter, x, y, kToolbarButtonWidth, kToolbarButtonHeight);
    };

    float buttonX = layout_.toolbar.x + kPadding;
    drawToolbarButton(buttonX, "New", false);
    buttonX += kToolbarButtonWidth + 8.0f;
    drawToolbarButton(buttonX, "Open Sample", false);
    buttonX += kToolbarButtonWidth + 8.0f;
    drawToolbarButton(buttonX, "Save", false);
    buttonX += kToolbarButtonWidth + 8.0f;
    drawToolbarButton(buttonX, "Save As Debug", true);

    canvas.setColor(0xfff3f5f8);
    canvas.text("Ctrl+N  New    Ctrl+O  Open Sample    Ctrl+S  Save    Ctrl+Shift+S  Save As Debug",
        labelFont_, visage::Font::kTopRight,
        layout_.toolbar.x + layout_.toolbar.width * 0.45f, layout_.toolbar.y + 6.0f,
        layout_.toolbar.width * 0.53f - kPadding, layout_.toolbar.height - 10.0f);
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
    if (!layout_.toolbar.isVisible()) {
        return ToolbarAction::None;
    }

    const float top = layout_.toolbar.y + 8.0f;
    const float bottom = top + kToolbarButtonHeight;
    if (y < top || y > bottom) {
        return ToolbarAction::None;
    }

    float left = layout_.toolbar.x + kPadding;
    const auto hitButton = [&](float buttonLeft) {
        return x >= buttonLeft && x <= buttonLeft + kToolbarButtonWidth;
    };

    if (hitButton(left)) {
        return ToolbarAction::NewProject;
    }
    left += kToolbarButtonWidth + 8.0f;
    if (hitButton(left)) {
        return ToolbarAction::OpenSample;
    }
    left += kToolbarButtonWidth + 8.0f;
    if (hitButton(left)) {
        return ToolbarAction::SaveProject;
    }
    left += kToolbarButtonWidth + 8.0f;
    if (hitButton(left)) {
        return ToolbarAction::SaveProjectAsDebug;
    }

    return ToolbarAction::None;
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

bool MainWindow::canDrawText() const
{
    return labelFont_.packedFont() != nullptr;
}

} // namespace visiform::ui
