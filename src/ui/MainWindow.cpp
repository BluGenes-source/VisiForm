#include "ui/MainWindow.h"

#include "app/Version.h"
#include "commands/Command.h"
#include "commands/CommandIds.h"
#include "commands/CommandRegistry.h"
#include "model/LayoutEngine.h"
#include "model/LookAndFeelRegistry.h"
#include "model/WidgetItemUtils.h"
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
#include <iomanip>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace visiform::ui {
namespace {

constexpr float kMenuBarHeight = 30.0f;
constexpr float kToolbarHeight = 42.0f;
constexpr float kStatusBarHeight = 28.0f;
constexpr float kLeftPanelWidth = 220.0f;
constexpr float kRightPanelWidth = 300.0f;
constexpr float kGap = 8.0f;
constexpr float kProjectTreeMinHeight = 160.0f;
constexpr float kProjectTreePreferredHeight = 180.0f;
constexpr float kPadding = 12.0f;
constexpr float kToolbarButtonMinWidth = 42.0f;
constexpr float kToolbarButtonHeight = 26.0f;
constexpr float kToolbarButtonSpacing = 6.0f;
constexpr float kMenuBarButtonSpacing = 4.0f;
constexpr float kMenuBarDropdownMinWidth = 220.0f;
constexpr float kMenuBarItemHeight = 28.0f;
constexpr float kMenuBarSeparatorHeight = 10.0f;
constexpr float kNewWidgetStartX = 40.0f;
constexpr float kNewWidgetStartY = 40.0f;
constexpr float kNewWidgetSpacing = 12.0f;
constexpr float kLayoutMargin = 20.0f;
constexpr float kGroupBoxChildStartX = 20.0f;
constexpr float kGroupBoxChildStartY = 36.0f;
constexpr float kGroupBoxChildOffsetStep = 16.0f;
constexpr float kGroupBoxContentTopInset = 20.0f;
constexpr float kTabControlPageInset = 6.0f;
constexpr float kTabControlHeaderHeight = 30.0f;
constexpr float kTabPageChildStartX = 20.0f;
constexpr float kTabPageChildStartY = 20.0f;
constexpr float kTabPageChildOffsetStep = 16.0f;
constexpr float kMarqueeDragThreshold = 4.0f;
constexpr float kSmartGuideSnapThreshold = 6.0f;
constexpr float kEditorModalPreferredWidth = 560.0f;
constexpr float kEditorModalPreferredHeight = 320.0f;
constexpr float kEditorModalMaxWidth = 720.0f;
constexpr float kEditorModalMaxHeight = 520.0f;
constexpr float kEditorModalMinWidth = 420.0f;
constexpr float kEditorModalMinHeight = 240.0f;
constexpr std::size_t kEditorModalMaxBodyLines = 10;
constexpr float kEditorModalButtonWidth = 96.0f;
constexpr float kEditorModalButtonHeight = 32.0f;
constexpr float kEditorModalButtonSpacing = 12.0f;
constexpr float kEditorModalSectionSpacing = 10.0f;
constexpr float kEditorModalFormRowHeight = 34.0f;
constexpr float kEditorModalFormRowSpacing = 10.0f;
constexpr float kEditorModalFormLabelWidth = 190.0f;
constexpr float kEditorModalFormStatusHeight = 40.0f;
constexpr float kResourceManagerSplitGap = 14.0f;
constexpr float kResourceManagerPreviewMinWidth = 210.0f;
constexpr float kResourceManagerPreviewMaxWidth = 250.0f;
constexpr float kResourceManagerFieldLabelWidth = 140.0f;
constexpr float kWizardModalWidth = 640.0f;
constexpr float kWizardModalHeight = 520.0f;
constexpr float kProjectSettingsModalWidth = 640.0f;
constexpr float kProjectSettingsModalHeight = 520.0f;
constexpr float kItemListEditorModalWidth = 700.0f;
constexpr float kItemListEditorModalHeight = 540.0f;
constexpr float kItemListEditorPreviewHeight = 220.0f;
constexpr float kItemListEditorPreviewRowHeight = 28.0f;
constexpr float kItemListEditorPreviewHeaderHeight = 34.0f;
constexpr float kItemListEditorPreviewBottomPadding = 8.0f;
constexpr float kItemListEditorPreviewRowInsetX = 8.0f;
constexpr float kItemListEditorPreviewRowTextInsetX = 16.0f;
constexpr float kItemListEditorPreviewRowFillHeight = kItemListEditorPreviewRowHeight - 2.0f;
constexpr float kItemListEditorPreviewGap = 14.0f;
constexpr std::size_t kItemListEditorBaseFieldCount = 3;
constexpr std::size_t kItemListEditorActionFieldCount = 4;
constexpr float kTableGridEditorModalWidth = 820.0f;
constexpr float kTableGridEditorModalHeight = 720.0f;
constexpr float kTableGridEditorMaxWidth = 920.0f;
constexpr float kTableGridEditorMaxHeight = 760.0f;
constexpr float kTableGridEditorOuterMargin = 80.0f;
constexpr float kTableGridEditorHeadingHeight = 20.0f;
constexpr float kTableGridEditorHeadingGap = 4.0f;
constexpr float kTableGridEditorInstructionHeight = 20.0f;
constexpr float kTableGridEditorIntroHeight = kTableGridEditorHeadingHeight + kTableGridEditorHeadingGap + kTableGridEditorInstructionHeight;
constexpr float kTableGridEditorGridPreferredHeight = 170.0f;
constexpr float kTableGridEditorGridMinHeight = 120.0f;
constexpr float kTableGridEditorSectionGap = 10.0f;
constexpr float kTableGridEditorActionLabelHeight = 22.0f;
constexpr float kTableGridEditorActionButtonHeight = 32.0f;
constexpr float kTableGridEditorActionRowGap = 8.0f;
constexpr float kTableGridEditorFieldRowHeight = 30.0f;
constexpr float kTableGridEditorFieldRowSpacing = 8.0f;
constexpr float kTableGridEditorFieldCount = 6.0f;
constexpr float kTableGridEditorHeaderHeight = 30.0f;
constexpr float kTableGridEditorRowHeight = 28.0f;
constexpr float kTreeNodeEditorModalWidth = 760.0f;
constexpr float kTreeNodeEditorModalHeight = 620.0f;
constexpr float kTreeNodeEditorLineHeight = 20.0f;
constexpr float kTreeNodeEditorRowHeight = 26.0f;
constexpr float kTreeNodeEditorRowIndent = 20.0f;
constexpr float kTreeNodeEditorInstructionHeight = 24.0f;
constexpr float kTreeNodeEditorListPreferredHeight = 220.0f;
constexpr float kTreeNodeEditorListMinHeight = 190.0f;
constexpr float kTreeNodeEditorSectionGap = 10.0f;
constexpr float kTreeNodeEditorActionLabelHeight = 22.0f;
constexpr float kTreeNodeEditorActionButtonHeight = 32.0f;

bool pointInBounds(float x, float y, float left, float top, float width, float height)
{
    return x >= left && x <= left + width
        && y >= top && y <= top + height;
}

std::string normalizedPathText(const std::filesystem::path& path)
{
    return utils::FileUtils::normalizeSeparators(path.string());
}

std::string itemListSummaryText(std::size_t itemCount)
{
    return std::to_string(itemCount) + (itemCount == 1 ? " item" : " items");
}

std::string itemListWidgetTypeLabel(model::WidgetType type)
{
    switch (type) {
    case model::WidgetType::ComboBox:
        return "ComboBox";
    case model::WidgetType::ListBox:
        return "ListBox";
    case model::WidgetType::MenuBar:
        return "MenuBar";
    case model::WidgetType::ToolBar:
        return "ToolBar";
    default:
        return model::toString(type);
    }
}

std::string itemListEditorDialogTitle(model::WidgetType type)
{
    switch (type) {
    case model::WidgetType::MenuBar:
        return "Edit Menu Items";
    case model::WidgetType::ToolBar:
        return "Edit Tool Items";
    default:
        return "Edit " + itemListWidgetTypeLabel(type) + " Items";
    }
}

std::string itemListEditorRowLabel(const std::vector<std::string>& items, int index)
{
    if (index < 0 || index >= static_cast<int>(items.size())) {
        return "<none>";
    }

    const std::string& item = items[static_cast<std::size_t>(index)];
    return std::to_string(index + 1) + ". " + (item.empty() ? std::string{ "<empty>" } : item);
}

std::string itemListEditorRowLabel(const std::vector<std::string>& items,
    const std::vector<std::string>& actions,
    bool includeActions,
    int index)
{
    if (!includeActions) {
        return itemListEditorRowLabel(items, index);
    }

    if (index < 0 || index >= static_cast<int>(items.size())) {
        return "<none>";
    }

    const std::string& item = items[static_cast<std::size_t>(index)];
    const std::string action = index < static_cast<int>(actions.size())
        ? actions[static_cast<std::size_t>(index)]
        : std::string{};
    return std::to_string(index + 1) + ". "
        + (item.empty() ? std::string{ "<empty>" } : item)
        + " -> " + (action.empty() ? std::string{ "<none>" } : action);
}

std::size_t itemListEditorVisibleRowCount(float previewHeight, std::size_t itemCount)
{
    if (itemCount == 0) {
        return 0;
    }

    const float visibleHeight = std::max(0.0f, previewHeight - kItemListEditorPreviewHeaderHeight - kItemListEditorPreviewBottomPadding);
    const std::size_t maxVisibleRows = std::max<std::size_t>(1,
        static_cast<std::size_t>(std::floor(visibleHeight / kItemListEditorPreviewRowHeight)));
    return std::min(itemCount, maxVisibleRows);
}

float itemListEditorRowTop(float previewY, std::size_t rowIndex)
{
    return previewY + kItemListEditorPreviewHeaderHeight
        + static_cast<float>(rowIndex) * kItemListEditorPreviewRowHeight;
}

std::string itemListEditorDefaultStatus(bool supportsActions)
{
    return supportsActions
        ? "Select a row to edit Label and Callback / Action. Click Apply to save."
        : "Select a row to edit Item Text. Click Apply to save.";
}

std::string itemListEditorNoSelectionStatus(bool supportsActions)
{
    return supportsActions
        ? "No row selected. Click Add Item to create a label and optional action."
        : "No row selected. Click Add Item to create an item.";
}

std::string itemListEditorSelectedStatus(int selectedIndex, std::size_t itemCount)
{
    return "Editing row " + std::to_string(selectedIndex + 1)
        + " of " + std::to_string(itemCount)
        + ". Update the fields, then Apply.";
}

std::string tableGridEditorColumnLabel(const std::vector<std::string>& columns, int index)
{
    if (index < 0 || index >= static_cast<int>(columns.size())) {
        return "<none>";
    }

    const std::string& title = columns[static_cast<std::size_t>(index)];
    return std::to_string(index) + ": " + (title.empty() ? std::string{ "<empty>" } : title);
}

bool isAdditiveSelectionModifierDown()
{
#ifdef _WIN32
    return (GetKeyState(VK_CONTROL) & 0x8000) != 0
        || (GetKeyState(VK_SHIFT) & 0x8000) != 0;
#else
    return false;
#endif
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

bool isUnsetValueText(std::string_view value)
{
    return value.empty() || value == "<unset>";
}

float verticallyCenteredTextTop(float top, float height, float textHeight = 18.0f, float minimumPadding = 4.0f)
{
    return top + std::max(minimumPadding, (height - textHeight) * 0.5f);
}

std::optional<float> tryParseFloat(std::string_view text)
{
    if (isUnsetValueText(text)) {
        return std::nullopt;
    }

    std::istringstream stream(std::string{ text });
    float value = 0.0f;
    char trailing = '\0';
    if (!(stream >> value)) {
        return std::nullopt;
    }
    if (stream >> trailing) {
        return std::nullopt;
    }
    return value;
}

std::optional<int> tryParseInt(std::string_view text)
{
    if (isUnsetValueText(text)) {
        return std::nullopt;
    }

    std::istringstream stream(std::string{ text });
    int value = 0;
    char trailing = '\0';
    if (!(stream >> value)) {
        return std::nullopt;
    }
    if (stream >> trailing) {
        return std::nullopt;
    }
    return value;
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

std::vector<std::string> availableLookAndFeelIds()
{
    std::vector<std::string> result;
    for (const auto& definition : model::LookAndFeelRegistry::instance().definitions()) {
        result.push_back(definition.id);
    }
    return result;
}

std::vector<std::string> newProjectTemplateIds()
{
    return {
        "blank",
        "basic_app",
        "form_with_status",
        "control_panel",
        "dialog_test"
    };
}

std::vector<PropertyInspector::PropertyChoice> propertyChoicesFromValues(const std::vector<std::string>& values)
{
    std::vector<PropertyInspector::PropertyChoice> choices;
    choices.reserve(values.size());
    for (const auto& value : values) {
        choices.push_back({ value, value });
    }
    return choices;
}

bool containsText(const std::vector<std::string>& values, const std::string& value)
{
    return std::find(values.begin(), values.end(), value) != values.end();
}

std::string templateDescription(const std::string& templateId)
{
    if (templateId == "blank") {
        return "Template: blank - Empty form.";
    }
    if (templateId == "basic_app") {
        return "Template: basic_app - Label, button, and docked status bar.";
    }
    if (templateId == "form_with_status") {
        return "Template: form_with_status - Label, button, progress bar, and status bar.";
    }
    if (templateId == "control_panel") {
        return "Template: control_panel - Common controls plus a docked status bar.";
    }
    if (templateId == "dialog_test") {
        return "Template: dialog_test - Button, modal dialog, and docked status bar.";
    }
    return "Template: custom";
}

std::string resourceTypeDisplayName(model::ProjectResourceType type)
{
    switch (type) {
    case model::ProjectResourceType::Image:
        return "image";
    case model::ProjectResourceType::Font:
        return "font";
    case model::ProjectResourceType::Icon:
        return "icon";
    case model::ProjectResourceType::Theme:
        return "theme";
    case model::ProjectResourceType::Other:
        return "resource";
    }

    return "resource";
}

std::string defaultResourceFolder(model::ProjectResourceType type)
{
    switch (type) {
    case model::ProjectResourceType::Image:
        return "assets/images";
    case model::ProjectResourceType::Font:
        return "assets/fonts";
    case model::ProjectResourceType::Icon:
        return "assets/icons";
    case model::ProjectResourceType::Theme:
        return "assets/themes";
    case model::ProjectResourceType::Other:
        return "assets/resources";
    }

    return "assets/resources";
}

std::string defaultResourceDisplayName(const std::filesystem::path& sourcePath)
{
    return utils::FileUtils::fileStem(sourcePath);
}

std::string resourceDisplayName(const model::ProjectResource& resource)
{
    if (!resource.displayName.empty()) {
        return resource.displayName;
    }

    const std::string fileName = std::filesystem::path{ resource.sourcePath }.filename().string();
    if (!fileName.empty()) {
        return fileName;
    }

    return resource.id;
}

std::string resourceDisplayLabel(const model::ProjectResource& resource)
{
    return resourceDisplayName(resource) + " (" + resource.id + ")";
}

std::string resourceManagerChoiceLabel(const model::ProjectResource& resource)
{
    std::string sourceName = std::filesystem::path{ resource.sourcePath }.filename().string();
    if (sourceName.empty()) {
        sourceName = normalizedPathText(std::filesystem::path{ resource.sourcePath });
    }

    return resourceDisplayName(resource)
        + " | " + resource.id
        + " | " + model::toString(resource.type)
        + " | " + sourceName;
}

std::string defaultResourceExportRelativePath(model::ProjectResourceType type,
    const std::filesystem::path& sourcePath,
    const model::ProjectDocument& document)
{
    const std::string folder = defaultResourceFolder(type);
    const std::string extension = sourcePath.extension().string();
    const std::string baseName = utils::FileUtils::sanitizeFileName(utils::FileUtils::fileStem(sourcePath));

    std::size_t suffix = 0;
    while (true) {
        std::string fileName = baseName;
        if (suffix > 0) {
            fileName += "_" + std::to_string(suffix + 1);
        }
        fileName += extension;

        const std::string candidate = utils::FileUtils::sanitizeRelativeAssetPath(folder + "/" + fileName);
        const auto iterator = std::find_if(document.resources.begin(), document.resources.end(), [&candidate](const model::ProjectResource& resource) {
            return resource.exportRelativePath == candidate;
        });
        if (iterator == document.resources.end()) {
            return candidate;
        }

        ++suffix;
    }
}

std::string imageWidgetPath(const model::WidgetNode& widget)
{
    return widget.getStringProperty("imagePath", widget.getStringProperty("source", {}));
}

void updateDerivedProjectNames(const std::string& previousProjectName,
    const std::string& nextProjectName,
    std::string& executableName,
    std::string& windowTitle)
{
    const std::string previousExecutableName = sanitizeExecutableName(previousProjectName, "VisiFormProject");
    const std::string nextExecutableName = sanitizeExecutableName(nextProjectName, "VisiFormProject");
    if (executableName.empty() || executableName == previousExecutableName) {
        executableName = nextExecutableName;
    }
    if (windowTitle.empty() || windowTitle == previousProjectName) {
        windowTitle = nextProjectName;
    }
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

std::vector<std::string> splitMessageLines(const std::string& text)
{
    std::vector<std::string> lines;
    if (text.empty()) {
        return lines;
    }

    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }

    if (lines.empty()) {
        lines.push_back(text);
    }

    return lines;
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
    case model::WidgetType::ComboBox:
        return "comboBox" + suffix;
    case model::WidgetType::ListBox:
        return "listBox" + suffix;
    case model::WidgetType::CheckBox:
        return "checkBox" + suffix;
    case model::WidgetType::RadioButton:
        return "radioButton" + suffix;
    case model::WidgetType::Slider:
        return "slider" + suffix;
    case model::WidgetType::ScrollBar:
        return "scrollBar" + suffix;
    case model::WidgetType::Frame:
        return "frame" + suffix;
    case model::WidgetType::GroupBox:
        return "groupBox" + suffix;
    case model::WidgetType::Panel:
        return "panel" + suffix;
    case model::WidgetType::TabControl:
        return "tabControl" + suffix;
    case model::WidgetType::StatusBar:
        return "statusBar" + suffix;
    case model::WidgetType::ProgressBar:
        return "progressBar" + suffix;
    case model::WidgetType::ColorPicker:
        return "colorPicker" + suffix;
    case model::WidgetType::ModalDialog:
        return "modalDialog" + suffix;
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

const model::WidgetNode* nearestAncestorOfType(const model::ProjectDocument& document,
    const model::WidgetNode* start,
    model::WidgetType type)
{
    const model::WidgetNode* current = start;
    while (current != nullptr) {
        if (current->type == type) {
            return current;
        }

        current = document.findParentOf(current->id);
    }

    return nullptr;
}

bool isRepairPassParentTarget(const model::WidgetNode& widget)
{
    return widget.type == model::WidgetType::FormWindow
        || widget.type == model::WidgetType::GroupBox
        || widget.type == model::WidgetType::TabPage;
}

std::string insertionParentIdForNewWidget(const model::ProjectDocument& document, model::WidgetType type)
{
    const model::WidgetNode* selectedWidget = document.selectedWidget();
    if (selectedWidget == nullptr) {
        return document.root.id;
    }

    if (type == model::WidgetType::TabControl
        || type == model::WidgetType::MenuBar
        || type == model::WidgetType::ToolBar) {
        return document.root.id;
    }

    if (type != model::WidgetType::GroupBox && type != model::WidgetType::TabPage) {
        if (const auto* groupBox = nearestAncestorOfType(document, selectedWidget, model::WidgetType::GroupBox)) {
            return groupBox->id;
        }
    }

    if (type == model::WidgetType::TabPage) {
        if (const auto* tabControl = document.findTabControlFor(selectedWidget->id)) {
            return tabControl->id;
        }

        return document.root.id;
    }

    if (selectedWidget->type == model::WidgetType::TabControl) {
        if (const auto* selectedTabPage = document.selectedTabPageFor(selectedWidget->id)) {
            return selectedTabPage->id;
        }
    }
    else if (selectedWidget->type == model::WidgetType::TabPage) {
        return selectedWidget->id;
    }
    else if (const auto* tabPage = document.findTabPageFor(selectedWidget->id)) {
        return tabPage->id;
    }

    return document.root.id;
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

model::Rect absoluteBoundsForWidget(const model::ProjectDocument& document, const std::string& widgetId)
{
    const model::WidgetNode* widget = document.findWidgetById(widgetId);
    if (widget == nullptr) {
        return {};
    }

    model::Rect absoluteBounds = widget->bounds;
    const model::WidgetNode* parent = document.findParentOf(widgetId);
    while (parent != nullptr) {
        absoluteBounds.x += parent->bounds.x;
        absoluteBounds.y += parent->bounds.y;
        parent = document.findParentOf(parent->id);
    }

    return absoluteBounds;
}

model::Rect tabPageBoundsForTabControl(const model::WidgetNode& tabControl)
{
    return {
        kTabControlPageInset,
        kTabControlHeaderHeight + kTabControlPageInset,
        std::max(40.0f, tabControl.bounds.width - kTabControlPageInset * 2.0f),
        std::max(40.0f, tabControl.bounds.height - kTabControlHeaderHeight - kTabControlPageInset * 2.0f)
    };
}

model::Rect boundsRelativeToParent(const model::ProjectDocument& document, const std::string& parentId, const model::Rect& absoluteBounds)
{
    if (parentId.empty() || document.isRootWidgetId(parentId)) {
        return absoluteBounds;
    }

    const model::Rect parentAbsoluteBounds = absoluteBoundsForWidget(document, parentId);
    return {
        absoluteBounds.x - parentAbsoluteBounds.x,
        absoluteBounds.y - parentAbsoluteBounds.y,
        absoluteBounds.width,
        absoluteBounds.height
    };
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

bool isChildVisibleForMarqueeSelection(const model::WidgetNode& parent, const model::WidgetNode& child)
{
    if (parent.type != model::WidgetType::TabControl) {
        return true;
    }

    return child.getIntProperty("tabIndex", 0) == std::max(0, parent.getIntProperty("selectedTab", 0));
}

bool isDescendantOf(const model::ProjectDocument& document, const std::string& widgetId, const std::string& ancestorId)
{
    if (widgetId.empty() || ancestorId.empty() || widgetId == ancestorId) {
        return false;
    }

    const model::WidgetNode* parent = document.findParentOf(widgetId);
    while (parent != nullptr) {
        if (parent->id == ancestorId) {
            return true;
        }
        parent = document.findParentOf(parent->id);
    }

    return false;
}

bool groupBoxHasActiveSelection(const model::ProjectDocument& document, const std::string& groupBoxId)
{
    if (groupBoxId.empty()) {
        return false;
    }

    if (document.isSelected(groupBoxId)) {
        return true;
    }

    return std::any_of(document.selectedWidgetIds().begin(), document.selectedWidgetIds().end(),
        [&document, &groupBoxId](const std::string& selectedId) {
            return isDescendantOf(document, selectedId, groupBoxId);
        });
}

std::optional<std::string> resolveMarqueeGroupBoxScope(const model::ProjectDocument& document, const std::string& hitWidgetId)
{
    const model::WidgetNode* current = document.findWidgetById(hitWidgetId);
    while (current != nullptr) {
        if (current->type == model::WidgetType::GroupBox && groupBoxHasActiveSelection(document, current->id)) {
            return current->id;
        }

        current = document.findParentOf(current->id);
    }

    return std::nullopt;
}

bool isPointInGroupBoxContent(const model::ProjectDocument& document, const std::string& groupBoxId, const DesignerCanvas::FormPoint& point)
{
    const model::Rect groupBoxBounds = absoluteBoundsForWidget(document, groupBoxId);
    if (!groupBoxBounds.isValid()) {
        return false;
    }

    const float contentTop = groupBoxBounds.y + std::min(kGroupBoxContentTopInset, groupBoxBounds.height);
    return point.x >= groupBoxBounds.x && point.x <= groupBoxBounds.x + groupBoxBounds.width
        && point.y >= contentTop && point.y <= groupBoxBounds.y + groupBoxBounds.height;
}

std::string formatCanvasCoordinate(float value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << value;
    return stream.str();
}

void writeCanvasInteractionDebug(std::string message)
{
#ifdef _WIN32
    message.insert(0, "[VisiForm] ");
    message.push_back('\n');
    OutputDebugStringA(message.c_str());
#else
    (void)message;
#endif
}

void logCanvasHitDiagnostic(const model::ProjectDocument& document,
    const DesignerCanvas::FormPoint& point,
    const model::WidgetNode* hitWidget,
    bool dragStartCreated,
    std::string_view detail)
{
    if (hitWidget == nullptr) {
        return;
    }

    const model::WidgetNode* parent = document.findParentOf(hitWidget->id);
    const bool hitGroupBox = hitWidget->type == model::WidgetType::GroupBox;
    const bool hitInsideGroupBox = parent != nullptr && parent->type == model::WidgetType::GroupBox;
    if (!hitGroupBox && !hitInsideGroupBox) {
        return;
    }

    const bool isContainer = model::WidgetRegistry::instance().canContainChildren(hitWidget->type);
    std::ostringstream stream;
    stream << "Hit test: (" << formatCanvasCoordinate(point.x) << ", " << formatCanvasCoordinate(point.y) << ") -> "
           << hitWidget->id << " [" << hitWidget->typeName() << "]"
           << " | kind=" << (isContainer ? "container" : "child")
           << " | groupBox=" << (hitGroupBox ? "yes" : "no");
    if (hitInsideGroupBox) {
        stream << " | parent=" << parent->id;
    }
    stream << " | dragStart=" << (dragStartCreated ? "yes" : "no");
    if (!detail.empty()) {
        stream << " | " << detail;
    }

    writeCanvasInteractionDebug(stream.str());
}

bool collectIntersectingWidgetIds(const model::WidgetNode& widget,
    float parentX,
    float parentY,
    const DesignerCanvas::SelectionRect& selectionRect,
    std::vector<std::string>& widgetIds)
{
    const float absoluteX = parentX + widget.bounds.x;
    const float absoluteY = parentY + widget.bounds.y;
    const model::Rect absoluteBounds{ absoluteX, absoluteY, widget.bounds.width, widget.bounds.height };

    bool matchedDescendant = false;
    for (const auto& child : widget.children) {
        if (!isChildVisibleForMarqueeSelection(widget, child)) {
            continue;
        }
        if (collectIntersectingWidgetIds(child, absoluteX, absoluteY, selectionRect, widgetIds)) {
            matchedDescendant = true;
        }
    }

    const bool intersects = widget.type != model::WidgetType::FormWindow && rectsIntersect(absoluteBounds, selectionRect);
    if (intersects && !matchedDescendant) {
        widgetIds.push_back(widget.id);
    }

    return intersects || matchedDescendant;
}

void collectRootIntersectingWidgetIds(const model::ProjectDocument& document,
    const DesignerCanvas::SelectionRect& selectionRect,
    std::vector<std::string>& widgetIds)
{
    for (const auto& child : document.root.children) {
        if (!isChildVisibleForMarqueeSelection(document.root, child)) {
            continue;
        }

        collectIntersectingWidgetIds(child, document.root.bounds.x, document.root.bounds.y, selectionRect, widgetIds);
    }
}

void collectScopedGroupBoxIntersectingWidgetIds(const model::ProjectDocument& document,
    const std::string& groupBoxId,
    const DesignerCanvas::SelectionRect& selectionRect,
    std::vector<std::string>& widgetIds)
{
    const model::WidgetNode* groupBox = document.findWidgetById(groupBoxId);
    if (groupBox == nullptr) {
        return;
    }

    const model::Rect groupBoxBounds = absoluteBoundsForWidget(document, groupBoxId);
    for (const auto& child : groupBox->children) {
        if (!isChildVisibleForMarqueeSelection(*groupBox, child)) {
            continue;
        }

        collectIntersectingWidgetIds(child, groupBoxBounds.x, groupBoxBounds.y, selectionRect, widgetIds);
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
    setTitle(makeWindowTitle(false));
    loadLabelFont();
    loadAppSettings();
    applyCanvasSettings();
    updateLayout();
}

bool MainWindow::newProject()
{
    return openNewProjectWizard();
}

bool MainWindow::openNewProjectWizard()
{
    cancelInspectorEdit();
    cancelEditorModalFieldEdit();
    clearCanvasInteraction();
    requestKeyboardFocus();

    resetNewProjectWizard();
    newProjectWizard_.visible = true;

    editorModal_.visible = true;
    editorModal_.mode = EditorModalMode::NewProjectWizard;
    editorModal_.title = "New Project Wizard";
    editorModal_.message.clear();
    editorModal_.lines.clear();
    editorModal_.buttons = { { "create", "Create" }, { "cancel", "Cancel" } };
    editorModal_.result.clear();
    editorModal_.statusText = templateDescription(newProjectWizard_.templateId);
    editorModal_.preferredWidth = kWizardModalWidth;
    editorModal_.preferredHeight = kWizardModalHeight;
    redraw();
    return true;
}

bool MainWindow::openKeyboardShortcutsDialog()
{
    cancelInspectorEdit();
    cancelEditorModalFieldEdit();
    clearCanvasInteraction();
    requestKeyboardFocus();

    populateKeyboardShortcutsDialog();
    keyboardShortcutDialog_.visible = true;

    editorModal_.visible = true;
    editorModal_.mode = EditorModalMode::KeyboardShortcuts;
    editorModal_.title = "Keyboard Shortcuts";
    editorModal_.message.clear();
    editorModal_.lines.clear();
    editorModal_.buttons = { { "apply_shortcuts", "Apply" }, { "reset_shortcut", "Reset" }, { "close", "Close" } };
    editorModal_.result.clear();
    editorModal_.statusText = "Edit shortcuts using forms like Ctrl+S, Ctrl+Shift+S, Delete, or Alt+F4. Leave a shortcut empty to disable it.";
    editorModal_.preferredWidth = kProjectSettingsModalWidth;
    editorModal_.preferredHeight = kProjectSettingsModalHeight;
    redraw();
    return true;
}

bool MainWindow::openProjectSettingsDialog()
{
    cancelInspectorEdit();
    cancelEditorModalFieldEdit();
    clearCanvasInteraction();
    requestKeyboardFocus();

    populateProjectSettingsDialog();
    projectSettingsDialog_.visible = true;

    editorModal_.visible = true;
    editorModal_.mode = EditorModalMode::ProjectSettings;
    editorModal_.title = "Project Settings";
    editorModal_.message.clear();
    editorModal_.lines.clear();
    editorModal_.buttons = { { "apply", "Apply" }, { "cancel", "Cancel" } };
    editorModal_.result.clear();
    editorModal_.statusText = "Update project naming, look and feel, and local Visage export settings. Use Project > Resources to manage assets.";
    editorModal_.preferredWidth = kProjectSettingsModalWidth;
    editorModal_.preferredHeight = kProjectSettingsModalHeight;
    redraw();
    return true;
}

bool MainWindow::openResourceManagerDialog()
{
    cancelInspectorEdit();
    cancelEditorModalFieldEdit();
    clearCanvasInteraction();
    requestKeyboardFocus();

    populateResourceManagerDialog();
    resourceManagerDialog_.visible = true;

    editorModal_.visible = true;
    editorModal_.mode = EditorModalMode::ResourceManager;
    editorModal_.title = "Resource Manager";
    editorModal_.message.clear();
    editorModal_.lines.clear();
    editorModal_.buttons = { { "add_image", "Add Image" }, { "add_font", "Add Font" }, { "remove_resource", "Remove" }, { "close", "Close" } };
    editorModal_.result.clear();
    editorModal_.statusText = "Manage project assets and exported relative paths.";
    editorModal_.preferredWidth = kProjectSettingsModalWidth;
    editorModal_.preferredHeight = kProjectSettingsModalHeight;
    redraw();
    return true;
}

void MainWindow::resetNewProjectWizard()
{
    newProjectWizard_ = {};
}

void MainWindow::populateProjectSettingsDialog()
{
    projectSettingsDialog_ = {};
    projectSettingsDialog_.projectName = document_.projectName;
    projectSettingsDialog_.executableName = document_.executableName;
    projectSettingsDialog_.userSubclassName = document_.userSubclassName;
    projectSettingsDialog_.windowTitle = document_.windowTitle;
    projectSettingsDialog_.lookAndFeelId = document_.lookAndFeelId.empty() ? std::string{ "VisiFormDark" } : document_.lookAndFeelId;
    projectSettingsDialog_.localVisageSourceDirectory = normalizedPathText(settings_.localVisageSourceDirectory);
    projectSettingsDialog_.visageGitRepository = settings_.visageGitRepository;
    projectSettingsDialog_.visageGitTag = settings_.visageGitTag;
}

void MainWindow::populateResourceManagerDialog()
{
    resourceManagerDialog_.confirmReferencedRemoval = false;
    if (resourceManagerDialog_.selectedResourceId.empty()
        || document_.findResourceById(resourceManagerDialog_.selectedResourceId) == nullptr) {
        resourceManagerDialog_.selectedResourceId = document_.resources.empty()
            ? std::string{}
            : document_.resources.front().id;
    }

    refreshResourceManagerPreview();
}

void MainWindow::populateKeyboardShortcutsDialog()
{
    keyboardShortcutDialog_ = {};
    keyboardShortcutDialog_.pendingShortcuts = settings_.keyboardShortcuts;
    const auto& definitions = commands::CommandRegistry::definitions();
    if (!definitions.empty()) {
        keyboardShortcutDialog_.selectedCommandId = std::string{ definitions.front().id };
    }
}

void MainWindow::refreshResourceManagerPreview()
{
    resourceManagerDialog_.previewSourcePath.clear();
    resourceManagerDialog_.previewImageAvailable = false;
    resourceManagerDialog_.previewImageWidth = 0;
    resourceManagerDialog_.previewImageHeight = 0;
    resourceManagerDialog_.previewStatus.clear();

    if (resourceManagerDialog_.selectedResourceId.empty()) {
        resourceManagerDialog_.previewStatus = "No resource selected.";
        return;
    }

    const auto* selectedResource = document_.findResourceById(resourceManagerDialog_.selectedResourceId);
    if (selectedResource == nullptr) {
        resourceManagerDialog_.previewStatus = "Selected resource is no longer available.";
        return;
    }

    if (selectedResource->type != model::ProjectResourceType::Image) {
        resourceManagerDialog_.previewStatus = "Preview is available for image resources only.";
        return;
    }

    const std::filesystem::path sourcePath{ selectedResource->sourcePath };
    resourceManagerDialog_.previewSourcePath = normalizedPathText(sourcePath);

    const auto cachedImage = imageResourceCache_.getOrLoad(sourcePath);
    resourceManagerDialog_.previewImageAvailable = cachedImage.info.available
        && cachedImage.encodedBytes != nullptr
        && !cachedImage.encodedBytes->empty();
    resourceManagerDialog_.previewImageWidth = cachedImage.info.width;
    resourceManagerDialog_.previewImageHeight = cachedImage.info.height;

    if (resourceManagerDialog_.previewImageAvailable) {
        resourceManagerDialog_.previewStatus = "Scaled preview from " + normalizedPathText(sourcePath.filename());
        return;
    }

    if (!cachedImage.info.error.empty()) {
        resourceManagerDialog_.previewStatus = cachedImage.info.error;
        return;
    }

    resourceManagerDialog_.previewStatus = "No preview available for " + normalizedPathText(sourcePath.filename());
}

bool MainWindow::addResourceFromDialog(model::ProjectResourceType resourceType)
{
    const std::filesystem::path initialDirectory = !settings_.lastProjectDirectory.empty() && std::filesystem::exists(settings_.lastProjectDirectory)
        ? settings_.lastProjectDirectory
        : projectRootPath();

    std::optional<std::filesystem::path> selectedPath;
    if (resourceType == model::ProjectResourceType::Image) {
        selectedPath = utils::showOpenImageResourceDialog(initialDirectory);
    }
    else if (resourceType == model::ProjectResourceType::Font) {
        selectedPath = utils::showOpenFontResourceDialog(initialDirectory);
    }

    if (!selectedPath.has_value()) {
        editorModal_.statusText = "Resource selection cancelled.";
        redraw();
        return false;
    }

    model::ProjectResource resource;
    resource.id = idGenerator_.next(resourceType, document_);
    resource.type = resourceType;
    resource.displayName = defaultResourceDisplayName(*selectedPath);
    resource.sourcePath = normalizedPathText(*selectedPath);
    resource.exportRelativePath = defaultResourceExportRelativePath(resourceType, *selectedPath, document_);
    applyUndoableDocumentChange("Add resource", [this, &resource]() {
        document_.resources.push_back(resource);
        return true;
    });

    resourceManagerDialog_.selectedResourceId = resource.id;
    resourceManagerDialog_.confirmReferencedRemoval = false;
    refreshResourceManagerPreview();
    editorModal_.statusText = "Added " + resourceTypeDisplayName(resourceType) + " resource: " + resourceDisplayLabel(resource);
    redraw();
    return true;
}

std::string MainWindow::keyboardShortcutDialogEffectiveText(const std::string& commandId) const
{
    if (commandId.empty()) {
        return {};
    }

    if (const auto iterator = keyboardShortcutDialog_.pendingShortcuts.find(commandId); iterator != keyboardShortcutDialog_.pendingShortcuts.end()) {
        return iterator->second;
    }

    if (const auto* definition = commands::CommandRegistry::find(commandId)) {
        return std::string{ definition->defaultShortcut };
    }

    return {};
}

std::string MainWindow::validateKeyboardShortcutDialog() const
{
    std::map<std::string, std::string> normalizedByGesture;
    for (const auto& definition : commands::CommandRegistry::definitions()) {
        const std::string commandId{ definition.id };
        std::string shortcut = keyboardShortcutDialogEffectiveText(commandId);
        if (shortcut.empty()) {
            continue;
        }

        const auto parsedShortcut = commands::CommandRegistry::parseShortcutString(shortcut);
        if (!parsedShortcut.has_value()) {
            return std::string{ definition.displayName } + ": invalid shortcut format.";
        }

        shortcut = commands::CommandRegistry::formatShortcut(*parsedShortcut);
        if (const auto iterator = normalizedByGesture.find(shortcut); iterator != normalizedByGesture.end()) {
            const auto* existingDefinition = commands::CommandRegistry::find(iterator->second);
            const std::string existingName = existingDefinition != nullptr
                ? std::string{ existingDefinition->displayName }
                : iterator->second;
            return std::string{ definition.displayName } + " conflicts with " + existingName + ".";
        }

        normalizedByGesture.emplace(shortcut, commandId);
    }

    return {};
}

bool MainWindow::applyKeyboardShortcutsDialog()
{
    const std::string validationError = validateKeyboardShortcutDialog();
    if (!validationError.empty()) {
        editorModal_.statusText = validationError;
        redraw();
        return false;
    }

    std::map<std::string, std::string> normalizedOverrides;
    for (const auto& definition : commands::CommandRegistry::definitions()) {
        const std::string commandId{ definition.id };
        const std::string pendingShortcut = keyboardShortcutDialogEffectiveText(commandId);
        const auto parsedShortcut = pendingShortcut.empty()
            ? std::optional<commands::ShortcutGesture>{}
            : commands::CommandRegistry::parseShortcutString(pendingShortcut);
        const std::string normalizedShortcut = parsedShortcut.has_value()
            ? commands::CommandRegistry::formatShortcut(*parsedShortcut)
            : std::string{};
        const auto parsedDefault = std::string{ definition.defaultShortcut }.empty()
            ? std::optional<commands::ShortcutGesture>{}
            : commands::CommandRegistry::parseShortcutString(definition.defaultShortcut);
        const std::string normalizedDefault = parsedDefault.has_value()
            ? commands::CommandRegistry::formatShortcut(*parsedDefault)
            : std::string{};

        if (normalizedShortcut == normalizedDefault) {
            continue;
        }

        if (normalizedShortcut.empty() || !pendingShortcut.empty()) {
            normalizedOverrides[commandId] = normalizedShortcut;
        }
    }

    settings_.keyboardShortcuts = normalizedOverrides;
    saveAppSettings();
    keyboardShortcutDialog_.pendingShortcuts = settings_.keyboardShortcuts;
    editorModal_.statusText = "Keyboard shortcuts updated.";
    redraw();
    return true;
}

void MainWindow::resetSelectedKeyboardShortcut()
{
    if (keyboardShortcutDialog_.selectedCommandId.empty()) {
        return;
    }

    keyboardShortcutDialog_.pendingShortcuts.erase(keyboardShortcutDialog_.selectedCommandId);
    editorModal_.statusText = "Shortcut reset to default.";
    redraw();
}

bool MainWindow::removeSelectedResourceFromManager()
{
    populateResourceManagerDialog();
    if (resourceManagerDialog_.selectedResourceId.empty()) {
        editorModal_.statusText = "No resource is selected.";
        redraw();
        return false;
    }

    const auto referencedBy = document_.widgetIdsReferencingResource(resourceManagerDialog_.selectedResourceId);
    if (!referencedBy.empty() && !resourceManagerDialog_.confirmReferencedRemoval) {
        resourceManagerDialog_.confirmReferencedRemoval = true;
        editorModal_.statusText = "Resource is used by " + std::to_string(referencedBy.size()) + " widget(s). Click Remove again to confirm.";
        redraw();
        return false;
    }

    const std::string removedResourceId = resourceManagerDialog_.selectedResourceId;
    if (!applyUndoableDocumentChange("Remove resource", [this, &removedResourceId]() {
            return document_.removeResourceById(removedResourceId);
        })) {
        editorModal_.statusText = "Failed to remove the selected resource.";
        redraw();
        return false;
    }

    resourceManagerDialog_.selectedResourceId.clear();
    populateResourceManagerDialog();
    editorModal_.statusText = "Removed resource: " + removedResourceId;
    redraw();
    return true;
}

bool MainWindow::applyUndoableDocumentChange(const std::string& description, const std::function<bool()>& applyChange)
{
    model::ProjectDocument beforeDocument = document_;
    if (!applyChange()) {
        return false;
    }

    model::ProjectDocument afterDocument = document_;
    document_ = beforeDocument;
    undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
        document_,
        description,
        std::move(beforeDocument),
        std::move(afterDocument)));
    document_.markDirty();
    return true;
}

std::string MainWindow::validateNewProjectWizard() const
{
    const std::string projectName = trimWhitespace(newProjectWizard_.projectName);
    const std::string executableName = trimWhitespace(newProjectWizard_.executableName);
    const std::string userSubclassName = trimWhitespace(newProjectWizard_.userSubclassName);
    if (projectName.empty()) {
        return "Project name cannot be empty.";
    }
    if (executableName.empty()) {
        return "Executable name cannot be empty.";
    }
    if (sanitizeExecutableName(executableName, projectName) != executableName) {
        return "Executable name must use only letters, numbers, underscores, or hyphens.";
    }
    if (userSubclassName.empty() || !utils::isValidCppIdentifier(userSubclassName)) {
        return "User subclass name must be a valid C++ identifier.";
    }
    if (userSubclassName == "MainWindow") {
        return "User subclass name must not be MainWindow.";
    }
    if (newProjectWizard_.formWidth < 200 || newProjectWizard_.formWidth > 4000
        || newProjectWizard_.formHeight < 160 || newProjectWizard_.formHeight > 4000) {
        return "Form width and height must stay within a reasonable range.";
    }
    if (model::LookAndFeelRegistry::instance().findById(newProjectWizard_.lookAndFeelId) == nullptr) {
        return "Choose a valid look and feel preset.";
    }
    if (!containsText(newProjectTemplateIds(), newProjectWizard_.templateId)) {
        return "Choose a valid project template.";
    }
    return {};
}

std::string MainWindow::validateProjectSettingsDialog() const
{
    const std::string projectName = trimWhitespace(projectSettingsDialog_.projectName);
    const std::string executableName = trimWhitespace(projectSettingsDialog_.executableName);
    const std::string userSubclassName = trimWhitespace(projectSettingsDialog_.userSubclassName);
    if (projectName.empty()) {
        return "Project name cannot be empty.";
    }
    if (executableName.empty()) {
        return "Executable name cannot be empty.";
    }
    if (sanitizeExecutableName(executableName, projectName) != executableName) {
        return "Executable name must use only letters, numbers, underscores, or hyphens.";
    }
    if (userSubclassName.empty() || !utils::isValidCppIdentifier(userSubclassName)) {
        return "User subclass name must be a valid C++ identifier.";
    }
    if (userSubclassName == "MainWindow") {
        return "User subclass name must not be MainWindow.";
    }
    if (model::LookAndFeelRegistry::instance().findById(projectSettingsDialog_.lookAndFeelId) == nullptr) {
        return "Choose a valid look and feel preset.";
    }
    return {};
}

model::ProjectDocument MainWindow::createDocumentFromWizard()
{
    model::ProjectDocument document = model::ProjectDocument::createDefault();
    document.projectName = trimWhitespace(newProjectWizard_.projectName);
    document.executableName = trimWhitespace(newProjectWizard_.executableName);
    document.generatedBaseClassName = "MainWindow";
    document.userSubclassName = trimWhitespace(newProjectWizard_.userSubclassName);
    document.mainFormClassName = document.userSubclassName;
    document.windowTitle = trimWhitespace(newProjectWizard_.windowTitle);
    if (document.windowTitle.empty()) {
        document.windowTitle = document.projectName;
    }
    document.lookAndFeelId = newProjectWizard_.lookAndFeelId;
    document.root = model::WidgetRegistry::instance().createDefaultWidget(model::WidgetType::FormWindow, "form_main");
    document.root.name = "MainWindow";
    document.root.bounds = { 0.0f, 0.0f, static_cast<float>(newProjectWizard_.formWidth), static_cast<float>(newProjectWizard_.formHeight) };
    document.root.children.clear();
    document.root.setProperty("title", document.windowTitle);
    document.selectedWidgetId = document.root.id;
    document.clearSelection();
    document.setSelection(document.root.id);
    applyWizardTemplate(document, newProjectWizard_.templateId);
    document.normalizeRadioGroups();
    document.markDirty();
    return document;
}

void MainWindow::applyWizardTemplate(model::ProjectDocument& document, const std::string& templateId)
{
    auto addWidget = [this, &document](model::WidgetType type, const model::Rect& bounds, const auto& configure) {
        const std::string id = idGenerator_.next(type, document);
        model::WidgetNode widget = model::WidgetRegistry::instance().createDefaultWidget(type, id);
        widget.bounds = bounds;
        configure(widget);
        document.root.children.push_back(std::move(widget));
    };

    auto addStatusBar = [&]() {
        addWidget(model::WidgetType::StatusBar,
            { 0.0f, document.root.bounds.height - 50.0f, document.root.bounds.width, 50.0f },
            [](model::WidgetNode& widget) {
                widget.setProperty("dock", "Bottom");
                widget.setProperty("fillWidth", true);
                widget.setProperty("fields", 1);
                widget.setProperty("text0", "Ready");
            });
    };

    if (templateId == "blank") {
        return;
    }

    if (templateId == "basic_app") {
        addWidget(model::WidgetType::Label, { 40.0f, 40.0f, 280.0f, 58.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("text", "Welcome");
        });
        addWidget(model::WidgetType::Button, { 40.0f, 120.0f, 180.0f, 52.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("text", "Click Me");
        });
        addStatusBar();
        return;
    }

    if (templateId == "form_with_status") {
        addWidget(model::WidgetType::Label, { 40.0f, 40.0f, 320.0f, 58.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("text", "Processing status");
        });
        addWidget(model::WidgetType::Button, { 40.0f, 118.0f, 180.0f, 52.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("text", "Start Task");
        });
        addWidget(model::WidgetType::ProgressBar, { 40.0f, 190.0f, 320.0f, 32.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("value", 35);
            widget.setProperty("showText", true);
        });
        addStatusBar();
        return;
    }

    if (templateId == "control_panel") {
        addWidget(model::WidgetType::Button, { 40.0f, 40.0f, 180.0f, 52.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("text", "Apply");
        });
        addWidget(model::WidgetType::CheckBox, { 40.0f, 110.0f, 220.0f, 62.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("text", "Enable Option");
            widget.setProperty("checked", true);
        });
        addWidget(model::WidgetType::RadioButton, { 40.0f, 184.0f, 220.0f, 48.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("text", "Mode A");
            widget.setProperty("group", "mode");
            widget.setProperty("selected", true);
        });
        addWidget(model::WidgetType::RadioButton, { 40.0f, 236.0f, 220.0f, 48.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("text", "Mode B");
            widget.setProperty("group", "mode");
            widget.setProperty("selected", false);
        });
        addWidget(model::WidgetType::RadioButton, { 40.0f, 288.0f, 220.0f, 48.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("text", "Mode C");
            widget.setProperty("group", "mode");
            widget.setProperty("selected", false);
        });
        addWidget(model::WidgetType::Slider, { 300.0f, 110.0f, 220.0f, 40.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("value", 65);
        });
        addWidget(model::WidgetType::ScrollBar, { 300.0f, 180.0f, 220.0f, 28.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("value", 20);
        });
        addStatusBar();
        return;
    }

    if (templateId == "dialog_test") {
        addWidget(model::WidgetType::Button, { 40.0f, 40.0f, 220.0f, 52.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("text", "Show Dialog");
            widget.setProperty("onClick", "handleShowDialog");
        });
        addWidget(model::WidgetType::ModalDialog, { 120.0f, 140.0f, 420.0f, 220.0f }, [](model::WidgetNode& widget) {
            widget.setProperty("title", "Preview Dialog");
            widget.setProperty("message", "Hello from the dialog template.");
            widget.setProperty("buttons", "OK,Cancel");
            widget.setProperty("visibleAtStartup", false);
        });
        addStatusBar();
    }
}

bool MainWindow::applyNewProjectWizard()
{
    const std::string validationError = validateNewProjectWizard();
    if (!validationError.empty()) {
        editorModal_.statusText = validationError;
        redraw();
        return false;
    }

    if (!confirmSaveIfDirty()) {
        return false;
    }

    cancelInspectorEdit();
    cancelEditorModalFieldEdit();
    idGenerator_ = {};
    document_ = createDocumentFromWizard();
    normalizeWidgetBoundsForEditor();
    currentProjectPath_.clear();
    undoRedo_.clear();
    document_.setSelection(document_.root.id);
    setOperationStatus("Created new project: " + document_.projectName);
    closeEditorModalDialog("create");
    redraw();
    return true;
}

bool MainWindow::applyProjectSettingsDialog()
{
    const std::string validationError = validateProjectSettingsDialog();
    if (!validationError.empty()) {
        editorModal_.statusText = validationError;
        redraw();
        return false;
    }

    bool projectChanged = false;
    bool settingsChanged = false;

    const std::string projectName = trimWhitespace(projectSettingsDialog_.projectName);
    const std::string executableName = trimWhitespace(projectSettingsDialog_.executableName);
    const std::string userSubclassName = trimWhitespace(projectSettingsDialog_.userSubclassName);
    const std::string windowTitle = trimWhitespace(projectSettingsDialog_.windowTitle).empty()
        ? projectName
        : trimWhitespace(projectSettingsDialog_.windowTitle);

    if (document_.projectName != projectName) {
        document_.projectName = projectName;
        projectChanged = true;
    }
    if (document_.executableName != executableName) {
        document_.executableName = executableName;
        projectChanged = true;
    }
    if (document_.userSubclassName != userSubclassName || document_.mainFormClassName != userSubclassName) {
        document_.userSubclassName = userSubclassName;
        document_.mainFormClassName = userSubclassName;
        document_.generatedBaseClassName = "MainWindow";
        projectChanged = true;
    }
    if (document_.windowTitle != windowTitle) {
        document_.windowTitle = windowTitle;
        document_.root.setProperty("title", document_.windowTitle);
        projectChanged = true;
    }
    if (document_.lookAndFeelId != projectSettingsDialog_.lookAndFeelId) {
        document_.lookAndFeelId = projectSettingsDialog_.lookAndFeelId;
        projectChanged = true;
    }

    const std::filesystem::path localVisageSourceDirectory = trimWhitespace(projectSettingsDialog_.localVisageSourceDirectory).empty()
        ? std::filesystem::path{}
        : std::filesystem::path{ utils::FileUtils::normalizeSeparators(trimWhitespace(projectSettingsDialog_.localVisageSourceDirectory)) };
    if (settings_.localVisageSourceDirectory != localVisageSourceDirectory) {
        settings_.localVisageSourceDirectory = localVisageSourceDirectory;
        settingsChanged = true;
    }

    const std::string visageGitRepository = trimWhitespace(projectSettingsDialog_.visageGitRepository).empty()
        ? std::string{ utils::AppSettings::defaultVisageGitRepository }
        : trimWhitespace(projectSettingsDialog_.visageGitRepository);
    if (settings_.visageGitRepository != visageGitRepository) {
        settings_.visageGitRepository = visageGitRepository;
        settingsChanged = true;
    }

    const std::string visageGitTag = trimWhitespace(projectSettingsDialog_.visageGitTag).empty()
        ? std::string{ utils::AppSettings::defaultVisageGitTag }
        : trimWhitespace(projectSettingsDialog_.visageGitTag);
    if (settings_.visageGitTag != visageGitTag) {
        settings_.visageGitTag = visageGitTag;
        settingsChanged = true;
    }

    if (projectChanged) {
        document_.markDirty();
    }
    if (settingsChanged) {
        saveAppSettings();
    }

    closeEditorModalDialog("apply");
    setOperationStatus("Project settings updated.");
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
        showEditorValidationDialog(validationResult.report, validationReportPathText, validationResult.reportWriteError);
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
    showEditorValidationDialog(validationResult.report, validationReportPathText, validationResult.reportWriteError);
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
    const bool defaultProjectDirectoryReady = utils::FileUtils::ensureDirectoryExists(defaultProjectDir, ensureDirectoryError);
    (void)defaultProjectDirectoryReady;
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
    updateEditorModalEditorBounds();
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

    drawMenuBar(canvas);
    drawToolbar(canvas);
    widgetPalette_.draw(canvas, labelFont_, canDrawText());
    const bool simplifySelectedImages = canvasInteraction_.mode == CanvasInteractionState::Mode::Move
        || canvasInteraction_.mode == CanvasInteractionState::Mode::Resize;
    designerCanvas_.draw(canvas, labelFont_, canDrawText(), document_, &imageResourceCache_, simplifySelectedImages, marqueeRect, canvasInteraction_.smartGuides);
    propertyInspector_.draw(canvas, labelFont_, canDrawText(), document_, settings_, document_.selectedWidgetIds().size());
    if (layout_.showProjectTree) {
        projectTree_.drawPanel(canvas, labelFont_, canDrawText(), document_);
    }
    drawStatusBar(canvas);
    if (openMenuIndex_ >= 0) {
        drawMenuBar(canvas);
    }
    if (isEditorModalVisible()) {
        drawEditorModalDialog(canvas);
    }
    textEditControl_.draw(canvas, labelFont_, canDrawText());
    dropdownControl_.draw(canvas, labelFont_, canDrawText());
}

void MainWindow::mouseDown(const visage::MouseEvent& e)
{
    if (!e.isLeftButton()) {
        return;
    }

    requestKeyboardFocus();

    if (isEditorModalVisible()) {
        handleEditorModalMouseDown(e);
        return;
    }

    if (dropdownControl_.isOpen()) {
        const bool handledDropdownClick = dropdownControl_.mouseDown(e.position.x, e.position.y);
        handleDropdownSelection();
        if (handledDropdownClick) {
            redraw();
            return;
        }
    }

    if (textEditControl_.isActive()) {
        if (textEditControl_.mouseDown(e.position.x, e.position.y)) {
            redraw();
            return;
        }
        if (!commitInspectorEdit()) {
            return;
        }
    }

    if (handleMenuMouseDown(e)) {
        return;
    }

    if (const auto button = toolbarButtonAt(e.position.x, e.position.y)) {
        if (isCommandEnabled(button->command)) {
            executeCommand(button->command);
        }
        redraw();
        return;
    }

    if (widgetPalette_.mouseDown(e.position.x, e.position.y)) {
        redraw();
        return;
    }

    if (const auto widgetType = widgetPalette_.hitTestWidgetType(e.position.x, e.position.y)) {
        cancelInspectorEdit();
        addWidgetFromPalette(*widgetType);
        return;
    }

    if (e.isLeftButton() && propertyInspector_.mouseDown(document_, settings_, e.position.x, e.position.y)) {
        applyPendingInspectorInteractionEdit();
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    if (layout_.showProjectTree && projectTree_.mouseDown(document_, e.position.x, e.position.y)) {
        redraw();
        return;
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
            textEditControl_.clear();
            dropdownControl_.close();
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
            if (row->editKind == PropertyInspector::PropertyEditKind::Choice) {
                beginInspectorEdit(*row);
            }
            else {
                beginInspectorEdit(*row);
            }
        }
        return;
    }

    if (layout_.showProjectTree) {
        const bool additiveSelection = multiSelectMode_ || isAdditiveSelectionModifierDown();
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
        const bool additiveSelection = multiSelectMode_ || isAdditiveSelectionModifierDown();
        if (const auto* hitWidget = document_.findWidgetById(*widgetId); hitWidget != nullptr && hitWidget->type == model::WidgetType::TabControl) {
            if (const auto tabIndex = designerCanvas_.hitTestTabHeader(document_, *widgetId, e.position.x, e.position.y)) {
                if (!document_.isSelected(*widgetId)) {
                    handleWidgetClicked(*widgetId, additiveSelection);
                }
                if (hitWidget->selectedTabIndex() != *tabIndex) {
                    applyUndoableDocumentChange("Change selected tab", [this, widgetId, tabIndex]() {
                        if (auto* tabControl = document_.findWidgetById(*widgetId)) {
                            tabControl->setSelectedTabIndex(*tabIndex);
                            return true;
                        }
                        return false;
                    });
                    setOperationStatus("Selected tab " + std::to_string(*tabIndex + 1) + " on " + widgetDisplayName(*hitWidget));
                }
                redraw();
                return;
            }
        }

        if (*widgetId == document_.root.id) {
            if (const auto dragStart = designerCanvas_.toFormPoint(document_, e.position.x, e.position.y)) {
                clearCanvasInteraction();
                canvasInteraction_.mode = CanvasInteractionState::Mode::MarqueeSelect;
                canvasInteraction_.marqueeScopeWidgetId = document_.root.id;
                canvasInteraction_.dragStart = *dragStart;
                canvasInteraction_.currentPoint = *dragStart;
                canvasInteraction_.changed = false;
                redraw();
            }
            return;
        }

        const auto dragStart = designerCanvas_.toFormPoint(document_, e.position.x, e.position.y);
        if (!dragStart.has_value()) {
            clearCanvasInteraction();
            logCanvasHitDiagnostic(document_, DesignerCanvas::FormPoint{}, document_.findWidgetById(*widgetId), false, "Drag blocked: click is outside form preview");
            return;
        }

        const auto marqueeGroupBoxId = resolveMarqueeGroupBoxScope(document_, *widgetId);
        if (additiveSelection
            && marqueeGroupBoxId.has_value()
            && *widgetId == *marqueeGroupBoxId
            && isPointInGroupBoxContent(document_, *marqueeGroupBoxId, *dragStart)) {
            clearCanvasInteraction();
            canvasInteraction_.mode = CanvasInteractionState::Mode::MarqueeSelect;
            canvasInteraction_.marqueeScopeWidgetId = *marqueeGroupBoxId;
            canvasInteraction_.dragStart = *dragStart;
            canvasInteraction_.currentPoint = *dragStart;
            canvasInteraction_.changed = false;
            logCanvasHitDiagnostic(document_, *dragStart, document_.findWidgetById(*widgetId), true, "Marquee start: GroupBox scope");
            redraw();
            return;
        }

        const bool clickedPrimarySelected = document_.selectedWidgetId == *widgetId;
        const bool keepMultiSelectionForDrag = multiSelectMode_ && clickedPrimarySelected;

        if (!keepMultiSelectionForDrag) {
            handleWidgetClicked(*widgetId, additiveSelection);

            if (additiveSelection) {
                clearCanvasInteraction();
                logCanvasHitDiagnostic(document_, *dragStart, document_.findWidgetById(*widgetId), false, "Drag blocked: additive selection toggle");
                return;
            }
        }

        if (document_.selectedWidgetId == *widgetId && *widgetId != document_.root.id) {
            const auto interactionHit = designerCanvas_.hitTestInteraction(document_, e.position.x, e.position.y, *widgetId);
            auto* widget = document_.findWidgetById(*widgetId);
            if (interactionHit.has_value() && widget != nullptr) {
                canvasInteraction_.widgetId = *widgetId;
                canvasInteraction_.originalParentId = document_.findParentOf(*widgetId) != nullptr
                    ? document_.findParentOf(*widgetId)->id
                    : document_.root.id;
                canvasInteraction_.region = interactionHit->region;
                canvasInteraction_.originalBounds = widget->bounds;
                canvasInteraction_.dragStart = *dragStart;
                canvasInteraction_.currentPoint = *dragStart;
                canvasInteraction_.dropTargetWidgetId = canvasInteraction_.originalParentId;
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

                const std::string interactionDetail = interactionHit->region == DesignerCanvas::HitRegion::Body
                    ? "Drag start: move"
                    : "Drag start: resize";
                logCanvasHitDiagnostic(document_, *dragStart, widget, true, interactionDetail);
            }
            else {
                clearCanvasInteraction();
                logCanvasHitDiagnostic(document_, *dragStart, document_.findWidgetById(*widgetId), false, "Drag blocked: interaction hit missing");
            }
        }
        else {
            clearCanvasInteraction();
            logCanvasHitDiagnostic(document_, *dragStart, document_.findWidgetById(*widgetId), false, "Drag blocked: widget is not primary selection");
        }

        return;
    }

    if (const auto dragStart = designerCanvas_.toFormPoint(document_, e.position.x, e.position.y)) {
        clearCanvasInteraction();
        canvasInteraction_.mode = CanvasInteractionState::Mode::MarqueeSelect;
        canvasInteraction_.marqueeScopeWidgetId = document_.root.id;
        canvasInteraction_.dragStart = *dragStart;
        canvasInteraction_.currentPoint = *dragStart;
        canvasInteraction_.changed = false;
        redraw();
    }
}

void MainWindow::mouseMove(const visage::MouseEvent& e)
{
    if (isEditorModalVisible()) {
        return;
    }

    if (openMenuIndex_ >= 0) {
        updateHoverHint(e.position.x, e.position.y);
        return;
    }

    if (canvasInteraction_.mode != CanvasInteractionState::Mode::None) {
        return;
    }

    updateHoverHint(e.position.x, e.position.y);
}

void MainWindow::mouseDrag(const visage::MouseEvent& e)
{
    if (isEditorModalVisible()) {
        return;
    }

    if (openMenuIndex_ >= 0) {
        return;
    }

    if (widgetPalette_.mouseDrag(e.position.x, e.position.y)) {
        redraw();
        return;
    }

    if (propertyInspector_.mouseDrag(document_, settings_, e.position.x, e.position.y)) {
        applyPendingInspectorInteractionEdit();
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

        if (canvasInteraction_.selectionBounds.size() <= 1) {
            canvasInteraction_.dropTargetWidgetId = resolveDropParentId(widget->id, e.position.x, e.position.y);
            if (!canvasInteraction_.dropTargetWidgetId.empty()) {
                const auto* dropTarget = document_.findWidgetById(canvasInteraction_.dropTargetWidgetId);
                if (dropTarget != nullptr && canvasInteraction_.dropTargetWidgetId != canvasInteraction_.originalParentId) {
                    setOperationStatus("Drop into " + widgetDisplayName(*dropTarget));
                }
            }
        }
    }
    else if (canvasInteraction_.mode == CanvasInteractionState::Mode::Resize) {
        canvasInteraction_.smartGuides.clear();
        canvasInteraction_.smartGuideSnapUsed = false;
        canvasInteraction_.dropTargetWidgetId.clear();
        updatedBounds = designerCanvas_.resizeBounds(canvasInteraction_.originalBounds, canvasInteraction_.region,
            canvasInteraction_.dragStart, *currentPoint);
    }

    if (updatedBounds.x != widget->bounds.x || updatedBounds.y != widget->bounds.y
        || updatedBounds.width != widget->bounds.width || updatedBounds.height != widget->bounds.height) {
        widget->bounds = updatedBounds;
        if (widget->type == model::WidgetType::TabControl) {
            document_.refreshHierarchyMetadata();
        }
        canvasInteraction_.changed = true;
        redraw();
    }
}

void MainWindow::mouseUp(const visage::MouseEvent& e)
{
    if (isEditorModalVisible()) {
        return;
    }

    if (openMenuIndex_ >= 0) {
        return;
    }

    const bool releasedPaletteScrollBar = widgetPalette_.mouseUp();
    if (releasedPaletteScrollBar && canvasInteraction_.mode == CanvasInteractionState::Mode::None) {
        redraw();
        return;
    }

    const bool releasedInspectorScrollBar = propertyInspector_.mouseUp();
    if (releasedInspectorScrollBar && canvasInteraction_.mode == CanvasInteractionState::Mode::None) {
        applyPendingInspectorInteractionEdit();
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

    if (canvasInteraction_.mode == CanvasInteractionState::Mode::MarqueeSelect) {
        const DesignerCanvas::SelectionRect selectionRect = normalizedSelectionRect(canvasInteraction_.dragStart, canvasInteraction_.currentPoint);
        const std::string marqueeScopeWidgetId = canvasInteraction_.marqueeScopeWidgetId.empty()
            ? document_.root.id
            : canvasInteraction_.marqueeScopeWidgetId;
        const bool groupBoxScope = !document_.isRootWidgetId(marqueeScopeWidgetId);
        const std::string statusSuffix = groupBoxScope ? " in " + marqueeScopeWidgetId : std::string{};
        if (!isMeaningfulMarquee(selectionRect)) {
            document_.setSelection(groupBoxScope ? marqueeScopeWidgetId : document_.root.id);
            setOperationStatus("Selected 0 widgets" + statusSuffix);
        }
        else {
            std::vector<std::string> intersectingIds;
            if (groupBoxScope) {
                collectScopedGroupBoxIntersectingWidgetIds(document_, marqueeScopeWidgetId, selectionRect, intersectingIds);
            }
            else {
                collectRootIntersectingWidgetIds(document_, selectionRect, intersectingIds);
            }

            if (intersectingIds.empty()) {
                document_.setSelection(groupBoxScope ? marqueeScopeWidgetId : document_.root.id);
                setOperationStatus("Selected 0 widgets" + statusSuffix);
            }
            else {
                document_.clearSelection();
                for (const auto& id : intersectingIds) {
                    document_.addToSelection(id);
                }
                setOperationStatus("Selected " + std::to_string(intersectingIds.size()) + " widgets" + statusSuffix);
            }
        }

        clearCanvasInteraction();
        redraw();
        return;
    }

    auto* widget = document_.findWidgetById(canvasInteraction_.widgetId);
    if (canvasInteraction_.changed && widget != nullptr) {
        if (canvasInteraction_.mode == CanvasInteractionState::Mode::Move && canvasInteraction_.selectionBounds.size() > 1) {
            model::ProjectDocument afterDocument = document_;
            model::ProjectDocument beforeDocument = document_;
            for (const auto& snapshot : canvasInteraction_.selectionBounds) {
                if (auto* selectedWidget = beforeDocument.findWidgetById(snapshot.widgetId)) {
                    selectedWidget->bounds = snapshot.originalBounds;
                }
            }

            document_ = beforeDocument;
            undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
                document_,
                "Move widgets",
                std::move(beforeDocument),
                std::move(afterDocument)));
            document_.markDirty();
            setOperationStatus(canvasInteraction_.smartGuideSnapUsed
                ? "Moved " + std::to_string(canvasInteraction_.selectionBounds.size()) + " widgets with smart guide snap"
                : "Moved " + std::to_string(canvasInteraction_.selectionBounds.size()) + " widgets");
            clearCanvasInteraction();
            redraw();
            return;
        }

        const model::Rect finalBounds = widget->bounds;
        const std::string widgetId = widget->id;
        const std::string displayName = widget->name.empty() ? widgetId : widget->name;
        const model::Rect finalAbsoluteBounds = absoluteBoundsForWidget(document_, widgetId);
        widget->bounds = canvasInteraction_.originalBounds;
        if (canvasInteraction_.mode == CanvasInteractionState::Mode::Move) {
            const std::string dropParentId = canvasInteraction_.dropTargetWidgetId.empty()
                ? canvasInteraction_.originalParentId
                : canvasInteraction_.dropTargetWidgetId;
            if (!dropParentId.empty() && dropParentId != canvasInteraction_.originalParentId) {
                model::ProjectDocument beforeDocument = document_;
                model::ProjectDocument afterDocument = document_;
                if (auto* movedWidget = beforeDocument.findWidgetById(widgetId)) {
                    movedWidget->bounds = canvasInteraction_.originalBounds;
                }
                if (auto* movedWidget = afterDocument.findWidgetById(widgetId)) {
                    movedWidget->bounds = finalBounds;
                }
                const model::Rect reparentedBounds = boundsRelativeToParent(afterDocument, dropParentId, finalAbsoluteBounds);
                if (afterDocument.reparentWidget(widgetId, dropParentId, reparentedBounds)) {
                    document_ = beforeDocument;
                    undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
                        document_,
                        "Reparent widget",
                        std::move(beforeDocument),
                        std::move(afterDocument)));
                }
            }
            else {
                undoRedo_.executeCommand(std::make_unique<commands::MoveWidgetCommand>(
                    document_, widgetId, canvasInteraction_.originalBounds, finalBounds));
            }
        }
        else {
            undoRedo_.executeCommand(std::make_unique<commands::ResizeWidgetCommand>(
                document_, widgetId, canvasInteraction_.originalBounds, finalBounds));
        }

        document_.markDirty();
        if (canvasInteraction_.mode == CanvasInteractionState::Mode::Move) {
            if (!canvasInteraction_.dropTargetWidgetId.empty() && canvasInteraction_.dropTargetWidgetId != canvasInteraction_.originalParentId) {
                if (const auto* parent = document_.findWidgetById(canvasInteraction_.dropTargetWidgetId)) {
                    setOperationStatus("Reparented widget: " + displayName + " -> " + widgetDisplayName(*parent));
                }
            }
            else {
                setOperationStatus(canvasInteraction_.smartGuideSnapUsed
                    ? "Moved widget: " + displayName + " (" + widgetId + ") with smart guide snap"
                    : "Moved widget: " + displayName + " (" + widgetId + ")");
            }
        }
        else {
            setOperationStatus("Resized widget: " + displayName + " (" + widgetId + ")");
        }
    }

    clearCanvasInteraction();
    redraw();
}

bool MainWindow::mouseWheel(const visage::MouseEvent& e)
{
    const float deltaY = e.precise_wheel_delta_y != 0.0f ? e.precise_wheel_delta_y : e.wheel_delta_y;
    if (!isEditorModalVisible() && propertyInspector_.contains(e.position.x, e.position.y)) {
        if (propertyInspector_.mouseWheel(document_, settings_, deltaY, e.position.x, e.position.y)) {
            if (propertyInspector_.consumeScrollInteraction()) {
                cancelPopupEditors();
            }
            updatePropertyEditorBounds();
            redraw();
            return true;
        }
    }

    if (!isEditorModalVisible() && widgetPalette_.mouseWheel(deltaY, e.position.x, e.position.y)) {
        redraw();
        return true;
    }

    if (dropdownControl_.mouseWheel(deltaY, e.position.x, e.position.y)) {
        redraw();
        return true;
    }

    if (isEditorModalVisible()) {
        // Allow item list editor preview to handle wheel scrolling when visible
        if (editorModal_.mode == EditorModalMode::ItemListEditor) {
            const PanelBounds preview = itemListEditorPreviewBounds();
            if (pointInBounds(e.position.x, e.position.y, preview.x, preview.y, preview.width, preview.height)) {
                // positive deltaY -> scroll up (towards earlier items)
                const int delta = deltaY > 0.0f ? -1 : 1;
                const int maxOffset = std::max(0, static_cast<int>(itemListEditorDialog_.items.size()) - static_cast<int>(itemListEditorVisibleRowCount(preview.height, itemListEditorDialog_.items.size())));
                itemListEditorDialog_.previewScrollOffset = std::clamp(itemListEditorDialog_.previewScrollOffset + delta, 0, maxOffset);
                redraw();
                return true;
            }
        }
        return true;
    }

    if (openMenuIndex_ >= 0) {
        return true;
    }

    if (layout_.showProjectTree && projectTree_.mouseWheel(document_, deltaY, e.position.x, e.position.y)) {
        redraw();
        return true;
    }

    if (propertyInspector_.mouseWheel(document_, settings_, deltaY, e.position.x, e.position.y)) {
        if (propertyInspector_.consumeScrollInteraction()) {
            cancelPopupEditors();
        }
        updatePropertyEditorBounds();
        redraw();
        return true;
    }

    return false;
}

bool MainWindow::keyPress(const visage::KeyEvent& e)
{
    using KeyCode = visage::KeyCode;
    if (dropdownControl_.isOpen() && dropdownControl_.keyPress(e)) {
        handleDropdownSelection();
        redraw();
        return true;
    }

    if (textEditControl_.isActive() && textEditControl_.keyPress(e)) {
        handleTextEditPendingAction();
        redraw();
        return true;
    }

    if (isEditorModalVisible()) {
        if (editorModalEdit_.active) {
            return false;
        }
        if (e.keyCode() == KeyCode::Escape) {
            const bool hasCancel = std::any_of(editorModal_.buttons.begin(), editorModal_.buttons.end(), [](const EditorModalButton& button) {
                return button.id == "cancel";
            });
            activateEditorModalButton(hasCancel ? "cancel" : "ok");
            return true;
        }
        if (e.keyCode() == KeyCode::Return) {
            if (!editorModal_.buttons.empty()) {
                activateEditorModalButton(editorModal_.buttons.front().id);
            }
            return true;
        }
        return true;
    }

    if (openMenuIndex_ >= 0) {
        if (e.keyCode() == KeyCode::Escape) {
            openMenuIndex_ = -1;
            redraw();
        }
        return true;
    }

    if (propertyInspector_.isEditing()) {
        return false;
    }

    const bool usesGlobalShortcutModifier = e.isCtrlDown() || e.isAltDown() || e.isCmdDown() || e.isMetaDown();
    if (!usesGlobalShortcutModifier
        && (e.keyCode() == KeyCode::Left || e.keyCode() == KeyCode::Right
            || e.keyCode() == KeyCode::Up || e.keyCode() == KeyCode::Down)) {
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

    const auto shortcut = commands::CommandRegistry::shortcutFromKeyEvent(e);
    if (!shortcut.has_value()) {
        return false;
    }

    const std::string shortcutText = commands::CommandRegistry::formatShortcut(*shortcut);
    for (const auto& definition : commands::CommandRegistry::definitions()) {
        const CommandId command = commandFromRegistryId(definition.id);
        if (command == CommandId::None || commandShortcutText(command) != shortcutText) {
            continue;
        }

        if (!isCommandEnabled(command)) {
            return reportUnavailableCommand(command);
        }

        return executeCommand(command);
    }

    if (shortcutText == "Ctrl+Shift+Z" && commandShortcutText(CommandId::RedoAction) == "Ctrl+Y") {
        if (!isCommandEnabled(CommandId::RedoAction)) {
            return reportUnavailableCommand(CommandId::RedoAction);
        }

        return executeCommand(CommandId::RedoAction);
    }

    return false;
}

bool MainWindow::receivesTextInput()
{
    return textEditControl_.isActive();
}

void MainWindow::textInput(const std::string& text)
{
    if (textEditControl_.textInput(text)) {
        redraw();
    }
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

bool MainWindow::hasSelectedNonRootWidgets(std::size_t minimumCount) const
{
    if (minimumCount == 0) {
        return true;
    }

    std::size_t count = 0;
    for (const auto* widget : document_.selectedWidgets()) {
        if (widget == nullptr || document_.isRootWidgetId(widget->id)) {
            continue;
        }

        ++count;
        if (count >= minimumCount) {
            return true;
        }
    }

    return false;
}

model::WidgetNode* MainWindow::selectedNonRootWidget()
{
    auto* widget = document_.selectedWidget();
    if (widget == nullptr || document_.isRootWidgetId(widget->id)) {
        return nullptr;
    }

    return widget;
}

bool MainWindow::requireSelectedNonRootWidgets(std::size_t minimumCount, std::vector<model::WidgetNode*>& selectedWidgets)
{
    selectedWidgets = selectedNonRootWidgets(document_);
    if (selectedWidgets.size() >= minimumCount) {
        return true;
    }

    setOperationStatus(minimumCount <= 1
            ? "Select one or more widgets first."
            : "Select two or more widgets for this layout command.");
    redraw();
    return false;
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
        document_.setSelection(widgetId);
        updatePropertyEditorBounds();
        setOperationStatus("Selected: " + widgetDisplayName(*widget) + " (" + widgetId + ")");
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
    updatePropertyEditorBounds();
    redraw();
}

void MainWindow::addWidgetFromPalette(model::WidgetType type)
{
    if (document_.root.type != model::WidgetType::FormWindow) {
        setOperationStatus("Add widget failed: root form is invalid");
        redraw();
        return;
    }

    const std::string parentId = insertionParentIdForNewWidget(document_, type);
    const model::WidgetNode* parent = document_.findWidgetById(parentId);
    if (parent == nullptr) {
        setOperationStatus("Add widget failed: insertion parent is invalid");
        redraw();
        return;
    }

    model::WidgetNode widget = createDefaultWidget(type, parentId);
    const std::string addedId = widget.id;
    const std::string parentLabel = widgetDisplayName(*parent);
    clearCanvasInteraction();
    undoRedo_.executeCommand(std::make_unique<commands::AddWidgetCommand>(document_, parentId, std::move(widget), addedId));
    normalizeWidgetBoundsForEditor();
    document_.markDirty();
    updatePropertyEditorBounds();
    if (type == model::WidgetType::GroupBox && parentId == document_.root.id) {
        setOperationStatus("Added GroupBox");
    }
    else {
        setOperationStatus("Added " + model::toString(type) + " to " + parentLabel);
    }
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
        model::ProjectDocument beforeDocument = document_;
        for (const auto& id : ids) {
            if (document_.findWidgetById(id) != nullptr && document_.removeWidgetById(id)) {
                ++removedCount;
            }
        }

        document_.setSelection(document_.root.id);
        if (removedCount > 0) {
            model::ProjectDocument afterDocument = document_;
            document_ = beforeDocument;
            undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
                document_,
                removedCount == 1 ? std::string{ "Delete widget" } : std::string{ "Delete widgets" },
                std::move(beforeDocument),
                std::move(afterDocument)));
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
    model::ProjectDocument beforeDocument = document_;
    if (!document_.removeWidgetById(widgetId)) {
        setOperationStatus("Delete failed: " + widgetId);
        redraw();
        return;
    }

    document_.selectWidget(document_.root.id);
    model::ProjectDocument afterDocument = document_;
    document_ = beforeDocument;
    undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
        document_,
        "Delete widget",
        std::move(beforeDocument),
        std::move(afterDocument)));
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
    model::ProjectDocument beforeDocument = document_;

    auto* duplicate = document_.duplicateWidgetById(selectedId, idGenerator_);
    if (duplicate == nullptr) {
        setOperationStatus("Duplicate failed: " + selectedId);
        redraw();
        return;
    }

    const std::string duplicateId = duplicate->id;
    const std::string displayName = duplicate->name.empty() ? duplicate->id : duplicate->name;
    document_.normalizeRadioGroups();

    document_.setSelection(duplicateId);
    model::ProjectDocument afterDocument = document_;
    document_ = beforeDocument;
    undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
        document_,
        "Duplicate widget",
        std::move(beforeDocument),
        std::move(afterDocument)));
    document_.markDirty();
    setOperationStatus(hadMultiSelection
        ? "Duplicated primary widget: " + displayName + " (" + duplicateId + ")"
        : "Duplicated widget: " + displayName + " (" + duplicateId + ")");
    redraw();
}

void MainWindow::undo()
{
    if (!undoRedo_.canUndo()) {
        setOperationStatus("Nothing to undo");
        redraw();
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
        setOperationStatus("Nothing to redo");
        redraw();
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

model::WidgetNode MainWindow::createDefaultWidget(model::WidgetType type, const std::string& parentId)
{
    const std::string id = idGenerator_.next(type, document_);
    model::WidgetNode widget = model::WidgetRegistry::instance().createDefaultWidget(type, id);
    if (type == model::WidgetType::TabControl) {
        widget.bounds = nextDefaultWidgetBounds(type, parentId);
        widget.setSelectedTabIndex(0);

        const auto appendDefaultTabPage = [this, &widget](const std::string& title) {
            model::WidgetNode page = model::WidgetRegistry::instance().createDefaultWidget(model::WidgetType::TabPage,
                idGenerator_.next(model::WidgetType::TabPage, document_));
            page.setProperty("title", title);
            page.bounds = tabPageBoundsForTabControl(widget);
            widget.appendChild(std::move(page));
        };

        appendDefaultTabPage("Tab 1");
        appendDefaultTabPage("Tab 2");
        return widget;
    }

    if (type == model::WidgetType::StatusBar) {
        const model::WidgetNode* parent = parentId.empty() ? &document_.root : document_.findWidgetById(parentId);
        const model::Rect parentClientBounds = parent != nullptr
            ? model::LayoutEngine::clientBoundsForParent(*parent)
            : model::LayoutEngine::clientBoundsForParent(document_.root);
        widget.bounds = {
            parentClientBounds.x,
            std::max(parentClientBounds.y, parentClientBounds.y + parentClientBounds.height - widget.bounds.height),
            parentClientBounds.width,
            widget.bounds.height
        };
        widget.setProperty("dock", "Bottom");
        widget.setProperty("fillWidth", true);
    }
    else if (type == model::WidgetType::MenuBar || type == model::WidgetType::ToolBar) {
        const model::WidgetNode* parent = parentId.empty() ? &document_.root : document_.findWidgetById(parentId);
        const model::Rect parentClientBounds = parent != nullptr
            ? model::LayoutEngine::clientBoundsForParent(*parent)
            : model::LayoutEngine::clientBoundsForParent(document_.root);
        widget.bounds = {
            parentClientBounds.x,
            parentClientBounds.y,
            parentClientBounds.width,
            widget.bounds.height
        };
        widget.setProperty("dock", "Top");
        widget.setProperty("anchor", "Top Left");
    }
    else {
        widget.bounds = nextDefaultWidgetBounds(type, parentId);
    }

    return widget;
}

model::Rect MainWindow::nextDefaultWidgetBounds(model::WidgetType type, const std::string& parentId) const
{
    const WidgetSizeMetrics metrics = getWidgetSizeMetrics(type);
    const float width = metrics.defaultWidth;
    const float height = metrics.defaultHeight;

    if (!parentId.empty() && parentId != document_.root.id) {
        const model::WidgetNode* parent = document_.findWidgetById(parentId);
        if (parent != nullptr && parent->type == model::WidgetType::GroupBox) {
            const float maxWidth = std::max(metrics.minWidth, parent->bounds.width - kLayoutMargin * 2.0f);
            const float maxHeight = std::max(metrics.minHeight, parent->bounds.height - kGroupBoxChildStartY - kLayoutMargin);
            const float clampedWidth = std::min(width, maxWidth);
            const float clampedHeight = std::min(height, maxHeight);
            const float staggerOffset = static_cast<float>(parent->children.size()) * kGroupBoxChildOffsetStep;
            const float maxX = std::max(kGroupBoxChildStartX, parent->bounds.width - clampedWidth - kLayoutMargin);
            const float maxY = std::max(kGroupBoxChildStartY, parent->bounds.height - clampedHeight - kLayoutMargin);
            return {
                std::min(kGroupBoxChildStartX + staggerOffset, maxX),
                std::min(kGroupBoxChildStartY + staggerOffset, maxY),
                clampedWidth,
                clampedHeight
            };
        }

        if (parent != nullptr && parent->type == model::WidgetType::TabPage) {
            const float maxWidth = std::max(metrics.minWidth, parent->bounds.width - kLayoutMargin * 2.0f);
            const float maxHeight = std::max(metrics.minHeight, parent->bounds.height - kLayoutMargin * 2.0f);
            const float clampedWidth = std::min(width, maxWidth);
            const float clampedHeight = std::min(height, maxHeight);
            const float staggerOffset = static_cast<float>(parent->children.size()) * kTabPageChildOffsetStep;
            const float maxX = std::max(kTabPageChildStartX, parent->bounds.width - clampedWidth - kLayoutMargin);
            const float maxY = std::max(kTabPageChildStartY, parent->bounds.height - clampedHeight - kLayoutMargin);
            return {
                std::min(kTabPageChildStartX + staggerOffset, maxX),
                std::min(kTabPageChildStartY + staggerOffset, maxY),
                clampedWidth,
                clampedHeight
            };
        }
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

std::string MainWindow::resolveDropParentId(const std::string& movingWidgetId, float x, float y) const
{
    if (movingWidgetId.empty()) {
        return document_.root.id;
    }

    const model::WidgetNode* movingWidget = document_.findWidgetById(movingWidgetId);
    if (movingWidget == nullptr) {
        return document_.root.id;
    }
    if (movingWidget->type == model::WidgetType::TabControl) {
        if (const auto* currentParent = document_.findParentOf(movingWidgetId)) {
            return currentParent->id;
        }
        return document_.root.id;
    }

    const auto dropPoint = designerCanvas_.toFormPoint(document_, x, y);
    if (!dropPoint.has_value()) {
        return document_.root.id;
    }

    std::optional<std::string> hitWidgetId = designerCanvas_.hitTestWidgetId(document_, x, y);
    if (!hitWidgetId.has_value()) {
        return document_.root.id;
    }

    std::string candidateId = *hitWidgetId;
    while (!candidateId.empty()) {
        if (candidateId != movingWidgetId) {
            if (const auto* candidate = document_.findWidgetById(candidateId);
                candidate != nullptr
                && isRepairPassParentTarget(*candidate)
                && movingWidget->findById(candidateId) == nullptr) {
                const model::Rect candidateBounds = absoluteBoundsForWidget(document_, candidateId);
                if (candidate->type == model::WidgetType::FormWindow
                    || candidateBounds.contains(dropPoint->x, dropPoint->y)) {
                return candidate->id;
                }
            }
        }

        const model::WidgetNode* parent = document_.findParentOf(candidateId);
        if (parent == nullptr) {
            break;
        }
        candidateId = parent->id;
    }

    return document_.root.id;
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
        if (key != "text" && key != "normalText" && key != "pressedText") {
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
    auto* widget = selectedNonRootWidget();
    if (widget == nullptr) {
        setOperationStatus("Select one or more widgets first.");
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
    float newWidth = oldWidth;
    float newHeight = oldHeight;
    const std::string displayName = widget->name.empty() ? widget->id : widget->name;
    if (!applyUndoableDocumentChange("Fit text", [this, widget, &key, &valueText, &newWidth, &newHeight]() {
            const bool changed = autoSizeWidgetForTextProperty(*widget, key, valueText);
            newWidth = widget->bounds.width;
            newHeight = widget->bounds.height;
            return changed;
        })) {
        setOperationStatus("Fit text: already large enough");
        redraw();
        return;
    }

    setOperationStatus("Fit text: " + displayName + " width " + std::to_string(static_cast<int>(oldWidth))
        + " -> " + std::to_string(static_cast<int>(newWidth))
        + ", height " + std::to_string(static_cast<int>(oldHeight))
        + " -> " + std::to_string(static_cast<int>(newHeight)));
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
    model::ProjectDocument beforeDocument = document_;
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
    model::ProjectDocument afterDocument = document_;
    document_ = beforeDocument;
    undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
        document_,
        pastedIds.size() == 1 ? std::string{ "Paste widget" } : std::string{ "Paste widgets" },
        std::move(beforeDocument),
        std::move(afterDocument)));
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
        const std::size_t selectedCount = selectedWidgets.size();
        float targetX = selectedWidgets.front()->bounds.x;
        for (const auto* selected : selectedWidgets) {
            if (!document_.isRootWidgetId(selected->id)) {
                targetX = std::min(targetX, selected->bounds.x);
            }
        }
        targetX = snapToCanvasGrid(designerCanvas_, targetX);
        applyUndoableDocumentChange("Align left", [this, &selectedWidgets, targetX]() {
            for (auto* selected : selectedWidgets) {
                if (!document_.isRootWidgetId(selected->id)) {
                    selected->bounds.x = targetX;
                }
            }
            return true;
        });
        setOperationStatus("Aligned left: " + std::to_string(selectedCount) + " widgets");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    const std::string displayName = widgetDisplayName(*widget);
    const std::string widgetId = widget->id;
    applyUndoableDocumentChange("Align left", [this, widget]() {
        widget->bounds.x = snapToCanvasGrid(designerCanvas_, kLayoutMargin);
        return true;
    });
    setOperationStatus("Aligned left: " + displayName + " (" + widgetId + ")");
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
        const std::size_t selectedCount = selectedWidgets.size();
        float targetY = selectedWidgets.front()->bounds.y;
        for (const auto* selected : selectedWidgets) {
            if (!document_.isRootWidgetId(selected->id)) {
                targetY = std::min(targetY, selected->bounds.y);
            }
        }
        targetY = snapToCanvasGrid(designerCanvas_, targetY);
        applyUndoableDocumentChange("Align top", [this, &selectedWidgets, targetY]() {
            for (auto* selected : selectedWidgets) {
                if (!document_.isRootWidgetId(selected->id)) {
                    selected->bounds.y = targetY;
                }
            }
            return true;
        });
        setOperationStatus("Aligned top: " + std::to_string(selectedCount) + " widgets");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    const std::string displayName = widgetDisplayName(*widget);
    const std::string widgetId = widget->id;
    applyUndoableDocumentChange("Align top", [this, widget]() {
        widget->bounds.y = snapToCanvasGrid(designerCanvas_, kLayoutMargin);
        return true;
    });
    setOperationStatus("Aligned top: " + displayName + " (" + widgetId + ")");
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
        const std::size_t selectedCount = selectedWidgets.size();
        float targetRight = selectedWidgets.front()->bounds.x + selectedWidgets.front()->bounds.width;
        for (const auto* selected : selectedWidgets) {
            targetRight = std::max(targetRight, selected->bounds.x + selected->bounds.width);
        }
        applyUndoableDocumentChange("Align right", [this, &selectedWidgets, targetRight]() {
            for (auto* selected : selectedWidgets) {
                selected->bounds.x = snapToCanvasGrid(designerCanvas_, targetRight - selected->bounds.width);
            }
            return true;
        });
        setOperationStatus("Aligned right: " + std::to_string(selectedCount) + " widget(s)");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    auto* widget = selectedWidgets.front();
    applyUndoableDocumentChange("Align right", [this, widget]() {
        widget->bounds.x = snapToCanvasGrid(designerCanvas_, document_.root.bounds.width - kLayoutMargin - widget->bounds.width);
        return true;
    });
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
        const std::size_t selectedCount = selectedWidgets.size();
        float targetBottom = selectedWidgets.front()->bounds.y + selectedWidgets.front()->bounds.height;
        for (const auto* selected : selectedWidgets) {
            targetBottom = std::max(targetBottom, selected->bounds.y + selected->bounds.height);
        }
        applyUndoableDocumentChange("Align bottom", [this, &selectedWidgets, targetBottom]() {
            for (auto* selected : selectedWidgets) {
                selected->bounds.y = snapToCanvasGrid(designerCanvas_, targetBottom - selected->bounds.height);
            }
            return true;
        });
        setOperationStatus("Aligned bottom: " + std::to_string(selectedCount) + " widget(s)");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    auto* widget = selectedWidgets.front();
    applyUndoableDocumentChange("Align bottom", [this, widget]() {
        widget->bounds.y = snapToCanvasGrid(designerCanvas_, document_.root.bounds.height - kLayoutMargin - widget->bounds.height);
        return true;
    });
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
        const std::size_t selectedCount = selectedWidgets.size();
        const SelectionBoundsInfo bounds = calculateSelectionBounds(selectedWidgets);
        const float centerX = (bounds.left + bounds.right) * 0.5f;
        applyUndoableDocumentChange("Center horizontally", [this, &selectedWidgets, centerX]() {
            for (auto* selected : selectedWidgets) {
                selected->bounds.x = snapToCanvasGrid(designerCanvas_, centerX - selected->bounds.width * 0.5f);
            }
            return true;
        });
        setOperationStatus("Centered horizontally: " + std::to_string(selectedCount) + " widget(s)");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    auto* widget = selectedWidgets.front();
    applyUndoableDocumentChange("Center horizontally", [this, widget]() {
        widget->bounds.x = snapToCanvasGrid(designerCanvas_, (document_.root.bounds.width - widget->bounds.width) * 0.5f);
        return true;
    });
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
        const std::size_t selectedCount = selectedWidgets.size();
        const SelectionBoundsInfo bounds = calculateSelectionBounds(selectedWidgets);
        const float centerY = (bounds.top + bounds.bottom) * 0.5f;
        applyUndoableDocumentChange("Center vertically", [this, &selectedWidgets, centerY]() {
            for (auto* selected : selectedWidgets) {
                selected->bounds.y = snapToCanvasGrid(designerCanvas_, centerY - selected->bounds.height * 0.5f);
            }
            return true;
        });
        setOperationStatus("Centered vertically: " + std::to_string(selectedCount) + " widget(s)");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    auto* widget = selectedWidgets.front();
    applyUndoableDocumentChange("Center vertically", [this, widget]() {
        widget->bounds.y = snapToCanvasGrid(designerCanvas_, (document_.root.bounds.height - widget->bounds.height) * 0.5f);
        return true;
    });
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
        const std::size_t selectedCount = selectedWidgets.size();
        const float referenceWidth = widget->bounds.width;
        const std::string selectedId = widget->id;
        applyUndoableDocumentChange("Match width", [this, &selectedWidgets, referenceWidth, &selectedId]() {
            for (auto* selected : selectedWidgets) {
                if (selected->id == selectedId || document_.isRootWidgetId(selected->id)) {
                    continue;
                }
                const WidgetSizeMetrics metrics = getWidgetSizeMetrics(selected->type);
                selected->bounds.width = std::max(metrics.minWidth, referenceWidth);
            }
            return true;
        });
        setOperationStatus("Same width: " + std::to_string(selectedCount) + " widgets");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    const WidgetSizeMetrics metrics = getWidgetSizeMetrics(widget->type);
    const model::WidgetNode* reference = document_.previousSiblingOf(widget->id);
    const float fallbackWidth = std::max(metrics.minWidth, document_.root.bounds.width - 40.0f);
    const float targetWidth = std::max(metrics.minWidth, reference != nullptr ? reference->bounds.width : fallbackWidth);
    const std::string displayName = widgetDisplayName(*widget);
    const std::string widgetId = widget->id;
    applyUndoableDocumentChange("Match width", [widget, targetWidth]() {
        widget->bounds.width = targetWidth;
        return true;
    });
    setOperationStatus("Same width: " + displayName + " (" + widgetId + ")");
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
        const std::size_t selectedCount = selectedWidgets.size();
        const float referenceHeight = widget->bounds.height;
        const std::string selectedId = widget->id;
        applyUndoableDocumentChange("Match height", [this, &selectedWidgets, referenceHeight, &selectedId]() {
            for (auto* selected : selectedWidgets) {
                if (selected->id == selectedId || document_.isRootWidgetId(selected->id)) {
                    continue;
                }
                const WidgetSizeMetrics metrics = getWidgetSizeMetrics(selected->type);
                selected->bounds.height = std::max(metrics.minHeight, referenceHeight);
            }
            return true;
        });
        setOperationStatus("Same height: " + std::to_string(selectedCount) + " widgets");
        updatePropertyEditorBounds();
        redraw();
        return;
    }

    const WidgetSizeMetrics metrics = getWidgetSizeMetrics(widget->type);
    const model::WidgetNode* reference = document_.previousSiblingOf(widget->id);
    const float targetHeight = std::max(metrics.minHeight, reference != nullptr ? reference->bounds.height : metrics.defaultHeight);
    const std::string displayName = widgetDisplayName(*widget);
    const std::string widgetId = widget->id;
    applyUndoableDocumentChange("Match height", [widget, targetHeight]() {
        widget->bounds.height = targetHeight;
        return true;
    });
    setOperationStatus("Same height: " + displayName + " (" + widgetId + ")");
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
    const std::size_t selectedCount = selectedWidgets.size();
    applyUndoableDocumentChange("Distribute horizontally", [this, &selectedWidgets, leftX, step]() {
        for (std::size_t index = 1; index + 1 < selectedWidgets.size(); ++index) {
            selectedWidgets[index]->bounds.x = snapToCanvasGrid(designerCanvas_, leftX + step * static_cast<float>(index));
        }
        return true;
    });

    setOperationStatus("Distributed horizontally: " + std::to_string(selectedCount) + " widgets");
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
    const std::size_t selectedCount = selectedWidgets.size();
    applyUndoableDocumentChange("Distribute vertically", [this, &selectedWidgets, topY, step]() {
        for (std::size_t index = 1; index + 1 < selectedWidgets.size(); ++index) {
            selectedWidgets[index]->bounds.y = snapToCanvasGrid(designerCanvas_, topY + step * static_cast<float>(index));
        }
        return true;
    });

    setOperationStatus("Distributed vertically: " + std::to_string(selectedCount) + " widgets");
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

    const std::size_t selectedCount = selectedWidgets.size();
    applyUndoableDocumentChange("Nudge widgets", [&selectedWidgets, dx, dy]() {
        for (auto* selected : selectedWidgets) {
            selected->bounds.x += dx;
            selected->bounds.y += dy;
        }
        return true;
    });

    setOperationStatus("Nudged " + std::to_string(selectedCount) + " widget(s)");
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
    if (!applyUndoableDocumentChange("Bring forward", [this, &selectedId]() {
            return document_.bringWidgetForward(selectedId);
        })) {
        setOperationStatus(document_.hasMultiSelection()
            ? "Already in front: " + displayName + " (" + selectedId + ") - primary only"
            : "Already in front: " + displayName + " (" + selectedId + ")");
        redraw();
        return;
    }

    document_.selectWidget(selectedId);
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
    if (!applyUndoableDocumentChange("Send backward", [this, &selectedId]() {
            return document_.sendWidgetBackward(selectedId);
        })) {
        setOperationStatus(document_.hasMultiSelection()
            ? "Already in back: " + displayName + " (" + selectedId + ") - primary only"
            : "Already in back: " + displayName + " (" + selectedId + ")");
        redraw();
        return;
    }

    document_.selectWidget(selectedId);
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

    if (widget->name == trimmedName) {
        return true;
    }

    model::ProjectDocument beforeDocument = document_;
    widget->name = trimmedName;
    model::ProjectDocument afterDocument = document_;
    document_ = beforeDocument;
    undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
        document_,
        "Rename widget",
        std::move(beforeDocument),
        std::move(afterDocument)));
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

    if (widget->bounds.x == x && widget->bounds.y == y && widget->bounds.width == clampedWidth && widget->bounds.height == clampedHeight) {
        return true;
    }

    model::ProjectDocument beforeDocument = document_;
    widget->bounds = { x, y, clampedWidth, clampedHeight };
    document_.applyLayoutFromPrevious(beforeDocument);
    model::ProjectDocument afterDocument = document_;
    document_ = beforeDocument;
    undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
        document_,
        "Edit widget bounds",
        std::move(beforeDocument),
        std::move(afterDocument)));
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

    model::ProjectDocument beforeDocument = document_;
    const bool radioSelectedTrue = widget->type == model::WidgetType::RadioButton
        && key == "selected" && value.isBool() && value.asBool(false);
    if (widget->type == model::WidgetType::TabControl && (key == "selectedTabIndex" || key == "selectedTab")) {
        widget->setSelectedTabIndex(value.asInt(widget->selectedTabIndex()));
    }
    else {
        widget->setProperty(key, std::move(value));
    }
    const std::string selectedIndexKey = std::string(model::selectedItemIndexPropertyKey(widget->type));
    if (model::supportsItemList(widget->type)
        && (key == "items" || key == "itemActions" || key == selectedIndexKey || key == "text")) {
        model::normalizeItemListProperties(*widget);
    }
    if (model::supportsTableGrid(widget->type) && (key == "columns" || key == "rows" || key == "selectedRow" || key == "selectedColumn")) {
        model::normalizeTableGridProperties(*widget);
    }
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
    document_.applyLayoutFromPrevious(beforeDocument);
    const std::string displayName = widget->name.empty() ? widget->id : widget->name;
    const std::string operationStatus = autoSized
        ? "Auto-sized " + displayName + ": width " + std::to_string(static_cast<int>(previousWidth))
            + " -> " + std::to_string(static_cast<int>(widget->bounds.width))
        : "Property changed: " + key;
    model::ProjectDocument afterDocument = document_;
    document_ = beforeDocument;
    undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
        document_,
        "Edit property: " + key,
        std::move(beforeDocument),
        std::move(afterDocument)));
    document_.markDirty();
    setOperationStatus(operationStatus);
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

    if ((key == "selectedRow" || key == "selectedColumn") && widget->type == model::WidgetType::TableGrid) {
        const auto parsedValue = tryParseInt(trimmedValue);
        if (!parsedValue.has_value()) {
            setOperationStatus("Invalid table selection index");
            redraw();
            return false;
        }

        return setSelectedWidgetProperty(key, *parsedValue);
    }

    if ((key == "selectedTabIndex" || key == "selectedTab") && widget->type == model::WidgetType::TabControl) {
        const auto parsedValue = tryParseInt(trimmedValue);
        if (!parsedValue.has_value()) {
            setOperationStatus("Invalid selected tab index");
            redraw();
            return false;
        }

        return setSelectedWidgetProperty("selectedTabIndex", *parsedValue);
    }

    if (key == "__tabcontrol_selected_tab_title") {
        const auto* tabControl = widget->type == model::WidgetType::TabControl ? widget : document_.findTabControlFor(widget->id);
        if (tabControl == nullptr) {
            setOperationStatus("No tab control selected");
            redraw();
            return false;
        }

        const auto* selectedPage = document_.selectedTabPageFor(tabControl->id);
        if (selectedPage == nullptr || trimmedValue.empty()) {
            setOperationStatus(trimmedValue.empty() ? "Tab title cannot be empty" : "Selected tab page is invalid");
            redraw();
            return false;
        }

        model::ProjectDocument beforeDocument = document_;
        model::ProjectDocument afterDocument = document_;
        if (auto* page = afterDocument.findWidgetById(selectedPage->id)) {
            page->setProperty("title", trimmedValue);
        }
        else {
            return false;
        }

        document_ = beforeDocument;
        undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
            document_,
            "Rename tab",
            std::move(beforeDocument),
            std::move(afterDocument)));
        document_.markDirty();
        setOperationStatus("Renamed selected tab to " + trimmedValue);
        updatePropertyEditorBounds();
        redraw();
        return true;
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

    if (key == "__tabcontrol_tabs"
        || key == "__tabcontrol_add_tab"
        || key == "__tabcontrol_remove_selected_tab") {
        const bool applied = key == "__tabcontrol_tabs"
            ? selectTabPageFromInspector(trimmedValue)
            : (key == "__tabcontrol_add_tab"
                    ? addTabPageToSelectedTabControl()
                    : removeSelectedTabPageFromSelectedTabControl());
        if (!applied) {
            redraw();
            return false;
        }

        propertyInspector_.clearEditing();
        textEditControl_.clear();
        dropdownControl_.close();
        requestKeyboardFocus();
        updatePropertyEditorBounds();
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

    if (key == "scaleMode") {
        static constexpr std::array<const char*, 4> kScaleModes = { "Stretch", "Fit", "Fill", "Center" };
        const bool isValidScaleMode = std::any_of(kScaleModes.begin(), kScaleModes.end(), [&trimmedValue](const char* value) {
            return trimmedValue == value;
        });
        if (!isValidScaleMode) {
            setOperationStatus("Invalid image scale mode");
            redraw();
            return false;
        }

        return setSelectedWidgetProperty(key, trimmedValue);
    }

    if (key == "dock") {
        if (!model::dockModeFromString(trimmedValue).has_value()) {
            setOperationStatus("Invalid dock value");
            redraw();
            return false;
        }

        return setSelectedWidgetProperty(key, trimmedValue);
    }

    if (key == "anchor") {
        if (!model::anchorModeFromString(trimmedValue).has_value()) {
            setOperationStatus("Invalid anchor value");
            redraw();
            return false;
        }

        return setSelectedWidgetProperty(key, trimmedValue);
    }

    if (key == "imagePath") {
        const bool updated = setSelectedWidgetProperty(key, trimmedValue);
        if (updated && widget->getProperty("source") != nullptr) {
            setSelectedWidgetProperty("source", std::string{});
        }
        return updated;
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
        if (key == "fontSize") {
            const auto parsedValue = tryParseFloat(trimmedValue);
            if (!parsedValue.has_value()) {
                return setSelectedWidgetProperty(key, model::PropertyValue{});
            }

            return setSelectedWidgetProperty(key, std::clamp(*parsedValue, 8.0f, 72.0f));
        }

        if (key == "borderThickness" || key == "cornerRadius") {
            const auto parsedValue = tryParseFloat(trimmedValue);
            if (!parsedValue.has_value()) {
                if (isUnsetValueText(trimmedValue)) {
                    return setSelectedWidgetProperty(key, 1.0f);
                }

                setOperationStatus("Invalid value for " + key + ". Use 1 to 25.");
                redraw();
                return false;
            }

            const float clampedValue = std::clamp(*parsedValue, 1.0f, 25.0f);
            const bool updated = setSelectedWidgetProperty(key, clampedValue);
            if (updated) {
                setOperationStatus("Property changed: " + key + " = " + std::to_string(static_cast<int>(std::round(clampedValue))));
                redraw();
            }
            return updated;
        }

        const auto parsedValue = tryParseFloat(trimmedValue);
        if (!parsedValue.has_value()) {
            return setSelectedWidgetProperty(key, model::PropertyValue{});
        }

        return setSelectedWidgetProperty(key, *parsedValue);
    }

    if (key == "onClick" || key == "onToggle" || key == "onChanged"
        || key == "onTextChanged" || key == "onLoad" || key == "onClose" || key == "onSelected"
        || key == "onRelease" || key == "onDoubleClick"
        || key == "onAccepted" || key == "onCancelled"
        || key == "onSelectionChanged" || key == "onCellDoubleClick") {
        if (!trimmedValue.empty() && !utils::isValidCppIdentifier(trimmedValue)) {
            setOperationStatus("Invalid event handler name");
            redraw();
            return false;
        }

        return setSelectedWidgetProperty(key, trimmedValue);
    }

    if (key == "x" || key == "y" || key == "width" || key == "height") {
        const auto numericValue = tryParseFloat(trimmedValue);
        if (!numericValue.has_value()) {
            setOperationStatus("Invalid value for " + key);
            redraw();
            return false;
        }

        float x = widget->bounds.x;
        float y = widget->bounds.y;
        float width = widget->bounds.width;
        float height = widget->bounds.height;
        if (key == "x") {
            x = *numericValue;
        }
        else if (key == "y") {
            y = *numericValue;
        }
        else if (key == "width") {
            width = *numericValue;
        }
        else {
            height = *numericValue;
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
            const auto parsedValue = tryParseInt(trimmedValue);
            if (!parsedValue.has_value()) {
                setOperationStatus("Invalid value for " + key);
                redraw();
                return false;
            }

            return setSelectedWidgetProperty(key, *parsedValue);
        }
        if (existingProperty->isFloat()) {
            const auto parsedValue = tryParseFloat(trimmedValue);
            if (!parsedValue.has_value()) {
                setOperationStatus("Invalid value for " + key);
                redraw();
                return false;
            }

            return setSelectedWidgetProperty(key, *parsedValue);
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

    layout.menuBar = { 0.0f, 0.0f, windowWidth, kMenuBarHeight };
    layout.toolbar = { 0.0f, layout.menuBar.height, windowWidth, kToolbarHeight };
    layout.statusBar = { 0.0f, std::max(0.0f, windowHeight - kStatusBarHeight), windowWidth, kStatusBarHeight };

    const float contentTop = layout.toolbar.y + layout.toolbar.height + kGap;
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
    setTitle(makeWindowTitle(document_.dirty));
}

std::string_view MainWindow::commandRegistryId(CommandId command)
{
    using namespace commands::ids;
    switch (command) {
    case CommandId::NewProject:
        return kFileNew;
    case CommandId::OpenProject:
        return kFileOpen;
    case CommandId::OpenSample:
        return kFileOpenSample;
    case CommandId::SaveProject:
        return kFileSave;
    case CommandId::SaveProjectAsDialog:
        return kFileSaveAs;
    case CommandId::ExportCode:
        return kFileExport;
    case CommandId::ValidateProject:
        return kProjectValidate;
    case CommandId::ShowValidationReport:
        return kViewValidationReport;
    case CommandId::ShowAboutDialog:
        return kHelpAbout;
    case CommandId::ShowKeyboardShortcuts:
        return kProjectKeyboardShortcuts;
    case CommandId::ShowGeneratedCodeGuide:
        return kHelpGeneratedCodeGuide;
    case CommandId::ShowProjectSettings:
        return kProjectSettings;
    case CommandId::ShowResourceManager:
        return kProjectResources;
    case CommandId::ShowExportDependencies:
        return kProjectExportDependencies;
    case CommandId::FitText:
        return kLayoutFitText;
    case CommandId::CopyWidgets:
        return kEditCopy;
    case CommandId::PasteWidgets:
        return kEditPaste;
    case CommandId::DeleteWidget:
        return kEditDelete;
    case CommandId::DuplicateWidget:
        return kEditDuplicate;
    case CommandId::ToggleMultiSelect:
        return kViewMultiSelect;
    case CommandId::AlignLeft:
        return kLayoutAlignLeft;
    case CommandId::AlignTop:
        return kLayoutAlignTop;
    case CommandId::AlignRight:
        return kLayoutAlignRight;
    case CommandId::AlignBottom:
        return kLayoutAlignBottom;
    case CommandId::CenterHorizontally:
        return kLayoutCenterHorizontal;
    case CommandId::CenterVertically:
        return kLayoutCenterVertical;
    case CommandId::SameWidth:
        return kLayoutSameWidth;
    case CommandId::SameHeight:
        return kLayoutSameHeight;
    case CommandId::DistributeHorizontally:
        return kLayoutDistributeHorizontal;
    case CommandId::DistributeVertically:
        return kLayoutDistributeVertical;
    case CommandId::ToggleSmartGuides:
        return kViewGuides;
    case CommandId::BringForward:
        return kLayoutBringForward;
    case CommandId::SendBackward:
        return kLayoutSendBackward;
    case CommandId::ToggleGrid:
        return kViewGrid;
    case CommandId::ToggleSnap:
        return kViewSnap;
    case CommandId::UndoAction:
        return kEditUndo;
    case CommandId::RedoAction:
        return kEditRedo;
    case CommandId::None:
    default:
        return {};
    }
}

MainWindow::CommandId MainWindow::commandFromRegistryId(std::string_view registryId)
{
    using namespace commands::ids;

    if (registryId == kFileNew) {
        return CommandId::NewProject;
    }
    if (registryId == kFileOpen) {
        return CommandId::OpenProject;
    }
    if (registryId == kFileOpenSample) {
        return CommandId::OpenSample;
    }
    if (registryId == kFileSave) {
        return CommandId::SaveProject;
    }
    if (registryId == kFileSaveAs) {
        return CommandId::SaveProjectAsDialog;
    }
    if (registryId == kFileExport) {
        return CommandId::ExportCode;
    }
    if (registryId == kEditUndo) {
        return CommandId::UndoAction;
    }
    if (registryId == kEditRedo) {
        return CommandId::RedoAction;
    }
    if (registryId == kEditCopy) {
        return CommandId::CopyWidgets;
    }
    if (registryId == kEditPaste) {
        return CommandId::PasteWidgets;
    }
    if (registryId == kEditDelete) {
        return CommandId::DeleteWidget;
    }
    if (registryId == kEditDuplicate) {
        return CommandId::DuplicateWidget;
    }
    if (registryId == kViewGrid) {
        return CommandId::ToggleGrid;
    }
    if (registryId == kViewSnap) {
        return CommandId::ToggleSnap;
    }
    if (registryId == kViewGuides) {
        return CommandId::ToggleSmartGuides;
    }
    if (registryId == kViewMultiSelect) {
        return CommandId::ToggleMultiSelect;
    }
    if (registryId == kLayoutFitText) {
        return CommandId::FitText;
    }
    if (registryId == kLayoutAlignLeft) {
        return CommandId::AlignLeft;
    }
    if (registryId == kLayoutAlignTop) {
        return CommandId::AlignTop;
    }
    if (registryId == kLayoutAlignRight) {
        return CommandId::AlignRight;
    }
    if (registryId == kLayoutAlignBottom) {
        return CommandId::AlignBottom;
    }
    if (registryId == kLayoutCenterHorizontal) {
        return CommandId::CenterHorizontally;
    }
    if (registryId == kLayoutCenterVertical) {
        return CommandId::CenterVertically;
    }
    if (registryId == kLayoutSameWidth) {
        return CommandId::SameWidth;
    }
    if (registryId == kLayoutSameHeight) {
        return CommandId::SameHeight;
    }
    if (registryId == kLayoutDistributeHorizontal) {
        return CommandId::DistributeHorizontally;
    }
    if (registryId == kLayoutDistributeVertical) {
        return CommandId::DistributeVertically;
    }
    if (registryId == kLayoutBringForward) {
        return CommandId::BringForward;
    }
    if (registryId == kLayoutSendBackward) {
        return CommandId::SendBackward;
    }
    if (registryId == kProjectValidate) {
        return CommandId::ValidateProject;
    }
    if (registryId == kProjectSettings) {
        return CommandId::ShowProjectSettings;
    }
    if (registryId == kProjectResources) {
        return CommandId::ShowResourceManager;
    }
    if (registryId == kProjectKeyboardShortcuts) {
        return CommandId::ShowKeyboardShortcuts;
    }
    if (registryId == kProjectExportDependencies) {
        return CommandId::ShowExportDependencies;
    }
    if (registryId == kHelpAbout) {
        return CommandId::ShowAboutDialog;
    }
    if (registryId == kHelpGeneratedCodeGuide) {
        return CommandId::ShowGeneratedCodeGuide;
    }
    if (registryId == kViewValidationReport) {
        return CommandId::ShowValidationReport;
    }

    return CommandId::None;
}

std::string MainWindow::commandShortcutText(CommandId command) const
{
    const std::string_view registryId = commandRegistryId(command);
    if (registryId.empty()) {
        return {};
    }

    std::string shortcutText;
    if (const auto iterator = settings_.keyboardShortcuts.find(std::string{ registryId }); iterator != settings_.keyboardShortcuts.end()) {
        shortcutText = iterator->second;
    }
    else if (const auto* definition = commands::CommandRegistry::find(registryId)) {
        shortcutText = std::string{ definition->defaultShortcut };
    }

    if (shortcutText.empty()) {
        return {};
    }

    if (const auto parsedShortcut = commands::CommandRegistry::parseShortcutString(shortcutText)) {
        return commands::CommandRegistry::formatShortcut(*parsedShortcut);
    }

    return shortcutText;
}

bool MainWindow::reportUnavailableCommand(CommandId command)
{
    switch (command) {
    case CommandId::UndoAction:
        setOperationStatus("Nothing to undo");
        redraw();
        return true;
    case CommandId::RedoAction:
        setOperationStatus("Nothing to redo");
        redraw();
        return true;
    default:
        break;
    }

    const std::string_view registryId = commandRegistryId(command);
    if (const auto* definition = registryId.empty() ? nullptr : commands::CommandRegistry::find(registryId)) {
        setOperationStatus("Command unavailable: " + std::string{ definition->displayName });
        redraw();
        return true;
    }

    return false;
}

std::string MainWindow::commandHintText(CommandId command) const
{
    const auto baseHint = [this, command]() -> std::string {
        const std::string shortcutText = commandShortcutText(command);
        if (shortcutText.empty()) {
            switch (command) {
            case CommandId::ShowKeyboardShortcuts:
                return "Open keyboard shortcut settings";
            default:
                break;
            }
        }

        const auto shortcutSuffix = shortcutText.empty() ? std::string{} : " (" + shortcutText + ")";
        switch (command) {
        case CommandId::NewProject:
            return "Create a new VisiForm project" + shortcutSuffix;
        case CommandId::OpenProject:
            return "Open a .vfb.json project" + shortcutSuffix;
        case CommandId::OpenSample:
            return "Open the sample project" + shortcutSuffix;
        case CommandId::SaveProject:
            return "Save the current project" + shortcutSuffix;
        case CommandId::SaveProjectAsDialog:
            return "Save the project to a new .vfb.json file" + shortcutSuffix;
        case CommandId::ExportCode:
            return "Export generated Visage C++ project" + shortcutSuffix;
        case CommandId::ValidateProject:
            return "Validate the current project before export" + shortcutSuffix;
        case CommandId::ShowValidationReport:
            return "Show where the latest validation report was written" + shortcutSuffix;
        case CommandId::ShowAboutDialog:
            return "Show information about VisiForm" + shortcutSuffix;
        case CommandId::ShowKeyboardShortcuts:
            return "Open keyboard shortcut settings" + shortcutSuffix;
        case CommandId::ShowGeneratedCodeGuide:
            return "Show a short guide to generated code output" + shortcutSuffix;
        case CommandId::ShowProjectSettings:
            return "Open the project settings dialog" + shortcutSuffix;
        case CommandId::ShowResourceManager:
            return "Open the project resource manager" + shortcutSuffix;
        case CommandId::ShowExportDependencies:
            return "Show where export dependency settings are edited" + shortcutSuffix;
        case CommandId::FitText:
            return "Fit the selected widget to its text" + shortcutSuffix;
        case CommandId::CopyWidgets:
            return "Copy selected widgets" + shortcutSuffix;
        case CommandId::PasteWidgets:
            return "Paste copied widgets" + shortcutSuffix;
        case CommandId::DeleteWidget:
            return "Delete the selected widget or widgets" + shortcutSuffix;
        case CommandId::DuplicateWidget:
            return "Duplicate the primary selected widget" + shortcutSuffix;
        case CommandId::ToggleMultiSelect:
            return "Toggle multi-select mode" + shortcutSuffix;
        case CommandId::AlignLeft:
            return "Align selected widgets left" + shortcutSuffix;
        case CommandId::AlignTop:
            return "Align selected widgets top" + shortcutSuffix;
        case CommandId::AlignRight:
            return "Align selected widgets right" + shortcutSuffix;
        case CommandId::AlignBottom:
            return "Align selected widgets bottom" + shortcutSuffix;
        case CommandId::CenterHorizontally:
            return "Center selected widgets horizontally" + shortcutSuffix;
        case CommandId::CenterVertically:
            return "Center selected widgets vertically" + shortcutSuffix;
        case CommandId::SameWidth:
            return "Match selected widget widths" + shortcutSuffix;
        case CommandId::SameHeight:
            return "Match selected widget heights" + shortcutSuffix;
        case CommandId::DistributeHorizontally:
            return "Distribute selected widgets horizontally" + shortcutSuffix;
        case CommandId::DistributeVertically:
            return "Distribute selected widgets vertically" + shortcutSuffix;
        case CommandId::ToggleSmartGuides:
            return "Toggle smart guides" + shortcutSuffix;
        case CommandId::BringForward:
            return "Bring the selected widget forward" + shortcutSuffix;
        case CommandId::SendBackward:
            return "Send the selected widget backward" + shortcutSuffix;
        case CommandId::ToggleGrid:
            return "Toggle grid visibility" + shortcutSuffix;
        case CommandId::ToggleSnap:
            return "Toggle snap-to-grid" + shortcutSuffix;
        case CommandId::UndoAction:
            return "Undo the last command" + shortcutSuffix;
        case CommandId::RedoAction:
            return "Redo the last undone command" + shortcutSuffix;
        case CommandId::None:
        default:
            return {};
        }
    };

    return baseHint();
}

bool MainWindow::isCommandEnabled(CommandId command) const
{
    const bool hasNonRootSelection = hasSelectedNonRootWidgets(1);
    const bool hasMultiSelection = hasSelectedNonRootWidgets(2);

    switch (command) {
    case CommandId::UndoAction:
        return canUndo();
    case CommandId::RedoAction:
        return canRedo();
    case CommandId::PasteWidgets:
        return !clipboardWidgets_.empty();
    case CommandId::CopyWidgets:
    case CommandId::DeleteWidget:
    case CommandId::DuplicateWidget:
    case CommandId::FitText:
    case CommandId::BringForward:
    case CommandId::SendBackward:
        return hasNonRootSelection;
    case CommandId::AlignLeft:
    case CommandId::AlignTop:
    case CommandId::AlignRight:
    case CommandId::AlignBottom:
    case CommandId::CenterHorizontally:
    case CommandId::CenterVertically:
    case CommandId::SameWidth:
    case CommandId::SameHeight:
    case CommandId::DistributeHorizontally:
    case CommandId::DistributeVertically:
        return hasMultiSelection;
    case CommandId::ShowValidationReport:
        return std::filesystem::exists(projectRootPath() / "Generated" / "validation_report.md");
    case CommandId::None:
        return false;
    default:
        return true;
    }
}

bool MainWindow::isCommandChecked(CommandId command) const
{
    switch (command) {
    case CommandId::ToggleGrid:
        return designerCanvas_.showGrid();
    case CommandId::ToggleSnap:
        return designerCanvas_.snapToGrid();
    case CommandId::ToggleSmartGuides:
        return settings_.smartGuidesEnabled;
    case CommandId::ToggleMultiSelect:
        return multiSelectMode_;
    default:
        return false;
    }
}

std::vector<MainWindow::Menu> MainWindow::menus() const
{
    std::vector<Menu> result;

    auto addCommand = [this](Menu& menu, CommandId command, const std::string& label) {
        menu.items.push_back(MenuItem{
            label,
            label,
            commandShortcutText(command),
            command,
            isCommandEnabled(command),
            isCommandChecked(command)
        });
    };
    auto addSeparator = [](Menu& menu) {
        MenuItem item;
        item.separator = true;
        item.enabled = false;
        menu.items.push_back(std::move(item));
    };
    auto addWidgetItem = [](Menu& menu, const std::string& id, const std::string& label, model::WidgetType widgetType) {
        MenuItem item;
        item.id = id;
        item.label = label;
        item.enabled = true;
        item.widgetType = widgetType;
        menu.items.push_back(std::move(item));
    };

    Menu fileMenu{ "File" };
    addCommand(fileMenu, CommandId::NewProject, "New");
    addCommand(fileMenu, CommandId::OpenProject, "Open");
    addCommand(fileMenu, CommandId::OpenSample, "Open Sample");
    addCommand(fileMenu, CommandId::SaveProject, "Save");
    addCommand(fileMenu, CommandId::SaveProjectAsDialog, "Save As");
    if (!settings_.recentFiles.empty()) {
        addSeparator(fileMenu);
        const std::size_t recentLimit = std::min<std::size_t>(5, settings_.recentFiles.size());
        for (std::size_t index = 0; index < recentLimit; ++index) {
            MenuItem item;
            item.id = "recent-" + std::to_string(index);
            item.label = normalizedPathText(settings_.recentFiles[index]);
            item.enabled = std::filesystem::exists(settings_.recentFiles[index]);
            item.filePath = settings_.recentFiles[index];
            fileMenu.items.push_back(std::move(item));
        }
    }
    result.push_back(std::move(fileMenu));

    Menu editMenu{ "Edit" };
    addCommand(editMenu, CommandId::UndoAction, "Undo");
    addCommand(editMenu, CommandId::RedoAction, "Redo");
    addSeparator(editMenu);
    addCommand(editMenu, CommandId::CopyWidgets, "Copy");
    addCommand(editMenu, CommandId::PasteWidgets, "Paste");
    addCommand(editMenu, CommandId::DuplicateWidget, "Duplicate");
    addCommand(editMenu, CommandId::DeleteWidget, "Delete");
    result.push_back(std::move(editMenu));

    Menu viewMenu{ "View" };
    addCommand(viewMenu, CommandId::ToggleGrid, "Grid");
    addCommand(viewMenu, CommandId::ToggleSnap, "Snap");
    addCommand(viewMenu, CommandId::ToggleSmartGuides, "Guides");
    addCommand(viewMenu, CommandId::ToggleMultiSelect, "Multi Select");
    addSeparator(viewMenu);
    addCommand(viewMenu, CommandId::ShowValidationReport, "Validation Report");
    result.push_back(std::move(viewMenu));

    Menu insertMenu{ "Insert" };
    addWidgetItem(insertMenu, "insert-frame", "Frame", model::WidgetType::Frame);
    addWidgetItem(insertMenu, "insert-label", "Label", model::WidgetType::Label);
    addWidgetItem(insertMenu, "insert-button", "Button", model::WidgetType::Button);
    addWidgetItem(insertMenu, "insert-text-box", "Text Box", model::WidgetType::TextBox);
    addWidgetItem(insertMenu, "insert-check-box", "Check Box", model::WidgetType::CheckBox);
    addWidgetItem(insertMenu, "insert-radio-button", "Radio Button", model::WidgetType::RadioButton);
    addWidgetItem(insertMenu, "insert-slider", "Slider", model::WidgetType::Slider);
    addWidgetItem(insertMenu, "insert-scroll-bar", "Scroll Bar", model::WidgetType::ScrollBar);
    addWidgetItem(insertMenu, "insert-status-bar", "Status Bar", model::WidgetType::StatusBar);
    addWidgetItem(insertMenu, "insert-progress-bar", "Progress Bar", model::WidgetType::ProgressBar);
    addWidgetItem(insertMenu, "insert-color-picker", "Color Picker", model::WidgetType::ColorPicker);
    addWidgetItem(insertMenu, "insert-modal-dialog", "Modal Dialog", model::WidgetType::ModalDialog);
    addWidgetItem(insertMenu, "insert-image", "Image", model::WidgetType::Image);
    addWidgetItem(insertMenu, "insert-spacer", "Spacer", model::WidgetType::Spacer);
    result.push_back(std::move(insertMenu));

    Menu layoutMenu{ "Layout" };
    addCommand(layoutMenu, CommandId::AlignLeft, "Align Left");
    addCommand(layoutMenu, CommandId::AlignTop, "Align Top");
    addCommand(layoutMenu, CommandId::AlignRight, "Align Right");
    addCommand(layoutMenu, CommandId::AlignBottom, "Align Bottom");
    addCommand(layoutMenu, CommandId::CenterHorizontally, "Center Horizontally");
    addCommand(layoutMenu, CommandId::CenterVertically, "Center Vertically");
    addCommand(layoutMenu, CommandId::SameWidth, "Same Width");
    addCommand(layoutMenu, CommandId::SameHeight, "Same Height");
    addCommand(layoutMenu, CommandId::DistributeHorizontally, "Distribute Horizontally");
    addCommand(layoutMenu, CommandId::DistributeVertically, "Distribute Vertically");
    addCommand(layoutMenu, CommandId::BringForward, "Bring Forward");
    addCommand(layoutMenu, CommandId::SendBackward, "Send Backward");
    addSeparator(layoutMenu);
    addCommand(layoutMenu, CommandId::FitText, "Fit Text");
    result.push_back(std::move(layoutMenu));

    Menu projectMenu{ "Project" };
    addCommand(projectMenu, CommandId::ValidateProject, "Validate / Check");
    addCommand(projectMenu, CommandId::ShowProjectSettings, "Project Settings");
    addCommand(projectMenu, CommandId::ShowResourceManager, "Resources");
    addCommand(projectMenu, CommandId::ShowKeyboardShortcuts, "Keyboard Shortcuts");
    addCommand(projectMenu, CommandId::ShowExportDependencies, "Export Dependencies");
    result.push_back(std::move(projectMenu));

    Menu exportMenu{ "Export" };
    addCommand(exportMenu, CommandId::ExportCode, "Export");
    MenuItem exportTodo;
    exportTodo.id = "open-export-folder";
    exportTodo.label = "Open Export Folder (TODO)";
    exportTodo.enabled = false;
    exportMenu.items.push_back(std::move(exportTodo));
    result.push_back(std::move(exportMenu));

    Menu helpMenu{ "Help" };
    addCommand(helpMenu, CommandId::ShowAboutDialog, "About VisiForm");
    addCommand(helpMenu, CommandId::ShowKeyboardShortcuts, "Keyboard Shortcuts");
    addCommand(helpMenu, CommandId::ShowGeneratedCodeGuide, "Generated Code Guide");
    result.push_back(std::move(helpMenu));

    return result;
}

std::vector<MainWindow::MenuBarButton> MainWindow::menuBarButtons() const
{
    std::vector<MenuBarButton> buttons;
    if (!layout_.menuBar.isVisible()) {
        return buttons;
    }

    const auto allMenus = menus();
    float left = layout_.menuBar.x + 8.0f;
    buttons.reserve(allMenus.size());
    for (std::size_t index = 0; index < allMenus.size(); ++index) {
        const float buttonWidth = std::max(56.0f, 24.0f + static_cast<float>(allMenus[index].label.size()) * 8.0f);
        buttons.push_back(MenuBarButton{
            static_cast<int>(index),
            { left, layout_.menuBar.y + 2.0f, buttonWidth, layout_.menuBar.height - 4.0f }
        });
        left += buttonWidth + kMenuBarButtonSpacing;
    }

    return buttons;
}

std::optional<int> MainWindow::menuIndexAt(float x, float y) const
{
    for (const auto& button : menuBarButtons()) {
        if (pointInBounds(x, y, button.bounds.x, button.bounds.y, button.bounds.width, button.bounds.height)) {
            return button.menuIndex;
        }
    }

    return std::nullopt;
}

MainWindow::PanelBounds MainWindow::menuDropdownBounds(int menuIndex) const
{
    const auto allMenus = menus();
    const auto buttons = menuBarButtons();
    if (menuIndex < 0 || static_cast<std::size_t>(menuIndex) >= allMenus.size() || static_cast<std::size_t>(menuIndex) >= buttons.size()) {
        return {};
    }

    float longestTextWidth = 0.0f;
    float dropdownHeight = 8.0f;
    for (const auto& item : allMenus[menuIndex].items) {
        if (item.separator) {
            dropdownHeight += kMenuBarSeparatorHeight;
            continue;
        }

        const float itemWidthEstimate = static_cast<float>(item.label.size() + item.shortcut.size()) * 7.6f;
        longestTextWidth = std::max(longestTextWidth, itemWidthEstimate);
        dropdownHeight += kMenuBarItemHeight;
    }
    dropdownHeight += 8.0f;

    const float dropdownWidth = std::min(width() - 16.0f,
        std::max(kMenuBarDropdownMinWidth, 54.0f + longestTextWidth));
    const float maxX = std::max(8.0f, width() - dropdownWidth - 8.0f);
    const float dropdownX = std::clamp(buttons[menuIndex].bounds.x, 8.0f, maxX);

    return {
        dropdownX,
        layout_.menuBar.y + layout_.menuBar.height - 1.0f,
        dropdownWidth,
        dropdownHeight
    };
}

std::optional<MainWindow::MenuItemHit> MainWindow::menuItemAt(float x, float y) const
{
    const auto allMenus = menus();
    if (openMenuIndex_ < 0 || static_cast<std::size_t>(openMenuIndex_) >= allMenus.size()) {
        return std::nullopt;
    }

    const PanelBounds dropdownBounds = menuDropdownBounds(openMenuIndex_);
    if (!pointInBounds(x, y, dropdownBounds.x, dropdownBounds.y, dropdownBounds.width, dropdownBounds.height)) {
        return std::nullopt;
    }

    float top = dropdownBounds.y + 4.0f;
    for (std::size_t index = 0; index < allMenus[openMenuIndex_].items.size(); ++index) {
        const auto& item = allMenus[openMenuIndex_].items[index];
        const float itemHeight = item.separator ? kMenuBarSeparatorHeight : kMenuBarItemHeight;
        if (pointInBounds(x, y, dropdownBounds.x + 4.0f, top, dropdownBounds.width - 8.0f, itemHeight)) {
            return MenuItemHit{ openMenuIndex_, static_cast<int>(index), { dropdownBounds.x + 4.0f, top, dropdownBounds.width - 8.0f, itemHeight } };
        }
        top += itemHeight;
    }

    return std::nullopt;
}

void MainWindow::drawMenuBar(visage::Canvas& canvas) const
{
    if (!layout_.menuBar.isVisible()) {
        return;
    }

    canvas.setColor(0xff242a34);
    canvas.fill(layout_.menuBar.x, layout_.menuBar.y, layout_.menuBar.width, layout_.menuBar.height);
    canvas.setColor(0xff14161b);
    canvas.fill(layout_.menuBar.x, layout_.menuBar.y + layout_.menuBar.height - 1.0f, layout_.menuBar.width, 1.0f);

    if (!canDrawText()) {
        return;
    }

    const auto allMenus = menus();
    const auto buttons = menuBarButtons();
    for (const auto& button : buttons) {
        const bool isOpen = openMenuIndex_ == button.menuIndex;
        canvas.setColor(isOpen ? 0xff355382 : 0xff2b313c);
        canvas.fill(button.bounds.x, button.bounds.y, button.bounds.width, button.bounds.height);
        canvas.setColor(0xfff3f5f8);
        canvas.text(allMenus[button.menuIndex].label, labelFont_, visage::Font::kCenter,
            button.bounds.x, button.bounds.y, button.bounds.width, button.bounds.height);
    }

    if (openMenuIndex_ < 0 || static_cast<std::size_t>(openMenuIndex_) >= allMenus.size()) {
        return;
    }

    const PanelBounds dropdownBounds = menuDropdownBounds(openMenuIndex_);
    canvas.setColor(0xff232a34);
    canvas.fill(dropdownBounds.x, dropdownBounds.y, dropdownBounds.width, dropdownBounds.height);
    canvas.setColor(0xff101318);
    canvas.fill(dropdownBounds.x, dropdownBounds.y, dropdownBounds.width, 1.0f);
    canvas.fill(dropdownBounds.x, dropdownBounds.y + dropdownBounds.height - 1.0f, dropdownBounds.width, 1.0f);
    canvas.fill(dropdownBounds.x, dropdownBounds.y, 1.0f, dropdownBounds.height);
    canvas.fill(dropdownBounds.x + dropdownBounds.width - 1.0f, dropdownBounds.y, 1.0f, dropdownBounds.height);

    float top = dropdownBounds.y + 4.0f;
    for (const auto& item : allMenus[openMenuIndex_].items) {
        if (item.separator) {
            const float separatorY = top + (kMenuBarSeparatorHeight * 0.5f);
            canvas.setColor(0xff343c48);
            canvas.fill(dropdownBounds.x + 10.0f, separatorY, dropdownBounds.width - 20.0f, 1.0f);
            top += kMenuBarSeparatorHeight;
            continue;
        }

        canvas.setColor(item.enabled ? 0xff2c3340 : 0xff262c36);
        canvas.fill(dropdownBounds.x + 4.0f, top, dropdownBounds.width - 8.0f, kMenuBarItemHeight - 2.0f);
        if (item.checked) {
            canvas.setColor(0xff3c8c68);
            canvas.fill(dropdownBounds.x + 8.0f, top + 6.0f, 10.0f, 10.0f);
        }

        canvas.setColor(item.enabled ? 0xffe2e6ed : 0xff808999);
        canvas.text(item.label, labelFont_, visage::Font::kTopLeft,
            dropdownBounds.x + 24.0f, top + 4.0f, dropdownBounds.width - 110.0f, kMenuBarItemHeight - 6.0f);
        if (!item.shortcut.empty()) {
            canvas.setColor(item.enabled ? 0xffaab4c3 : 0xff6d7685);
            canvas.text(item.shortcut, labelFont_, visage::Font::kTopRight,
                dropdownBounds.x + dropdownBounds.width - 18.0f, top + 4.0f, 84.0f, kMenuBarItemHeight - 6.0f);
        }

        top += kMenuBarItemHeight;
    }
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
    hoverHint_.clear();
    cancelInspectorEdit();
    document_.selectWidget(widgetId);
    updatePropertyEditorBounds();
    if (const auto* widget = document_.selectedWidget()) {
        setOperationStatus("Selected: " + widgetDisplayName(*widget) + " (" + widget->id + ")");
    }
    else {
        statusMessage_.clear();
    }
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
        if (pointInBounds(x, y, button.bounds.x, button.bounds.y, button.bounds.width, button.bounds.height)) {
            return button;
        }
    }

    return std::nullopt;
}

void MainWindow::updateHoverHint(float x, float y)
{
    std::string nextHint;
    if (const auto menuIndex = menuIndexAt(x, y)) {
        nextHint = "Menu: " + menus()[*menuIndex].label;
    }
    else if (const auto itemHit = menuItemAt(x, y)) {
        const auto allMenus = menus();
        const auto& item = allMenus[itemHit->menuIndex].items[itemHit->itemIndex];
        nextHint = item.enabled ? "Hint: " + item.label : "Hint: Not available in this phase";
    }
    else if (const auto button = toolbarButtonAt(x, y)) {
        nextHint = "Hint: " + button->hint;
    }
    else if (const auto hint = widgetPalette_.hitTestHint(x, y)) {
        nextHint = "Hint: " + *hint;
    }
    else if (propertyInspector_.contains(x, y)) {
        if (const auto row = propertyInspector_.hitTestRow(document_, settings_, x, y)) {
            if (!row->isSection && !row->hint.empty()) {
                nextHint = "Hint: " + row->hint;
            }
        }
    }
    else if (const auto widgetId = designerCanvas_.hitTestWidgetId(document_, x, y)) {
        if (const auto* widget = document_.findWidgetById(*widgetId)) {
            const std::string widgetHint = widget->getStringProperty("hint", {});
            if (!widgetHint.empty()) {
                nextHint = "Hint: " + widgetHint;
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
    const auto addButton = [&](CommandId command, std::string label) {
        const float buttonWidth = std::max(kToolbarButtonMinWidth, 14.0f + static_cast<float>(label.size()) * 8.0f);
        const bool accent = isCommandChecked(command) || command == CommandId::SaveProjectAsDialog || command == CommandId::ExportCode || command == CommandId::ValidateProject;
        buttons.push_back(ToolbarButton{ command, std::move(label), commandHintText(command), { left, top, buttonWidth, kToolbarButtonHeight }, accent });
        left += buttonWidth + kToolbarButtonSpacing;
    };

    addButton(CommandId::NewProject, "New");
    addButton(CommandId::OpenProject, "Open");
    addButton(CommandId::SaveProject, "Save");
    addButton(CommandId::SaveProjectAsDialog, "Save As");
    addButton(CommandId::ExportCode, "Export");
    addButton(CommandId::ValidateProject, "Chk");
    addButton(CommandId::UndoAction, "Undo");
    addButton(CommandId::RedoAction, "Redo");
    addButton(CommandId::CopyWidgets, "Copy");
    addButton(CommandId::PasteWidgets, "Paste");
    addButton(CommandId::DeleteWidget, "Delete");
    addButton(CommandId::ToggleMultiSelect, "Multi");
    addButton(CommandId::ToggleGrid, "Grid");
    addButton(CommandId::ToggleSnap, "Snap");
    addButton(CommandId::ToggleSmartGuides, "Guides");

    return buttons;
}

bool MainWindow::activateMenuItem(const MenuItem& item)
{
    if (item.separator || !item.enabled) {
        return false;
    }

    if (item.filePath.has_value()) {
        return openRecentFile(*item.filePath);
    }

    if (item.widgetType.has_value()) {
        addWidgetFromPalette(*item.widgetType);
        return true;
    }

    if (item.command != CommandId::None) {
        return executeCommand(item.command);
    }

    return false;
}

bool MainWindow::handleMenuMouseDown(const visage::MouseEvent& e)
{
    if (const auto menuIndex = menuIndexAt(e.position.x, e.position.y)) {
        openMenuIndex_ = openMenuIndex_ == *menuIndex ? -1 : *menuIndex;
        redraw();
        return true;
    }

    if (openMenuIndex_ < 0) {
        return false;
    }

    if (const auto itemHit = menuItemAt(e.position.x, e.position.y)) {
        const auto allMenus = menus();
        if (static_cast<std::size_t>(itemHit->menuIndex) < allMenus.size()) {
            const auto& menu = allMenus[itemHit->menuIndex];
            if (static_cast<std::size_t>(itemHit->itemIndex) < menu.items.size()) {
                activateMenuItem(menu.items[itemHit->itemIndex]);
            }
        }
        openMenuIndex_ = -1;
        redraw();
        return true;
    }

    openMenuIndex_ = -1;
    redraw();
    return true;
}

bool MainWindow::executeCommand(CommandId command)
{
    switch (command) {
    case CommandId::NewProject:
        return newProject();
    case CommandId::OpenProject:
        return openProjectDialog();
    case CommandId::OpenSample:
        return openSampleProject();
    case CommandId::SaveProject:
        return saveProject();
    case CommandId::SaveProjectAsDialog:
        return saveProjectAsDialog();
    case CommandId::ExportCode:
        return exportGeneratedCode();
    case CommandId::ValidateProject:
        return validateProject();
    case CommandId::ShowValidationReport:
        showEditorMessageDialog("Validation Report",
            "Full report written to Generated/validation_report.md\n"
            "Open that file from the workspace to inspect the full markdown report.");
        return true;
    case CommandId::ShowAboutDialog:
        showEditorMessageDialog("About VisiForm", visiform::aboutDialogText());
        return true;
    case CommandId::ShowKeyboardShortcuts:
        return openKeyboardShortcutsDialog();
    case CommandId::ShowGeneratedCodeGuide:
        showEditorMessageDialog("Generated Code Guide",
            "Use Export to write a generated Visage project to the selected export folder.\n"
            "Use Project Settings to adjust executable naming, subclass naming, and local Visage export settings.\n"
            "See docs/code_generation.md for generated files, presets, and validation behavior.");
        return true;
    case CommandId::ShowProjectSettings:
        return openProjectSettingsDialog();
    case CommandId::ShowResourceManager:
        return openResourceManagerDialog();
    case CommandId::ShowExportDependencies:
        showEditorMessageDialog("Export Dependencies",
            "Use Project Settings to adjust local Visage source, repository, and tag values used during export.");
        return true;
    case CommandId::FitText:
        fitSelectedWidgetToText();
        return true;
    case CommandId::CopyWidgets:
        copySelectedWidgets();
        return true;
    case CommandId::PasteWidgets:
        pasteWidgets();
        return true;
    case CommandId::DeleteWidget:
        deleteSelectedWidget();
        return true;
    case CommandId::DuplicateWidget:
        duplicateSelectedWidget();
        return true;
    case CommandId::ToggleMultiSelect:
        toggleMultiSelectMode();
        return true;
    case CommandId::AlignLeft:
        alignSelectedLeft();
        return true;
    case CommandId::AlignTop:
        alignSelectedTop();
        return true;
    case CommandId::AlignRight:
        alignSelectedRight();
        return true;
    case CommandId::AlignBottom:
        alignSelectedBottom();
        return true;
    case CommandId::CenterHorizontally:
        centerSelectedHorizontally();
        return true;
    case CommandId::CenterVertically:
        centerSelectedVertically();
        return true;
    case CommandId::SameWidth:
        makeSelectedSameWidth();
        return true;
    case CommandId::SameHeight:
        makeSelectedSameHeight();
        return true;
    case CommandId::DistributeHorizontally:
        distributeSelectedHorizontally();
        return true;
    case CommandId::DistributeVertically:
        distributeSelectedVertically();
        return true;
    case CommandId::ToggleSmartGuides:
        toggleSmartGuides();
        return true;
    case CommandId::BringForward:
        bringSelectedForward();
        return true;
    case CommandId::SendBackward:
        sendSelectedBackward();
        return true;
    case CommandId::ToggleGrid:
        toggleGrid();
        return true;
    case CommandId::ToggleSnap:
        toggleSnapToGrid();
        return true;
    case CommandId::UndoAction:
        undo();
        return true;
    case CommandId::RedoAction:
        redo();
        return true;
    case CommandId::None:
    default:
        return false;
    }
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
    const bool saved = settings_.save(errorMessage);
    (void)saved;
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

std::string MainWindow::inspectorPropertyLabel(const std::string& key) const
{
    if (const auto row = propertyInspector_.activeRow(document_, settings_)) {
        if (row->key == key && !row->label.empty()) {
            return row->label;
        }
    }

    return key;
}

std::string MainWindow::editorModalFieldLabel(const std::string& key) const
{
    for (const auto& field : editorModalFields()) {
        if (field.key == key && !field.label.empty()) {
            return field.label;
        }
    }

    return key;
}

bool MainWindow::beginInspectorEdit(const PropertyInspector::PropertyRow& row)
{
    if (row.key == "items" || row.key == "itemActions") {
        const auto* selectedWidget = document_.selectedWidget();
        if (selectedWidget != nullptr && model::supportsItemList(selectedWidget->type)) {
            return openSelectedWidgetItemEditor();
        }
    }

    if (row.key == "columns" || row.key == "rows") {
        const auto* selectedWidget = document_.selectedWidget();
        if (selectedWidget != nullptr && model::supportsTableGrid(selectedWidget->type)) {
            return openSelectedTableGridEditor();
        }
    }

    if (row.key == "nodes") {
        const auto* selectedWidget = document_.selectedWidget();
        if (selectedWidget != nullptr && model::supportsTreeNodes(selectedWidget->type)) {
            return openSelectedTreeNodeEditor();
        }
    }

    if (row.editKind == PropertyInspector::PropertyEditKind::ReadOnly
        || row.editKind == PropertyInspector::PropertyEditKind::Bool
        || row.editKind == PropertyInspector::PropertyEditKind::Slider) {
        return false;
    }

    if (!propertyInspector_.beginEditing(document_, settings_, row.key)) {
        return false;
    }

    if (row.editKind == PropertyInspector::PropertyEditKind::Choice) {
        openInspectorDropdown(row);
        redraw();
        return true;
    }

    textEditControl_.begin(row.displayValue);
    updatePropertyEditorBounds();
    if (!row.choices.empty()) {
        openInspectorDropdown(row);
    }
    redraw();
    return true;
}

bool MainWindow::applyPendingInspectorInteractionEdit()
{
    const auto pendingEdit = propertyInspector_.consumeInteractionEdit();
    if (!pendingEdit.has_value()) {
        return false;
    }

    return setSelectedWidgetPropertyFromString(pendingEdit->key, pendingEdit->valueText);
}

bool MainWindow::commitInspectorEdit()
{
    if (!propertyInspector_.isEditing()) {
        return true;
    }

    std::string propertyLabel;
    if (const auto activeRow = propertyInspector_.activeRow(document_, settings_)) {
        propertyLabel = activeRow->label.empty() ? activeRow->key : activeRow->label;
    }
    const auto pendingEdit = propertyInspector_.buildPendingEdit(textEditControl_.text());
    if (!pendingEdit.has_value()) {
        cancelInspectorEdit();
        return true;
    }

    if (!setSelectedWidgetPropertyFromString(pendingEdit->key, pendingEdit->valueText)) {
        redraw();
        return false;
    }

    propertyInspector_.clearEditing();
    textEditControl_.clear();
    dropdownControl_.close();
    requestKeyboardFocus();
    if (!propertyLabel.empty()) {
        setOperationStatus("Property changed: " + propertyLabel);
    }
    redraw();
    return true;
}

void MainWindow::cancelInspectorEdit()
{
    propertyInspector_.cancelEditing();
    textEditControl_.clear();
    dropdownControl_.close();
    requestKeyboardFocus();
    redraw();
}

void MainWindow::cancelPopupEditors()
{
    dropdownControl_.close();

    if (isEditorModalVisible() || !propertyInspector_.isEditing()) {
        return;
    }

    const auto activeRow = propertyInspector_.activeRow(document_, settings_);
    if (!activeRow.has_value() || activeRow->editKind != PropertyInspector::PropertyEditKind::Choice) {
        return;
    }

    propertyInspector_.clearEditing();
    textEditControl_.clear();
    requestKeyboardFocus();
    updatePropertyEditorBounds();
}

bool MainWindow::applySelectedWidgetCallbackProperty(const std::string& propertyKey, const std::string& callbackName)
{
    if (!setSelectedWidgetPropertyFromString(propertyKey, callbackName)) {
        setOperationStatus(std::string("Failed to apply callback: ") + callbackName);
        redraw();
        return false;
    }

    propertyInspector_.clearEditing();
    textEditControl_.clear();
    dropdownControl_.close();
    requestKeyboardFocus();
    setOperationStatus(std::string("Callback selected: ") + callbackName);
    redraw();
    return true;
}

void MainWindow::updatePropertyEditorBounds()
{
    if (editorModalEdit_.active) {
        updateEditorModalEditorBounds();
        return;
    }

    if (!propertyInspector_.isEditing()) {
        return;
    }

    const auto bounds = propertyInspector_.activeEditorBounds(document_, settings_);
    if (!bounds.has_value()) {
        return;
    }

    textEditControl_.setBounds(bounds->x, bounds->y, bounds->width, bounds->height);
}

void MainWindow::openInspectorDropdown(const PropertyInspector::PropertyRow& row)
{
    const auto anchor = propertyInspector_.activeEditorBounds(document_, settings_);
    const auto viewport = activeDropdownViewportBounds();
    if (!anchor.has_value() || !viewport.has_value() || row.choices.empty()) {
        dropdownControl_.close();
        return;
    }

    dropdownControl_.open(row.key,
        { anchor->x, anchor->y, anchor->width, anchor->height },
        *viewport,
        dropdownItemsFromChoices(row.choices),
        document_.selectedWidget() != nullptr ? document_.selectedWidget()->getStringProperty(row.key, row.displayValue) : row.displayValue);
}

bool MainWindow::applyInspectorDropdownSelection(const std::string& key, const std::string& value, const std::string& label)
{
    if ((key == "items" && value == "__edit_items") || (key == "__edit_items" && value == "edit")) {
        const bool opened = openSelectedWidgetItemEditor();
        if (!opened) {
            redraw();
            return false;
        }

        propertyInspector_.clearEditing();
        textEditControl_.clear();
        dropdownControl_.close();
        requestKeyboardFocus();
        redraw();
        return true;
    }

    if (key == "__groupbox_select_child"
        || key == "__groupbox_add_existing_child"
        || key == "__groupbox_remove_child") {
        const bool applied = key == "__groupbox_select_child"
            ? selectGroupBoxChildFromInspector(value)
            : (key == "__groupbox_add_existing_child"
                    ? addExistingWidgetToSelectedGroupBox(value)
                    : removeSelectedGroupBoxChildToRoot(value));
        if (!applied) {
            redraw();
            return false;
        }

        propertyInspector_.clearEditing();
        textEditControl_.clear();
        dropdownControl_.close();
        requestKeyboardFocus();
        updatePropertyEditorBounds();
        redraw();
        return true;
    }

    const std::string propertyLabel = inspectorPropertyLabel(key);
    if (!setSelectedWidgetPropertyFromString(key, value)) {
        redraw();
        return false;
    }

    propertyInspector_.clearEditing();
    textEditControl_.clear();
    dropdownControl_.close();
    requestKeyboardFocus();
    if (key == "resourceId") {
        if (value.empty()) {
            setOperationStatus(propertyLabel + ": cleared");
        }
        else if (const auto* resource = document_.findResourceById(value)) {
            setOperationStatus(propertyLabel + ": " + resourceDisplayName(*resource));
        }
        else {
            setOperationStatus(propertyLabel + ": " + label);
        }
    }
    else {
        setOperationStatus(propertyLabel + ": " + label);
    }
    redraw();
    return true;
}

bool MainWindow::selectGroupBoxChildFromInspector(const std::string& childId)
{
    const auto* groupBox = document_.selectedWidget();
    if (groupBox == nullptr || groupBox->type != model::WidgetType::GroupBox || childId.empty()) {
        return false;
    }

    const auto* parent = document_.findParentOf(childId);
    const auto* child = document_.findWidgetById(childId);
    if (child == nullptr || parent == nullptr || parent->id != groupBox->id) {
        return false;
    }

    document_.setSelection(childId);
    setOperationStatus("Selected: " + widgetDisplayName(*child) + " (" + childId + ")");
    return true;
}

bool MainWindow::selectTabPageFromInspector(const std::string& tabPageId)
{
    const auto* selectedWidget = document_.selectedWidget();
    if (selectedWidget == nullptr || tabPageId.empty()) {
        return false;
    }

    const auto* tabPage = document_.findWidgetById(tabPageId);
    const auto* tabControl = selectedWidget->type == model::WidgetType::TabControl
        ? selectedWidget
        : document_.findTabControlFor(selectedWidget->id);
    if (tabPage == nullptr || tabPage->type != model::WidgetType::TabPage || tabControl == nullptr) {
        return false;
    }

    const auto* pageParent = document_.findParentOf(tabPageId);
    if (pageParent == nullptr || pageParent->id != tabControl->id) {
        return false;
    }

    int tabIndex = 0;
    for (const auto& child : tabControl->children) {
        if (child.type != model::WidgetType::TabPage) {
            continue;
        }
        if (child.id == tabPageId) {
            return applyUndoableDocumentChange("Change selected tab", [this, tabControl, tabIndex]() {
                if (auto* currentTabControl = document_.findWidgetById(tabControl->id)) {
                    currentTabControl->setSelectedTabIndex(tabIndex);
                    document_.setSelection(currentTabControl->id);
                    return true;
                }
                return false;
            });
        }
        ++tabIndex;
    }

    return false;
}

bool MainWindow::addExistingWidgetToSelectedGroupBox(const std::string& childId)
{
    const auto* groupBox = document_.selectedWidget();
    if (groupBox == nullptr || groupBox->type != model::WidgetType::GroupBox || childId.empty()) {
        return false;
    }

    const auto* child = document_.findWidgetById(childId);
    const auto* currentParent = document_.findParentOf(childId);
    if (child == nullptr || currentParent == nullptr || currentParent->id != document_.root.id || childId == groupBox->id) {
        return false;
    }

    const std::string childLabel = widgetDisplayName(*child);
    const std::string groupBoxLabel = widgetDisplayName(*groupBox);
    model::ProjectDocument beforeDocument = document_;
    model::ProjectDocument afterDocument = document_;
    const model::Rect childAbsoluteBounds = absoluteBoundsForWidget(afterDocument, childId);
    const model::Rect localBounds = boundsRelativeToParent(afterDocument, groupBox->id, childAbsoluteBounds);
    if (!afterDocument.reparentWidget(childId, groupBox->id, localBounds)) {
        return false;
    }

    afterDocument.setSelection(childId);
    document_ = beforeDocument;
    undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
        document_,
        "Add GroupBox child",
        std::move(beforeDocument),
        std::move(afterDocument)));
    document_.markDirty();
    setOperationStatus("Added " + childLabel + " to " + groupBoxLabel);
    return true;
}

bool MainWindow::removeSelectedGroupBoxChildToRoot(const std::string& childId)
{
    const auto* groupBox = document_.selectedWidget();
    if (groupBox == nullptr || groupBox->type != model::WidgetType::GroupBox || childId.empty()) {
        return false;
    }

    const auto* currentParent = document_.findParentOf(childId);
    const auto* child = document_.findWidgetById(childId);
    if (child == nullptr || currentParent == nullptr || currentParent->id != groupBox->id) {
        return false;
    }

    const std::string childLabel = widgetDisplayName(*child);
    const std::string groupBoxLabel = widgetDisplayName(*groupBox);
    model::ProjectDocument beforeDocument = document_;
    model::ProjectDocument afterDocument = document_;
    const model::Rect childAbsoluteBounds = absoluteBoundsForWidget(afterDocument, childId);
    if (!afterDocument.reparentWidget(childId, afterDocument.root.id, childAbsoluteBounds)) {
        return false;
    }

    afterDocument.setSelection(childId);
    document_ = beforeDocument;
    undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
        document_,
        "Remove GroupBox child",
        std::move(beforeDocument),
        std::move(afterDocument)));
    document_.markDirty();
    setOperationStatus("Removed " + childLabel + " from " + groupBoxLabel);
    return true;
}

bool MainWindow::addTabPageToSelectedTabControl()
{
    const auto* tabControl = document_.selectedWidget();
    if (tabControl == nullptr || tabControl->type != model::WidgetType::TabControl) {
        return false;
    }

    const std::string newPageId = idGenerator_.next(model::WidgetType::TabPage, document_);
    const std::string newTitle = "Tab " + std::to_string(tabControl->tabPageCount() + 1);
    model::ProjectDocument beforeDocument = document_;
    model::ProjectDocument afterDocument = document_;
    auto* updatedTabControl = afterDocument.findWidgetById(tabControl->id);
    if (updatedTabControl == nullptr) {
        return false;
    }

    model::WidgetNode newPage = model::WidgetRegistry::instance().createDefaultWidget(model::WidgetType::TabPage, newPageId);
    newPage.setProperty("title", newTitle);
    newPage.bounds = tabPageBoundsForTabControl(*updatedTabControl);
    updatedTabControl->appendChild(std::move(newPage));
    updatedTabControl->setSelectedTabIndex(static_cast<int>(updatedTabControl->tabPageCount()) - 1);
    afterDocument.setSelection(updatedTabControl->id);

    document_ = beforeDocument;
    undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
        document_,
        "Add tab page",
        std::move(beforeDocument),
        std::move(afterDocument)));
    document_.markDirty();
    setOperationStatus("Added " + newTitle + " to " + widgetDisplayName(*tabControl));
    return true;
}

bool MainWindow::removeSelectedTabPageFromSelectedTabControl()
{
    const auto* tabControl = document_.selectedWidget();
    if (tabControl == nullptr || tabControl->type != model::WidgetType::TabControl) {
        return false;
    }

    const auto* selectedPage = document_.selectedTabPageFor(tabControl->id);
    if (selectedPage == nullptr) {
        return false;
    }
    if (tabControl->tabPageCount() <= 1) {
        setOperationStatus("Cannot remove the last tab page");
        return false;
    }
    if (!selectedPage->children.empty()) {
        setOperationStatus("Cannot remove tab with child widgets yet.");
        return false;
    }

    model::ProjectDocument beforeDocument = document_;
    model::ProjectDocument afterDocument = document_;
    const int previousIndex = tabControl->selectedTabIndex();
    if (!afterDocument.removeWidgetById(selectedPage->id)) {
        return false;
    }

    if (auto* updatedTabControl = afterDocument.findWidgetById(tabControl->id)) {
        const int remainingCount = static_cast<int>(updatedTabControl->tabPageCount());
        updatedTabControl->setSelectedTabIndex(std::clamp(previousIndex, 0, std::max(0, remainingCount - 1)));
        afterDocument.setSelection(updatedTabControl->id);
    }
    else {
        return false;
    }

    document_ = beforeDocument;
    undoRedo_.executeCommand(std::make_unique<commands::DocumentStateCommand>(
        document_,
        "Remove tab page",
        std::move(beforeDocument),
        std::move(afterDocument)));
    document_.markDirty();
    setOperationStatus("Removed " + selectedPage->tabTitle() + " from " + widgetDisplayName(*tabControl));
    return true;
}

bool MainWindow::openSelectedWidgetItemEditor()
{
    const auto* widget = document_.selectedWidget();
    if (widget == nullptr || !model::supportsItemList(widget->type)) {
        return false;
    }

    cancelInspectorEdit();
    cancelEditorModalFieldEdit();
    clearCanvasInteraction();
    requestKeyboardFocus();

    itemListEditorDialog_.visible = true;
    itemListEditorDialog_.widgetId = widget->id;
    itemListEditorDialog_.widgetType = widget->type;
    itemListEditorDialog_.originalItemsText = widget->getStringProperty("items", {});
    itemListEditorDialog_.originalActionsText = widget->getStringProperty("itemActions", {});
    itemListEditorDialog_.items = model::getWidgetItems(*widget);
    itemListEditorDialog_.supportsActions = model::supportsItemActions(widget->type);
    itemListEditorDialog_.actions = itemListEditorDialog_.supportsActions
        ? model::getWidgetItemActions(*widget)
        : std::vector<std::string>{};
    const std::string selectedIndexKey = std::string(model::selectedItemIndexPropertyKey(widget->type));
    itemListEditorDialog_.originalSelectedIndex = model::clampSelectedIndex(
        itemListEditorDialog_.items,
        widget->getIntProperty(selectedIndexKey, itemListEditorDialog_.items.empty() ? -1 : 0));
    itemListEditorDialog_.selectedItemIndex = itemListEditorDialog_.originalSelectedIndex;
    itemListEditorDialog_.previewScrollOffset = 0;

    editorModal_.visible = true;
    editorModal_.mode = EditorModalMode::ItemListEditor;
    editorModal_.title = itemListEditorDialogTitle(widget->type);
    editorModal_.message.clear();
    editorModal_.lines.clear();
    editorModal_.buttons = {
        { "add_item", "Add Item" },
        { "remove_item", "Remove Item" },
        { "move_up_item", "Move Up" },
        { "move_down_item", "Move Down" },
        { "apply_items", "Apply" },
        { "cancel", "Cancel" }
    };
    editorModal_.result.clear();
    editorModal_.statusText = itemListEditorDefaultStatus(itemListEditorDialog_.supportsActions);
    editorModal_.preferredWidth = kItemListEditorModalWidth;
    editorModal_.preferredHeight = kItemListEditorModalHeight;
    newProjectWizard_.visible = false;
    projectSettingsDialog_.visible = false;
    resourceManagerDialog_.visible = false;
    keyboardShortcutDialog_.visible = false;
    setItemListEditorSelectedIndex(itemListEditorDialog_.selectedItemIndex);
    redraw();
    return true;
}

void MainWindow::setItemListEditorSelectedIndex(int index)
{
    const int safeIndex = model::sanitizeSelectedIndex(itemListEditorDialog_.items, index);
    itemListEditorDialog_.selectedItemIndex = safeIndex;

    // Ensure the selected index remains visible by clamping previewScrollOffset
    if (safeIndex >= 0) {
        const PanelBounds preview = itemListEditorPreviewBounds();
        const std::size_t visibleCount = itemListEditorVisibleRowCount(preview.height, itemListEditorDialog_.items.size());
        if (visibleCount > 0) {
            const int maxOffset = std::max(0, static_cast<int>(itemListEditorDialog_.items.size()) - static_cast<int>(visibleCount));
            if (safeIndex < itemListEditorDialog_.previewScrollOffset) {
                itemListEditorDialog_.previewScrollOffset = safeIndex;
            }
            else if (safeIndex >= itemListEditorDialog_.previewScrollOffset + static_cast<int>(visibleCount)) {
                itemListEditorDialog_.previewScrollOffset = safeIndex - static_cast<int>(visibleCount) + 1;
            }
            itemListEditorDialog_.previewScrollOffset = std::clamp(itemListEditorDialog_.previewScrollOffset, 0, maxOffset);
        }

        editorModal_.statusText = itemListEditorSelectedStatus(safeIndex, itemListEditorDialog_.items.size());
    }
    else {
        editorModal_.statusText = itemListEditorNoSelectionStatus(itemListEditorDialog_.supportsActions);
    }
}

bool MainWindow::applyItemListEditor()
{
    const auto* selectedWidget = document_.selectedWidget();
    if (selectedWidget == nullptr || selectedWidget->id != itemListEditorDialog_.widgetId || !model::supportsItemList(selectedWidget->type)) {
        return false;
    }

    if (editorModalEdit_.active && !commitEditorModalFieldEdit()) {
        return false;
    }

    const std::string itemsText = model::joinItems(itemListEditorDialog_.items);
    const std::string actionsText = itemListEditorDialog_.supportsActions
        ? model::joinItemActions(itemListEditorDialog_.actions)
        : std::string{};
    const int selectedIndex = model::clampSelectedIndex(itemListEditorDialog_.items, itemListEditorDialog_.selectedItemIndex);
    const std::string selectedIndexKey = std::string(model::selectedItemIndexPropertyKey(selectedWidget->type));
    const std::string widgetTypeName = itemListWidgetTypeLabel(selectedWidget->type);

    const bool applied = applyUndoableDocumentChange("Edit items", [this, &itemsText, &actionsText, &selectedIndexKey, selectedIndex]() {
        auto* widget = document_.selectedWidget();
        if (widget == nullptr || !model::supportsItemList(widget->type)) {
            return false;
        }

        widget->setProperty("items", itemsText);
        if (model::supportsItemActions(widget->type)) {
            widget->setProperty("itemActions", actionsText);
        }
        widget->setProperty(selectedIndexKey, selectedIndex);
        model::normalizeItemListProperties(*widget);
        return true;
    });
    if (!applied) {
        return false;
    }

    setOperationStatus("Updated " + widgetTypeName + " items");
    closeEditorModalDialog("apply_items");
    return true;
}

bool MainWindow::openSelectedTableGridEditor()
{
    const auto* widget = document_.selectedWidget();
    if (widget == nullptr || !model::supportsTableGrid(widget->type)) {
        return false;
    }

    cancelInspectorEdit();
    cancelEditorModalFieldEdit();
    clearCanvasInteraction();
    requestKeyboardFocus();

    tableGridEditorDialog_.visible = true;
    tableGridEditorDialog_.widgetId = widget->id;
    tableGridEditorDialog_.originalColumnsText = widget->getStringProperty("columns", {});
    tableGridEditorDialog_.originalRowsText = widget->getStringProperty("rows", {});
    tableGridEditorDialog_.originalSelectedRow = widget->getIntProperty("selectedRow", -1);
    tableGridEditorDialog_.originalSelectedColumn = widget->getIntProperty("selectedColumn", -1);

    const auto data = model::normalizeTableData(
        tableGridEditorDialog_.originalColumnsText,
        tableGridEditorDialog_.originalRowsText);
    tableGridEditorDialog_.columns = data.columns;
    tableGridEditorDialog_.rows = data.rows;
    refreshTableGridEditorState();
    const auto selection = model::clampSelectedCell(
        tableGridEditorDialog_.columns,
        tableGridEditorDialog_.rows,
        tableGridEditorDialog_.originalSelectedRow,
        tableGridEditorDialog_.originalSelectedColumn);
    tableGridEditorDialog_.selectedRow = selection.row;
    tableGridEditorDialog_.selectedColumn = selection.column;

    editorModal_.visible = true;
    editorModal_.mode = EditorModalMode::TableGridEditor;
    editorModal_.title = "Edit Table/Grid Data";
    editorModal_.message.clear();
    editorModal_.lines.clear();
    editorModal_.buttons = {
        { "apply_table_grid", "Apply" },
        { "cancel", "Cancel" }
    };
    editorModal_.result.clear();
    editorModal_.statusText = tableGridEditorDialog_.rows.empty()
        ? "No rows yet. Add a row, select a cell, then click Apply to commit the table."
        : "Select a cell, edit Cell Text below, then click Apply to commit the table.";
    editorModal_.preferredWidth = kTableGridEditorModalWidth;
    editorModal_.preferredHeight = kTableGridEditorModalHeight;
    newProjectWizard_.visible = false;
    projectSettingsDialog_.visible = false;
    resourceManagerDialog_.visible = false;
    keyboardShortcutDialog_.visible = false;
    updateEditorModalEditorBounds();
    redraw();
    return true;
}

void MainWindow::refreshTableGridEditorState()
{
    const auto selection = model::clampSelectedCell(
        tableGridEditorDialog_.columns,
        tableGridEditorDialog_.rows,
        tableGridEditorDialog_.selectedRow,
        tableGridEditorDialog_.selectedColumn);
    tableGridEditorDialog_.selectedRow = selection.row;
    tableGridEditorDialog_.selectedColumn = selection.column;
}

void MainWindow::selectTableGridEditorCell(int row, int column, bool preserveRow)
{
    refreshTableGridEditorState();
    if (!preserveRow) {
        tableGridEditorDialog_.selectedRow = row;
    }
    tableGridEditorDialog_.selectedColumn = column;
    refreshTableGridEditorState();

    if (tableGridEditorDialog_.selectedRow >= 0 && tableGridEditorDialog_.selectedColumn >= 0) {
        editorModal_.statusText = "Selected cell R" + std::to_string(tableGridEditorDialog_.selectedRow + 1)
            + " C" + std::to_string(tableGridEditorDialog_.selectedColumn + 1) + ".";
    }
    else if (tableGridEditorDialog_.selectedColumn >= 0) {
        editorModal_.statusText = "Selected column " + tableGridEditorColumnLabel(tableGridEditorDialog_.columns, tableGridEditorDialog_.selectedColumn) + ".";
    }
    else if (tableGridEditorDialog_.rows.empty()) {
        editorModal_.statusText = "No rows yet. Add a row, select a cell, then click Apply to commit the table.";
    }
    else {
        editorModal_.statusText = "Select a cell to edit its text.";
    }
}

std::vector<MainWindow::TableGridEditorActionButton> MainWindow::tableGridEditorActionButtons() const
{
    std::vector<TableGridEditorActionButton> buttons;
    if (editorModal_.mode != EditorModalMode::TableGridEditor) {
        return buttons;
    }

    const PanelBounds formBounds = tableGridEditorFormBounds();
    const float gap = 10.0f;
    const float buttonWidth = std::max(120.0f, (formBounds.width - gap * 3.0f) / 4.0f);
    const bool hasColumn = tableGridEditorDialog_.selectedColumn >= 0 && tableGridEditorDialog_.selectedColumn < static_cast<int>(tableGridEditorDialog_.columns.size());
    const bool hasRow = tableGridEditorDialog_.selectedRow >= 0 && tableGridEditorDialog_.selectedRow < static_cast<int>(tableGridEditorDialog_.rows.size());

    const std::array<std::pair<const char*, const char*>, 7> definitions = {{
        { "add_column", "Add Column" },
        { "remove_column", "Remove Column" },
        { "rename_column", "Rename Column" },
        { "add_row", "Add Row" },
        { "remove_row", "Remove Row" },
        { "move_row_up", "Move Row Up" },
        { "move_row_down", "Move Row Down" }
    }};

    buttons.reserve(definitions.size());
    for (std::size_t index = 0; index < definitions.size(); ++index) {
        const int row = index < 4 ? 0 : 1;
        const int column = index < 4 ? static_cast<int>(index) : static_cast<int>(index - 4);
        bool enabled = true;
        if (std::string_view{ definitions[index].first } == "remove_column" || std::string_view{ definitions[index].first } == "rename_column") {
            enabled = hasColumn;
        }
        else if (std::string_view{ definitions[index].first } == "remove_row") {
            enabled = hasRow;
        }
        else if (std::string_view{ definitions[index].first } == "move_row_up") {
            enabled = hasRow && tableGridEditorDialog_.selectedRow > 0;
        }
        else if (std::string_view{ definitions[index].first } == "move_row_down") {
            enabled = hasRow && tableGridEditorDialog_.selectedRow + 1 < static_cast<int>(tableGridEditorDialog_.rows.size());
        }

        buttons.push_back({
            definitions[index].first,
            definitions[index].second,
            {
                formBounds.x + static_cast<float>(column) * (buttonWidth + gap),
                formBounds.y + kTableGridEditorActionLabelHeight + static_cast<float>(row) * (kTableGridEditorActionButtonHeight + kTableGridEditorActionRowGap),
                buttonWidth,
                kTableGridEditorActionButtonHeight
            },
            enabled
        });
    }

    return buttons;
}

std::optional<MainWindow::TableGridEditorCellHit> MainWindow::tableGridEditorCellAt(float x, float y) const
{
    if (editorModal_.mode != EditorModalMode::TableGridEditor || tableGridEditorDialog_.columns.empty()) {
        return std::nullopt;
    }

    const PanelBounds bounds = tableGridEditorGridBounds();
    if (!pointInBounds(x, y, bounds.x, bounds.y, bounds.width, bounds.height)) {
        return std::nullopt;
    }

    const float innerX = bounds.x + 6.0f;
    const float innerY = bounds.y + 6.0f;
    const float innerWidth = std::max(0.0f, bounds.width - 12.0f);
    const float cellWidth = innerWidth / static_cast<float>(std::max<std::size_t>(1, tableGridEditorDialog_.columns.size()));
    if (x < innerX || y < innerY || cellWidth <= 0.0f) {
        return std::nullopt;
    }

    const int column = std::clamp(static_cast<int>((x - innerX) / cellWidth), 0, static_cast<int>(tableGridEditorDialog_.columns.size()) - 1);
    if (y <= innerY + kTableGridEditorHeaderHeight) {
        return TableGridEditorCellHit{ -1, column, true };
    }

    const float rowAreaY = y - (innerY + kTableGridEditorHeaderHeight);
    if (rowAreaY < 0.0f) {
        return std::nullopt;
    }

    const int row = static_cast<int>(rowAreaY / kTableGridEditorRowHeight);
    if (row < 0 || row >= static_cast<int>(tableGridEditorDialog_.rows.size())) {
        return std::nullopt;
    }

    return TableGridEditorCellHit{ row, column, false };
}

bool MainWindow::activateTableGridEditorAction(const std::string& actionId)
{
    if (editorModalEdit_.active && !commitEditorModalFieldEdit()) {
        return false;
    }

    if (actionId == "add_column") {
        tableGridEditorDialog_.columns.push_back("Column " + std::to_string(tableGridEditorDialog_.columns.size() + 1));
        for (auto& row : tableGridEditorDialog_.rows) {
            row.push_back({});
        }
        selectTableGridEditorCell(tableGridEditorDialog_.selectedRow, static_cast<int>(tableGridEditorDialog_.columns.size()) - 1, true);
        editorModal_.statusText = "Added a new column.";
        redraw();
        return true;
    }

    if (actionId == "remove_column") {
        if (tableGridEditorDialog_.selectedColumn < 0 || tableGridEditorDialog_.selectedColumn >= static_cast<int>(tableGridEditorDialog_.columns.size())) {
            editorModal_.statusText = "Select a column before removing it.";
            redraw();
            return false;
        }

        const int selectedColumn = tableGridEditorDialog_.selectedColumn;
        tableGridEditorDialog_.columns.erase(tableGridEditorDialog_.columns.begin() + selectedColumn);
        for (auto& row : tableGridEditorDialog_.rows) {
            if (selectedColumn >= 0 && selectedColumn < static_cast<int>(row.size())) {
                row.erase(row.begin() + selectedColumn);
            }
        }
        refreshTableGridEditorState();
        editorModal_.statusText = "Removed the selected column.";
        redraw();
        return true;
    }

    if (actionId == "rename_column") {
        if (tableGridEditorDialog_.selectedColumn < 0 || tableGridEditorDialog_.selectedColumn >= static_cast<int>(tableGridEditorDialog_.columns.size())) {
            editorModal_.statusText = "Select a column before renaming it.";
            redraw();
            return false;
        }

        for (const auto& field : editorModalFields()) {
            if (field.key == "columnName") {
                beginEditorModalFieldEdit(field);
                editorModal_.statusText = "Edit Column Name, then press Enter to rename it.";
                return true;
            }
        }
        return false;
    }

    if (actionId == "add_row") {
        tableGridEditorDialog_.rows.push_back(std::vector<std::string>(tableGridEditorDialog_.columns.size()));
        selectTableGridEditorCell(static_cast<int>(tableGridEditorDialog_.rows.size()) - 1,
            tableGridEditorDialog_.selectedColumn >= 0 ? tableGridEditorDialog_.selectedColumn : (tableGridEditorDialog_.columns.empty() ? -1 : 0));
        editorModal_.statusText = "Added a new row.";
        redraw();
        return true;
    }

    if (actionId == "remove_row") {
        if (tableGridEditorDialog_.selectedRow < 0 || tableGridEditorDialog_.selectedRow >= static_cast<int>(tableGridEditorDialog_.rows.size())) {
            editorModal_.statusText = "Select a row before removing it.";
            redraw();
            return false;
        }

        tableGridEditorDialog_.rows.erase(tableGridEditorDialog_.rows.begin() + tableGridEditorDialog_.selectedRow);
        refreshTableGridEditorState();
        editorModal_.statusText = "Removed the selected row.";
        redraw();
        return true;
    }

    if (actionId == "move_row_up" || actionId == "move_row_down") {
        const int currentRow = tableGridEditorDialog_.selectedRow;
        if (currentRow < 0 || currentRow >= static_cast<int>(tableGridEditorDialog_.rows.size())) {
            editorModal_.statusText = "Select a row before reordering it.";
            redraw();
            return false;
        }

        const int targetRow = actionId == "move_row_up" ? currentRow - 1 : currentRow + 1;
        if (targetRow < 0 || targetRow >= static_cast<int>(tableGridEditorDialog_.rows.size())) {
            editorModal_.statusText = actionId == "move_row_up"
                ? "Selected row is already first."
                : "Selected row is already last.";
            redraw();
            return false;
        }

        std::swap(tableGridEditorDialog_.rows[static_cast<std::size_t>(currentRow)], tableGridEditorDialog_.rows[static_cast<std::size_t>(targetRow)]);
        tableGridEditorDialog_.selectedRow = targetRow;
        editorModal_.statusText = actionId == "move_row_up" ? "Moved row up." : "Moved row down.";
        redraw();
        return true;
    }

    return false;
}

bool MainWindow::applyTableGridEditor()
{
    const auto* selectedWidget = document_.selectedWidget();
    if (selectedWidget == nullptr || selectedWidget->id != tableGridEditorDialog_.widgetId || !model::supportsTableGrid(selectedWidget->type)) {
        return false;
    }

    if (editorModalEdit_.active && !commitEditorModalFieldEdit()) {
        return false;
    }

    const std::string columnsText = model::joinTableColumns(tableGridEditorDialog_.columns);
    const std::string rowsText = model::joinTableRows(tableGridEditorDialog_.rows);
    const auto selection = model::clampSelectedCell(
        tableGridEditorDialog_.columns,
        tableGridEditorDialog_.rows,
        tableGridEditorDialog_.selectedRow,
        tableGridEditorDialog_.selectedColumn);

    const bool applied = applyUndoableDocumentChange("Edit table data", [this, &columnsText, &rowsText, &selection]() {
        auto* widget = document_.selectedWidget();
        if (widget == nullptr || !model::supportsTableGrid(widget->type)) {
            return false;
        }

        widget->setProperty("columns", columnsText);
        widget->setProperty("rows", rowsText);
        widget->setProperty("selectedRow", selection.row);
        widget->setProperty("selectedColumn", selection.column);
        model::normalizeTableGridProperties(*widget);
        return true;
    });
    if (!applied) {
        return false;
    }

    setOperationStatus("Updated Table/Grid data.");
    closeEditorModalDialog("apply_table_grid");
    return true;
}

void MainWindow::handleTextEditPendingAction()
{
    const auto action = textEditControl_.consumePendingAction();
    if (!action.has_value()) {
        return;
    }

    if (*action == editors::TextEditControl::PendingAction::Commit) {
        if (editorModalEdit_.active) {
            commitEditorModalFieldEdit();
        }
        else if (editorModal_.mode == EditorModalMode::TreeNodeEditor) {
            redraw();
        }
        else {
            commitInspectorEdit();
        }
        return;
    }

    if (editorModalEdit_.active) {
        cancelEditorModalFieldEdit();
        return;
    }

    if (editorModal_.mode == EditorModalMode::TreeNodeEditor) {
        closeEditorModalDialog("cancel");
        return;
    }

    cancelInspectorEdit();
}

void MainWindow::handleDropdownSelection()
{
    const auto selection = dropdownControl_.consumeSelection();
    if (!selection.has_value()) {
        return;
    }

    if (isEditorModalVisible()) {
        applyEditorModalDropdownSelection(selection->key, selection->value, selection->label);
    }
    else {
        applyInspectorDropdownSelection(selection->key, selection->value, selection->label);
    }
}

void MainWindow::clearCanvasInteraction()
{
    canvasInteraction_ = {};
}

std::vector<MainWindow::EditorModalField> MainWindow::editorModalFields() const
{
    std::vector<EditorModalField> fields;
    const std::vector<PropertyInspector::PropertyChoice> lookAndFeelChoices = propertyChoicesFromValues(availableLookAndFeelIds());
    const std::vector<PropertyInspector::PropertyChoice> templateChoices = propertyChoicesFromValues(newProjectTemplateIds());
    if (editorModal_.mode == EditorModalMode::NewProjectWizard) {
        fields.push_back(EditorModalField{ "projectName", "Project Name", newProjectWizard_.projectName, PropertyInspector::PropertyEditKind::Text });
        fields.push_back(EditorModalField{ "executableName", "Executable Name", newProjectWizard_.executableName, PropertyInspector::PropertyEditKind::Text });
        fields.push_back(EditorModalField{ "windowTitle", "Window Title", newProjectWizard_.windowTitle, PropertyInspector::PropertyEditKind::Text });
        fields.push_back(EditorModalField{ "userSubclassName", "User Subclass Name", newProjectWizard_.userSubclassName, PropertyInspector::PropertyEditKind::Text });
        fields.push_back(EditorModalField{ "formWidth", "Form Width", std::to_string(newProjectWizard_.formWidth), PropertyInspector::PropertyEditKind::Integer });
        fields.push_back(EditorModalField{ "formHeight", "Form Height", std::to_string(newProjectWizard_.formHeight), PropertyInspector::PropertyEditKind::Integer });
        fields.push_back(EditorModalField{ "lookAndFeelId", "Look And Feel", newProjectWizard_.lookAndFeelId, PropertyInspector::PropertyEditKind::Choice, lookAndFeelChoices });
        fields.push_back(EditorModalField{ "templateId", "Template", newProjectWizard_.templateId, PropertyInspector::PropertyEditKind::Choice, templateChoices });
    }
    else if (editorModal_.mode == EditorModalMode::ProjectSettings) {
        fields.push_back(EditorModalField{ "projectName", "Project Name", projectSettingsDialog_.projectName, PropertyInspector::PropertyEditKind::Text });
        fields.push_back(EditorModalField{ "executableName", "Executable Name", projectSettingsDialog_.executableName, PropertyInspector::PropertyEditKind::Text });
        fields.push_back(EditorModalField{ "windowTitle", "Window Title", projectSettingsDialog_.windowTitle, PropertyInspector::PropertyEditKind::Text });
        fields.push_back(EditorModalField{ "userSubclassName", "User Subclass Name", projectSettingsDialog_.userSubclassName, PropertyInspector::PropertyEditKind::Text });
        fields.push_back(EditorModalField{ "lookAndFeelId", "Look And Feel", projectSettingsDialog_.lookAndFeelId, PropertyInspector::PropertyEditKind::Choice, lookAndFeelChoices });
        fields.push_back(EditorModalField{ "localVisageSourceDirectory", "Local Visage Source", projectSettingsDialog_.localVisageSourceDirectory, PropertyInspector::PropertyEditKind::Text });
        fields.push_back(EditorModalField{ "visageGitRepository", "Visage Git Repository", projectSettingsDialog_.visageGitRepository, PropertyInspector::PropertyEditKind::Text });
        fields.push_back(EditorModalField{ "visageGitTag", "Visage Git Tag", projectSettingsDialog_.visageGitTag, PropertyInspector::PropertyEditKind::Text });
    }
    else if (editorModal_.mode == EditorModalMode::ResourceManager) {
        std::vector<PropertyInspector::PropertyChoice> resourceChoices;
        resourceChoices.reserve(document_.resources.size());
        for (const auto& resource : document_.resources) {
            resourceChoices.push_back({ resource.id, resourceManagerChoiceLabel(resource) });
        }

        const auto* selectedResource = document_.findResourceById(resourceManagerDialog_.selectedResourceId);
        fields.push_back(EditorModalField{ "selectedResourceId", "Resource", resourceManagerDialog_.selectedResourceId, PropertyInspector::PropertyEditKind::Choice, std::move(resourceChoices) });
        fields.push_back(EditorModalField{ "resourceIdValue", "Resource ID", selectedResource == nullptr ? std::string{} : selectedResource->id, PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "resourceType", "Type", selectedResource == nullptr ? std::string{} : model::toString(selectedResource->type), PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "displayName", "Display Name", selectedResource == nullptr ? std::string{} : selectedResource->displayName, PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "sourcePath", "Source Path", selectedResource == nullptr ? std::string{} : selectedResource->sourcePath, PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "exportRelativePath", "Export Path", selectedResource == nullptr ? std::string{} : selectedResource->exportRelativePath, PropertyInspector::PropertyEditKind::ReadOnly });
    }
    else if (editorModal_.mode == EditorModalMode::KeyboardShortcuts) {
        std::vector<PropertyInspector::PropertyChoice> commandChoices;
        commandChoices.reserve(commands::CommandRegistry::definitions().size());
        for (const auto& definition : commands::CommandRegistry::definitions()) {
            commandChoices.push_back({
                std::string{ definition.id },
                std::string{ definition.menuPath } + " - " + std::string{ definition.displayName }
            });
        }

        const auto* selectedDefinition = keyboardShortcutDialog_.selectedCommandId.empty()
            ? nullptr
            : commands::CommandRegistry::find(keyboardShortcutDialog_.selectedCommandId);
        fields.push_back(EditorModalField{ "selectedCommandId", "Command", keyboardShortcutDialog_.selectedCommandId, PropertyInspector::PropertyEditKind::Choice, std::move(commandChoices) });
        fields.push_back(EditorModalField{ "commandCategory", "Category", selectedDefinition == nullptr ? std::string{} : std::string{ selectedDefinition->menuPath }, PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "defaultShortcut", "Default Shortcut", selectedDefinition == nullptr ? std::string{} : std::string{ selectedDefinition->defaultShortcut }, PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "shortcutText", "Current Shortcut", keyboardShortcutDialog_.selectedCommandId.empty() ? std::string{} : keyboardShortcutDialogEffectiveText(keyboardShortcutDialog_.selectedCommandId), PropertyInspector::PropertyEditKind::Text });
        fields.push_back(EditorModalField{ "commandHint", "Description", selectedDefinition == nullptr ? std::string{} : std::string{ selectedDefinition->hint }, PropertyInspector::PropertyEditKind::ReadOnly });
    }
    else if (editorModal_.mode == EditorModalMode::ItemListEditor) {
        fields.push_back(EditorModalField{ "itemCount", "Item Count", std::to_string(itemListEditorDialog_.items.size()), PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "itemIndex", "Selected Row", itemListEditorDialog_.selectedItemIndex >= 0
                ? std::to_string(itemListEditorDialog_.selectedItemIndex + 1)
                : std::string{ "<none>" }, PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "itemText", itemListEditorDialog_.supportsActions ? "Label" : "Item Text", itemListEditorDialog_.selectedItemIndex >= 0
                ? itemListEditorDialog_.items[static_cast<std::size_t>(itemListEditorDialog_.selectedItemIndex)]
                : std::string{},
            itemListEditorDialog_.selectedItemIndex >= 0 ? PropertyInspector::PropertyEditKind::Text : PropertyInspector::PropertyEditKind::ReadOnly });
        if (itemListEditorDialog_.supportsActions) {
            fields.push_back(EditorModalField{ "itemAction", "Callback / Action", itemListEditorDialog_.selectedItemIndex >= 0
                    ? (itemListEditorDialog_.selectedItemIndex < static_cast<int>(itemListEditorDialog_.actions.size())
                            ? itemListEditorDialog_.actions[static_cast<std::size_t>(itemListEditorDialog_.selectedItemIndex)]
                            : std::string{})
                    : std::string{},
                itemListEditorDialog_.selectedItemIndex >= 0 ? PropertyInspector::PropertyEditKind::Text : PropertyInspector::PropertyEditKind::ReadOnly });
        }
    }
    else if (editorModal_.mode == EditorModalMode::TableGridEditor) {
        fields.push_back(EditorModalField{ "columnCount", "Column Count", std::to_string(tableGridEditorDialog_.columns.size()), PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "rowCount", "Row Count", std::to_string(tableGridEditorDialog_.rows.size()), PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "selectedRow", "Selected Row", tableGridEditorDialog_.selectedRow >= 0
                ? std::to_string(tableGridEditorDialog_.selectedRow)
                : std::string{ "<none>" }, PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "selectedColumn", "Selected Column", tableGridEditorDialog_.selectedColumn >= 0
                ? tableGridEditorColumnLabel(tableGridEditorDialog_.columns, tableGridEditorDialog_.selectedColumn)
                : std::string{ "<none>" }, PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "columnName", "Column Name", tableGridEditorDialog_.selectedColumn >= 0
                ? tableGridEditorDialog_.columns[static_cast<std::size_t>(tableGridEditorDialog_.selectedColumn)]
                : std::string{},
            tableGridEditorDialog_.selectedColumn >= 0 ? PropertyInspector::PropertyEditKind::Text : PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "cellText", "Cell Text", model::getCellText(tableGridEditorDialog_.rows, tableGridEditorDialog_.selectedRow, tableGridEditorDialog_.selectedColumn),
            tableGridEditorDialog_.selectedRow >= 0 && tableGridEditorDialog_.selectedColumn >= 0
                ? PropertyInspector::PropertyEditKind::Text
                : PropertyInspector::PropertyEditKind::ReadOnly });
    }
    else if (editorModal_.mode == EditorModalMode::TreeNodeEditor) {
        const auto* selectedNode = findTreeEditorNode(treeNodeEditorDialog_.selectedNodeId);
        fields.push_back(EditorModalField{ "nodeCount", "Node Count", std::to_string(visibleTreeNodeEditorRows().size()), PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "selectedNodePath", "Selected Node", selectedNode == nullptr
                ? std::string{ "<none>" }
                : treeNodeEditorNodePath(selectedNode->id), PropertyInspector::PropertyEditKind::ReadOnly });
        fields.push_back(EditorModalField{ "nodeText", "Node Text", selectedNode == nullptr ? std::string{} : selectedNode->title,
            selectedNode == nullptr ? PropertyInspector::PropertyEditKind::ReadOnly : PropertyInspector::PropertyEditKind::Text });
    }
    return fields;
}

MainWindow::PanelBounds MainWindow::editorModalBodyBounds() const
{
    const PanelBounds dialogBounds = editorModalDialogBounds();
    const float top = dialogBounds.y + 52.0f;
    const float bottom = dialogBounds.y + dialogBounds.height - kEditorModalButtonHeight - 16.0f
        - (editorModal_.mode == EditorModalMode::Message ? 0.0f : (kEditorModalFormStatusHeight + 10.0f));
    return {
        dialogBounds.x + 14.0f,
        top,
        dialogBounds.width - 28.0f,
        std::max(0.0f, bottom - top)
    };
}

MainWindow::PanelBounds MainWindow::tableGridEditorGridBounds() const
{
    const PanelBounds bodyBounds = editorModalBodyBounds();
    if (editorModal_.mode != EditorModalMode::TableGridEditor) {
        return bodyBounds;
    }

    const float top = bodyBounds.y + kTableGridEditorIntroHeight + kTableGridEditorSectionGap;
    const float reservedBottom = kTableGridEditorSectionGap
        + kTableGridEditorActionLabelHeight
        + kTableGridEditorActionButtonHeight * 2.0f
        + kTableGridEditorActionRowGap
        + kTableGridEditorSectionGap
        + (kTableGridEditorFieldRowHeight * kTableGridEditorFieldCount)
        + (kTableGridEditorFieldRowSpacing * (kTableGridEditorFieldCount - 1.0f));
    const float availableHeight = std::max(0.0f, bodyBounds.y + bodyBounds.height - top - reservedBottom);
    const float gridHeight = availableHeight <= 0.0f
        ? 0.0f
        : (availableHeight >= kTableGridEditorGridMinHeight
                ? std::min(kTableGridEditorGridPreferredHeight, availableHeight)
                : availableHeight);
    return {
        bodyBounds.x,
        top,
        bodyBounds.width,
        gridHeight
    };
}

MainWindow::PanelBounds MainWindow::tableGridEditorFormBounds() const
{
    const PanelBounds bodyBounds = editorModalBodyBounds();
    if (editorModal_.mode != EditorModalMode::TableGridEditor) {
        return bodyBounds;
    }

    const PanelBounds gridBounds = tableGridEditorGridBounds();
    const float top = gridBounds.y + gridBounds.height + kTableGridEditorSectionGap;
    return {
        bodyBounds.x,
        top,
        bodyBounds.width,
        std::max(0.0f, bodyBounds.y + bodyBounds.height - top)
    };
}

MainWindow::PanelBounds MainWindow::resourceManagerDetailBounds() const
{
    const PanelBounds bodyBounds = editorModalBodyBounds();
    if (editorModal_.mode != EditorModalMode::ResourceManager) {
        return bodyBounds;
    }

    const PanelBounds previewBounds = resourceManagerPreviewBounds();
    return {
        bodyBounds.x,
        bodyBounds.y,
        std::max(0.0f, previewBounds.x - bodyBounds.x - kResourceManagerSplitGap),
        bodyBounds.height
    };
}

MainWindow::PanelBounds MainWindow::resourceManagerPreviewBounds() const
{
    const PanelBounds bodyBounds = editorModalBodyBounds();
    if (editorModal_.mode != EditorModalMode::ResourceManager) {
        return {};
    }

    const float previewWidth = std::clamp(bodyBounds.width * 0.38f, kResourceManagerPreviewMinWidth, kResourceManagerPreviewMaxWidth);
    return {
        bodyBounds.x + std::max(0.0f, bodyBounds.width - previewWidth),
        bodyBounds.y,
        previewWidth,
        bodyBounds.height
    };
}

MainWindow::PanelBounds MainWindow::itemListEditorPreviewBounds() const
{
    const PanelBounds bodyBounds = editorModalBodyBounds();
    if (editorModal_.mode != EditorModalMode::ItemListEditor) {
        return bodyBounds;
    }

    const std::size_t fieldCount = itemListEditorDialog_.supportsActions
        ? kItemListEditorActionFieldCount
        : kItemListEditorBaseFieldCount;
    const float requiredFormHeight = kEditorModalFormRowHeight * static_cast<float>(fieldCount)
        + kEditorModalFormRowSpacing * static_cast<float>(fieldCount - 1);
    const float availablePreviewHeight = std::max(0.0f,
        bodyBounds.height - kItemListEditorPreviewGap - requiredFormHeight);

    return {
        bodyBounds.x,
        bodyBounds.y,
        bodyBounds.width,
        std::min(kItemListEditorPreviewHeight, availablePreviewHeight)
    };
}

MainWindow::PanelBounds MainWindow::itemListEditorFormBounds() const
{
    const PanelBounds bodyBounds = editorModalBodyBounds();
    if (editorModal_.mode != EditorModalMode::ItemListEditor) {
        return bodyBounds;
    }

    const PanelBounds previewBounds = itemListEditorPreviewBounds();
    const float top = previewBounds.y + previewBounds.height + kItemListEditorPreviewGap;
    return {
        bodyBounds.x,
        top,
        bodyBounds.width,
        std::max(0.0f, bodyBounds.y + bodyBounds.height - top)
    };
}

MainWindow::PanelBounds MainWindow::treeNodeEditorTextBounds() const
{
    const PanelBounds bodyBounds = editorModalBodyBounds();
    if (editorModal_.mode != EditorModalMode::TreeNodeEditor) {
        return bodyBounds;
    }

    const float top = bodyBounds.y + kTreeNodeEditorInstructionHeight + kTreeNodeEditorSectionGap;
    const float reservedBottom = kTreeNodeEditorSectionGap
        + kTreeNodeEditorActionLabelHeight
        + kTreeNodeEditorActionButtonHeight
        + kTreeNodeEditorSectionGap
        + (kEditorModalFormRowHeight * 3.0f)
        + (kEditorModalFormRowSpacing * 2.0f);
    const float availableHeight = std::max(0.0f, bodyBounds.y + bodyBounds.height - top - reservedBottom);
    const float listHeight = availableHeight >= kTreeNodeEditorListMinHeight
        ? std::min(kTreeNodeEditorListPreferredHeight, availableHeight)
        : availableHeight;
    return {
        bodyBounds.x,
        top,
        bodyBounds.width,
        listHeight
    };
}

MainWindow::PanelBounds MainWindow::treeNodeEditorFormBounds() const
{
    const PanelBounds bodyBounds = editorModalBodyBounds();
    if (editorModal_.mode != EditorModalMode::TreeNodeEditor) {
        return bodyBounds;
    }

    const PanelBounds listBounds = treeNodeEditorTextBounds();
    const float top = listBounds.y + listBounds.height + kTreeNodeEditorSectionGap;
    return {
        bodyBounds.x,
        top,
        bodyBounds.width,
        std::max(0.0f, bodyBounds.y + bodyBounds.height - top)
    };
}

MainWindow::PanelBounds MainWindow::editorModalStatusBounds() const
{
    const PanelBounds dialogBounds = editorModalDialogBounds();
    const float y = dialogBounds.y + dialogBounds.height - kEditorModalButtonHeight - 16.0f - kEditorModalFormStatusHeight - 8.0f;
    return {
        dialogBounds.x + 14.0f,
        y,
        dialogBounds.width - 28.0f,
        kEditorModalFormStatusHeight
    };
}

std::vector<MainWindow::EditorModalFieldHit> MainWindow::editorModalFieldHits() const
{
    std::vector<EditorModalFieldHit> hits;
    const auto fields = editorModalFields();
    if (fields.empty()) {
        return hits;
    }

    const PanelBounds bodyBounds = editorModal_.mode == EditorModalMode::ResourceManager
        ? resourceManagerDetailBounds()
        : (editorModal_.mode == EditorModalMode::ItemListEditor
                ? itemListEditorFormBounds()
                : (editorModal_.mode == EditorModalMode::TableGridEditor
                        ? tableGridEditorFormBounds()
                        : (editorModal_.mode == EditorModalMode::TreeNodeEditor ? treeNodeEditorFormBounds() : editorModalBodyBounds())));
    const float labelWidth = editorModal_.mode == EditorModalMode::ResourceManager
        ? kResourceManagerFieldLabelWidth
        : kEditorModalFormLabelWidth;
    const float rowHeight = editorModal_.mode == EditorModalMode::TableGridEditor
        ? kTableGridEditorFieldRowHeight
        : kEditorModalFormRowHeight;
    const float rowSpacing = editorModal_.mode == EditorModalMode::TableGridEditor
        ? kTableGridEditorFieldRowSpacing
        : kEditorModalFormRowSpacing;
    float top = editorModal_.mode == EditorModalMode::TreeNodeEditor
        ? bodyBounds.y + kTreeNodeEditorActionLabelHeight + kTreeNodeEditorActionButtonHeight + kTreeNodeEditorSectionGap
        : (editorModal_.mode == EditorModalMode::TableGridEditor
                ? bodyBounds.y + kTableGridEditorActionLabelHeight + kTableGridEditorActionButtonHeight * 2.0f + kTableGridEditorActionRowGap + kTableGridEditorSectionGap
                : bodyBounds.y);
    hits.reserve(fields.size());
    for (const auto& field : fields) {
        const PanelBounds valueBounds{
            bodyBounds.x + labelWidth,
            top,
            std::max(0.0f, bodyBounds.width - labelWidth),
            rowHeight
        };
        hits.push_back({ field, valueBounds });
        top += rowHeight + rowSpacing;
    }
    return hits;
}

std::optional<MainWindow::EditorModalFieldHit> MainWindow::editorModalFieldAt(float x, float y) const
{
    for (const auto& hit : editorModalFieldHits()) {
        if (pointInBounds(x, y, hit.bounds.x, hit.bounds.y, hit.bounds.width, hit.bounds.height)) {
            return hit;
        }
    }
    return std::nullopt;
}

std::optional<int> MainWindow::itemListEditorPreviewIndexAt(float x, float y) const
{
    if (editorModal_.mode != EditorModalMode::ItemListEditor) {
        return std::nullopt;
    }

    const PanelBounds bounds = itemListEditorPreviewBounds();
    if (!pointInBounds(x, y, bounds.x, bounds.y, bounds.width, bounds.height)) {
        return std::nullopt;
    }

    const float listTop = bounds.y + kItemListEditorPreviewHeaderHeight;
    const float rowOffset = y - listTop;
    if (rowOffset < 0.0f) {
        return std::nullopt;
    }


    const int rowIndex = static_cast<int>(rowOffset / kItemListEditorPreviewRowHeight);
    const std::size_t visibleCount = itemListEditorVisibleRowCount(bounds.height, itemListEditorDialog_.items.size());
    if (rowIndex < 0 || rowIndex >= static_cast<int>(visibleCount)) {
        return std::nullopt;
    }

    const float rowTop = itemListEditorRowTop(bounds.y, static_cast<std::size_t>(rowIndex));
    if (!pointInBounds(x,
            y,
            bounds.x + kItemListEditorPreviewRowInsetX,
            rowTop,
            std::max(0.0f, bounds.width - kItemListEditorPreviewRowInsetX * 2.0f),
            kItemListEditorPreviewRowFillHeight)) {
        return std::nullopt;
    }

    // Map visible row index to actual item index using scroll offset
    const int realIndex = itemListEditorDialog_.previewScrollOffset + rowIndex;
    if (realIndex < 0 || realIndex >= static_cast<int>(itemListEditorDialog_.items.size())) {
        return std::nullopt;
    }
    return realIndex;
}

std::optional<int> MainWindow::treeNodeEditorRowAt(float x, float y) const
{
    if (editorModal_.mode != EditorModalMode::TreeNodeEditor) {
        return std::nullopt;
    }

    const PanelBounds bounds = treeNodeEditorTextBounds();
    if (!pointInBounds(x, y, bounds.x, bounds.y, bounds.width, bounds.height)) {
        return std::nullopt;
    }

    const float rowTop = bounds.y + 8.0f;
    const float rowOffset = y - rowTop;
    if (rowOffset < 0.0f) {
        return std::nullopt;
    }

    const int rowIndex = static_cast<int>(rowOffset / kTreeNodeEditorRowHeight);
    const auto rows = visibleTreeNodeEditorRows();
    const std::size_t visibleCount = std::min<std::size_t>(
        rows.size(),
        std::max<std::size_t>(1, static_cast<std::size_t>(std::floor((bounds.height - 16.0f) / kTreeNodeEditorRowHeight))));
    if (rowIndex < 0 || static_cast<std::size_t>(rowIndex) >= visibleCount) {
        return std::nullopt;
    }

    return rows[static_cast<std::size_t>(rowIndex)].nodeId;
}

std::vector<MainWindow::TreeNodeEditorRow> MainWindow::visibleTreeNodeEditorRows() const
{
    std::vector<TreeNodeEditorRow> rows;
    auto appendRows = [&](auto&& self, int nodeId, int depth) -> void {
        const auto* node = findTreeEditorNode(nodeId);
        if (node == nullptr) {
            return;
        }

        rows.push_back({ nodeId, depth, !node->childIds.empty(), treeNodeEditorNodePath(nodeId) });
        for (int childId : node->childIds) {
            self(self, childId, depth + 1);
        }
    };

    for (int rootId : treeNodeEditorDialog_.rootNodeIds) {
        appendRows(appendRows, rootId, 0);
    }

    return rows;
}

std::vector<MainWindow::TreeNodeEditorActionButton> MainWindow::treeNodeEditorActionButtons() const
{
    std::vector<TreeNodeEditorActionButton> buttons;
    if (editorModal_.mode != EditorModalMode::TreeNodeEditor) {
        return buttons;
    }

    const PanelBounds formBounds = treeNodeEditorFormBounds();
    const float gap = 10.0f;
    const float buttonWidth = std::max(80.0f, (formBounds.width - gap * 4.0f) / 5.0f);
    const auto* selectedNode = findTreeEditorNode(treeNodeEditorDialog_.selectedNodeId);
    const auto* siblings = selectedNode == nullptr ? &treeNodeEditorDialog_.rootNodeIds : treeNodeEditorSiblingList(selectedNode->parentId);
    int selectedIndex = -1;
    if (selectedNode != nullptr && siblings != nullptr) {
        const auto iterator = std::find(siblings->begin(), siblings->end(), selectedNode->id);
        if (iterator != siblings->end()) {
            selectedIndex = static_cast<int>(std::distance(siblings->begin(), iterator));
        }
    }

    const std::array<std::pair<const char*, const char*>, 5> definitions = {{
        { "add_child_node", "Add Child" },
        { "add_sibling_node", "Add Sibling" },
        { "remove_node", "Remove" },
        { "move_node_up", "Move Up" },
        { "move_node_down", "Move Down" }
    }};

    buttons.reserve(definitions.size());
    float left = formBounds.x;
    for (const auto& definition : definitions) {
        bool enabled = true;
        if (std::string_view{ definition.first } == "remove_node") {
            enabled = selectedNode != nullptr;
        }
        else if (std::string_view{ definition.first } == "move_node_up") {
            enabled = selectedNode != nullptr && selectedIndex > 0;
        }
        else if (std::string_view{ definition.first } == "move_node_down") {
            enabled = selectedNode != nullptr && siblings != nullptr && selectedIndex >= 0 && selectedIndex + 1 < static_cast<int>(siblings->size());
        }

        buttons.push_back({
            definition.first,
            definition.second,
            { left, formBounds.y + kTreeNodeEditorActionLabelHeight, buttonWidth, kTreeNodeEditorActionButtonHeight },
            enabled
        });
        left += buttonWidth + gap;
    }

    return buttons;
}

MainWindow::TreeEditorNode* MainWindow::findTreeEditorNode(int nodeId)
{
    const auto iterator = std::find_if(treeNodeEditorDialog_.nodes.begin(), treeNodeEditorDialog_.nodes.end(), [nodeId](const TreeEditorNode& node) {
        return node.id == nodeId;
    });
    return iterator == treeNodeEditorDialog_.nodes.end() ? nullptr : &(*iterator);
}

const MainWindow::TreeEditorNode* MainWindow::findTreeEditorNode(int nodeId) const
{
    const auto iterator = std::find_if(treeNodeEditorDialog_.nodes.begin(), treeNodeEditorDialog_.nodes.end(), [nodeId](const TreeEditorNode& node) {
        return node.id == nodeId;
    });
    return iterator == treeNodeEditorDialog_.nodes.end() ? nullptr : &(*iterator);
}

std::vector<int>* MainWindow::treeNodeEditorSiblingList(int parentId)
{
    if (parentId < 0) {
        return &treeNodeEditorDialog_.rootNodeIds;
    }

    auto* parentNode = findTreeEditorNode(parentId);
    return parentNode == nullptr ? nullptr : &parentNode->childIds;
}

const std::vector<int>* MainWindow::treeNodeEditorSiblingList(int parentId) const
{
    if (parentId < 0) {
        return &treeNodeEditorDialog_.rootNodeIds;
    }

    const auto* parentNode = findTreeEditorNode(parentId);
    return parentNode == nullptr ? nullptr : &parentNode->childIds;
}

std::string MainWindow::treeNodeEditorNodePath(int nodeId) const
{
    std::vector<std::string> parts;
    int currentId = nodeId;
    while (currentId >= 0) {
        const auto* node = findTreeEditorNode(currentId);
        if (node == nullptr) {
            break;
        }

        std::string title = trimWhitespace(node->title);
        if (title.empty()) {
            title = "Node";
        }
        parts.push_back(std::move(title));
        currentId = node->parentId;
    }

    std::reverse(parts.begin(), parts.end());
    std::string path;
    for (std::size_t index = 0; index < parts.size(); ++index) {
        if (index > 0) {
            path += '/';
        }
        path += parts[index];
    }
    return path;
}

void MainWindow::refreshTreeNodeEditorState()
{
    const auto nodeExists = [this](int nodeId) {
        return findTreeEditorNode(nodeId) != nullptr;
    };

    auto removeMissingIds = [&nodeExists](std::vector<int>& ids) {
        ids.erase(std::remove_if(ids.begin(), ids.end(), [&nodeExists](int nodeId) {
            return !nodeExists(nodeId);
        }), ids.end());
    };

    removeMissingIds(treeNodeEditorDialog_.rootNodeIds);
    for (auto& node : treeNodeEditorDialog_.nodes) {
        removeMissingIds(node.childIds);
        const std::string trimmedTitle = trimWhitespace(node.title);
        node.title = trimmedTitle.empty() ? std::string{ "Node" } : trimmedTitle;
    }

    if (!nodeExists(treeNodeEditorDialog_.selectedNodeId)) {
        treeNodeEditorDialog_.selectedNodeId = treeNodeEditorDialog_.rootNodeIds.empty() ? -1 : treeNodeEditorDialog_.rootNodeIds.front();
    }

    int maximumNodeId = 0;
    for (const auto& node : treeNodeEditorDialog_.nodes) {
        maximumNodeId = std::max(maximumNodeId, node.id);
    }
    treeNodeEditorDialog_.nextNodeId = std::max(treeNodeEditorDialog_.nextNodeId, maximumNodeId + 1);
}

void MainWindow::selectTreeEditorNode(int nodeId)
{
    treeNodeEditorDialog_.selectedNodeId = findTreeEditorNode(nodeId) != nullptr ? nodeId : -1;
    if (treeNodeEditorDialog_.selectedNodeId >= 0) {
        editorModal_.statusText = "Selected node: " + treeNodeEditorNodePath(treeNodeEditorDialog_.selectedNodeId);
    }
    else if (treeNodeEditorDialog_.nodes.empty()) {
        editorModal_.statusText = "No nodes yet. Use Add Child to create the first root node.";
    }
    else {
        editorModal_.statusText = "No node selected.";
    }
}

bool MainWindow::renameSelectedTreeEditorNode(const std::string& valueText)
{
    auto* node = findTreeEditorNode(treeNodeEditorDialog_.selectedNodeId);
    if (node == nullptr) {
        editorModal_.statusText = "Select a node before renaming it.";
        return false;
    }

    std::string title = trimWhitespace(valueText);
    if (title.empty()) {
        title = "Node";
    }

    node->title = title;
    editorModal_.statusText = "Node renamed.";
    return true;
}

bool MainWindow::activateTreeNodeEditorAction(const std::string& actionId)
{
    if (editorModalEdit_.active && !commitEditorModalFieldEdit()) {
        return false;
    }

    if (actionId == "add_child_node") {
        const int newNodeId = treeNodeEditorDialog_.nextNodeId++;
        const int parentId = treeNodeEditorDialog_.selectedNodeId;
        if (parentId >= 0) {
            if (auto* parentNode = findTreeEditorNode(parentId)) {
                parentNode->childIds.push_back(newNodeId);
                parentNode->expanded = true;
            }
        }
        else {
            treeNodeEditorDialog_.rootNodeIds.push_back(newNodeId);
        }

        treeNodeEditorDialog_.nodes.push_back({ newNodeId, "New Node", parentId, {}, true });
        refreshTreeNodeEditorState();
        selectTreeEditorNode(newNodeId);
        editorModal_.statusText = parentId >= 0 ? "Added child node." : "Added root node.";
        redraw();
        return true;
    }

    if (actionId == "add_sibling_node") {
        const auto* selectedNode = findTreeEditorNode(treeNodeEditorDialog_.selectedNodeId);
        const int newNodeId = treeNodeEditorDialog_.nextNodeId++;
        const int parentId = selectedNode == nullptr ? -1 : selectedNode->parentId;
        auto* siblings = treeNodeEditorSiblingList(parentId);
        if (siblings == nullptr) {
            return false;
        }

        if (selectedNode == nullptr) {
            siblings->push_back(newNodeId);
        }
        else {
            const auto iterator = std::find(siblings->begin(), siblings->end(), selectedNode->id);
            siblings->insert(iterator == siblings->end() ? siblings->end() : iterator + 1, newNodeId);
        }

        treeNodeEditorDialog_.nodes.push_back({ newNodeId, "New Node", parentId, {}, true });
        refreshTreeNodeEditorState();
        selectTreeEditorNode(newNodeId);
        editorModal_.statusText = selectedNode == nullptr ? "Added root node." : "Added sibling node.";
        redraw();
        return true;
    }

    if (actionId == "remove_node") {
        const int selectedNodeId = treeNodeEditorDialog_.selectedNodeId;
        const auto* selectedNode = findTreeEditorNode(selectedNodeId);
        if (selectedNode == nullptr) {
            editorModal_.statusText = "Select a node before removing it.";
            redraw();
            return false;
        }

        const int parentId = selectedNode->parentId;
        auto* siblings = treeNodeEditorSiblingList(parentId);
        int nextSelection = parentId;
        int previousIndex = -1;
        if (siblings != nullptr) {
            const auto iterator = std::find(siblings->begin(), siblings->end(), selectedNodeId);
            if (iterator != siblings->end()) {
                previousIndex = static_cast<int>(std::distance(siblings->begin(), iterator));
            }
        }

        std::vector<int> removedIds;
        auto collectIds = [&](auto&& self, int nodeId) -> void {
            removedIds.push_back(nodeId);
            if (const auto* node = findTreeEditorNode(nodeId)) {
                for (int childId : node->childIds) {
                    self(self, childId);
                }
            }
        };
        collectIds(collectIds, selectedNodeId);
        const std::set<int> removedIdSet(removedIds.begin(), removedIds.end());

        if (siblings != nullptr) {
            siblings->erase(std::remove_if(siblings->begin(), siblings->end(), [&removedIdSet](int nodeId) {
                return removedIdSet.contains(nodeId);
            }), siblings->end());
        }

        for (auto& node : treeNodeEditorDialog_.nodes) {
            node.childIds.erase(std::remove_if(node.childIds.begin(), node.childIds.end(), [&removedIdSet](int nodeId) {
                return removedIdSet.contains(nodeId);
            }), node.childIds.end());
        }
        treeNodeEditorDialog_.rootNodeIds.erase(std::remove_if(treeNodeEditorDialog_.rootNodeIds.begin(), treeNodeEditorDialog_.rootNodeIds.end(), [&removedIdSet](int nodeId) {
            return removedIdSet.contains(nodeId);
        }), treeNodeEditorDialog_.rootNodeIds.end());
        treeNodeEditorDialog_.nodes.erase(std::remove_if(treeNodeEditorDialog_.nodes.begin(), treeNodeEditorDialog_.nodes.end(), [&removedIdSet](const TreeEditorNode& node) {
            return removedIdSet.contains(node.id);
        }), treeNodeEditorDialog_.nodes.end());

        refreshTreeNodeEditorState();
        if (const auto* updatedSiblings = treeNodeEditorSiblingList(parentId); updatedSiblings != nullptr && !updatedSiblings->empty()) {
            nextSelection = (*updatedSiblings)[std::clamp(previousIndex, 0, static_cast<int>(updatedSiblings->size()) - 1)];
        }
        else if (!treeNodeEditorDialog_.rootNodeIds.empty()) {
            nextSelection = treeNodeEditorDialog_.rootNodeIds.front();
        }
        else {
            nextSelection = -1;
        }

        selectTreeEditorNode(nextSelection);
        editorModal_.statusText = "Removed node subtree.";
        redraw();
        return true;
    }

    if (actionId == "move_node_up" || actionId == "move_node_down") {
        const auto* selectedNode = findTreeEditorNode(treeNodeEditorDialog_.selectedNodeId);
        if (selectedNode == nullptr) {
            editorModal_.statusText = "Select a node before reordering it.";
            redraw();
            return false;
        }

        auto* siblings = treeNodeEditorSiblingList(selectedNode->parentId);
        if (siblings == nullptr) {
            return false;
        }

        const auto iterator = std::find(siblings->begin(), siblings->end(), selectedNode->id);
        if (iterator == siblings->end()) {
            return false;
        }

        const int index = static_cast<int>(std::distance(siblings->begin(), iterator));
        const int targetIndex = actionId == "move_node_up" ? index - 1 : index + 1;
        if (targetIndex < 0 || targetIndex >= static_cast<int>(siblings->size())) {
            editorModal_.statusText = actionId == "move_node_up"
                ? "Selected node is already first among its siblings."
                : "Selected node is already last among its siblings.";
            redraw();
            return false;
        }

        std::swap((*siblings)[index], (*siblings)[targetIndex]);
        selectTreeEditorNode(selectedNode->id);
        editorModal_.statusText = actionId == "move_node_up" ? "Moved node up." : "Moved node down.";
        redraw();
        return true;
    }

    return false;
}

std::string MainWindow::editorModalFieldValue(const std::string& key) const
{
    for (const auto& field : editorModalFields()) {
        if (field.key == key) {
            return field.value;
        }
    }
    return {};
}

void MainWindow::setEditorModalFieldValue(const std::string& key, const std::string& valueText)
{
    const std::string trimmedValue = trimWhitespace(valueText);
    if (editorModal_.mode == EditorModalMode::NewProjectWizard) {
        if (key == "projectName") {
            const std::string previousProjectName = newProjectWizard_.projectName;
            newProjectWizard_.projectName = trimmedValue;
            if (!trimmedValue.empty()) {
                updateDerivedProjectNames(previousProjectName, trimmedValue, newProjectWizard_.executableName, newProjectWizard_.windowTitle);
            }
        }
        else if (key == "executableName") {
            newProjectWizard_.executableName = trimmedValue;
        }
        else if (key == "windowTitle") {
            newProjectWizard_.windowTitle = trimmedValue;
        }
        else if (key == "userSubclassName") {
            newProjectWizard_.userSubclassName = trimmedValue;
        }
        else if (key == "formWidth") {
            newProjectWizard_.formWidth = std::max(1, tryParseInt(trimmedValue).value_or(newProjectWizard_.formWidth));
        }
        else if (key == "formHeight") {
            newProjectWizard_.formHeight = std::max(1, tryParseInt(trimmedValue).value_or(newProjectWizard_.formHeight));
        }
        else if (key == "lookAndFeelId" && containsText(availableLookAndFeelIds(), trimmedValue)) {
            newProjectWizard_.lookAndFeelId = trimmedValue;
        }
        else if (key == "templateId" && containsText(newProjectTemplateIds(), trimmedValue)) {
            newProjectWizard_.templateId = trimmedValue;
        }

        editorModal_.statusText = key == "templateId"
            ? templateDescription(newProjectWizard_.templateId)
            : "Configure the new project and click Create.";
        return;
    }

    if (editorModal_.mode == EditorModalMode::ProjectSettings) {
        if (key == "projectName") {
            const std::string previousProjectName = projectSettingsDialog_.projectName;
            projectSettingsDialog_.projectName = trimmedValue;
            if (!trimmedValue.empty()) {
                updateDerivedProjectNames(previousProjectName, trimmedValue, projectSettingsDialog_.executableName, projectSettingsDialog_.windowTitle);
            }
        }
        else if (key == "executableName") {
            projectSettingsDialog_.executableName = trimmedValue;
        }
        else if (key == "windowTitle") {
            projectSettingsDialog_.windowTitle = trimmedValue;
        }
        else if (key == "userSubclassName") {
            projectSettingsDialog_.userSubclassName = trimmedValue;
        }
        else if (key == "lookAndFeelId" && containsText(availableLookAndFeelIds(), trimmedValue)) {
            projectSettingsDialog_.lookAndFeelId = trimmedValue;
        }
        else if (key == "localVisageSourceDirectory") {
            projectSettingsDialog_.localVisageSourceDirectory = trimmedValue;
        }
        else if (key == "visageGitRepository") {
            projectSettingsDialog_.visageGitRepository = trimmedValue;
        }
        else if (key == "visageGitTag") {
            projectSettingsDialog_.visageGitTag = trimmedValue;
        }

        editorModal_.statusText = "Update project naming, look and feel, and local Visage export settings. Use Project > Resources to manage assets.";
        return;
    }

    if (editorModal_.mode == EditorModalMode::ResourceManager) {
        if (key == "selectedResourceId") {
            resourceManagerDialog_.selectedResourceId = trimmedValue;
            resourceManagerDialog_.confirmReferencedRemoval = false;
            refreshResourceManagerPreview();
            if (const auto* selectedResource = document_.findResourceById(resourceManagerDialog_.selectedResourceId)) {
                editorModal_.statusText = "Selected " + resourceTypeDisplayName(selectedResource->type) + " resource: " + resourceDisplayLabel(*selectedResource);
            }
            else if (document_.resources.empty()) {
                editorModal_.statusText = "No resources have been added yet.";
            }
            else {
                editorModal_.statusText = "Select a resource to inspect its source and export path.";
            }
        }
        return;
    }

    if (editorModal_.mode == EditorModalMode::KeyboardShortcuts) {
        if (key == "selectedCommandId") {
            keyboardShortcutDialog_.selectedCommandId = trimmedValue;
            if (const auto* definition = commands::CommandRegistry::find(keyboardShortcutDialog_.selectedCommandId)) {
                editorModal_.statusText = std::string{ definition->displayName } + ": edit or clear the shortcut, then click Apply.";
            }
            else {
                editorModal_.statusText = "Select a command to edit its shortcut.";
            }
        }
        else if (key == "shortcutText" && !keyboardShortcutDialog_.selectedCommandId.empty()) {
            keyboardShortcutDialog_.pendingShortcuts[keyboardShortcutDialog_.selectedCommandId] = trimmedValue;
            editorModal_.statusText = trimmedValue.empty()
                ? "Shortcut cleared for the selected command. Click Apply to save."
                : "Shortcut updated for the selected command. Click Apply to save.";
        }
        return;
    }

    if (editorModal_.mode == EditorModalMode::TableGridEditor) {
        if (key == "columnName") {
            if (tableGridEditorDialog_.selectedColumn >= 0 && tableGridEditorDialog_.selectedColumn < static_cast<int>(tableGridEditorDialog_.columns.size())) {
                tableGridEditorDialog_.columns[static_cast<std::size_t>(tableGridEditorDialog_.selectedColumn)] = trimmedValue;
                editorModal_.statusText = "Updated selected column name. Click Apply to commit the table.";
            }
        }
        else if (key == "cellText") {
            model::setCellText(tableGridEditorDialog_.rows, tableGridEditorDialog_.selectedRow, tableGridEditorDialog_.selectedColumn, valueText);
            editorModal_.statusText = "Updated selected cell text. Click Apply to commit the table.";
        }
        return;
    }

    if (editorModal_.mode == EditorModalMode::TreeNodeEditor) {
        if (key == "nodeText") {
            renameSelectedTreeEditorNode(valueText);
        }
        return;
    }

    if (editorModal_.mode == EditorModalMode::ItemListEditor) {
        if (key == "itemText" && itemListEditorDialog_.selectedItemIndex >= 0) {
            itemListEditorDialog_.items[static_cast<std::size_t>(itemListEditorDialog_.selectedItemIndex)] = valueText;
            editorModal_.statusText = itemListEditorDialog_.supportsActions
                ? "Label updated for row " + std::to_string(itemListEditorDialog_.selectedItemIndex + 1) + ". Click Apply to save."
                : "Item Text updated for row " + std::to_string(itemListEditorDialog_.selectedItemIndex + 1) + ". Click Apply to save.";
        }
        else if (key == "itemAction" && itemListEditorDialog_.selectedItemIndex >= 0) {
            if (itemListEditorDialog_.selectedItemIndex >= static_cast<int>(itemListEditorDialog_.actions.size())) {
                itemListEditorDialog_.actions.resize(itemListEditorDialog_.items.size());
            }
            itemListEditorDialog_.actions[static_cast<std::size_t>(itemListEditorDialog_.selectedItemIndex)] = trimmedValue;
            editorModal_.statusText = "Updated Callback / Action for row " + std::to_string(itemListEditorDialog_.selectedItemIndex + 1) + ". Click Apply to save.";
        }
    }
}

bool MainWindow::beginEditorModalFieldEdit(const EditorModalField& field)
{
    if (field.editKind == PropertyInspector::PropertyEditKind::ReadOnly
        || field.editKind == PropertyInspector::PropertyEditKind::Bool) {
        return false;
    }

    if (field.editKind == PropertyInspector::PropertyEditKind::Choice) {
        editorModalEdit_ = {};
        openEditorModalDropdown(field);
        redraw();
        return true;
    }

    editorModalEdit_.active = true;
    editorModalEdit_.key = field.key;
    editorModalEdit_.editKind = field.editKind;
    textEditControl_.begin(field.value);
    updateEditorModalEditorBounds();
    redraw();
    return true;

}

bool MainWindow::openSelectedTreeNodeEditor()
{
    const auto* widget = document_.selectedWidget();
    if (widget == nullptr || !model::supportsTreeNodes(widget->type)) {
        return false;
    }

    cancelInspectorEdit();
    cancelEditorModalFieldEdit();
    clearCanvasInteraction();
    requestKeyboardFocus();

    treeNodeEditorDialog_.visible = true;
    treeNodeEditorDialog_.widgetId = widget->id;
    treeNodeEditorDialog_.originalNodesText = widget->getStringProperty("nodes", {});
    treeNodeEditorDialog_.originalSelectedNodePath = widget->getStringProperty("selectedNodePath", {});
    treeNodeEditorDialog_.originalExpandedNodePaths = widget->getStringProperty("expandedNodePaths", {});
    treeNodeEditorDialog_.nodes.clear();
    treeNodeEditorDialog_.rootNodeIds.clear();
    treeNodeEditorDialog_.nextNodeId = 1;
    treeNodeEditorDialog_.selectedNodeId = -1;

    const auto parseResult = model::parseTreeNodes(treeNodeEditorDialog_.originalNodesText);
    const auto expandedPathList = model::splitTreeNodePaths(treeNodeEditorDialog_.originalExpandedNodePaths);
    const std::set<std::string> expandedPaths(expandedPathList.begin(), expandedPathList.end());
    std::vector<int> nodeIds;
    nodeIds.reserve(parseResult.nodes.size());
    for (const auto& parsedNode : parseResult.nodes) {
        const int nodeId = treeNodeEditorDialog_.nextNodeId++;
        nodeIds.push_back(nodeId);
        treeNodeEditorDialog_.nodes.push_back({
            nodeId,
            parsedNode.text,
            parsedNode.parentIndex >= 0 ? nodeIds[static_cast<std::size_t>(parsedNode.parentIndex)] : -1,
            {},
            expandedPaths.contains(parsedNode.path)
        });
    }

    for (std::size_t index = 0; index < parseResult.nodes.size(); ++index) {
        const int nodeId = nodeIds[index];
        const int parentIndex = parseResult.nodes[index].parentIndex;
        if (parentIndex < 0) {
            treeNodeEditorDialog_.rootNodeIds.push_back(nodeId);
        }
        else if (auto* parentNode = findTreeEditorNode(nodeIds[static_cast<std::size_t>(parentIndex)])) {
            parentNode->childIds.push_back(nodeId);
        }

        if (parseResult.nodes[index].path == treeNodeEditorDialog_.originalSelectedNodePath) {
            treeNodeEditorDialog_.selectedNodeId = nodeId;
        }
    }
    refreshTreeNodeEditorState();
    selectTreeEditorNode(treeNodeEditorDialog_.selectedNodeId);

    editorModal_.visible = true;
    editorModal_.mode = EditorModalMode::TreeNodeEditor;
    editorModal_.title = "Edit Tree Nodes";
    editorModal_.message.clear();
    editorModal_.lines.clear();
    editorModal_.buttons = {
        { "apply_tree_nodes", "Apply" },
        { "cancel", "Cancel" }
    };
    editorModal_.result.clear();
    editorModal_.statusText = treeNodeEditorDialog_.nodes.empty()
        ? "No nodes yet. Use Add Child to create the first root node."
        : "Select a node, edit Node Text, then click Apply to commit the tree.";
    editorModal_.preferredWidth = kTreeNodeEditorModalWidth;
    editorModal_.preferredHeight = kTreeNodeEditorModalHeight;
    newProjectWizard_.visible = false;
    projectSettingsDialog_.visible = false;
    resourceManagerDialog_.visible = false;
    keyboardShortcutDialog_.visible = false;

    updateEditorModalEditorBounds();
    redraw();
    return true;
}

bool MainWindow::applyTreeNodeEditor()
{
    const auto* selectedWidget = document_.selectedWidget();
    if (selectedWidget == nullptr || selectedWidget->id != treeNodeEditorDialog_.widgetId || !model::supportsTreeNodes(selectedWidget->type)) {
        return false;
    }

    if (editorModalEdit_.active && !commitEditorModalFieldEdit()) {
        return false;
    }

    std::vector<model::TreeNodeEntry> serializedNodes;
    std::vector<std::string> expandedPaths;
    auto appendNode = [&](auto&& self, int nodeId, int depth, const std::string& parentPath) -> void {
        const auto* node = findTreeEditorNode(nodeId);
        if (node == nullptr) {
            return;
        }

        const std::string title = trimWhitespace(node->title).empty() ? std::string{ "Node" } : trimWhitespace(node->title);
        const std::string path = parentPath.empty() ? title : parentPath + "/" + title;
        model::TreeNodeEntry entry;
        entry.text = title;
        entry.depth = depth;
        serializedNodes.push_back(std::move(entry));
        if (!node->childIds.empty() && node->expanded) {
            expandedPaths.push_back(path);
        }
        for (int childId : node->childIds) {
            self(self, childId, depth + 1, path);
        }
    };

    for (int rootNodeId : treeNodeEditorDialog_.rootNodeIds) {
        appendNode(appendNode, rootNodeId, 0, {});
    }

    const std::string nodesText = model::serializeTreeNodes(serializedNodes);
    const std::string selectedNodePath = treeNodeEditorDialog_.selectedNodeId >= 0
        ? treeNodeEditorNodePath(treeNodeEditorDialog_.selectedNodeId)
        : std::string{};
    const std::string expandedNodePaths = model::normalizeExpandedTreeNodePaths(nodesText, model::joinTreeNodePaths(expandedPaths));

    const bool applied = applyUndoableDocumentChange("Edit tree nodes", [this, &nodesText, &selectedNodePath, &expandedNodePaths]() {
        auto* widget = document_.selectedWidget();
        if (widget == nullptr || !model::supportsTreeNodes(widget->type)) {
            return false;
        }

        widget->setProperty("nodes", nodesText);
        widget->setProperty("selectedNodePath", selectedNodePath);
        widget->setProperty("expandedNodePaths", expandedNodePaths);
        model::normalizeTreeViewProperties(*widget);
        return true;
    });
    if (!applied) {
        return false;
    }

    setOperationStatus("Updated TreeView nodes.");
    closeEditorModalDialog("apply_tree_nodes");
    return true;
}

bool MainWindow::commitEditorModalFieldEdit()
{
    if (!editorModalEdit_.active) {
        return true;
    }

    const auto fields = editorModalFields();
    const auto iterator = std::find_if(fields.begin(), fields.end(), [this](const EditorModalField& field) {
        return field.key == editorModalEdit_.key;
    });
    if (iterator == fields.end()) {
        cancelEditorModalFieldEdit();
        return true;
    }

    const std::string valueText = trimWhitespace(textEditControl_.text());
    if (iterator->editKind == PropertyInspector::PropertyEditKind::Integer) {
        if (const auto value = tryParseInt(valueText)) {
            setEditorModalFieldValue(iterator->key, std::to_string(*value));
        }
        else {
            editorModal_.statusText = "Invalid integer value for " + iterator->label + ".";
            redraw();
            return false;
        }
    }
    else {
        setEditorModalFieldValue(iterator->key, valueText);
    }

    editorModalEdit_ = {};
    textEditControl_.clear();
    dropdownControl_.close();
    requestKeyboardFocus();
    redraw();
    return true;
}

void MainWindow::cancelEditorModalFieldEdit()
{
    if (!editorModalEdit_.active) {
        return;
    }

    editorModalEdit_ = {};
    textEditControl_.clear();
    dropdownControl_.close();
    requestKeyboardFocus();
    redraw();
}

void MainWindow::updateEditorModalEditorBounds()
{
    if (!editorModalEdit_.active) {
        return;
    }

    for (const auto& hit : editorModalFieldHits()) {
        if (hit.field.key == editorModalEdit_.key) {
            textEditControl_.setBounds(hit.bounds.x + 1.0f, hit.bounds.y + 1.0f, hit.bounds.width - 2.0f, hit.bounds.height - 2.0f);
            return;
        }
    }
}

void MainWindow::openEditorModalDropdown(const EditorModalField& field)
{
    const auto viewport = activeDropdownViewportBounds();
    if (!viewport.has_value()) {
        dropdownControl_.close();
        return;
    }

    for (const auto& hit : editorModalFieldHits()) {
        if (hit.field.key != field.key) {
            continue;
        }

        dropdownControl_.open(field.key,
            { hit.bounds.x, hit.bounds.y, hit.bounds.width, hit.bounds.height },
            *viewport,
            dropdownItemsFromChoices(field.choices),
            field.value);
        return;
    }

    dropdownControl_.close();
}

bool MainWindow::applyEditorModalDropdownSelection(const std::string& key, const std::string& value, const std::string& label)
{
    const std::string fieldLabel = editorModalFieldLabel(key);
    setEditorModalFieldValue(key, value);
    dropdownControl_.close();
    editorModal_.statusText = fieldLabel + ": " + label;
    redraw();
    return true;
}

std::optional<editors::DropdownControl::Bounds> MainWindow::activeDropdownViewportBounds() const
{
    if (isEditorModalVisible()) {
        const PanelBounds bodyBounds = editorModalBodyBounds();
        return editors::DropdownControl::Bounds{ bodyBounds.x, bodyBounds.y, bodyBounds.width, bodyBounds.height };
    }

    if (!layout_.propertyInspector.isVisible()) {
        return std::nullopt;
    }

    return editors::DropdownControl::Bounds{
        layout_.propertyInspector.x + 8.0f,
        layout_.propertyInspector.y + 42.0f,
        layout_.propertyInspector.width - 16.0f,
        layout_.propertyInspector.height - 50.0f
    };
}

std::vector<editors::DropdownControl::Item> MainWindow::dropdownItemsFromChoices(const std::vector<PropertyInspector::PropertyChoice>& choices) const
{
    std::vector<editors::DropdownControl::Item> items;
    items.reserve(choices.size());
    for (const auto& choice : choices) {
        items.push_back({ choice.value, choice.label, choice.hint });
    }
    return items;
}

bool MainWindow::activateEditorModalButton(const std::string& buttonId)
{
    if (buttonId == "cancel") {
        closeEditorModalDialog(buttonId);
        return true;
    }
    if (buttonId == "close") {
        closeEditorModalDialog(buttonId);
        return true;
    }

    if (editorModalEdit_.active && !commitEditorModalFieldEdit()) {
        return false;
    }

    if (buttonId == "create" && editorModal_.mode == EditorModalMode::NewProjectWizard) {
        return applyNewProjectWizard();
    }
    if (buttonId == "apply" && editorModal_.mode == EditorModalMode::ProjectSettings) {
        return applyProjectSettingsDialog();
    }
    if (buttonId == "add_image" && editorModal_.mode == EditorModalMode::ResourceManager) {
        return addResourceFromDialog(model::ProjectResourceType::Image);
    }
    if (buttonId == "add_font" && editorModal_.mode == EditorModalMode::ResourceManager) {
        return addResourceFromDialog(model::ProjectResourceType::Font);
    }
    if (buttonId == "remove_resource" && editorModal_.mode == EditorModalMode::ResourceManager) {
        return removeSelectedResourceFromManager();
    }
    if (buttonId == "apply_shortcuts" && editorModal_.mode == EditorModalMode::KeyboardShortcuts) {
        return applyKeyboardShortcutsDialog();
    }
    if (buttonId == "reset_shortcut" && editorModal_.mode == EditorModalMode::KeyboardShortcuts) {
        resetSelectedKeyboardShortcut();
        return true;
    }
    if (buttonId == "apply_table_grid" && editorModal_.mode == EditorModalMode::TableGridEditor) {
        return applyTableGridEditor();
    }
    if (buttonId == "apply_tree_nodes" && editorModal_.mode == EditorModalMode::TreeNodeEditor) {
        return applyTreeNodeEditor();
    }
    if (editorModal_.mode == EditorModalMode::TableGridEditor) {
        return activateTableGridEditorAction(buttonId);
    }
    if (editorModal_.mode == EditorModalMode::ItemListEditor) {
        if (buttonId == "add_item") {
            if (editorModalEdit_.active && !commitEditorModalFieldEdit()) {
                return false;
            }

            const int insertIndex = itemListEditorDialog_.selectedItemIndex >= 0
                ? itemListEditorDialog_.selectedItemIndex + 1
                : static_cast<int>(itemListEditorDialog_.items.size());
            const auto iterator = itemListEditorDialog_.items.begin() + std::clamp(insertIndex, 0, static_cast<int>(itemListEditorDialog_.items.size()));
            itemListEditorDialog_.items.insert(iterator, "Item " + std::to_string(itemListEditorDialog_.items.size() + 1));
            if (itemListEditorDialog_.supportsActions) {
                const auto actionIterator = itemListEditorDialog_.actions.begin() + std::clamp(insertIndex, 0, static_cast<int>(itemListEditorDialog_.actions.size()));
                itemListEditorDialog_.actions.insert(actionIterator, {});
            }
            setItemListEditorSelectedIndex(std::clamp(insertIndex, 0, static_cast<int>(itemListEditorDialog_.items.size()) - 1));
            redraw();
            return true;
        }
        if (buttonId == "remove_item") {
            if (editorModalEdit_.active && !commitEditorModalFieldEdit()) {
                return false;
            }

            if (itemListEditorDialog_.selectedItemIndex < 0 || itemListEditorDialog_.selectedItemIndex >= static_cast<int>(itemListEditorDialog_.items.size())) {
                editorModal_.statusText = "Select an item before removing it.";
                redraw();
                return false;
            }

            itemListEditorDialog_.items.erase(itemListEditorDialog_.items.begin() + itemListEditorDialog_.selectedItemIndex);
            if (itemListEditorDialog_.supportsActions && itemListEditorDialog_.selectedItemIndex < static_cast<int>(itemListEditorDialog_.actions.size())) {
                itemListEditorDialog_.actions.erase(itemListEditorDialog_.actions.begin() + itemListEditorDialog_.selectedItemIndex);
            }
            setItemListEditorSelectedIndex(std::min(itemListEditorDialog_.selectedItemIndex, static_cast<int>(itemListEditorDialog_.items.size()) - 1));
            if (editorModalEdit_.active) {
                cancelEditorModalFieldEdit();
            }
            redraw();
            return true;
        }
        if (buttonId == "move_up_item" || buttonId == "move_down_item") {
            if (editorModalEdit_.active && !commitEditorModalFieldEdit()) {
                return false;
            }

            const int currentIndex = itemListEditorDialog_.selectedItemIndex;
            if (currentIndex < 0 || currentIndex >= static_cast<int>(itemListEditorDialog_.items.size())) {
                editorModal_.statusText = "Select an item before reordering it.";
                redraw();
                return false;
            }

            const int delta = buttonId == "move_up_item" ? -1 : 1;
            const int targetIndex = currentIndex + delta;
            if (targetIndex < 0 || targetIndex >= static_cast<int>(itemListEditorDialog_.items.size())) {
                editorModal_.statusText = buttonId == "move_up_item"
                    ? "Selected item is already at the top."
                    : "Selected item is already at the bottom.";
                redraw();
                return false;
            }

            std::swap(itemListEditorDialog_.items[static_cast<std::size_t>(currentIndex)], itemListEditorDialog_.items[static_cast<std::size_t>(targetIndex)]);
            if (itemListEditorDialog_.supportsActions
                && currentIndex < static_cast<int>(itemListEditorDialog_.actions.size())
                && targetIndex < static_cast<int>(itemListEditorDialog_.actions.size())) {
                std::swap(itemListEditorDialog_.actions[static_cast<std::size_t>(currentIndex)], itemListEditorDialog_.actions[static_cast<std::size_t>(targetIndex)]);
            }
            setItemListEditorSelectedIndex(targetIndex);
            redraw();
            return true;
        }
        if (buttonId == "apply_items") {
            return applyItemListEditor();
        }
    }

    closeEditorModalDialog(buttonId);
    return true;
}

void MainWindow::showEditorMessageDialog(const std::string& title, const std::string& message)
{
    cancelInspectorEdit();
    cancelEditorModalFieldEdit();
    clearCanvasInteraction();
    requestKeyboardFocus();

    editorModal_.visible = true;
    editorModal_.mode = EditorModalMode::Message;
    editorModal_.title = title;
    editorModal_.message = message;
    editorModal_.lines.clear();
    editorModal_.buttons = { { "ok", "OK" } };
    editorModal_.result.clear();
    editorModal_.statusText.clear();
    editorModal_.preferredWidth = 0.0f;
    editorModal_.preferredHeight = 0.0f;
    newProjectWizard_.visible = false;
    projectSettingsDialog_.visible = false;
    resourceManagerDialog_.visible = false;
    keyboardShortcutDialog_.visible = false;
    redraw();
}

void MainWindow::showEditorValidationDialog(const validation::ValidationReport& report,
    const std::string& reportPathText,
    const std::string& reportWriteError)
{
    cancelInspectorEdit();
    cancelEditorModalFieldEdit();
    clearCanvasInteraction();
    requestKeyboardFocus();

    editorModal_.visible = true;
    editorModal_.mode = EditorModalMode::Message;
    editorModal_.message.clear();
    editorModal_.lines.clear();
    editorModal_.buttons = { { "ok", "OK" } };
    editorModal_.result.clear();
    editorModal_.statusText.clear();
    editorModal_.preferredWidth = 0.0f;
    editorModal_.preferredHeight = 0.0f;
    newProjectWizard_.visible = false;
    projectSettingsDialog_.visible = false;
    resourceManagerDialog_.visible = false;
    keyboardShortcutDialog_.visible = false;

    if (report.hasErrors()) {
        editorModal_.title = "Validation Errors";
        editorModal_.lines.push_back("Validation found " + std::to_string(report.errorCount())
            + " errors and " + std::to_string(report.warningCount()) + " warnings.");
    }
    else if (report.hasWarnings()) {
        editorModal_.title = "Validation Warnings";
        editorModal_.lines.push_back("Validation found 0 errors and " + std::to_string(report.warningCount()) + " warnings.");
    }
    else {
        editorModal_.title = "Validation Passed";
        editorModal_.lines.push_back("Validation passed with no errors.");
    }

    const int infoCount = validationInfoCount(report);
    if (infoCount > 0) {
        editorModal_.lines.push_back("Info messages: " + std::to_string(infoCount) + ".");
    }

    constexpr std::size_t kPreviewMessageCount = 10;
    std::size_t previewCount = 0;
    for (const auto& message : report.messages) {
        if (previewCount >= kPreviewMessageCount) {
            break;
        }

        std::string severity;
        switch (message.severity) {
        case validation::ValidationSeverity::Info:
            severity = "Info";
            break;
        case validation::ValidationSeverity::Warning:
            severity = "Warning";
            break;
        case validation::ValidationSeverity::Error:
            severity = "Error";
            break;
        }

        std::string line = severity;
        if (!message.code.empty()) {
            line += " [" + message.code + "]";
        }
        line += ": " + message.message;
        editorModal_.lines.push_back(std::move(line));
        ++previewCount;
    }

    if (report.messages.size() > kPreviewMessageCount) {
        editorModal_.lines.push_back("Showing first " + std::to_string(kPreviewMessageCount)
            + " of " + std::to_string(report.messages.size()) + " messages.");
    }

    if (!reportPathText.empty()) {
        editorModal_.lines.push_back("Full report written to " + reportPathText);
    }
    if (!reportWriteError.empty()) {
        editorModal_.lines.push_back("Validation report write failed: " + reportWriteError);
    }

    redraw();
}

void MainWindow::closeEditorModalDialog(const std::string& result)
{
    editorModalEdit_ = {};
    textEditControl_.clear();
    dropdownControl_.close();
    editorModal_.result = result;
    editorModal_.visible = false;
    editorModal_.mode = EditorModalMode::Message;
    editorModal_.statusText.clear();
    editorModal_.preferredWidth = 0.0f;
    editorModal_.preferredHeight = 0.0f;
    newProjectWizard_.visible = false;
    projectSettingsDialog_.visible = false;
    resourceManagerDialog_.visible = false;
    keyboardShortcutDialog_.visible = false;
    itemListEditorDialog_ = {};
    tableGridEditorDialog_ = {};
    treeNodeEditorDialog_ = {};
    requestKeyboardFocus();
    redraw();
}

bool MainWindow::isEditorModalVisible() const
{
    return editorModal_.visible;
}

MainWindow::PanelBounds MainWindow::editorModalDialogBounds() const
{
    const float preferredWidth = editorModal_.preferredWidth > 0.0f ? editorModal_.preferredWidth : kEditorModalPreferredWidth;
    const float preferredHeight = editorModal_.preferredHeight > 0.0f ? editorModal_.preferredHeight : kEditorModalPreferredHeight;
    const bool tableGridEditor = editorModal_.mode == EditorModalMode::TableGridEditor;
    const float maxWidthLimit = tableGridEditor ? kTableGridEditorMaxWidth : kEditorModalMaxWidth;
    const float maxHeightLimit = tableGridEditor ? kTableGridEditorMaxHeight : kEditorModalMaxHeight;
    const float outerMargin = tableGridEditor ? kTableGridEditorOuterMargin : 120.0f;
    const float maxWidth = std::max(0.0f, std::min(maxWidthLimit, width() - outerMargin));
    const float minWidth = std::min(kEditorModalMinWidth, maxWidth);
    const float dialogWidth = maxWidth <= 0.0f
        ? 0.0f
        : std::clamp(preferredWidth, minWidth, maxWidth);

    std::vector<std::string> bodyLines = splitMessageLines(editorModal_.message);
    bodyLines.insert(bodyLines.end(), editorModal_.lines.begin(), editorModal_.lines.end());

    const float visibleLineCount = static_cast<float>(std::min<std::size_t>(kEditorModalMaxBodyLines,
        std::max<std::size_t>(1, bodyLines.size())));
    const float bodyHeight = visibleLineCount * 22.0f;
    const float buttonSectionHeight = editorModal_.buttons.empty() ? 0.0f : (kEditorModalButtonHeight + 24.0f);
    const float maxHeight = std::max(0.0f, std::min(maxHeightLimit, height() - outerMargin));
    const float minHeight = std::min(kEditorModalMinHeight, maxHeight);
    const float desiredHeight = editorModal_.mode == EditorModalMode::Message
        ? std::max(preferredHeight, 68.0f + bodyHeight + kEditorModalSectionSpacing + buttonSectionHeight)
        : preferredHeight;
    const float dialogHeight = maxHeight <= 0.0f
        ? 0.0f
        : std::clamp(desiredHeight, minHeight, maxHeight);

    return {
        std::max(0.0f, (width() - dialogWidth) * 0.5f),
        std::max(0.0f, (height() - dialogHeight) * 0.5f),
        dialogWidth,
        dialogHeight
    };
}

std::vector<MainWindow::PanelBounds> MainWindow::editorModalButtonBounds() const
{
    std::vector<PanelBounds> bounds;
    if (editorModal_.buttons.empty()) {
        return bounds;
    }

    const PanelBounds dialogBounds = editorModalDialogBounds();
    const float buttonWidth = editorModal_.mode == EditorModalMode::ItemListEditor
        ? 108.0f
        : kEditorModalButtonWidth;
    const float buttonSpacing = editorModal_.mode == EditorModalMode::ItemListEditor
        ? 8.0f
        : kEditorModalButtonSpacing;
    const float totalWidth = static_cast<float>(editorModal_.buttons.size()) * buttonWidth
        + static_cast<float>(std::max<std::size_t>(0, editorModal_.buttons.size() - 1)) * buttonSpacing;
    const float buttonX = dialogBounds.x + std::max(0.0f, (dialogBounds.width - totalWidth) * 0.5f);
    const float buttonY = dialogBounds.y + dialogBounds.height - kEditorModalButtonHeight - 16.0f;

    bounds.reserve(editorModal_.buttons.size());
    for (std::size_t index = 0; index < editorModal_.buttons.size(); ++index) {
        bounds.push_back({
            buttonX + static_cast<float>(index) * (buttonWidth + buttonSpacing),
            buttonY,
            buttonWidth,
            kEditorModalButtonHeight
        });
    }

    return bounds;
}

void MainWindow::drawEditorModalDialog(visage::Canvas& canvas) const
{
    if (!isEditorModalVisible()) {
        return;
    }

    canvas.setColor(0xc0101318);
    canvas.fill(0.0f, 0.0f, width(), height());

    const PanelBounds dialogBounds = editorModalDialogBounds();
    canvas.setColor(0xff101318);
    canvas.fill(dialogBounds.x + 6.0f, dialogBounds.y + 8.0f, dialogBounds.width, dialogBounds.height);
    canvas.setColor(0xff232a34);
    canvas.fill(dialogBounds.x, dialogBounds.y, dialogBounds.width, dialogBounds.height);
    canvas.setColor(0xff2f3948);
    canvas.fill(dialogBounds.x, dialogBounds.y, dialogBounds.width, 38.0f);

    canvas.setColor(0xff12161c);
    canvas.fill(dialogBounds.x, dialogBounds.y, dialogBounds.width, 1.0f);
    canvas.fill(dialogBounds.x, dialogBounds.y + dialogBounds.height - 1.0f, dialogBounds.width, 1.0f);
    canvas.fill(dialogBounds.x, dialogBounds.y, 1.0f, dialogBounds.height);
    canvas.fill(dialogBounds.x + dialogBounds.width - 1.0f, dialogBounds.y, 1.0f, dialogBounds.height);

    if (!canDrawText()) {
        return;
    }

    canvas.setColor(0xfff3f5f8);
    canvas.text(editorModal_.title, labelFont_, visage::Font::kTopLeft,
        dialogBounds.x + 14.0f, dialogBounds.y + 6.0f, dialogBounds.width - 28.0f, 26.0f);

    if (editorModal_.mode == EditorModalMode::Message) {
        std::vector<std::string> bodyLines = splitMessageLines(editorModal_.message);
        bodyLines.insert(bodyLines.end(), editorModal_.lines.begin(), editorModal_.lines.end());
        if (bodyLines.empty()) {
            bodyLines.push_back({});
        }

        float lineY = dialogBounds.y + 52.0f;
        const std::size_t visibleCount = std::min(bodyLines.size(), kEditorModalMaxBodyLines);
        for (std::size_t index = 0; index < visibleCount; ++index) {
            canvas.setColor(0xffdde2ea);
            canvas.text(bodyLines[index], labelFont_, visage::Font::kTopLeft,
                dialogBounds.x + 14.0f, lineY, dialogBounds.width - 28.0f, 20.0f);
            lineY += 22.0f;
        }

        if (bodyLines.size() > visibleCount) {
            canvas.setColor(0xffb6bfcc);
            canvas.text("...", labelFont_, visage::Font::kTopLeft,
                dialogBounds.x + 14.0f, lineY, dialogBounds.width - 28.0f, 20.0f);
        }
    }
    else {
        const PanelBounds bodyBounds = editorModal_.mode == EditorModalMode::ResourceManager
            ? resourceManagerDetailBounds()
            : (editorModal_.mode == EditorModalMode::ItemListEditor
                    ? itemListEditorFormBounds()
                    : (editorModal_.mode == EditorModalMode::TableGridEditor ? tableGridEditorFormBounds() : editorModalBodyBounds()));
        const float fieldLabelWidth = editorModal_.mode == EditorModalMode::ResourceManager
            ? kResourceManagerFieldLabelWidth
            : kEditorModalFormLabelWidth;

        if (editorModal_.mode == EditorModalMode::ItemListEditor) {
            const PanelBounds previewBounds = itemListEditorPreviewBounds();
            canvas.setColor(0xff1a2028);
            canvas.fill(previewBounds.x, previewBounds.y, previewBounds.width, previewBounds.height);
            canvas.setColor(0xff12161c);
            canvas.fill(previewBounds.x, previewBounds.y, previewBounds.width, 1.0f);
            canvas.fill(previewBounds.x, previewBounds.y + previewBounds.height - 1.0f, previewBounds.width, 1.0f);
            canvas.fill(previewBounds.x, previewBounds.y, 1.0f, previewBounds.height);
            canvas.fill(previewBounds.x + previewBounds.width - 1.0f, previewBounds.y, 1.0f, previewBounds.height);
            canvas.setColor(0xffd6dbe4);
            canvas.text(itemListEditorDialog_.supportsActions ? "Items / Actions" : "Items", labelFont_, visage::Font::kTopLeft,
                previewBounds.x + 10.0f, previewBounds.y + 6.0f, previewBounds.width - 20.0f, 20.0f);

            const std::size_t visibleCount = itemListEditorVisibleRowCount(previewBounds.height, itemListEditorDialog_.items.size());
            for (std::size_t index = 0; index < visibleCount; ++index) {
                const float rowTop = itemListEditorRowTop(previewBounds.y, index);
                const int realIndex = itemListEditorDialog_.previewScrollOffset + static_cast<int>(index);
                const bool selected = realIndex == itemListEditorDialog_.selectedItemIndex;
                canvas.setColor(selected ? 0xff355382 : (index % 2 == 0 ? 0xff222936 : 0xff1d2430));
                canvas.fill(previewBounds.x + kItemListEditorPreviewRowInsetX,
                    rowTop,
                    previewBounds.width - kItemListEditorPreviewRowInsetX * 2.0f,
                    kItemListEditorPreviewRowFillHeight);
                canvas.setColor(selected ? 0xfff3f7ff : 0xffdde2ea);
                canvas.text(itemListEditorRowLabel(itemListEditorDialog_.items,
                        itemListEditorDialog_.actions,
                        itemListEditorDialog_.supportsActions,
                        realIndex), labelFont_, visage::Font::kTopLeft,
                    previewBounds.x + kItemListEditorPreviewRowTextInsetX,
                    rowTop + 5.0f,
                    previewBounds.width - kItemListEditorPreviewRowTextInsetX * 2.0f,
                    kItemListEditorPreviewRowHeight - 8.0f);
            }

            if (itemListEditorDialog_.items.empty()) {
                canvas.setColor(0xff9eabbc);
                canvas.text("No items yet. Click Add to create the first item.", labelFont_, visage::Font::kCenter,
                    previewBounds.x + 12.0f, previewBounds.y + 50.0f, previewBounds.width - 24.0f, previewBounds.height - 62.0f);
            }
            else if (visibleCount < itemListEditorDialog_.items.size()) {
                std::string overflowHint;
                const int firstVisible = itemListEditorDialog_.previewScrollOffset;
                const int lastVisible = itemListEditorDialog_.previewScrollOffset + static_cast<int>(visibleCount) - 1;
                if (firstVisible > 0 && lastVisible < static_cast<int>(itemListEditorDialog_.items.size()) - 1) {
                    overflowHint = "More above/below";
                }
                else if (firstVisible > 0) {
                    overflowHint = "More above";
                }
                else {
                    overflowHint = "More below";
                }
                canvas.setColor(0xff9eabbc);
                canvas.text(overflowHint, labelFont_, visage::Font::kTopRight,
                    previewBounds.x + 10.0f, previewBounds.y + 6.0f, previewBounds.width - 20.0f, 20.0f);
            }
        }
        else if (editorModal_.mode == EditorModalMode::TableGridEditor) {
            const PanelBounds gridBounds = tableGridEditorGridBounds();
            const PanelBounds formBounds = tableGridEditorFormBounds();
            const std::size_t columnCount = std::max<std::size_t>(1, tableGridEditorDialog_.columns.size());
            const float innerX = gridBounds.x + 6.0f;
            const float innerY = gridBounds.y + 6.0f;
            const float innerWidth = std::max(0.0f, gridBounds.width - 12.0f);
            const float innerHeight = std::max(0.0f, gridBounds.height - 12.0f);
            const float cellWidth = innerWidth / static_cast<float>(columnCount);
            const std::size_t visibleRowCount = std::min<std::size_t>(
                tableGridEditorDialog_.rows.size(),
                std::max<std::size_t>(1, static_cast<std::size_t>(std::floor(std::max(0.0f, innerHeight - kTableGridEditorHeaderHeight) / kTableGridEditorRowHeight))));
            const float headingY = bodyBounds.y;
            const float instructionY = headingY + kTableGridEditorHeadingHeight + kTableGridEditorHeadingGap;
            const float headerTextTop = verticallyCenteredTextTop(innerY, kTableGridEditorHeaderHeight, 16.0f, 5.0f);

            canvas.setColor(0xffd6dbe4);
            canvas.text("Table / Grid", labelFont_, visage::Font::kTopLeft,
                bodyBounds.x, headingY, bodyBounds.width, kTableGridEditorHeadingHeight);
            canvas.setColor(0xff9eabbc);
            canvas.text("Select a cell, edit Cell Text below, then click Apply.", labelFont_, visage::Font::kTopLeft,
                bodyBounds.x, instructionY, bodyBounds.width, kTableGridEditorInstructionHeight);
            canvas.setColor(0xff1a2028);
            canvas.fill(gridBounds.x, gridBounds.y, gridBounds.width, gridBounds.height);
            canvas.setColor(0xff12161c);
            canvas.fill(gridBounds.x, gridBounds.y, gridBounds.width, 1.0f);
            canvas.fill(gridBounds.x, gridBounds.y + gridBounds.height - 1.0f, gridBounds.width, 1.0f);
            canvas.fill(gridBounds.x, gridBounds.y, 1.0f, gridBounds.height);
            canvas.fill(gridBounds.x + gridBounds.width - 1.0f, gridBounds.y, 1.0f, gridBounds.height);

            canvas.saveState();
            canvas.setClampBounds(gridBounds.x + 1.0f, gridBounds.y + 1.0f, std::max(0.0f, gridBounds.width - 2.0f), std::max(0.0f, gridBounds.height - 2.0f));

            for (std::size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
                const float columnX = innerX + static_cast<float>(columnIndex) * cellWidth;
                const bool selectedColumn = static_cast<int>(columnIndex) == tableGridEditorDialog_.selectedColumn;
                canvas.setColor(selectedColumn ? 0xff355382 : 0xff2a3442);
                canvas.fill(columnX, innerY, cellWidth - 1.0f, kTableGridEditorHeaderHeight - 1.0f);
                canvas.setColor(0xff12161c);
                canvas.fill(columnX + cellWidth - 1.0f, innerY, 1.0f, innerHeight);
                canvas.setColor(0xffeef2f8);
                const std::string headerText = columnIndex < tableGridEditorDialog_.columns.size()
                    ? tableGridEditorDialog_.columns[columnIndex]
                    : std::string{};
                canvas.text(headerText.empty() ? std::string{ "<empty>" } : headerText, labelFont_, visage::Font::kTopLeft,
                    columnX + 8.0f, headerTextTop, std::max(0.0f, cellWidth - 14.0f), kTableGridEditorHeaderHeight - 8.0f);
            }

            float rowTop = innerY + kTableGridEditorHeaderHeight;
            for (std::size_t rowIndex = 0; rowIndex < visibleRowCount; ++rowIndex) {
                const bool selectedRow = static_cast<int>(rowIndex) == tableGridEditorDialog_.selectedRow;
                for (std::size_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
                    const float columnX = innerX + static_cast<float>(columnIndex) * cellWidth;
                    const bool selectedCell = selectedRow && static_cast<int>(columnIndex) == tableGridEditorDialog_.selectedColumn;
                    const float cellTextTop = verticallyCenteredTextTop(rowTop, kTableGridEditorRowHeight, 16.0f, 4.0f);
                    canvas.setColor(selectedCell ? 0xff355382 : (selectedRow ? 0xff263749 : (rowIndex % 2 == 0 ? 0xff222936 : 0xff1d2430)));
                    canvas.fill(columnX, rowTop, cellWidth - 1.0f, kTableGridEditorRowHeight - 1.0f);
                    canvas.setColor(selectedCell ? 0xfff3f7ff : 0xffdde2ea);
                    canvas.text(model::getCellText(tableGridEditorDialog_.rows, static_cast<int>(rowIndex), static_cast<int>(columnIndex)),
                        labelFont_, visage::Font::kTopLeft,
                        columnX + 8.0f, cellTextTop, std::max(0.0f, cellWidth - 14.0f), kTableGridEditorRowHeight - 6.0f);
                }
                rowTop += kTableGridEditorRowHeight;
            }

            canvas.restoreState();

            if (tableGridEditorDialog_.columns.empty()) {
                canvas.setColor(0xff9eabbc);
                canvas.text("No columns yet. Add a column to start building the table.", labelFont_, visage::Font::kCenter,
                    gridBounds.x + 12.0f, gridBounds.y + 12.0f, gridBounds.width - 24.0f, gridBounds.height - 24.0f);
            }
            else if (tableGridEditorDialog_.rows.empty()) {
                canvas.setColor(0xff9eabbc);
                canvas.text("No rows yet. Add a row, then select a cell to edit.", labelFont_, visage::Font::kCenter,
                    gridBounds.x + 12.0f, gridBounds.y + 48.0f, gridBounds.width - 24.0f, gridBounds.height - 60.0f);
            }
            else if (visibleRowCount < tableGridEditorDialog_.rows.size()) {
                canvas.setColor(0xff9eabbc);
                canvas.text("...", labelFont_, visage::Font::kCenter,
                    gridBounds.x, gridBounds.y + gridBounds.height - 26.0f, gridBounds.width, 18.0f);
            }

            canvas.setColor(0xffd6dbe4);
            canvas.text("Actions", labelFont_, visage::Font::kTopLeft,
                formBounds.x, formBounds.y, formBounds.width, kTableGridEditorActionLabelHeight);
            for (const auto& button : tableGridEditorActionButtons()) {
                canvas.setColor(button.enabled ? 0xff39414e : 0xff2a3039);
                canvas.fill(button.bounds.x, button.bounds.y, button.bounds.width, button.bounds.height);
                canvas.setColor(0xff14161b);
                canvas.fill(button.bounds.x, button.bounds.y + button.bounds.height - 1.0f, button.bounds.width, 1.0f);
                canvas.setColor(button.enabled ? 0xfff3f5f8 : 0xff7f8997);
                canvas.text(button.text, labelFont_, visage::Font::kCenter,
                    button.bounds.x, button.bounds.y, button.bounds.width, button.bounds.height);
            }
        }
        else if (editorModal_.mode == EditorModalMode::TreeNodeEditor) {
            const PanelBounds listBounds = treeNodeEditorTextBounds();
            const PanelBounds formBounds = treeNodeEditorFormBounds();
            const auto rows = visibleTreeNodeEditorRows();
            canvas.setColor(0xffd6dbe4);
            canvas.text("Tree Nodes", labelFont_, visage::Font::kTopLeft,
                bodyBounds.x, bodyBounds.y, bodyBounds.width, 20.0f);
            canvas.setColor(0xff9eabbc);
            canvas.text("Select a node, edit Node Text below, then click Apply.", labelFont_, visage::Font::kTopLeft,
                bodyBounds.x, bodyBounds.y + 18.0f, bodyBounds.width, kTreeNodeEditorInstructionHeight - 2.0f);
            canvas.setColor(0xff1a2028);
            canvas.fill(listBounds.x, listBounds.y, listBounds.width, listBounds.height);
            canvas.setColor(0xff12161c);
            canvas.fill(listBounds.x, listBounds.y, listBounds.width, 1.0f);
            canvas.fill(listBounds.x, listBounds.y + listBounds.height - 1.0f, listBounds.width, 1.0f);
            canvas.fill(listBounds.x, listBounds.y, 1.0f, listBounds.height);
            canvas.fill(listBounds.x + listBounds.width - 1.0f, listBounds.y, 1.0f, listBounds.height);

            canvas.saveState();
            canvas.setClampBounds(listBounds.x + 1.0f, listBounds.y + 1.0f, std::max(0.0f, listBounds.width - 2.0f), std::max(0.0f, listBounds.height - 2.0f));
            float rowTop = listBounds.y + 8.0f;
            const std::size_t visibleCount = std::min<std::size_t>(
                rows.size(),
                std::max<std::size_t>(1, static_cast<std::size_t>(std::floor((listBounds.height - 16.0f) / kTreeNodeEditorRowHeight))));
            for (std::size_t index = 0; index < visibleCount; ++index) {
                const auto& row = rows[index];
                const bool selected = row.nodeId == treeNodeEditorDialog_.selectedNodeId;
                const float indent = 10.0f + static_cast<float>(row.depth) * kTreeNodeEditorRowIndent;
                const auto* node = findTreeEditorNode(row.nodeId);
                const std::string title = node == nullptr ? std::string{} : node->title;

                canvas.setColor(selected ? 0xff355382 : (index % 2 == 0 ? 0xff222936 : 0xff1d2430));
                canvas.fill(listBounds.x + 6.0f, rowTop, listBounds.width - 12.0f, kTreeNodeEditorRowHeight - 2.0f);
                if (row.hasChildren) {
                    canvas.setColor(0xff92b9ff);
                    canvas.fill(listBounds.x + indent, rowTop + 10.0f, 8.0f, 2.0f);
                }
                canvas.setColor(selected ? 0xfff3f7ff : 0xffdde2ea);
                canvas.text(title, labelFont_, visage::Font::kTopLeft,
                    listBounds.x + indent + 14.0f, rowTop + 4.0f,
                    listBounds.width - indent - 28.0f, kTreeNodeEditorRowHeight - 6.0f);
                rowTop += kTreeNodeEditorRowHeight;
            }
            canvas.restoreState();

            if (rows.empty()) {
                canvas.setColor(0xff9eabbc);
                canvas.text("No nodes yet. Use Add Child to create the first root node.", labelFont_, visage::Font::kCenter,
                    listBounds.x + 12.0f, listBounds.y + 12.0f, listBounds.width - 24.0f, listBounds.height - 24.0f);
            }
            else if (visibleCount < rows.size()) {
                canvas.setColor(0xff9eabbc);
                canvas.text("...", labelFont_, visage::Font::kCenter,
                    listBounds.x, listBounds.y + listBounds.height - 26.0f, listBounds.width, 18.0f);
            }

            canvas.setColor(0xffd6dbe4);
            canvas.text("Actions", labelFont_, visage::Font::kTopLeft,
                formBounds.x, formBounds.y, formBounds.width, kTreeNodeEditorActionLabelHeight);
            for (const auto& button : treeNodeEditorActionButtons()) {
                canvas.setColor(button.enabled ? 0xff39414e : 0xff2a3039);
                canvas.fill(button.bounds.x, button.bounds.y, button.bounds.width, button.bounds.height);
                canvas.setColor(0xff14161b);
                canvas.fill(button.bounds.x, button.bounds.y + button.bounds.height - 1.0f, button.bounds.width, 1.0f);
                canvas.setColor(button.enabled ? 0xfff3f5f8 : 0xff7f8997);
                canvas.text(button.text, labelFont_, visage::Font::kCenter,
                    button.bounds.x, button.bounds.y, button.bounds.width, button.bounds.height);
            }
        }

        for (const auto& hit : editorModalFieldHits()) {
            const bool active = editorModalEdit_.active && editorModalEdit_.key == hit.field.key;
            const bool drawInlineValue = !active || hit.field.editKind == PropertyInspector::PropertyEditKind::Choice;
            const float fieldTextTop = verticallyCenteredTextTop(hit.bounds.y, hit.bounds.height, 18.0f, 4.0f);
            canvas.setColor(0xffd6dbe4);
            canvas.text(hit.field.label, labelFont_, visage::Font::kTopLeft,
                bodyBounds.x, fieldTextTop, fieldLabelWidth - 14.0f, hit.bounds.height - 6.0f);

            canvas.setColor(active ? 0xff355382 : 0xff1a2028);
            canvas.fill(hit.bounds.x, hit.bounds.y, hit.bounds.width, hit.bounds.height);
            canvas.setColor(0xff12161c);
            canvas.fill(hit.bounds.x, hit.bounds.y, hit.bounds.width, 1.0f);
            canvas.fill(hit.bounds.x, hit.bounds.y + hit.bounds.height - 1.0f, hit.bounds.width, 1.0f);
            canvas.fill(hit.bounds.x, hit.bounds.y, 1.0f, hit.bounds.height);
            canvas.fill(hit.bounds.x + hit.bounds.width - 1.0f, hit.bounds.y, 1.0f, hit.bounds.height);

            std::string valueText = hit.field.value;
            if (valueText.empty()) {
                valueText = hit.field.editKind == PropertyInspector::PropertyEditKind::Choice ? "Select" : "";
            }
            if (hit.field.editKind == PropertyInspector::PropertyEditKind::Choice) {
                const auto iterator = std::find_if(hit.field.choices.begin(), hit.field.choices.end(), [&hit](const PropertyInspector::PropertyChoice& choice) {
                    return choice.value == hit.field.value;
                });
                if (iterator != hit.field.choices.end()) {
                    valueText = iterator->label;
                }
                valueText += "  >";
            }

            if (drawInlineValue) {
                canvas.setColor(0xffeef2f8);
                canvas.text(valueText, labelFont_, visage::Font::kTopLeft,
                    hit.bounds.x + 10.0f, fieldTextTop, hit.bounds.width - 20.0f, hit.bounds.height - 6.0f);
            }
        }

        if (editorModal_.mode == EditorModalMode::ResourceManager) {
            const PanelBounds previewBounds = resourceManagerPreviewBounds();
            const float previewHeaderHeight = 28.0f;
            const float previewFooterHeight = 44.0f;
            const PanelBounds previewContent{
                previewBounds.x + 10.0f,
                previewBounds.y + previewHeaderHeight + 8.0f,
                std::max(0.0f, previewBounds.width - 20.0f),
                std::max(0.0f, previewBounds.height - previewHeaderHeight - previewFooterHeight - 16.0f)
            };

            canvas.setColor(0xff1a2028);
            canvas.fill(previewBounds.x, previewBounds.y, previewBounds.width, previewBounds.height);
            canvas.setColor(0xff12161c);
            canvas.fill(previewBounds.x, previewBounds.y, previewBounds.width, 1.0f);
            canvas.fill(previewBounds.x, previewBounds.y + previewBounds.height - 1.0f, previewBounds.width, 1.0f);
            canvas.fill(previewBounds.x, previewBounds.y, 1.0f, previewBounds.height);
            canvas.fill(previewBounds.x + previewBounds.width - 1.0f, previewBounds.y, 1.0f, previewBounds.height);
            canvas.setColor(0xffd6dbe4);
            canvas.text("Preview", labelFont_, visage::Font::kTopLeft,
                previewBounds.x + 10.0f, previewBounds.y + 5.0f, previewBounds.width - 20.0f, previewHeaderHeight - 8.0f);

            canvas.setColor(0xff11151c);
            canvas.fill(previewContent.x, previewContent.y, previewContent.width, previewContent.height);
            canvas.setColor(0xff2b3340);
            canvas.fill(previewContent.x, previewContent.y, previewContent.width, 1.0f);
            canvas.fill(previewContent.x, previewContent.y + previewContent.height - 1.0f, previewContent.width, 1.0f);
            canvas.fill(previewContent.x, previewContent.y, 1.0f, previewContent.height);
            canvas.fill(previewContent.x + previewContent.width - 1.0f, previewContent.y, 1.0f, previewContent.height);

            const auto* selectedResource = document_.findResourceById(resourceManagerDialog_.selectedResourceId);
            if (resourceManagerDialog_.previewImageAvailable && !resourceManagerDialog_.previewSourcePath.empty()) {
                const auto cachedImage = imageResourceCache_.getOrLoad(resourceManagerDialog_.previewSourcePath);
                if (cachedImage.info.available && cachedImage.encodedBytes != nullptr && !cachedImage.encodedBytes->empty()) {
                    const auto drawRect = resources::ImageResourceCache::computeDrawRect(
                        previewContent.x + 4.0f,
                        previewContent.y + 4.0f,
                        std::max(0.0f, previewContent.width - 8.0f),
                        std::max(0.0f, previewContent.height - 8.0f),
                        cachedImage.info.width,
                        cachedImage.info.height,
                        resources::ImageScaleMode::Fit);
                    canvas.image(cachedImage.encodedBytes->data(),
                        static_cast<int>(cachedImage.encodedBytes->size()),
                        drawRect.x,
                        drawRect.y,
                        drawRect.width,
                        drawRect.height);
                }
            }
            if (!resourceManagerDialog_.previewImageAvailable) {
                std::string placeholderText = resourceManagerDialog_.previewStatus.empty()
                    ? std::string{ "No preview available." }
                    : resourceManagerDialog_.previewStatus;
                if (selectedResource != nullptr && selectedResource->type != model::ProjectResourceType::Image) {
                    placeholderText = "Preview available for image resources only.";
                }

                canvas.setColor(0xff9eabbc);
                canvas.text(placeholderText, labelFont_, visage::Font::kCenter,
                    previewContent.x + 10.0f, previewContent.y + 10.0f,
                    std::max(0.0f, previewContent.width - 20.0f), std::max(0.0f, previewContent.height - 20.0f));
            }

            canvas.setColor(0xffb8c3d1);
            canvas.text(resourceManagerDialog_.previewStatus, labelFont_, visage::Font::kTopLeft,
                previewBounds.x + 10.0f, previewBounds.y + previewBounds.height - previewFooterHeight + 6.0f,
                previewBounds.width - 20.0f, previewFooterHeight - 10.0f);
        }

        const PanelBounds statusBounds = editorModalStatusBounds();
        if (editorModal_.mode == EditorModalMode::ItemListEditor) {
            // Draw status as plain muted text for item list editor so it does not resemble an editable field
            canvas.setColor(0xff9eabbc);
            canvas.text(editorModal_.statusText, labelFont_, visage::Font::kTopLeft,
                statusBounds.x + 4.0f, statusBounds.y + 8.0f, statusBounds.width - 8.0f, statusBounds.height - 12.0f);
        }
        else {
            canvas.setColor(0xff1a2028);
            canvas.fill(statusBounds.x, statusBounds.y, statusBounds.width, statusBounds.height);
            canvas.setColor(0xff12161c);
            canvas.fill(statusBounds.x, statusBounds.y + statusBounds.height - 1.0f, statusBounds.width, 1.0f);
            canvas.setColor(0xffd6dbe4);
            canvas.text(editorModal_.statusText, labelFont_, visage::Font::kTopLeft,
                statusBounds.x + 8.0f, statusBounds.y + 6.0f, statusBounds.width - 16.0f, statusBounds.height - 8.0f);
        }
    }

    const auto buttonBounds = editorModalButtonBounds();
    for (std::size_t index = 0; index < buttonBounds.size(); ++index) {
        const bool accentButton = index == 0
            || editorModal_.buttons[index].id == "apply"
            || editorModal_.buttons[index].id == "create"
            || editorModal_.buttons[index].id == "apply_shortcuts"
            || editorModal_.buttons[index].id == "apply_table_grid"
            || editorModal_.buttons[index].id == "apply_items"
            || editorModal_.buttons[index].id == "apply_tree_nodes";
        canvas.setColor(accentButton ? 0xff355382 : 0xff39414e);
        canvas.fill(buttonBounds[index].x, buttonBounds[index].y, buttonBounds[index].width, buttonBounds[index].height);
        canvas.setColor(0xff14161b);
        canvas.fill(buttonBounds[index].x, buttonBounds[index].y + buttonBounds[index].height - 1.0f, buttonBounds[index].width, 1.0f);
        canvas.setColor(0xfff3f5f8);
        canvas.text(editorModal_.buttons[index].text, labelFont_, visage::Font::kCenter,
            buttonBounds[index].x, buttonBounds[index].y, buttonBounds[index].width, buttonBounds[index].height);
    }
}

bool MainWindow::handleEditorModalMouseDown(const visage::MouseEvent& e)
{
    if (!isEditorModalVisible()) {
        return false;
    }

    if (dropdownControl_.isOpen()) {
        const bool handledDropdownClick = dropdownControl_.mouseDown(e.position.x, e.position.y);
        handleDropdownSelection();
        if (handledDropdownClick) {
            redraw();
            return true;
        }
    }

    if (editorModalEdit_.active) {
        const auto activeField = editorModalFieldAt(e.position.x, e.position.y);
        if (activeField.has_value() && activeField->field.key == editorModalEdit_.key) {
            if (textEditControl_.mouseDown(e.position.x, e.position.y)) {
                redraw();
                return true;
            }
        }
        if (!commitEditorModalFieldEdit()) {
            return true;
        }
    }

    if (editorModal_.mode == EditorModalMode::TreeNodeEditor && textEditControl_.mouseDown(e.position.x, e.position.y)) {
        redraw();
        return true;
    }

    const auto buttonBounds = editorModalButtonBounds();
    for (std::size_t index = 0; index < buttonBounds.size(); ++index) {
        if (e.position.x >= buttonBounds[index].x && e.position.x <= buttonBounds[index].x + buttonBounds[index].width
            && e.position.y >= buttonBounds[index].y && e.position.y <= buttonBounds[index].y + buttonBounds[index].height) {
            activateEditorModalButton(editorModal_.buttons[index].id);
            return true;
        }
    }

    if (editorModal_.mode != EditorModalMode::Message) {
        if (editorModal_.mode == EditorModalMode::ItemListEditor) {
            if (const auto previewIndex = itemListEditorPreviewIndexAt(e.position.x, e.position.y)) {
                setItemListEditorSelectedIndex(*previewIndex);
                redraw();
                return true;
            }
        }
        else if (editorModal_.mode == EditorModalMode::TableGridEditor) {
            for (const auto& actionButton : tableGridEditorActionButtons()) {
                if (!actionButton.enabled) {
                    continue;
                }
                if (pointInBounds(e.position.x, e.position.y,
                        actionButton.bounds.x,
                        actionButton.bounds.y,
                        actionButton.bounds.width,
                        actionButton.bounds.height)) {
                    activateTableGridEditorAction(actionButton.id);
                    return true;
                }
            }

            if (const auto cellHit = tableGridEditorCellAt(e.position.x, e.position.y)) {
                selectTableGridEditorCell(cellHit->row, cellHit->column, cellHit->header);
                redraw();
                return true;
            }
        }
        else if (editorModal_.mode == EditorModalMode::TreeNodeEditor) {
            for (const auto& actionButton : treeNodeEditorActionButtons()) {
                if (!actionButton.enabled) {
                    continue;
                }
                if (pointInBounds(e.position.x, e.position.y,
                        actionButton.bounds.x,
                        actionButton.bounds.y,
                        actionButton.bounds.width,
                        actionButton.bounds.height)) {
                    activateTreeNodeEditorAction(actionButton.id);
                    return true;
                }
            }

            if (const auto selectedNodeId = treeNodeEditorRowAt(e.position.x, e.position.y)) {
                selectTreeEditorNode(*selectedNodeId);
                redraw();
                return true;
            }
        }

        if (const auto fieldHit = editorModalFieldAt(e.position.x, e.position.y)) {
            if (fieldHit->field.editKind == PropertyInspector::PropertyEditKind::Choice && !fieldHit->field.choices.empty()) {
                beginEditorModalFieldEdit(fieldHit->field);
                redraw();
                return true;
            }

            beginEditorModalFieldEdit(fieldHit->field);
            return true;
        }
    }

    redraw();
    return true;
}

bool MainWindow::canDrawText() const
{
    return labelFont_.packedFont() != nullptr;
}

} // namespace visiform::ui
