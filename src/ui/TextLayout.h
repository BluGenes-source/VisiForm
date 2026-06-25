#pragma once

#include <visage/graphics.h>

#include <string>
#include <vector>

namespace visiform::ui {

struct TextLayoutRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

enum class TextOverflowMode {
    Clip,
    Ellipsis
};

struct TextLayoutOptions {
    bool multiline = false;
    bool wordWrap = false;
    TextOverflowMode overflowMode = TextOverflowMode::Clip;
    std::string horizontalAlignment = "Default";
    std::string verticalAlignment = "Default";
};

struct TextLayoutLine {
    std::string text;
    float width = 0.0f;
    float x = 0.0f;
    float y = 0.0f;
};

struct TextLayoutResult {
    std::vector<TextLayoutLine> lines;
    float lineHeight = 0.0f;
    float totalTextHeight = 0.0f;
    TextLayoutRect bounds{};
};

[[nodiscard]] TextOverflowMode textOverflowModeFromString(const std::string& value);
[[nodiscard]] std::string textOverflowModeToString(TextOverflowMode value);

[[nodiscard]] TextLayoutResult layoutText(
    const std::string& text,
    const visage::Font& font,
    const TextLayoutRect& bounds,
    const TextLayoutOptions& options);

void drawTextLayout(
    visage::Canvas& canvas,
    const visage::Font& font,
    const TextLayoutResult& layout);

void drawLaidOutText(
    visage::Canvas& canvas,
    const std::string& text,
    const visage::Font& font,
    const TextLayoutRect& bounds,
    const TextLayoutOptions& options);

} // namespace visiform::ui
