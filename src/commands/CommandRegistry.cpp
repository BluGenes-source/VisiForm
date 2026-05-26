#include "commands/CommandRegistry.h"

#include "commands/CommandRegistry.h"

#include "commands/CommandIds.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace visiform::commands {
namespace {

const std::vector<CommandDefinition> kDefinitions = {
    { ids::kFileNew, "New", "File", "Ctrl+N", "Create a new VisiForm project" },
    { ids::kFileOpen, "Open", "File", "Ctrl+O", "Open a .vfb.json project" },
    { ids::kFileOpenSample, "Open Sample", "File", "", "Open the sample project" },
    { ids::kFileSave, "Save", "File", "Ctrl+S", "Save the current project" },
    { ids::kFileSaveAs, "Save As", "File", "Ctrl+Shift+S", "Save the project to a new .vfb.json file" },
    { ids::kFileExport, "Export", "File", "Ctrl+E", "Export generated Visage C++ project" },
    { ids::kEditUndo, "Undo", "Edit", "Ctrl+Z", "Undo the last command" },
    { ids::kEditRedo, "Redo", "Edit", "Ctrl+Y", "Redo the last undone command" },
    { ids::kEditCopy, "Copy", "Edit", "Ctrl+C", "Copy selected widgets" },
    { ids::kEditPaste, "Paste", "Edit", "Ctrl+V", "Paste copied widgets" },
    { ids::kEditDelete, "Delete", "Edit", "Delete", "Delete the selected widget or widgets" },
    { ids::kEditDuplicate, "Duplicate", "Edit", "Ctrl+D", "Duplicate the primary selected widget" },
    { ids::kViewGrid, "Grid", "View", "Ctrl+G", "Toggle grid visibility" },
    { ids::kViewSnap, "Snap", "View", "Ctrl+Alt+S", "Toggle snap-to-grid" },
    { ids::kViewGuides, "Guides", "View", "Ctrl+Shift+G", "Toggle smart guides" },
    { ids::kViewMultiSelect, "Multi Select", "View", "", "Toggle multi-select mode" },
    { ids::kLayoutFitText, "Fit Text", "Layout", "Ctrl+Alt+F", "Fit the selected widget to its text" },
    { ids::kLayoutAlignLeft, "Align Left", "Layout", "Ctrl+Alt+Left", "Align selected widgets left" },
    { ids::kLayoutAlignTop, "Align Top", "Layout", "Ctrl+Alt+Up", "Align selected widgets top" },
    { ids::kLayoutAlignRight, "Align Right", "Layout", "Ctrl+Alt+Right", "Align selected widgets right" },
    { ids::kLayoutAlignBottom, "Align Bottom", "Layout", "Ctrl+Alt+Down", "Align selected widgets bottom" },
    { ids::kLayoutCenterHorizontal, "Center Horizontally", "Layout", "", "Center selected widgets horizontally" },
    { ids::kLayoutCenterVertical, "Center Vertically", "Layout", "", "Center selected widgets vertically" },
    { ids::kLayoutSameWidth, "Same Width", "Layout", "", "Match selected widget widths" },
    { ids::kLayoutSameHeight, "Same Height", "Layout", "", "Match selected widget heights" },
    { ids::kLayoutDistributeHorizontal, "Distribute Horizontally", "Layout", "", "Distribute selected widgets horizontally" },
    { ids::kLayoutDistributeVertical, "Distribute Vertically", "Layout", "", "Distribute selected widgets vertically" },
    { ids::kLayoutBringForward, "Bring Forward", "Layout", "Ctrl+Alt+Shift+B", "Bring the selected widget forward" },
    { ids::kLayoutSendBackward, "Send Backward", "Layout", "Ctrl+Alt+B", "Send the selected widget backward" },
    { ids::kProjectValidate, "Validate / Check", "Project", "Ctrl+Shift+V", "Validate the current project before export" },
    { ids::kProjectSettings, "Project Settings", "Project", "", "Open the project settings dialog" },
    { ids::kProjectResources, "Resources", "Project", "Ctrl+Alt+R", "Open the project resource manager" },
    { ids::kProjectKeyboardShortcuts, "Keyboard Shortcuts", "Project", "Ctrl+Alt+K", "Open keyboard shortcut settings" },
    { ids::kProjectExportDependencies, "Export Dependencies", "Project", "", "Show where export dependency settings are edited" },
    { ids::kViewValidationReport, "Validation Report", "View", "", "Show where the latest validation report was written" },
    { ids::kHelpAbout, "About VisiForm", "Help", "", "Show information about VisiForm" },
    { ids::kHelpGeneratedCodeGuide, "Generated Code Guide", "Help", "", "Show a short guide to generated code output" }
};

std::string normalizedShortcutText(std::string_view text)
{
    std::string normalized;
    normalized.reserve(text.size());
    for (char character : text) {
        if (!std::isspace(static_cast<unsigned char>(character))) {
            normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(character))));
        }
    }
    return normalized;
}

visage::KeyCode parseKeyToken(std::string_view token)
{
    const std::string normalized = normalizedShortcutText(token);
    if (normalized.size() == 1) {
        const char character = normalized.front();
        if (character >= 'A' && character <= 'Z') {
            return static_cast<visage::KeyCode>(character);
        }
    }

    if (normalized == "DELETE") {
        return visage::KeyCode::Delete;
    }
    if (normalized == "ENTER" || normalized == "RETURN") {
        return visage::KeyCode::Return;
    }
    if (normalized == "ESC" || normalized == "ESCAPE") {
        return visage::KeyCode::Escape;
    }
    if (normalized == "LEFT") {
        return visage::KeyCode::Left;
    }
    if (normalized == "RIGHT") {
        return visage::KeyCode::Right;
    }
    if (normalized == "UP") {
        return visage::KeyCode::Up;
    }
    if (normalized == "DOWN") {
        return visage::KeyCode::Down;
    }
    if (normalized.size() == 2 && normalized.front() == 'F' && std::isdigit(static_cast<unsigned char>(normalized[1])) != 0) {
        switch (normalized[1]) {
        case '1':
            return visage::KeyCode::F1;
        case '2':
            return visage::KeyCode::F2;
        case '3':
            return visage::KeyCode::F3;
        case '4':
            return visage::KeyCode::F4;
        case '5':
            return visage::KeyCode::F5;
        case '6':
            return visage::KeyCode::F6;
        case '7':
            return visage::KeyCode::F7;
        case '8':
            return visage::KeyCode::F8;
        case '9':
            return visage::KeyCode::F9;
        default:
            break;
        }
    }

    return visage::KeyCode::Unknown;
}

std::string keyToken(visage::KeyCode key)
{
    if (key >= visage::KeyCode::A && key <= visage::KeyCode::Z) {
        return std::string(1, static_cast<char>(key));
    }

    switch (key) {
    case visage::KeyCode::Delete:
        return "Delete";
    case visage::KeyCode::Return:
        return "Enter";
    case visage::KeyCode::Escape:
        return "Escape";
    case visage::KeyCode::Left:
        return "Left";
    case visage::KeyCode::Right:
        return "Right";
    case visage::KeyCode::Up:
        return "Up";
    case visage::KeyCode::Down:
        return "Down";
    case visage::KeyCode::F1:
        return "F1";
    case visage::KeyCode::F2:
        return "F2";
    case visage::KeyCode::F3:
        return "F3";
    case visage::KeyCode::F4:
        return "F4";
    case visage::KeyCode::F5:
        return "F5";
    case visage::KeyCode::F6:
        return "F6";
    case visage::KeyCode::F7:
        return "F7";
    case visage::KeyCode::F8:
        return "F8";
    case visage::KeyCode::F9:
        return "F9";
    default:
        return {};
    }
}

} // namespace

const std::vector<CommandDefinition>& CommandRegistry::definitions()
{
    return kDefinitions;
}

const CommandDefinition* CommandRegistry::find(std::string_view id)
{
    const auto& definitions = CommandRegistry::definitions();
    const auto iterator = std::find_if(definitions.begin(), definitions.end(), [id](const CommandDefinition& definition) {
        return definition.id == id;
    });
    return iterator == definitions.end() ? nullptr : &*iterator;
}

std::optional<ShortcutGesture> CommandRegistry::parseShortcutString(std::string_view text)
{
    ShortcutGesture shortcut;
    std::string remaining(text);
    std::size_t start = 0;
    while (start < remaining.size()) {
        const std::size_t plus = remaining.find('+', start);
        const std::string token = plus == std::string::npos
            ? remaining.substr(start)
            : remaining.substr(start, plus - start);
        const std::string normalized = normalizedShortcutText(token);
        if (normalized.empty()) {
            return std::nullopt;
        }
        if (normalized == "CTRL") {
            shortcut.ctrl = true;
        }
        else if (normalized == "ALT") {
            shortcut.alt = true;
        }
        else if (normalized == "SHIFT") {
            shortcut.shift = true;
        }
        else {
            if (shortcut.key != visage::KeyCode::Unknown) {
                return std::nullopt;
            }
            shortcut.key = parseKeyToken(normalized);
            if (shortcut.key == visage::KeyCode::Unknown) {
                return std::nullopt;
            }
        }

        if (plus == std::string::npos) {
            break;
        }
        start = plus + 1;
    }

    if (!shortcut.isValid()) {
        return std::nullopt;
    }

    return shortcut;
}

std::string CommandRegistry::formatShortcut(const ShortcutGesture& shortcut)
{
    if (!shortcut.isValid()) {
        return {};
    }

    std::string text;
    if (shortcut.ctrl) {
        text += "Ctrl+";
    }
    if (shortcut.alt) {
        text += "Alt+";
    }
    if (shortcut.shift) {
        text += "Shift+";
    }
    text += keyToken(shortcut.key);
    return text;
}

bool CommandRegistry::matchesShortcut(const ShortcutGesture& shortcut, const visage::KeyEvent& event)
{
    if (!shortcut.isValid()) {
        return false;
    }

    return shortcut.key == event.keyCode()
        && shortcut.ctrl == event.isCtrlDown()
        && shortcut.alt == event.isAltDown()
        && shortcut.shift == event.isShiftDown();
}

} // namespace visiform::commands
