#include "ui/MainWindow.h"

#include "ui/MainWindow.h"

#include "commands/Command.h"
#include "model/LookAndFeelRegistry.h"
#include "model/WidgetRegistry.h"
#include "serialization/JsonProjectReader.h"
#include "serialization/JsonProjectWriter.h"
#include "ui/WidgetMetrics.h"
#include "utils/AppSettings.h"
#include "utils/CppIdentifier.h"
#include "utils/FileUtils.h"
#include "utils/NativeFileDialogs.h"
#include "validation/ProjectValidator.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

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
constexpr float kToolbarButtonWidth = 42.0f;
constexpr float kToolbarButtonHeight = 26.0f;
constexpr float kToolbarButtonSpacing = 2.0f;
constexpr float kNewWidgetStartX = 40.0f;
constexpr float kNewWidgetStartY = 40.0f;
constexpr float kNewWidgetSpacing = 12.0f;
constexpr float kLayoutMargin = 20.0f;
constexpr float kMarqueeDragThreshold = 4.0f;
constexpr float kSmartGuideSnapThreshold = 6.0f;

std::string normalizedPathText(const std::filesystem::path& path)
{
    return utils::FileUtils::normalizeSeparators(path.string());
}

bool isValidColorValue(const std::string& value)
{
    if (value.empty()) {
        return true;
    }
    if ((value.size() != 7 && value.size() != 9) || value.front() != '#') {
        return false;
    }
    return std::all_of(value.begin() + 1, value.end(), [](unsigned char character) {
        return std::isxdigit(character) != 0;
    });
}

std::string sanitizeExecutableName(const std::string& value, const std::string& fallback)
{
    const std::string source = value.empty() ? fallback : value;
    std::string sanitized;
    sanitized.reserve(source.size());
    for (char character : source) {
        if (std::isalnum(static_cast<unsigned char>(character)) != 0 || character == '_' || character == '-') {
            sanitized.push_back(character);
        }
        else {
            sanitized.push_back('_');
        }
    }

    if (sanitized.empty()) {
        sanitized = fallback.empty() ? std::string{"VisiFormProject"} : fallback;
    }
    if (std::isdigit(static_cast<unsigned char>(sanitized.front())) != 0) {
        sanitized.insert(sanitized.begin(), '_');
    }

    return sanitized;
}

bool isColorPropertyKey(const std::string& key)
{
    return key == "backgroundColor"
        || key == "fillColor"
        || key == "textColor"
        || key == "borderColor"
        || key == "accentColor"
        || key == "panelColor"
        || key == "controlFillColor"
        || key == "controlTextColor"
        || key == "controlBorderColor"
        || key == "disabledColor";
}

struct ValidationRunResult {
    validation::ValidationReport report{};
    std::filesystem::path reportPath{};
    bool reportWritten = false;
    std::string reportWriteError{};
};

int validationInfoCount(const validation::ValidationReport& report)
{
    return static_cast<int>(std::count_if(report.messages.begin(), report.messages.end(), [](const validation::ValidationMessage& message) {
        return message.severity == validation::ValidationSeverity::Info;
    }));
}

std::string validationReportDisplayPath(const std::filesystem::path& rootPath, const std::filesystem::path& reportPath)
{
    const std::filesystem::path relativePath = reportPath.lexically_relative(rootPath);
    if (!relativePath.empty()) {
        return normalizedPathText(relativePath);
    }

    return normalizedPathText(reportPath);
}

std::string buildValidationReportMarkdown(const validation::ValidationReport& report)
{
    const auto appendSection = [&report](std::ostringstream& stream, validation::ValidationSeverity severity, const char* heading) {
        stream << "## " << heading << "\n";

        bool foundAny = false;
        for (const auto& message : report.messages) {
            if (message.severity != severity) {
                continue;
            }

            foundAny = true;
            stream << "- [" << message.code << "] " << message.message << "\n";
            if (!message.widgetId.empty()) {
                stream << "  Widget: " << message.widgetId << "\n";
            }
            if (!message.propertyKey.empty()) {
                stream << "  Property: " << message.propertyKey << "\n";
            }
        }

        if (!foundAny) {
            stream << "- None.\n";
        }

        stream << "\n";
    };

    std::ostringstream stream;
    stream << "# VisiForm Validation Report\n\n";
    stream << "Summary:\n";
    stream << "- Errors: " << report.errorCount() << "\n";
    stream << "- Warnings: " << report.warningCount() << "\n";
    const int infoCount = validationInfoCount(report);
    if (infoCount > 0) {
        stream << "- Info: " << infoCount << "\n";
    }
    stream << "\n";

    appendSection(stream, validation::ValidationSeverity::Error, "Errors");
    appendSection(stream, validation::ValidationSeverity::Warning, "Warnings");
    if (infoCount > 0) {
        appendSection(stream, validation::ValidationSeverity::Info, "Info");
    }

    return stream.str();
}

ValidationRunResult runProjectValidation(const model::ProjectDocument& document,
    const utils::AppSettings& settings,
    const std::filesystem::path& reportPath)
{
    validation::ProjectValidator validator;
    ValidationRunResult result;
    result.report = validator.validate(document, settings);
    result.reportPath = reportPath;

    std::string errorMessage;
    if (!reportPath.parent_path().empty() && !utils::FileUtils::ensureDirectoryExists(reportPath.parent_path(), errorMessage)) {
        result.reportWriteError = errorMessage;
        return result;
    }

    if (!utils::FileUtils::writeTextFile(reportPath, buildValidationReportMarkdown(result.report), errorMessage)) {
        result.reportWriteError = errorMessage;
        return result;
    }

    result.reportWritten = true;
    return result;
}

bool isWidgetColorProperty(const model::WidgetNode& widget, const std::string& key)
{
    if (isColorPropertyKey(key)) {
        return true;
    }

    if (const auto* definition = model::WidgetRegistry::instance().find(widget.type)) {
        const auto iterator = std::find_if(definition->properties.begin(), definition->properties.end(), [&key](const model::WidgetPropertyDefinition& property) {
            return property.key == key && property.editKind == model::PropertyEditKind::Color;
        });
        return iterator != definition->properties.end();
    }

    return false;
}

bool isStyleFloatProperty(const std::string& key)
{
    return key == "borderThickness" || key == "cornerRadius" || key == "fontSize";
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
    case model::WidgetType::ColorPicker:
        return "colorPicker" + suffix;
    case model::WidgetType::Image:
        return "image" + suffix;
    case model::WidgetType::Spacer:
        return "spacer" + suffix;
    case model::WidgetType::FormWindow:
        return "form" + suffix;
    }

    return id;
}

std::string widgetDisplayName(const model::WidgetNode& widget)
{
    return widget.name.empty() ? widget.id : widget.name;
}

float snapToCanvasGrid(const DesignerCanvas& designerCanvas, float value);

std::vector<model::WidgetNode*> selectedNonRootWidgets(model::ProjectDocument& document)
{
    std::vector<model::WidgetNode*> widgets;
    for (auto* widget : document.selectedWidgets()) {
        if (widget != nullptr && !document.isRootWidgetId(widget->id)) {
            widgets.push_back(widget);
        }
    }
    return widgets;
}

struct SelectionBoundsInfo {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
};

SelectionBoundsInfo calculateSelectionBounds(const std::vector<model::WidgetNode*>& widgets)
{
    SelectionBoundsInfo bounds{};
    if (widgets.empty()) {
        return bounds;
    }

    bounds.left = widgets.front()->bounds.x;
    bounds.top = widgets.front()->bounds.y;
    bounds.right = widgets.front()->bounds.x + widgets.front()->bounds.width;
    bounds.bottom = widgets.front()->bounds.y + widgets.front()->bounds.height;
    for (const auto* widget : widgets) {
        bounds.left = std::min(bounds.left, widget->bounds.x);
        bounds.top = std::min(bounds.top, widget->bounds.y);
        bounds.right = std::max(bounds.right, widget->bounds.x + widget->bounds.width);
        bounds.bottom = std::max(bounds.bottom, widget->bounds.y + widget->bounds.height);
    }

    return bounds;
}

SelectionBoundsInfo calculateSelectionBounds(const std::vector<model::Rect>& widgets)
{
    SelectionBoundsInfo bounds{};
    if (widgets.empty()) {
        return bounds;
    }

    bounds.left = widgets.front().x;
    bounds.top = widgets.front().y;
    bounds.right = widgets.front().x + widgets.front().width;
    bounds.bottom = widgets.front().y + widgets.front().height;
    for (const auto& widget : widgets) {
        bounds.left = std::min(bounds.left, widget.x);
        bounds.top = std::min(bounds.top, widget.y);
        bounds.right = std::max(bounds.right, widget.x + widget.width);
        bounds.bottom = std::max(bounds.bottom, widget.y + widget.height);
    }

    return bounds;
}

struct AxisGuideMatch {
    float snappedPosition = 0.0f;
    float distance = std::numeric_limits<float>::max();
    DesignerCanvas::SmartGuide guide{};
    bool matched = false;
};

struct MoveSnapResult {
    float dx = 0.0f;
    float dy = 0.0f;
    bool usedGuideX = false;
    bool usedGuideY = false;
    std::vector<DesignerCanvas::SmartGuide> guides{};
};

struct WidgetAlignmentTargets {
    float left = 0.0f;
    float right = 0.0f;
    float centerX = 0.0f;
    float top = 0.0f;
    float bottom = 0.0f;
    float centerY = 0.0f;
};

void considerGuideMatch(AxisGuideMatch& bestMatch,
    float currentPosition,
    float targetPosition,
    DesignerCanvas::GuideOrientation orientation,
    const char* reason)
{
    const float distance = std::fabs(targetPosition - currentPosition);
    if (distance > kSmartGuideSnapThreshold || distance >= bestMatch.distance) {
        return;
    }

    bestMatch.matched = true;
    bestMatch.distance = distance;
    bestMatch.snappedPosition = targetPosition;
    bestMatch.guide = { orientation, targetPosition, reason };
}

WidgetAlignmentTargets alignmentTargetsForRect(const model::Rect& rect)
{
    return {
        rect.x,
        rect.x + rect.width,
        rect.x + rect.width * 0.5f,
        rect.y,
        rect.y + rect.height,
        rect.y + rect.height * 0.5f
    };
}

void collectAlignmentTargetRects(const model::WidgetNode& widget,
    float parentX,
    float parentY,
    const model::ProjectDocument& document,
    std::vector<model::Rect>& rects)
{
    const float absoluteX = parentX + widget.bounds.x;
    const float absoluteY = parentY + widget.bounds.y;
    const model::Rect absoluteBounds{ absoluteX, absoluteY, widget.bounds.width, widget.bounds.height };

    if (!document.isSelected(widget.id) && widget.type != model::WidgetType::FormWindow) {
        rects.push_back(absoluteBounds);
    }

    for (const auto& child : widget.children) {
        collectAlignmentTargetRects(child, absoluteX, absoluteY, document, rects);
    }
}

MoveSnapResult applyMoveSnapping(const DesignerCanvas& designerCanvas,
    const model::ProjectDocument& document,
    const model::Rect& movingBounds,
    float rawDx,
    float rawDy,
    bool smartGuidesEnabled)
{
    MoveSnapResult result{ rawDx, rawDy, false, false, {} };
    const model::Rect movedBounds{ movingBounds.x + rawDx, movingBounds.y + rawDy, movingBounds.width, movingBounds.height };

    AxisGuideMatch bestVertical{};
    AxisGuideMatch bestHorizontal{};
    if (smartGuidesEnabled) {
        const WidgetAlignmentTargets movingTargets = alignmentTargetsForRect(movedBounds);
        const model::Rect rootRect{ 0.0f, 0.0f, document.root.bounds.width, document.root.bounds.height };

        const auto considerRect = [&](const model::Rect& rect) {
            const WidgetAlignmentTargets targets = alignmentTargetsForRect(rect);
            considerGuideMatch(bestVertical, movingTargets.left, targets.left, DesignerCanvas::GuideOrientation::Vertical, "left");
            considerGuideMatch(bestVertical, movingTargets.right, targets.right, DesignerCanvas::GuideOrientation::Vertical, "right");
            considerGuideMatch(bestVertical, movingTargets.centerX, targets.centerX, DesignerCanvas::GuideOrientation::Vertical, "center-x");
            considerGuideMatch(bestHorizontal, movingTargets.top, targets.top, DesignerCanvas::GuideOrientation::Horizontal, "top");
            considerGuideMatch(bestHorizontal, movingTargets.bottom, targets.bottom, DesignerCanvas::GuideOrientation::Horizontal, "bottom");
            considerGuideMatch(bestHorizontal, movingTargets.centerY, targets.centerY, DesignerCanvas::GuideOrientation::Horizontal, "center-y");
        };

        considerRect(rootRect);
        considerGuideMatch(bestVertical, movingTargets.left, kLayoutMargin, DesignerCanvas::GuideOrientation::Vertical, "margin-left");
        considerGuideMatch(bestVertical, movingTargets.right, document.root.bounds.width - kLayoutMargin, DesignerCanvas::GuideOrientation::Vertical, "margin-right");
        considerGuideMatch(bestVertical, movingTargets.centerX, document.root.bounds.width * 0.5f, DesignerCanvas::GuideOrientation::Vertical, "root-center-x");
        considerGuideMatch(bestHorizontal, movingTargets.top, kLayoutMargin, DesignerCanvas::GuideOrientation::Horizontal, "margin-top");
        considerGuideMatch(bestHorizontal, movingTargets.bottom, document.root.bounds.height - kLayoutMargin, DesignerCanvas::GuideOrientation::Horizontal, "margin-bottom");
        considerGuideMatch(bestHorizontal, movingTargets.centerY, document.root.bounds.height * 0.5f, DesignerCanvas::GuideOrientation::Horizontal, "root-center-y");

        std::vector<model::Rect> widgetRects;
        collectAlignmentTargetRects(document.root, 0.0f, 0.0f, document, widgetRects);
        for (const auto& rect : widgetRects) {
            considerRect(rect);
        }

        if (bestVertical.matched) {
            const WidgetAlignmentTargets originalTargets = alignmentTargetsForRect(movingBounds);
            if (bestVertical.guide.reason == "right" || bestVertical.guide.reason == "margin-right") {
                result.dx = bestVertical.snappedPosition - originalTargets.right;
            }
            else if (bestVertical.guide.reason == "center-x" || bestVertical.guide.reason == "root-center-x") {
                result.dx = bestVertical.snappedPosition - originalTargets.centerX;
            }
            else {
                result.dx = bestVertical.snappedPosition - originalTargets.left;
            }
            result.usedGuideX = true;
            result.guides.push_back(bestVertical.guide);
        }

        if (bestHorizontal.matched) {
            const WidgetAlignmentTargets originalTargets = alignmentTargetsForRect(movingBounds);
            if (bestHorizontal.guide.reason == "bottom" || bestHorizontal.guide.reason == "margin-bottom") {
                result.dy = bestHorizontal.snappedPosition - originalTargets.bottom;
            }
            else if (bestHorizontal.guide.reason == "center-y" || bestHorizontal.guide.reason == "root-center-y") {
                result.dy = bestHorizontal.snappedPosition - originalTargets.centerY;
            }
            else {
                result.dy = bestHorizontal.snappedPosition - originalTargets.top;
            }
            result.usedGuideY = true;
            result.guides.push_back(bestHorizontal.guide);
        }
    }

    if (designerCanvas.snapToGrid()) {
        if (!result.usedGuideX) {
            result.dx = snapToCanvasGrid(designerCanvas, movingBounds.x + rawDx) - movingBounds.x;
        }
        if (!result.usedGuideY) {
            result.dy = snapToCanvasGrid(designerCanvas, movingBounds.y + rawDy) - movingBounds.y;
        }
    }

    return result;
}

float snapToCanvasGrid(const DesignerCanvas& designerCanvas, float value)
{
    if (!designerCanvas.snapToGrid()) {
        return value;
    }

    const float grid = static_cast<float>(std::max(1, designerCanvas.gridSize()));
    return std::round(value / grid) * grid;
}

DesignerCanvas::SelectionRect normalizedSelectionRect(const DesignerCanvas::FormPoint& start, const DesignerCanvas::FormPoint& end)
{
    const float left = std::min(start.x, end.x);
    const float top = std::min(start.y, end.y);
    return { left, top, std::fabs(end.x - start.x), std::fabs(end.y - start.y) };
}

bool rectsIntersect(const model::Rect& widgetBounds, const DesignerCanvas::SelectionRect& selectionRect)
{
    const float widgetRight = widgetBounds.x + widgetBounds.width;
    const float widgetBottom = widgetBounds.y + widgetBounds.height;
    const float selectionRight = selectionRect.x + selectionRect.width;
    const float selectionBottom = selectionRect.y + selectionRect.height;

    return widgetBounds.x <= selectionRight && widgetRight >= selectionRect.x
        && widgetBounds.y <= selectionBottom && widgetBottom >= selectionRect.y;
}

void collectIntersectingWidgetIds(const model::WidgetNode& widget,
    float parentX,
    float parentY,
    const DesignerCanvas::SelectionRect& selectionRect,
    std::vector<std::string>& widgetIds)
{
    const float absoluteX = parentX + widget.bounds.x;
    const float absoluteY = parentY + widget.bounds.y;
    const model::Rect absoluteBounds{ absoluteX, absoluteY, widget.bounds.width, widget.bounds.height };

    if (widget.type != model::WidgetType::FormWindow && rectsIntersect(absoluteBounds, selectionRect)) {
        widgetIds.push_back(widget.id);
    }

    for (const auto& child : widget.children) {
        collectIntersectingWidgetIds(child, absoluteX, absoluteY, selectionRect, widgetIds);
    }
}

bool isMeaningfulMarquee(const DesignerCanvas::SelectionRect& rect)
{
    return rect.width >= kMarqueeDragThreshold || rect.height >= kMarqueeDragThreshold;
}

std::vector<std::string> topLevelSelectedNonRootIds(const model::ProjectDocument& document)
{
    std::vector<std::string> result;
    for (const auto& id : document.selectedWidgetIds()) {
        if (document.isRootWidgetId(id)) {
            continue;
        }

        bool parentSelected = false;
        const model::WidgetNode* parent = document.findParentOf(id);
        while (parent != nullptr) {
            if (document.isSelected(parent->id)) {
                parentSelected = true;
                break;
            }
            parent = document.findParentOf(parent->id);
        }

        if (!parentSelected) {
            result.push_back(id);
        }
    }
    return result;
}

void assignNewIdsRecursive(model::WidgetNode& widget,
    model::ProjectDocument& document,
    utils::IdGenerator& idGenerator,
    std::set<std::string>& generatedIds)
{
    std::string newId;
    do {
        newId = idGenerator.next(widget.type, document);
    } while (generatedIds.contains(newId));

    generatedIds.insert(newId);
    widget.id = newId;
    for (auto& child : widget.children) {
        assignNewIdsRecursive(child, document, idGenerator, generatedIds);
    }
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
    loadAppSettings();
    applyCanvasSettings();
    updateLayout();
}

bool MainWindow::newProject()
{
    if (!confirmSaveIfDirty()) {
        return false;
    }

    cancelInspectorEdit();
    document_ = model::ProjectDocument::createDefault();
    normalizeWidgetBoundsForEditor();
    currentProjectPath_.clear();
    undoRedo_.clear();
    document_.clearDirty();
    setOperationStatus("New project created");
    redraw();
    return true;
}

bool MainWindow::openProjectDialog()
{
    if (!confirmSaveIfDirty()) {
        return false;
    }

    const std::filesystem::path defaultProjectDir = projectRootPath() / "Generated" / "Projects";
    const std::filesystem::path initialProjectDir = !settings_.lastProjectDirectory.empty() && std::filesystem::exists(settings_.lastProjectDirectory)
        ? settings_.lastProjectDirectory
        : defaultProjectDir;
    const auto selectedPath = utils::showOpenProjectDialog(initialProjectDir);
    if (!selectedPath.has_value()) {
        setOperationStatus("Open cancelled");
        redraw();
        return false;
    }

    return loadProjectFromPath(*selectedPath);
}

bool MainWindow::exportGeneratedCode()
{
    const ValidationRunResult validationResult = runProjectValidation(document_, settings_, projectRootPath() / "Generated" / "validation_report.md");
    const std::string validationReportPathText = validationReportDisplayPath(projectRootPath(), validationResult.reportPath);
    if (validationResult.report.hasErrors()) {
        std::string status = "Export blocked: " + std::to_string(validationResult.report.errorCount()) + " validation errors.";
        if (validationResult.report.warningCount() > 0) {
            status += " Warnings: " + std::to_string(validationResult.report.warningCount()) + ".";
        }
        if (validationResult.reportWritten) {
            status += " See " + validationReportPathText;
        }
        else if (!validationResult.reportWriteError.empty()) {
            status += " Validation report write failed: " + validationResult.reportWriteError;
        }
        setOperationStatus(status);
        redraw();
        return false;
    }

    // Prefer to prompt the user for an export folder. Use lastExportDirectory as initial folder.
    const std::filesystem::path initial = !settings_.lastExportDirectory.empty() ? settings_.lastExportDirectory : defaultExportPath();
    const auto selected = utils::showSelectExportFolderDialog(initial);
    if (!selected.has_value()) {
        setOperationStatus("Export cancelled");
        redraw();
        return false;
    }

    // Run generator with progress callback
    exportInProgress_ = true;
    exportProgressPercent_ = 0;
    exportProgressText_.clear();
    redraw();

    std::string errorMessage;
    generator::CodeGenerator generator;
    const auto progressCallback = [this](int percent, const std::string& message) {
        exportInProgress_ = true;
        exportProgressPercent_ = percent;
        exportProgressText_ = message;
        setOperationStatus("Export: " + message);
        redraw();
    };

    const bool ok = generator.generateProject(document_, settings_, *selected, errorMessage, progressCallback);
    exportInProgress_ = false;
    if (!ok) {
        setOperationStatus("Export failed: " + errorMessage);
        redraw();
        return false;
    }

    settings_.lastExportDirectory = *selected;
    saveAppSettings();
    if (validationResult.report.warningCount() > 0) {
        std::string status = "Export completed with " + std::to_string(validationResult.report.warningCount()) + " warnings.";
        if (validationResult.reportWritten) {
            status += " See " + validationReportPathText;
        }
        else if (!validationResult.reportWriteError.empty()) {
            status += " Validation report write failed: " + validationResult.reportWriteError;
        }
        setOperationStatus(status);
    }
    else {
        const std::filesystem::path localVisagePath = settings_.localVisageSourceDirectory;
        if (!localVisagePath.empty() && std::filesystem::exists(localVisagePath / "CMakeLists.txt")) {
            std::string status = "Exported with local Visage source: " + normalizedPathText(localVisagePath);
            if (!validationResult.reportWritten && !validationResult.reportWriteError.empty()) {
                status += " (validation report write failed: " + validationResult.reportWriteError + ")";
            }
            setOperationStatus(std::move(status));
        }
        else {
            std::string status = "Exported with FetchContent Visage fallback";
            if (!validationResult.reportWritten && !validationResult.reportWriteError.empty()) {
                status += " (validation report write failed: " + validationResult.reportWriteError + ")";
            }
            setOperationStatus(std::move(status));
        }
    }
    exportProgressPercent_ = 100;
    exportProgressText_ = "Export complete";
    redraw();
    return true;
}

bool MainWindow::validateProject()
{
    const ValidationRunResult validationResult = runProjectValidation(document_, settings_, projectRootPath() / "Generated" / "validation_report.md");
    const std::string validationReportPathText = validationReportDisplayPath(projectRootPath(), validationResult.reportPath);

    std::string status;
    if (validationResult.report.hasErrors() || validationResult.report.hasWarnings()) {
        status = "Validation found "
            + std::to_string(validationResult.report.errorCount()) + " errors, "
            + std::to_string(validationResult.report.warningCount()) + " warnings.";
    }
    else {
        status = "Validation passed.";
    }

    if (validationResult.reportWritten) {
        status += " See " + validationReportPathText;
    }
    else if (!validationResult.reportWriteError.empty()) {
        status += " Validation report write failed: " + validationResult.reportWriteError;
    }

    setOperationStatus(std::move(status));
    redraw();
    return !validationResult.report.hasErrors();
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
    const std::filesystem::path defaultProjectDir = projectRootPath() / "Generated" / "Projects";
    std::string ensureDirectoryError;
    utils::FileUtils::ensureDirectoryExists(defaultProjectDir, ensureDirectoryError);
    const std::filesystem::path initialProjectDir = !settings_.lastProjectDirectory.empty() && std::filesystem::exists(settings_.lastProjectDirectory)
        ? settings_.lastProjectDirectory
        : defaultProjectDir;
    const auto selectedPath = utils::showSaveProjectDialog(suggestedPath, initialProjectDir);
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
    settings_.lastProjectDirectory = currentProjectPath_.parent_path();
    addRecentFile(currentProjectPath_);
    setOperationStatus("Project saved: " + normalizedPathText(currentProjectPath_));
    redraw();
    return true;
}

bool MainWindow::loadProjectFromPath(const std::filesystem::path& path)
{
    cancelInspectorEdit();
    serialization::JsonProjectReader reader;
    std::string errorMessage;
    auto loadedDocument = reader.readFromFile(path, errorMessage);
    if (!loadedDocument.has_value()) {
        setOperationStatus("Load failed: " + errorMessage);
        redraw();
        return false;
    }

    document_ = std::move(*loadedDocument);
    const bool boundsNormalized = normalizeWidgetBoundsForEditor();
    const bool radioNormalized = document_.normalizeRadioGroups();
    if (!document_.selectedWidgetId.empty() && document_.findWidgetById(document_.selectedWidgetId) != nullptr) {
        document_.setSelection(document_.selectedWidgetId);
    }
    else {
        document_.setSelection(document_.root.id);
    }

    currentProjectPath_ = path;
    undoRedo_.clear();
    if (boundsNormalized || radioNormalized) {
        document_.markDirty();
    }
    else {
        document_.clearDirty();
    }
    settings_.lastProjectDirectory = currentProjectPath_.parent_path();
    addRecentFile(currentProjectPath_);
    std::string loadStatus = "Project loaded: " + normalizedPathText(currentProjectPath_);
    if (boundsNormalized && radioNormalized) {
        loadStatus += " (bounds and radio groups normalized)";
    }
    else if (boundsNormalized) {
        loadStatus += " (bounds normalized for editor readability)";
    }
    else if (radioNormalized) {
        loadStatus += " (radio groups normalized)";
    }
    setOperationStatus(loadStatus);
    redraw();
    return true;
}

bool MainWindow::openSampleProject()
{
    if (!confirmSaveIfDirty()) {
        return false;
    }

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
    updateWindowTitle();
    updateLayout();

    canvas.setColor(0xff1b1d23);
    canvas.fill(0, 0, width(), height());

    if (width() <= 0.0f || height() <= 0.0f) {
        return;
    }

    std::optional<DesignerCanvas::SelectionRect> marqueeRect;
    if (canvasInteraction_.mode == CanvasInteractionState::Mode::MarqueeSelect) {
        marqueeRect = normalizedSelectionRect(canvasInteraction_.dragStart, canvasInteraction_.currentPoint);
    }

    drawToolbar(canvas);
    widgetPalette_.draw(canvas, labelFont_, canDrawText());
    designerCanvas_.draw(canvas, labelFont_, canDrawText(), document_, marqueeRect, canvasInteraction_.smartGuides);
    propertyInspector_.draw(canvas, labelFont_, canDrawText(), document_, settings_, document_.selectedWidgetIds().size());
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
    case ToolbarAction::ValidateProject:
        validateProject();
        return;
    case ToolbarAction::FitText:
        fitSelectedWidgetToText();
        return;
    case ToolbarAction::CopyWidgets:
        copySelectedWidgets();
        return;
    case ToolbarAction::PasteWidgets:
        pasteWidgets();
        return;
    case ToolbarAction::ToggleMultiSelect:
        toggleMultiSelectMode();
        return;
    case ToolbarAction::AlignLeft:
        alignSelectedLeft();
        return;
    case ToolbarAction::AlignTop:
        alignSelectedTop();
        return;
    case ToolbarAction::AlignRight:
        alignSelectedRight();
        return;
    case ToolbarAction::AlignBottom:
        alignSelectedBottom();
        return;
    case ToolbarAction::CenterHorizontally:
        centerSelectedHorizontally();
        return;
    case ToolbarAction::CenterVertically:
        centerSelectedVertically();
        return;
    case ToolbarAction::SameWidth:
        makeSelectedSameWidth();
        return;
    case ToolbarAction::SameHeight:
        makeSelectedSameHeight();
        return;
    case ToolbarAction::DistributeHorizontally:
        distributeSelectedHorizontally();
        return;
    case ToolbarAction::DistributeVertically:
        distributeSelectedVertically();
        return;
    case ToolbarAction::ToggleSmartGuides:
        toggleSmartGuides();
        return;
    case ToolbarAction::BringForward:
        bringSelectedForward();
        return;
    case ToolbarAction::SendBackward:
        sendSelectedBackward();
        return;
    case ToolbarAction::ToggleGrid:
        toggleGrid();
        return;
    case ToolbarAction::ToggleSnap:
        toggleSnapToGrid();
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

    if (e.isLeftButton() && propertyInspector_.mouseDown(document_, settings_, e.position.x, e.position.y)) {
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    if (layout_.showProjectTree && projectTree_.mouseDown(document_, e.position.x, e.position.y)) {
        redraw();
        return;
    }

    // Check callback suggestion hit-test first so suggestion clicks are applied before committing edits.
    if (const auto suggestion = propertyInspector_.hitTestSuggestion(document_, settings_, e.position.x, e.position.y)) {
        // Only apply suggestion when we are actively editing an event property.
        if (propertyInspector_.isEditing()) {
            const auto active = propertyInspector_.activeRow(document_, settings_);
            if (active.has_value()) {
                if (applySelectedWidgetCallbackProperty(active->key, *suggestion)) {
                    suggestionAppliedThisClick_ = true;
                }
                return;
            }
        }
    }

    if (const auto colorPropertyKey = propertyInspector_.hitTestColorSwatch(document_, settings_, e.position.x, e.position.y)) {
        if (propertyInspector_.isEditing() && !commitInspectorEdit()) {
            return;
        }

        const model::WidgetNode* selectedWidget = document_.selectedWidget();
        const std::string initialColor = selectedWidget != nullptr
            ? selectedWidget->getStringProperty(*colorPropertyKey, {})
            : std::string{};
        const auto selectedColor = utils::showColorPickerDialog(initialColor);
        if (!selectedColor.has_value()) {
            return;
        }

        if (setSelectedWidgetPropertyFromString(*colorPropertyKey, *selectedColor)) {
            propertyInspector_.clearEditing();
            propertyEditor_.setVisible(false);
            requestKeyboardFocus();
            updatePropertyEditorBounds();
            redraw();
        }
        return;
    }

    if (propertyInspector_.isEditing()) {
        // Clicking another row or leaving the inspector attempts to commit the current edit.
        // If validation fails, editing stays active and the click is consumed.
        if (!commitInspectorEdit()) {
            return;
        }
    }

    if (const auto row = propertyInspector_.hitTestRow(document_, settings_, e.position.x, e.position.y)) {
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
        const bool additiveSelection = multiSelectMode_
            || (GetKeyState(VK_CONTROL) & 0x8000) != 0
            || (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        if (const auto widgetId = projectTree_.hitTestWidgetId(document_, e.position.x, e.position.y)) {
            handleWidgetClicked(*widgetId, additiveSelection);
            return;
        }
        if (const auto recentFileIndex = projectTree_.hitTestRecentFileIndex(document_, e.position.x, e.position.y)) {
            openRecentFile(settings_.recentFiles[*recentFileIndex]);
            return;
        }
    }

    if (const auto widgetId = designerCanvas_.hitTestWidgetId(document_, e.position.x, e.position.y)) {
        if (*widgetId == document_.root.id) {
            if (const auto dragStart = designerCanvas_.toFormPoint(document_, e.position.x, e.position.y)) {
                clearCanvasInteraction();
                canvasInteraction_.mode = CanvasInteractionState::Mode::MarqueeSelect;
                canvasInteraction_.dragStart = *dragStart;
                canvasInteraction_.currentPoint = *dragStart;
                canvasInteraction_.changed = false;
                redraw();
            }
            return;
        }

        // TODO: Replace Win32 key-state probing if the Visage mouse input layer later exposes reliable modifier state.
        const bool modifierAdditive = (GetKeyState(VK_CONTROL) & 0x8000) != 0
            || (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        const bool additiveSelection = multiSelectMode_ || modifierAdditive;
        const bool clickedPrimarySelected = document_.selectedWidgetId == *widgetId;
        const bool keepMultiSelectionForDrag = multiSelectMode_ && clickedPrimarySelected;
        const bool wasSelected = clickedPrimarySelected;

        if (!keepMultiSelectionForDrag) {
            handleWidgetClicked(*widgetId, additiveSelection);

            if (additiveSelection) {
                clearCanvasInteraction();
                return;
            }
        }

        if (wasSelected && *widgetId != document_.root.id) {
            const auto interactionHit = designerCanvas_.hitTestInteraction(document_, e.position.x, e.position.y, document_.selectedWidgetId);
            const auto dragStart = designerCanvas_.toFormPoint(document_, e.position.x, e.position.y);
            auto* widget = document_.findWidgetById(*widgetId);
            if (interactionHit.has_value() && dragStart.has_value() && widget != nullptr) {
                canvasInteraction_.widgetId = *widgetId;
                canvasInteraction_.region = interactionHit->region;
                canvasInteraction_.originalBounds = widget->bounds;
                canvasInteraction_.dragStart = *dragStart;
                canvasInteraction_.currentPoint = *dragStart;
                canvasInteraction_.selectionBounds.clear();
                canvasInteraction_.changed = false;
                canvasInteraction_.mode = interactionHit->region == DesignerCanvas::HitRegion::Body
                    ? CanvasInteractionState::Mode::Move
                    : CanvasInteractionState::Mode::Resize;

                if (canvasInteraction_.mode == CanvasInteractionState::Mode::Move && document_.hasMultiSelection()) {
                    for (const auto& id : document_.selectedWidgetIds()) {
                        if (document_.isRootWidgetId(id)) {
                            continue;
                        }
                        if (auto* selectedWidget = document_.findWidgetById(id)) {
                            canvasInteraction_.selectionBounds.push_back({ id, selectedWidget->bounds });
                        }
                    }
                }
            }
        }
        else {
            clearCanvasInteraction();
        }

        return;
    }

    if (const auto dragStart = designerCanvas_.toFormPoint(document_, e.position.x, e.position.y)) {
        clearCanvasInteraction();
        canvasInteraction_.mode = CanvasInteractionState::Mode::MarqueeSelect;
        canvasInteraction_.dragStart = *dragStart;
        canvasInteraction_.currentPoint = *dragStart;
        canvasInteraction_.changed = false;
        redraw();
    }
}

void MainWindow::mouseMove(const visage::MouseEvent& e)
{
    if (canvasInteraction_.mode != CanvasInteractionState::Mode::None) {
        return;
    }

    updateHoverHint(e.position.x, e.position.y);
}

void MainWindow::mouseDrag(const visage::MouseEvent& e)
{
    if (propertyInspector_.mouseDrag(document_, settings_, e.position.x, e.position.y)) {
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    if (layout_.showProjectTree && projectTree_.mouseDrag(document_, e.position.x, e.position.y)) {
        redraw();
        return;
    }

    if (canvasInteraction_.mode == CanvasInteractionState::Mode::None) {
        return;
    }

    auto* widget = document_.findWidgetById(canvasInteraction_.widgetId);
    const auto currentPoint = designerCanvas_.toFormPoint(document_, e.position.x, e.position.y);
    if (canvasInteraction_.mode == CanvasInteractionState::Mode::MarqueeSelect) {
        if (currentPoint == std::nullopt) {
            return;
        }

        canvasInteraction_.currentPoint = *currentPoint;
        canvasInteraction_.changed = true;
        redraw();
        return;
    }

    if (widget == nullptr || currentPoint == std::nullopt) {
        return;
    }

    model::Rect updatedBounds = canvasInteraction_.originalBounds;
    if (canvasInteraction_.mode == CanvasInteractionState::Mode::Move) {
        const float rawDx = currentPoint->x - canvasInteraction_.dragStart.x;
        const float rawDy = currentPoint->y - canvasInteraction_.dragStart.y;
        model::Rect movingBounds = canvasInteraction_.originalBounds;
        if (canvasInteraction_.selectionBounds.size() > 1) {
            std::vector<model::Rect> originalBounds;
            originalBounds.reserve(canvasInteraction_.selectionBounds.size());
            for (const auto& snapshot : canvasInteraction_.selectionBounds) {
                originalBounds.push_back(snapshot.originalBounds);
            }
            const SelectionBoundsInfo selectionBounds = calculateSelectionBounds(originalBounds);
            movingBounds = {
                selectionBounds.left,
                selectionBounds.top,
                selectionBounds.right - selectionBounds.left,
                selectionBounds.bottom - selectionBounds.top
            };
        }
        const MoveSnapResult snapResult = applyMoveSnapping(designerCanvas_, document_, movingBounds, rawDx, rawDy, settings_.smartGuidesEnabled);
        canvasInteraction_.smartGuides = snapResult.guides;
        canvasInteraction_.smartGuideSnapUsed = snapResult.usedGuideX || snapResult.usedGuideY;

        if (canvasInteraction_.selectionBounds.size() > 1) {
            const float deltaX = snapResult.dx;
            const float deltaY = snapResult.dy;
            bool anyChanged = false;
            for (const auto& snapshot : canvasInteraction_.selectionBounds) {
                auto* selectedWidget = document_.findWidgetById(snapshot.widgetId);
                if (selectedWidget == nullptr) {
                    continue;
                }

                const model::Rect newBounds{
                    snapshot.originalBounds.x + deltaX,
                    snapshot.originalBounds.y + deltaY,
                    snapshot.originalBounds.width,
                    snapshot.originalBounds.height
                };
                if (selectedWidget->bounds.x != newBounds.x || selectedWidget->bounds.y != newBounds.y) {
                    selectedWidget->bounds = newBounds;
                    anyChanged = true;
                }
            }
            canvasInteraction_.changed = anyChanged;
            if (anyChanged) {
                redraw();
            }
            return;
        }

        updatedBounds = canvasInteraction_.originalBounds;
        updatedBounds.x = canvasInteraction_.originalBounds.x + snapResult.dx;
        updatedBounds.y = canvasInteraction_.originalBounds.y + snapResult.dy;
    }
    else if (canvasInteraction_.mode == CanvasInteractionState::Mode::Resize) {
        canvasInteraction_.smartGuides.clear();
        canvasInteraction_.smartGuideSnapUsed = false;
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
    const bool releasedInspectorScrollBar = propertyInspector_.mouseUp();
    if (releasedInspectorScrollBar && canvasInteraction_.mode == CanvasInteractionState::Mode::None) {
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    const bool releasedProjectTreeScrollBar = projectTree_.mouseUp();
    if (releasedProjectTreeScrollBar && canvasInteraction_.mode == CanvasInteractionState::Mode::None) {
        redraw();
        return;
    }

    if (!e.isLeftButton() || canvasInteraction_.mode == CanvasInteractionState::Mode::None) {
        return;
    }

    // If a suggestion was applied during mouse down, consume this mouseUp and clear guard.
    if (suggestionAppliedThisClick_) {
        suggestionAppliedThisClick_ = false;
        return;
    }

    if (canvasInteraction_.mode == CanvasInteractionState::Mode::MarqueeSelect) {
        const DesignerCanvas::SelectionRect selectionRect = normalizedSelectionRect(canvasInteraction_.dragStart, canvasInteraction_.currentPoint);
        if (!isMeaningfulMarquee(selectionRect)) {
            document_.setSelection(document_.root.id);
            setOperationStatus("Box selected 0 widgets");
        }
        else {
            std::vector<std::string> intersectingIds;
            collectIntersectingWidgetIds(document_.root, 0.0f, 0.0f, selectionRect, intersectingIds);
            if (intersectingIds.empty()) {
                document_.setSelection(document_.root.id);
                setOperationStatus("Box selected 0 widgets");
            }
            else {
                document_.clearSelection();
                for (const auto& id : intersectingIds) {
                    document_.addToSelection(id);
                }
                setOperationStatus("Box selected " + std::to_string(intersectingIds.size()) + " widgets");
            }
        }

        clearCanvasInteraction();
        redraw();
        return;
    }

    auto* widget = document_.findWidgetById(canvasInteraction_.widgetId);
    if (canvasInteraction_.changed && widget != nullptr) {
        if (canvasInteraction_.mode == CanvasInteractionState::Mode::Move && canvasInteraction_.selectionBounds.size() > 1) {
            undoRedo_.clear();
            document_.markDirty();
            setOperationStatus(canvasInteraction_.smartGuideSnapUsed
                ? "Moved " + std::to_string(canvasInteraction_.selectionBounds.size()) + " widgets with smart guide snap"
                : "Moved " + std::to_string(canvasInteraction_.selectionBounds.size()) + " widgets");
            clearCanvasInteraction();
            redraw();
            return;
        }

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
            setOperationStatus(canvasInteraction_.smartGuideSnapUsed
                ? "Moved widget: " + displayName + " (" + widget->id + ") with smart guide snap"
                : "Moved widget: " + displayName + " (" + widget->id + ")");
        }
        else {
            setOperationStatus("Resized widget: " + displayName + " (" + widget->id + ")");
        }
    }

    clearCanvasInteraction();
    redraw();
}

bool MainWindow::mouseWheel(const visage::MouseEvent& e)
{
    const float deltaY = e.precise_wheel_delta_y != 0.0f ? e.precise_wheel_delta_y : e.wheel_delta_y;
    if (layout_.showProjectTree && projectTree_.mouseWheel(document_, deltaY, e.position.x, e.position.y)) {
        redraw();
        return true;
    }

    if (propertyInspector_.mouseWheel(document_, settings_, deltaY, e.position.x, e.position.y)) {
        updatePropertyEditorBounds();
        redraw();
        return true;
    }

    return false;
}

bool MainWindow::keyPress(const visage::KeyEvent& e)
{
    using KeyCode = visage::KeyCode;
    if (propertyInspector_.isEditing()) {
        return false;
    }

    if (e.keyCode() == KeyCode::Delete) {
        deleteSelectedWidget();
        return true;
    }

    if (e.keyCode() == KeyCode::Left || e.keyCode() == KeyCode::Right
        || e.keyCode() == KeyCode::Up || e.keyCode() == KeyCode::Down) {
        const float amount = e.isShiftDown() ? static_cast<float>(std::max(1, designerCanvas_.gridSize())) : 1.0f;
        if (e.keyCode() == KeyCode::Left) {
            nudgeSelectedWidgets(-amount, 0.0f);
        }
        else if (e.keyCode() == KeyCode::Right) {
            nudgeSelectedWidgets(amount, 0.0f);
        }
        else if (e.keyCode() == KeyCode::Up) {
            nudgeSelectedWidgets(0.0f, -amount);
        }
        else {
            nudgeSelectedWidgets(0.0f, amount);
        }
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
        duplicateSelectedWidget();
        return true;
    }
    if (e.keyCode() == KeyCode::C) {
        copySelectedWidgets();
        return true;
    }
    if (e.keyCode() == KeyCode::V) {
        pasteWidgets();
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

void MainWindow::toggleMultiSelectMode()
{
    multiSelectMode_ = !multiSelectMode_;
    setOperationStatus(std::string{"Multi-select: "} + (multiSelectMode_ ? "On" : "Off"));
    redraw();
}

void MainWindow::toggleSmartGuides()
{
    settings_.smartGuidesEnabled = !settings_.smartGuidesEnabled;
    saveAppSettings();
    setOperationStatus(std::string{"Smart guides: "} + (settings_.smartGuidesEnabled ? "On" : "Off"));
    redraw();
}

bool MainWindow::isMultiSelectModeEnabled() const
{
    return multiSelectMode_;
}

void MainWindow::handleWidgetClicked(const std::string& widgetId, bool additiveSelection)
{
    auto* widget = document_.findWidgetById(widgetId);
    if (widget == nullptr) {
        return;
    }

    clearCanvasInteraction();
    if (!additiveSelection) {
        statusMessage_.clear();
        document_.setSelection(widgetId);
        redraw();
        return;
    }

    const bool wasSelected = document_.isSelected(widgetId);
    document_.toggleSelection(widgetId);
    if (document_.hasSelection()) {
        const auto* primary = document_.selectedWidget();
        const std::string displayName = primary != nullptr ? widgetDisplayName(*primary) : widgetDisplayName(*widget);
        setOperationStatus(wasSelected
            ? "Removed from selection: " + widgetDisplayName(*widget) + " (" + widgetId + ")"
            : "Added to selection: " + widgetDisplayName(*widget) + " (" + widgetId + ")");
        if (document_.hasMultiSelection() && primary != nullptr) {
            setOperationStatus("Selected: " + std::to_string(document_.selectedWidgetIds().size())
                + " widgets, primary: " + displayName + " (" + primary->id + ")");
        }
    }
    else {
        setOperationStatus("Removed from selection: " + widgetDisplayName(*widget) + " (" + widgetId + ")");
    }
    redraw();
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
    const model::Rect widgetBounds = widget.bounds;
    undoRedo_.executeCommand(std::make_unique<commands::AddWidgetCommand>(document_, document_.root.id, std::move(widget), addedId));
    normalizeWidgetBoundsForEditor();
    document_.markDirty();
    setOperationStatus("Added widget: " + addedId + " size " + std::to_string(static_cast<int>(widgetBounds.width))
        + "x" + std::to_string(static_cast<int>(widgetBounds.height)));
    redraw();
}

void MainWindow::deleteSelectedWidget()
{
    cancelInspectorEdit();
    if (document_.hasMultiSelection()) {
        std::vector<std::string> ids = document_.selectedWidgetIds();
        ids.erase(std::remove_if(ids.begin(), ids.end(), [this](const std::string& id) { return document_.isRootWidgetId(id); }), ids.end());
        std::sort(ids.begin(), ids.end(), [this](const std::string& left, const std::string& right) {
            const auto depthOf = [this](const std::string& id) {
                int depth = 0;
                const model::WidgetNode* parent = document_.findParentOf(id);
                while (parent != nullptr) {
                    ++depth;
                    parent = document_.findParentOf(parent->id);
                }
                return depth;
            };
            return depthOf(left) > depthOf(right);
        });

        int removedCount = 0;
        for (const auto& id : ids) {
            if (document_.findWidgetById(id) != nullptr && document_.removeWidgetById(id)) {
                ++removedCount;
            }
        }

        document_.setSelection(document_.root.id);
        if (removedCount > 0) {
            undoRedo_.clear();
            document_.markDirty();
            setOperationStatus("Deleted " + std::to_string(removedCount) + " widgets");
        }
        else {
            setOperationStatus("No widget selected");
        }
        redraw();
        return;
    }

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
    const bool hadMultiSelection = document_.hasMultiSelection();

    auto* duplicate = document_.duplicateWidgetById(selectedId, idGenerator_);
    if (duplicate == nullptr) {
        setOperationStatus("Duplicate failed: " + selectedId);
        redraw();
        return;
    }

    const std::string duplicateId = duplicate->id;
    const std::string displayName = duplicate->name.empty() ? duplicate->id : duplicate->name;
    document_.normalizeRadioGroups();

    // TODO: Reconnect duplicate to `AddWidgetCommand` or a dedicated duplicate command after the direct flow is verified stable.
    undoRedo_.clear();
    document_.setSelection(duplicateId);
    document_.markDirty();
    setOperationStatus(hadMultiSelection
        ? "Duplicated primary widget: " + displayName + " (" + duplicateId + ")"
        : "Duplicated widget: " + displayName + " (" + duplicateId + ")");
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
    model::WidgetNode widget = model::WidgetRegistry::instance().createDefaultWidget(type, id);
    widget.bounds = nextDefaultWidgetBounds(type);

    return widget;
}

model::Rect MainWindow::nextDefaultWidgetBounds(model::WidgetType type) const
{
    const WidgetSizeMetrics metrics = getWidgetSizeMetrics(type);
    const float width = metrics.defaultWidth;
    const float height = metrics.defaultHeight;

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

bool MainWindow::enforceMinimumBoundsRecursive(model::WidgetNode& widget)
{
    bool changed = false;
    const WidgetSizeMetrics metrics = getWidgetSizeMetrics(widget.type);
    if (widget.bounds.width < metrics.minWidth) {
        widget.bounds.width = metrics.minWidth;
        changed = true;
    }
    if (widget.bounds.height < metrics.minHeight) {
        widget.bounds.height = metrics.minHeight;
        changed = true;
    }

    for (auto& child : widget.children) {
        changed = enforceMinimumBoundsRecursive(child) || changed;
    }

    return changed;
}

bool MainWindow::normalizeWidgetBoundsForEditor()
{
    return enforceMinimumBoundsRecursive(document_.root);
}

bool MainWindow::autoSizeWidgetForTextProperty(model::WidgetNode& widget, const std::string& key, const std::string& valueText)
{
    if (!autoSizeTextWidgets_) {
        return false;
    }

    float padding = 0.0f;
    const WidgetSizeMetrics metrics = getWidgetSizeMetrics(widget.type);
    float minimumWidth = metrics.minWidth;
    float minimumHeight = metrics.minHeight;
    const float fontSize = defaultDesignerFontSize();
    const float lineHeight = estimatedLineHeight(fontSize);
    switch (widget.type) {
    case model::WidgetType::Label:
        if (key != "text") {
            return false;
        }
        padding = 40.0f;
        minimumHeight = std::max(metrics.minHeight, lineHeight + 18.0f);
        break;
    case model::WidgetType::Button:
        if (key != "text") {
            return false;
        }
        padding = 80.0f;
        minimumHeight = metrics.minHeight;
        break;
    case model::WidgetType::TextBox:
        if (key != "text") {
            return false;
        }
        padding = 70.0f;
        minimumHeight = metrics.minHeight;
        break;
    case model::WidgetType::CheckBox:
    case model::WidgetType::RadioButton:
        if (key != "text") {
            return false;
        }
        padding = 16.0f + 12.0f + 70.0f;
        minimumHeight = std::max(metrics.minHeight, std::max(18.0f + 18.0f, lineHeight + 18.0f));
        break;
    case model::WidgetType::Frame:
        if (key != "title") {
            return false;
        }
        padding = 100.0f;
        minimumHeight = metrics.minHeight;
        break;
    case model::WidgetType::Slider:
    case model::WidgetType::ScrollBar:
    case model::WidgetType::Image:
    case model::WidgetType::Spacer:
    case model::WidgetType::FormWindow:
        return false;
    }

    const float desiredWidth = std::max(minimumWidth, estimateDesignerTextWidth(valueText) + padding);
    const float desiredHeight = std::max(widget.bounds.height, minimumHeight);
    const bool changed = desiredWidth > widget.bounds.width || desiredHeight > widget.bounds.height;
    widget.bounds.width = std::max(widget.bounds.width, desiredWidth);
    widget.bounds.height = desiredHeight;
    return changed;
}

void MainWindow::fitSelectedWidgetToText()
{
    auto* widget = document_.selectedWidget();
    if (widget == nullptr) {
        setOperationStatus("No widget selected");
        redraw();
        return;
    }

    std::string key;
    std::string valueText;
    switch (widget->type) {
    case model::WidgetType::Label:
    case model::WidgetType::Button:
    case model::WidgetType::TextBox:
    case model::WidgetType::CheckBox:
    case model::WidgetType::RadioButton:
        key = "text";
        valueText = widget->getStringProperty("text", {});
        break;
    case model::WidgetType::Frame:
        key = "title";
        valueText = widget->getStringProperty("title", {});
        break;
    case model::WidgetType::Slider:
    case model::WidgetType::ScrollBar:
    case model::WidgetType::Image:
    case model::WidgetType::Spacer:
    case model::WidgetType::FormWindow:
        setOperationStatus("Fit text not supported for selected widget");
        redraw();
        return;
    }

    const float oldWidth = widget->bounds.width;
    const float oldHeight = widget->bounds.height;
    if (!autoSizeWidgetForTextProperty(*widget, key, valueText)) {
        setOperationStatus("Fit text: already large enough");
        redraw();
        return;
    }

    document_.markDirty();
    const std::string displayName = widget->name.empty() ? widget->id : widget->name;
    setOperationStatus("Fit text: " + displayName + " width " + std::to_string(static_cast<int>(oldWidth))
        + " -> " + std::to_string(static_cast<int>(widget->bounds.width))
        + ", height " + std::to_string(static_cast<int>(oldHeight))
        + " -> " + std::to_string(static_cast<int>(widget->bounds.height)));
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::copySelectedWidgets()
{
    const std::vector<std::string> widgetIds = topLevelSelectedNonRootIds(document_);
    if (widgetIds.empty()) {
        setOperationStatus("No widgets selected to copy");
        redraw();
        return;
    }

    clipboardWidgets_.clear();
    clipboardWidgets_.reserve(widgetIds.size());
    for (const auto& id : widgetIds) {
        if (const auto* widget = document_.findWidgetById(id)) {
            clipboardWidgets_.push_back(*widget);
        }
    }

    pasteCount_ = 0;
    setOperationStatus("Copied " + std::to_string(clipboardWidgets_.size()) + " widgets");
    redraw();
}

void MainWindow::pasteWidgets()
{
    if (clipboardWidgets_.empty()) {
        setOperationStatus("Clipboard is empty");
        redraw();
        return;
    }

    ++pasteCount_;
    const float offset = static_cast<float>(pasteCount_ * 20);
    std::set<std::string> generatedIds;
    std::vector<std::string> pastedIds;
    document_.clearSelection();

    for (const auto& copiedWidget : clipboardWidgets_) {
        model::WidgetNode widget = copiedWidget;
        assignNewIdsRecursive(widget, document_, idGenerator_, generatedIds);
        widget.bounds.x += offset;
        widget.bounds.y += offset;

        const std::string pastedId = widget.id;
        if (!document_.addChildToRoot(std::move(widget))) {
            continue;
        }

        pastedIds.push_back(pastedId);
        document_.addToSelection(pastedId);
    }

    if (pastedIds.empty()) {
        setOperationStatus("Paste failed");
        redraw();
        return;
    }

    normalizeWidgetBoundsForEditor();
    document_.normalizeRadioGroups();
    undoRedo_.clear();
    document_.markDirty();
    setOperationStatus("Pasted " + std::to_string(pastedIds.size()) + " widgets");
    redraw();
}

void MainWindow::alignSelectedLeft()
{
    auto selectedWidgets = document_.selectedWidgets();
    if (selectedWidgets.empty()) {
        setOperationStatus("No widget selected");
        redraw();
        return;
    }
    auto* widget = document_.selectedWidget();
    if (document_.isRootWidgetId(widget->id)) {
        setOperationStatus("Cannot layout root form");
        redraw();
        return;
    }

    if (selectedWidgets.size() > 1) {
        float targetX = selectedWidgets.front()->bounds.x;
        for (const auto* selected : selectedWidgets) {
            if (!document_.isRootWidgetId(selected->id)) {
                targetX = std::min(targetX, selected->bounds.x);
            }
        }
        targetX = snapToCanvasGrid(designerCanvas_, targetX);
        for (auto* selected : selectedWidgets) {
            if (!document_.isRootWidgetId(selected->id)) {
                selected->bounds.x = targetX;
            }
        }
        document_.markDirty();
        setOperationStatus("Aligned left: " + std::to_string(selectedWidgets.size()) + " widgets");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    widget->bounds.x = snapToCanvasGrid(designerCanvas_, kLayoutMargin);
    document_.markDirty();
    setOperationStatus("Aligned left: " + widgetDisplayName(*widget) + " (" + widget->id + ")");
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::alignSelectedTop()
{
    auto selectedWidgets = selectedNonRootWidgets(document_);
    if (selectedWidgets.empty()) {
        setOperationStatus("No widget selected");
        redraw();
        return;
    }
    auto* widget = document_.selectedWidget();
    if (document_.isRootWidgetId(widget->id)) {
        setOperationStatus("Cannot layout root form");
        redraw();
        return;
    }

    if (selectedWidgets.size() > 1) {
        float targetY = selectedWidgets.front()->bounds.y;
        for (const auto* selected : selectedWidgets) {
            if (!document_.isRootWidgetId(selected->id)) {
                targetY = std::min(targetY, selected->bounds.y);
            }
        }
        targetY = snapToCanvasGrid(designerCanvas_, targetY);
        for (auto* selected : selectedWidgets) {
            if (!document_.isRootWidgetId(selected->id)) {
                selected->bounds.y = targetY;
            }
        }
        document_.markDirty();
        setOperationStatus("Aligned top: " + std::to_string(selectedWidgets.size()) + " widgets");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    widget->bounds.y = snapToCanvasGrid(designerCanvas_, kLayoutMargin);
    document_.markDirty();
    setOperationStatus("Aligned top: " + widgetDisplayName(*widget) + " (" + widget->id + ")");
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::alignSelectedRight()
{
    auto selectedWidgets = selectedNonRootWidgets(document_);
    if (selectedWidgets.empty()) {
        setOperationStatus(document_.hasSelection() ? "Cannot layout root form" : "No widget selected");
        redraw();
        return;
    }

    if (selectedWidgets.size() > 1) {
        float targetRight = selectedWidgets.front()->bounds.x + selectedWidgets.front()->bounds.width;
        for (const auto* selected : selectedWidgets) {
            targetRight = std::max(targetRight, selected->bounds.x + selected->bounds.width);
        }
        for (auto* selected : selectedWidgets) {
            selected->bounds.x = snapToCanvasGrid(designerCanvas_, targetRight - selected->bounds.width);
        }
        document_.markDirty();
        setOperationStatus("Aligned right: " + std::to_string(selectedWidgets.size()) + " widget(s)");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    auto* widget = selectedWidgets.front();
    widget->bounds.x = snapToCanvasGrid(designerCanvas_, document_.root.bounds.width - kLayoutMargin - widget->bounds.width);
    document_.markDirty();
    setOperationStatus("Aligned right: 1 widget(s)");
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::alignSelectedBottom()
{
    auto selectedWidgets = selectedNonRootWidgets(document_);
    if (selectedWidgets.empty()) {
        setOperationStatus(document_.hasSelection() ? "Cannot layout root form" : "No widget selected");
        redraw();
        return;
    }

    if (selectedWidgets.size() > 1) {
        float targetBottom = selectedWidgets.front()->bounds.y + selectedWidgets.front()->bounds.height;
        for (const auto* selected : selectedWidgets) {
            targetBottom = std::max(targetBottom, selected->bounds.y + selected->bounds.height);
        }
        for (auto* selected : selectedWidgets) {
            selected->bounds.y = snapToCanvasGrid(designerCanvas_, targetBottom - selected->bounds.height);
        }
        document_.markDirty();
        setOperationStatus("Aligned bottom: " + std::to_string(selectedWidgets.size()) + " widget(s)");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    auto* widget = selectedWidgets.front();
    widget->bounds.y = snapToCanvasGrid(designerCanvas_, document_.root.bounds.height - kLayoutMargin - widget->bounds.height);
    document_.markDirty();
    setOperationStatus("Aligned bottom: 1 widget(s)");
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::centerSelectedHorizontally()
{
    auto selectedWidgets = selectedNonRootWidgets(document_);
    if (selectedWidgets.empty()) {
        setOperationStatus(document_.hasSelection() ? "Cannot layout root form" : "No widget selected");
        redraw();
        return;
    }

    if (selectedWidgets.size() > 1) {
        const SelectionBoundsInfo bounds = calculateSelectionBounds(selectedWidgets);
        const float centerX = (bounds.left + bounds.right) * 0.5f;
        for (auto* selected : selectedWidgets) {
            selected->bounds.x = snapToCanvasGrid(designerCanvas_, centerX - selected->bounds.width * 0.5f);
        }
        document_.markDirty();
        setOperationStatus("Centered horizontally: " + std::to_string(selectedWidgets.size()) + " widget(s)");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    auto* widget = selectedWidgets.front();
    widget->bounds.x = snapToCanvasGrid(designerCanvas_, (document_.root.bounds.width - widget->bounds.width) * 0.5f);
    document_.markDirty();
    setOperationStatus("Centered horizontally: 1 widget(s)");
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::centerSelectedVertically()
{
    auto selectedWidgets = selectedNonRootWidgets(document_);
    if (selectedWidgets.empty()) {
        setOperationStatus(document_.hasSelection() ? "Cannot layout root form" : "No widget selected");
        redraw();
        return;
    }

    if (selectedWidgets.size() > 1) {
        const SelectionBoundsInfo bounds = calculateSelectionBounds(selectedWidgets);
        const float centerY = (bounds.top + bounds.bottom) * 0.5f;
        for (auto* selected : selectedWidgets) {
            selected->bounds.y = snapToCanvasGrid(designerCanvas_, centerY - selected->bounds.height * 0.5f);
        }
        document_.markDirty();
        setOperationStatus("Centered vertically: " + std::to_string(selectedWidgets.size()) + " widget(s)");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    auto* widget = selectedWidgets.front();
    widget->bounds.y = snapToCanvasGrid(designerCanvas_, (document_.root.bounds.height - widget->bounds.height) * 0.5f);
    document_.markDirty();
    setOperationStatus("Centered vertically: 1 widget(s)");
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::makeSelectedSameWidth()
{
    auto selectedWidgets = selectedNonRootWidgets(document_);
    if (selectedWidgets.empty()) {
        setOperationStatus(document_.hasSelection() ? "Cannot layout root form" : "No widget selected");
        redraw();
        return;
    }
    auto* widget = document_.selectedWidget();
    if (document_.isRootWidgetId(widget->id)) {
        setOperationStatus("Cannot layout root form");
        redraw();
        return;
    }

    if (selectedWidgets.size() > 1) {
        const float referenceWidth = widget->bounds.width;
        for (auto* selected : selectedWidgets) {
            if (selected->id == widget->id || document_.isRootWidgetId(selected->id)) {
                continue;
            }
            const WidgetSizeMetrics metrics = getWidgetSizeMetrics(selected->type);
            selected->bounds.width = std::max(metrics.minWidth, referenceWidth);
        }
        document_.markDirty();
        setOperationStatus("Same width: " + std::to_string(selectedWidgets.size()) + " widgets");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    const WidgetSizeMetrics metrics = getWidgetSizeMetrics(widget->type);
    const model::WidgetNode* reference = document_.previousSiblingOf(widget->id);
    const float fallbackWidth = std::max(metrics.minWidth, document_.root.bounds.width - 40.0f);
    widget->bounds.width = std::max(metrics.minWidth, reference != nullptr ? reference->bounds.width : fallbackWidth);
    document_.markDirty();
    setOperationStatus("Same width: " + widgetDisplayName(*widget) + " (" + widget->id + ")");
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::makeSelectedSameHeight()
{
    auto selectedWidgets = selectedNonRootWidgets(document_);
    if (selectedWidgets.empty()) {
        setOperationStatus(document_.hasSelection() ? "Cannot layout root form" : "No widget selected");
        redraw();
        return;
    }
    auto* widget = document_.selectedWidget();
    if (document_.isRootWidgetId(widget->id)) {
        setOperationStatus("Cannot layout root form");
        redraw();
        return;
    }

    if (selectedWidgets.size() > 1) {
        const float referenceHeight = widget->bounds.height;
        for (auto* selected : selectedWidgets) {
            if (selected->id == widget->id || document_.isRootWidgetId(selected->id)) {
                continue;
            }
            const WidgetSizeMetrics metrics = getWidgetSizeMetrics(selected->type);
            selected->bounds.height = std::max(metrics.minHeight, referenceHeight);
        }
        document_.markDirty();
        setOperationStatus("Same height: " + std::to_string(selectedWidgets.size()) + " widgets");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    const WidgetSizeMetrics metrics = getWidgetSizeMetrics(widget->type);
    const model::WidgetNode* reference = document_.previousSiblingOf(widget->id);
    widget->bounds.height = std::max(metrics.minHeight, reference != nullptr ? reference->bounds.height : metrics.defaultHeight);
    document_.markDirty();
    setOperationStatus("Same height: " + widgetDisplayName(*widget) + " (" + widget->id + ")");
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::distributeSelectedHorizontally()
{
    auto selectedWidgets = selectedNonRootWidgets(document_);
    if (selectedWidgets.size() < 3) {
        setOperationStatus("Select at least 3 widgets to distribute horizontally");
        redraw();
        return;
    }

    std::sort(selectedWidgets.begin(), selectedWidgets.end(),
        [](const model::WidgetNode* left, const model::WidgetNode* right) {
            return left->bounds.x < right->bounds.x;
        });

    const float leftX = selectedWidgets.front()->bounds.x;
    const float rightX = selectedWidgets.back()->bounds.x;
    const float step = (rightX - leftX) / static_cast<float>(selectedWidgets.size() - 1);
    for (std::size_t index = 1; index + 1 < selectedWidgets.size(); ++index) {
        selectedWidgets[index]->bounds.x = snapToCanvasGrid(designerCanvas_, leftX + step * static_cast<float>(index));
    }

    document_.markDirty();
    setOperationStatus("Distributed horizontally: " + std::to_string(selectedWidgets.size()) + " widgets");
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::distributeSelectedVertically()
{
    auto selectedWidgets = selectedNonRootWidgets(document_);
    if (selectedWidgets.size() < 3) {
        setOperationStatus("Select at least 3 widgets to distribute vertically");
        redraw();
        return;
    }

    std::sort(selectedWidgets.begin(), selectedWidgets.end(),
        [](const model::WidgetNode* top, const model::WidgetNode* bottom) {
            return top->bounds.y < bottom->bounds.y;
        });

    const float topY = selectedWidgets.front()->bounds.y;
    const float bottomY = selectedWidgets.back()->bounds.y;
    const float step = (bottomY - topY) / static_cast<float>(selectedWidgets.size() - 1);
    for (std::size_t index = 1; index + 1 < selectedWidgets.size(); ++index) {
        selectedWidgets[index]->bounds.y = snapToCanvasGrid(designerCanvas_, topY + step * static_cast<float>(index));
    }

    document_.markDirty();
    setOperationStatus("Distributed vertically: " + std::to_string(selectedWidgets.size()) + " widgets");
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::nudgeSelectedWidgets(float dx, float dy)
{
    auto selectedWidgets = selectedNonRootWidgets(document_);
    if (selectedWidgets.empty()) {
        setOperationStatus(document_.hasSelection() ? "Cannot layout root form" : "No widget selected");
        redraw();
        return;
    }

    for (auto* selected : selectedWidgets) {
        selected->bounds.x += dx;
        selected->bounds.y += dy;
    }

    document_.markDirty();
    setOperationStatus("Nudged " + std::to_string(selectedWidgets.size()) + " widget(s)");
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::bringSelectedForward()
{
    const std::string selectedId = document_.selectedWidgetId;
    auto* widget = document_.selectedWidget();
    if (widget == nullptr) {
        setOperationStatus("No widget selected");
        redraw();
        return;
    }
    if (document_.isRootWidgetId(widget->id)) {
        setOperationStatus("Cannot layout root form");
        redraw();
        return;
    }

    const std::string displayName = widgetDisplayName(*widget);
    if (!document_.bringWidgetForward(selectedId)) {
        setOperationStatus(document_.hasMultiSelection()
            ? "Already in front: " + displayName + " (" + selectedId + ") - primary only"
            : "Already in front: " + displayName + " (" + selectedId + ")");
        redraw();
        return;
    }

    document_.selectWidget(selectedId);
    document_.markDirty();
    setOperationStatus(document_.hasMultiSelection()
        ? "Brought forward: " + displayName + " (" + selectedId + ") - primary only"
        : "Brought forward: " + displayName + " (" + selectedId + ")");
    redraw();
}

void MainWindow::sendSelectedBackward()
{
    const std::string selectedId = document_.selectedWidgetId;
    auto* widget = document_.selectedWidget();
    if (widget == nullptr) {
        setOperationStatus("No widget selected");
        redraw();
        return;
    }
    if (document_.isRootWidgetId(widget->id)) {
        setOperationStatus("Cannot layout root form");
        redraw();
        return;
    }

    const std::string displayName = widgetDisplayName(*widget);
    if (!document_.sendWidgetBackward(selectedId)) {
        setOperationStatus(document_.hasMultiSelection()
            ? "Already in back: " + displayName + " (" + selectedId + ") - primary only"
            : "Already in back: " + displayName + " (" + selectedId + ")");
        redraw();
        return;
    }

    document_.selectWidget(selectedId);
    document_.markDirty();
    setOperationStatus(document_.hasMultiSelection()
        ? "Sent backward: " + displayName + " (" + selectedId + ") - primary only"
        : "Sent backward: " + displayName + " (" + selectedId + ")");
    redraw();
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

    const WidgetSizeMetrics metrics = getWidgetSizeMetrics(widget->type);
    const float clampedWidth = std::max(width, metrics.minWidth);
    const float clampedHeight = std::max(height, metrics.minHeight);
    if (x < 0.0f || y < 0.0f) {
        setOperationStatus("Invalid bounds for selected widget");
        redraw();
        return false;
    }

    widget->bounds = { x, y, clampedWidth, clampedHeight };
    document_.markDirty();
    if (height < metrics.minHeight && widget->type == model::WidgetType::Label) {
        setOperationStatus("Height clamped to minimum for Label");
    }
    else if (height < metrics.minHeight && widget->type == model::WidgetType::CheckBox) {
        setOperationStatus("Height clamped to minimum for CheckBox");
    }
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

    const bool radioSelectedTrue = widget->type == model::WidgetType::RadioButton
        && key == "selected" && value.isBool() && value.asBool(false);
    widget->setProperty(key, std::move(value));
    if (widget->type == model::WidgetType::FormWindow && document_.isRootWidgetId(widget->id) && key == "title") {
        document_.windowTitle = widget->getStringProperty("title", document_.projectName);
    }
    if (widget->type == model::WidgetType::RadioButton && key == "group" && widget->getBoolProperty("selected", false)) {
        document_.selectRadioButtonInGroup(widget->id);
    }
    else if (radioSelectedTrue) {
        document_.selectRadioButtonInGroup(widget->id);
    }
    const float previousWidth = widget->bounds.width;
    const bool autoSized = autoSizeWidgetForTextProperty(*widget, key, widget->getStringProperty(key, {}));
    document_.markDirty();
    const std::string displayName = widget->name.empty() ? widget->id : widget->name;
    setOperationStatus(autoSized
        ? "Auto-sized " + displayName + ": width " + std::to_string(static_cast<int>(previousWidth))
            + " -> " + std::to_string(static_cast<int>(widget->bounds.width))
        : "Property changed: " + key);
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

    if (key == "projectName") {
        if (trimmedValue.empty()) {
            setOperationStatus("Project name cannot be empty");
            redraw();
            return false;
        }

        const std::string previousProjectName = document_.projectName;
        const std::string previousExecutableName = document_.executableName;
        const std::string previousWindowTitle = document_.windowTitle;
        document_.projectName = trimmedValue;
        if (previousExecutableName.empty() || previousExecutableName == sanitizeExecutableName(previousProjectName, "VisiFormProject")) {
            document_.executableName = sanitizeExecutableName(trimmedValue, "VisiFormProject");
        }
        if (previousWindowTitle.empty() || previousWindowTitle == previousProjectName) {
            document_.windowTitle = trimmedValue;
            document_.root.setProperty("title", document_.windowTitle);
        }

        document_.markDirty();
        setOperationStatus("Project name changed: " + document_.projectName);
        redraw();
        return true;
    }

    if (key == "executableName") {
        if (trimmedValue.empty()) {
            setOperationStatus("Executable name cannot be empty");
            redraw();
            return false;
        }

        document_.executableName = sanitizeExecutableName(trimmedValue, document_.projectName.empty() ? std::string{"VisiFormProject"} : document_.projectName);
        document_.markDirty();
        setOperationStatus("Executable name changed: " + document_.executableName);
        redraw();
        return true;
    }

    if (key == "generatedBaseClassName" || key == "userSubclassName") {
        if (key == "generatedBaseClassName") {
            setOperationStatus("Generated base class is fixed: MainWindow");
            redraw();
            return false;
        }

        if (trimmedValue.empty() || !utils::isValidCppIdentifier(trimmedValue) || trimmedValue == "MainWindow") {
            setOperationStatus("Invalid class name");
            redraw();
            return false;
        }

        const std::string otherName = "MainWindow";
        if (trimmedValue == otherName) {
            setOperationStatus("Class names must be different");
            redraw();
            return false;
        }

        document_.generatedBaseClassName = "MainWindow";
        document_.userSubclassName = trimmedValue;
        document_.mainFormClassName = trimmedValue;

        document_.markDirty();
        setOperationStatus("User subclass changed: " + trimmedValue);
        redraw();
        return true;
    }

    if (key == "windowTitle") {
        document_.windowTitle = trimmedValue.empty() ? document_.projectName : trimmedValue;
        document_.root.setProperty("title", document_.windowTitle);
        document_.markDirty();
        setOperationStatus("Window title changed: " + document_.windowTitle);
        redraw();
        return true;
    }

    if (key == "lookAndFeelId") {
        if (!trimmedValue.empty() && model::LookAndFeelRegistry::instance().findById(trimmedValue) == nullptr) {
            setOperationStatus("Unknown look and feel preset");
            redraw();
            return false;
        }

        if (widget->type == model::WidgetType::FormWindow && document_.isRootWidgetId(widget->id)) {
            document_.lookAndFeelId = trimmedValue.empty() ? "VisiFormDark" : trimmedValue;
            document_.markDirty();
            setOperationStatus("Look and feel changed: " + document_.lookAndFeelId);
            redraw();
            return true;
        }

        return setSelectedWidgetProperty(key, trimmedValue);
    }

    if (key == "localVisageSourceDirectory") {
        settings_.localVisageSourceDirectory = trimmedValue.empty()
            ? std::filesystem::path{}
            : std::filesystem::path{ utils::FileUtils::normalizeSeparators(trimmedValue) };
        saveAppSettings();
        if (!settings_.localVisageSourceDirectory.empty() && !std::filesystem::exists(settings_.localVisageSourceDirectory / "CMakeLists.txt")) {
            setOperationStatus("Local Visage source path does not contain CMakeLists.txt");
        }
        else if (settings_.localVisageSourceDirectory.empty()) {
            setOperationStatus("Local Visage source path cleared");
        }
        else {
            setOperationStatus("Local Visage source path set: " + normalizedPathText(settings_.localVisageSourceDirectory));
        }
        updatePropertyEditorBounds();
        redraw();
        return true;
    }

    if (key == "visageGitRepository") {
        settings_.visageGitRepository = trimmedValue.empty()
            ? std::string{ utils::AppSettings::defaultVisageGitRepository }
            : trimmedValue;
        saveAppSettings();
        setOperationStatus("Visage Git repository changed: " + settings_.visageGitRepository);
        updatePropertyEditorBounds();
        redraw();
        return true;
    }

    if (key == "visageGitTag") {
        settings_.visageGitTag = trimmedValue.empty()
            ? std::string{ utils::AppSettings::defaultVisageGitTag }
            : trimmedValue;
        saveAppSettings();
        setOperationStatus("Visage Git tag changed: " + settings_.visageGitTag);
        updatePropertyEditorBounds();
        redraw();
        return true;
    }

    if (isWidgetColorProperty(*widget, key)) {
        if (!isValidColorValue(trimmedValue)) {
            setOperationStatus("Invalid color value");
            redraw();
            return false;
        }

        return setSelectedWidgetProperty(key, trimmedValue);
    }

    if (isStyleFloatProperty(key)) {
        float parsedValue = 0.0f;
        if (!trimmedValue.empty() && !parseFloat(trimmedValue, parsedValue)) {
            setOperationStatus("Invalid value for " + key);
            redraw();
            return false;
        }

        if (key == "fontSize") {
            parsedValue = std::clamp(parsedValue, 8.0f, 72.0f);
        }
        else if (key == "borderThickness") {
            parsedValue = std::clamp(parsedValue, 0.0f, 20.0f);
        }
        else if (key == "cornerRadius") {
            parsedValue = std::clamp(parsedValue, 0.0f, 50.0f);
        }

        if (trimmedValue.empty()) {
            return setSelectedWidgetProperty(key, model::PropertyValue{});
        }

        return setSelectedWidgetProperty(key, parsedValue);
    }

    if (key == "onClick" || key == "onToggle" || key == "onChanged"
        || key == "onTextChanged" || key == "onLoad" || key == "onClose" || key == "onSelected") {
        if (!trimmedValue.empty() && !utils::isValidCppIdentifier(trimmedValue)) {
            setOperationStatus("Invalid event handler name");
            redraw();
            return false;
        }

        return setSelectedWidgetProperty(key, trimmedValue);
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

void MainWindow::updateWindowTitle()
{
    setTitle(document_.dirty ? "VisiForm - Visage Form Builder *" : kWindowTitle);
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
    // Split status bar into fields: main status, selection info, progress
    const float totalWidth = layout_.statusBar.width - kPadding * 2.0f;
    const float rightWidth = 220.0f; // reserved for progress/status details
    const float middleWidth = 160.0f;
    const float leftWidth = std::max(0.0f, totalWidth - rightWidth - middleWidth);

    const float leftX = layout_.statusBar.x + kPadding;
    const float middleX = leftX + leftWidth + 8.0f;
    const float rightX = middleX + middleWidth + 8.0f;

    // Left: main status text
    canvas.text(statusText(), labelFont_, visage::Font::kTopLeft,
        leftX, layout_.statusBar.y + 4.0f, leftWidth, layout_.statusBar.height - 6.0f);

    // Middle: selection info (do not duplicate hints here)
    std::string middleText;
    if (document_.hasMultiSelection()) {
        middleText = "Selected: " + std::to_string(document_.selectedWidgetIds().size()) + " widgets";
    }
    else if (const auto* sel = document_.selectedWidget()) {
        middleText = widgetDisplayName(*sel) + " (" + sel->id + ")";
    }
    canvas.text(middleText, labelFont_, visage::Font::kTopLeft,
        middleX, layout_.statusBar.y + 4.0f, middleWidth, layout_.statusBar.height - 6.0f);

    // Right: export progress
    if (exportInProgress_ || exportProgressPercent_ > 0) {
        const float pw = std::min(rightWidth, totalWidth);
        const float ph = 12.0f;
        const float progressBarX = rightX;
        const float progressBarY = layout_.statusBar.y + (layout_.statusBar.height - ph) * 0.5f;
        // background
        canvas.setColor(0xffeef2f7);
        canvas.fill(progressBarX, progressBarY, pw, ph);
        // fill
        const float fillW = pw * (static_cast<float>(exportProgressPercent_) / 100.0f);
        canvas.setColor(0xff2d7ff9);
        canvas.fill(progressBarX, progressBarY, fillW, ph);
        // border (simple) and text
        canvas.setColor(0xff6c7788);
        canvas.fill(progressBarX, progressBarY, pw, 1.0f);
        canvas.fill(progressBarX, progressBarY + ph - 1.0f, pw, 1.0f);
        canvas.fill(progressBarX, progressBarY, 1.0f, ph);
        canvas.fill(progressBarX + pw - 1.0f, progressBarY, 1.0f, ph);
        const std::string progressText = exportInProgress_ ? (exportProgressText_.empty() ? ("Export " + std::to_string(exportProgressPercent_) + "%") : exportProgressText_) : (exportProgressText_.empty() ? "" : exportProgressText_);
        if (!progressText.empty()) {
            canvas.setColor(exportProgressPercent_ >= 50 ? 0xfff8fbff : 0xff182333);
            canvas.text(progressText, labelFont_, visage::Font::kCenter,
                progressBarX, progressBarY - 2.0f, pw, ph + 4.0f);
        }
    }
}

void MainWindow::selectWidget(const std::string& widgetId)
{
    statusMessage_.clear();
    hoverHint_.clear();
    cancelInspectorEdit();
    document_.selectWidget(widgetId);
    redraw();
}

std::string MainWindow::statusText() const
{
    if (!hoverHint_.empty()) {
        return hoverHint_;
    }

    if (!statusMessage_.empty()) {
        return statusMessage_;
    }

    const auto* selectedWidget = document_.selectedWidget();
    if (selectedWidget == nullptr) {
        return document_.dirty ? "Status: Modified" : "Status: Ready";
    }

    const std::string displayName = selectedWidget->name.empty() ? selectedWidget->id : selectedWidget->name;
    const std::string widgetHint = selectedWidget->getStringProperty("hint", {});
    if (document_.hasMultiSelection()) {
        std::string text = std::string(document_.dirty ? "Modified - " : "") + "Selected: "
            + std::to_string(document_.selectedWidgetIds().size()) + " widgets, primary: "
            + displayName + " (" + selectedWidget->id + ")";
        if (!widgetHint.empty()) {
            text += " - Hint: " + widgetHint;
        }
        return text;
    }
    std::string text = std::string(document_.dirty ? "Modified - " : "") + "Selected: " + displayName + " (" + selectedWidget->id + ")";
    if (!widgetHint.empty()) {
        text += " - Hint: " + widgetHint;
    }
    return text;
}

void MainWindow::setOperationStatus(std::string message)
{
    hoverHint_.clear();
    statusMessage_ = std::move(message);
}

std::optional<MainWindow::ToolbarButton> MainWindow::toolbarButtonAt(float x, float y) const
{
    for (const auto& button : toolbarButtons()) {
        if (x >= button.bounds.x && x <= button.bounds.x + button.bounds.width
            && y >= button.bounds.y && y <= button.bounds.y + button.bounds.height) {
            return button;
        }
    }

    return std::nullopt;
}

MainWindow::ToolbarAction MainWindow::toolbarActionAt(float x, float y) const
{
    if (const auto button = toolbarButtonAt(x, y)) {
        return button->action;
    }

    return ToolbarAction::None;
}

void MainWindow::updateHoverHint(float x, float y)
{
    std::string nextHint;
    if (const auto button = toolbarButtonAt(x, y)) {
        nextHint = "Hint: " + button->hint;
    }
    else if (const auto hint = widgetPalette_.hitTestHint(x, y)) {
        nextHint = "Hint: " + *hint;
    }
    else if (const auto widgetId = designerCanvas_.hitTestWidgetId(document_, x, y)) {
        if (const auto* widget = document_.findWidgetById(*widgetId)) {
            const std::string hint = widget->getStringProperty("hint", {});
            if (!hint.empty()) {
                nextHint = "Hint: " + hint;
            }
            else {
                nextHint = widgetDisplayName(*widget) + " [" + widget->typeName() + "]";
            }
        }
    }

    if (hoverHint_ != nextHint) {
        hoverHint_ = std::move(nextHint);
        redraw();
    }
}

std::vector<MainWindow::ToolbarButton> MainWindow::toolbarButtons() const
{
    std::vector<ToolbarButton> buttons;
    if (!layout_.toolbar.isVisible()) {
        return buttons;
    }

    const float top = layout_.toolbar.y + 8.0f;
    float left = layout_.toolbar.x + kPadding;
    const auto addButton = [&](ToolbarAction action, std::string label, std::string hint, bool accent = false) {
        buttons.push_back(ToolbarButton{ action, std::move(label), std::move(hint), { left, top, kToolbarButtonWidth, kToolbarButtonHeight }, accent });
        left += kToolbarButtonWidth + kToolbarButtonSpacing;
    };

    addButton(ToolbarAction::NewProject, "New", "Create a new VisiForm project");
    addButton(ToolbarAction::OpenProject, "Open", "Open a .vfb.json project");
    addButton(ToolbarAction::SaveProject, "Save", "Save the current project");
    addButton(ToolbarAction::SaveProjectAsDialog, "SAs", "Save the project to a new .vfb.json file", true);
    addButton(ToolbarAction::OpenSample, "Smp", "Open the sample project");
    addButton(ToolbarAction::SaveProjectAsDebug, "Dbg", "Save to the debug test project path");
    addButton(ToolbarAction::ExportCode, "Exp", "Export generated Visage C++ project");
    addButton(ToolbarAction::ValidateProject, "Chk", "Validate the current project before export");
    addButton(ToolbarAction::FitText, "Fit", "Fit the selected widget to its text");
    addButton(ToolbarAction::CopyWidgets, "Cp", "Copy selected widgets");
    addButton(ToolbarAction::PasteWidgets, "Pt", "Paste copied widgets");
    addButton(ToolbarAction::ToggleMultiSelect, "Multi", "Toggle multi-select mode", multiSelectMode_);
    addButton(ToolbarAction::AlignLeft, "L", "Align selected widgets left");
    addButton(ToolbarAction::AlignTop, "T", "Align selected widgets top");
    addButton(ToolbarAction::AlignRight, "R", "Align selected widgets right");
    addButton(ToolbarAction::AlignBottom, "B", "Align selected widgets bottom");
    addButton(ToolbarAction::CenterHorizontally, "CH", "Center selected widgets horizontally");
    addButton(ToolbarAction::CenterVertically, "CV", "Center selected widgets vertically");
    addButton(ToolbarAction::SameWidth, "W", "Match selected widget widths");
    addButton(ToolbarAction::SameHeight, "H", "Match selected widget heights");
    addButton(ToolbarAction::DistributeHorizontally, "DH", "Distribute selected widgets horizontally");
    addButton(ToolbarAction::DistributeVertically, "DV", "Distribute selected widgets vertically");
    addButton(ToolbarAction::ToggleSmartGuides, "Gde", "Toggle smart guides", settings_.smartGuidesEnabled);
    addButton(ToolbarAction::BringForward, "Fr", "Bring the selected widget forward");
    addButton(ToolbarAction::SendBackward, "Bk", "Send the selected widget backward");
    addButton(ToolbarAction::ToggleGrid, "Grid", "Toggle grid visibility", designerCanvas_.showGrid());
    addButton(ToolbarAction::ToggleSnap, "Snap", "Toggle snap-to-grid", designerCanvas_.snapToGrid());
    addButton(ToolbarAction::DeleteWidget, "Del", "Delete the selected widget or widgets");
    addButton(ToolbarAction::DuplicateWidget, "Dup", "Duplicate the primary selected widget");
    addButton(ToolbarAction::UndoAction, "Undo", "Undo the last command");
    addButton(ToolbarAction::RedoAction, "Redo", "Redo the last undone command");

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

std::filesystem::path MainWindow::defaultExportPath() const
{
    return projectRootPath() / "Generated" / "ExportedVisageProject";
}

std::filesystem::path MainWindow::sampleProjectPath() const
{
    return projectRootPath() / "templates" / "examples" / "BasicWindow.vfb.json";
}

std::filesystem::path MainWindow::defaultDebugSavePath() const
{
    return projectRootPath() / "Generated" / "debug_saved_project.vfb.json";
}

MainWindow::UnsavedChangesResult MainWindow::promptForUnsavedChanges()
{
    const int response = MessageBoxW(
        nullptr,
        L"The current project has unsaved changes. Save before continuing?",
        L"VisiForm - Unsaved Changes",
        MB_ICONWARNING | MB_YESNOCANCEL);

    switch (response) {
    case IDYES:
        return UnsavedChangesResult::Save;
    case IDNO:
        return UnsavedChangesResult::DontSave;
    case IDCANCEL:
    default:
        return UnsavedChangesResult::Cancel;
    }
}

bool MainWindow::confirmSaveIfDirty()
{
    if (!document_.dirty) {
        return true;
    }

    switch (promptForUnsavedChanges()) {
    case UnsavedChangesResult::Save:
        return saveProject();
    case UnsavedChangesResult::DontSave:
        return true;
    case UnsavedChangesResult::Cancel:
    default:
        setOperationStatus("Operation cancelled");
        redraw();
        return false;
    }
}

void MainWindow::loadAppSettings()
{
    std::string errorMessage;
    settings_ = utils::AppSettings::load(errorMessage);
    projectTree_.setRecentFiles(settings_.recentFiles);
}

void MainWindow::saveAppSettings()
{
    settings_.removeMissingRecentFiles();
    projectTree_.setRecentFiles(settings_.recentFiles);
    std::string errorMessage;
    settings_.save(errorMessage);
}

void MainWindow::applyCanvasSettings()
{
    designerCanvas_.setShowGrid(settings_.showGrid);
    designerCanvas_.setSnapToGrid(settings_.snapToGrid);
    designerCanvas_.setGridSize(settings_.gridSize);
    designerCanvas_.setMajorGridSize(settings_.majorGridSize);
}

void MainWindow::toggleGrid()
{
    settings_.showGrid = !designerCanvas_.showGrid();
    applyCanvasSettings();
    saveAppSettings();
    setOperationStatus(std::string{"Grid: "} + (settings_.showGrid ? "On" : "Off"));
    redraw();
}

void MainWindow::toggleSnapToGrid()
{
    settings_.snapToGrid = !designerCanvas_.snapToGrid();
    applyCanvasSettings();
    saveAppSettings();
    setOperationStatus(std::string{"Snap: "} + (settings_.snapToGrid ? "On" : "Off"));
    redraw();
}

void MainWindow::addRecentFile(const std::filesystem::path& path)
{
    settings_.addRecentFile(path);
    saveAppSettings();
}

void MainWindow::removeRecentFile(const std::filesystem::path& path)
{
    settings_.removeRecentFile(path);
    saveAppSettings();
}

bool MainWindow::openRecentFile(const std::filesystem::path& path)
{
    if (!confirmSaveIfDirty()) {
        return false;
    }

    if (!std::filesystem::exists(path)) {
        removeRecentFile(path);
        setOperationStatus("Recent file is missing: " + normalizedPathText(path));
        redraw();
        return false;
    }

    return loadProjectFromPath(path);
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

    if (!propertyInspector_.beginEditing(document_, settings_, row.key)) {
        return false;
    }

    propertyEditor_.setText(row.displayValue);
    if (row.editKind == PropertyInspector::PropertyEditKind::Choice) {
        propertyEditor_.setVisible(false);
        requestKeyboardFocus();
        redraw();
        return true;
    }

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

bool MainWindow::applySelectedWidgetCallbackProperty(const std::string& propertyKey, const std::string& callbackName)
{
    if (!setSelectedWidgetPropertyFromString(propertyKey, callbackName)) {
        setOperationStatus(std::string("Failed to apply callback: ") + callbackName);
        redraw();
        return false;
    }

    propertyInspector_.clearEditing();
    propertyEditor_.setVisible(false);
    requestKeyboardFocus();
    setOperationStatus(std::string("Callback selected: ") + callbackName);
    redraw();
    return true;
}

void MainWindow::updatePropertyEditorBounds()
{
    if (!propertyInspector_.isEditing()) {
        propertyEditor_.setVisible(false);
        return;
    }

    const auto bounds = propertyInspector_.activeEditorBounds(document_, settings_);
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
