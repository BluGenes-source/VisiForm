#pragma once

#pragma once

#include <visage/app.h>

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace visiform::commands {

struct ShortcutGesture {
    bool ctrl = false;
    bool alt = false;
    bool shift = false;
    bool meta = false;
    visage::KeyCode key = visage::KeyCode::Unknown;

    [[nodiscard]] bool isValid() const
    {
        return key != visage::KeyCode::Unknown;
    }
};

struct CommandDefinition {
    std::string_view id{};
    std::string_view displayName{};
    std::string_view menuPath{};
    std::string_view defaultShortcut{};
    std::string_view hint{};
};

class CommandRegistry {
public:
    [[nodiscard]] static const std::vector<CommandDefinition>& definitions();
    [[nodiscard]] static const CommandDefinition* find(std::string_view id);
    [[nodiscard]] static std::optional<ShortcutGesture> parseShortcutString(std::string_view text);
    [[nodiscard]] static std::string formatShortcut(const ShortcutGesture& shortcut);
    [[nodiscard]] static std::optional<ShortcutGesture> shortcutFromKeyEvent(const visage::KeyEvent& event);
    [[nodiscard]] static bool matchesShortcut(const ShortcutGesture& shortcut, const visage::KeyEvent& event);
};

} // namespace visiform::commands
