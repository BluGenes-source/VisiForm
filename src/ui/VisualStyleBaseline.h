#pragma once

#include <visage/graphics.h>

#include <algorithm>

namespace visiform::ui::visual_style {

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct Palette {
    int fill = 0xff2b313d;
    int border = 0xff97a3b7;
    int text = 0xffeef2f8;
    int accent = 0xff2d7ff9;
    int disabled = 0xff6c7788;
    int highlight = 0xffc8d2e2;
    int shadow = 0xff11151c;
    int hoverFill = 0xff354052;
    int pressedFill = 0xff232a35;
    int recessedFill = 0xff202630;
};

enum class BaseState {
    Normal,
    Hovered,
    CheckedOrSelected,
    Pressed,
    Disabled
};

struct State {
    bool enabled = true;
    bool hovered = false;
    bool pressed = false;
    bool focused = false;
    bool checkedOrSelected = false;
    bool active = false;
    bool readOnly = false;
};

inline BaseState resolveBaseState(const State& state)
{
    if (!state.enabled) {
        return BaseState::Disabled;
    }
    if (state.pressed) {
        return BaseState::Pressed;
    }
    if (state.checkedOrSelected || state.active) {
        return BaseState::CheckedOrSelected;
    }
    if (state.hovered) {
        return BaseState::Hovered;
    }
    return BaseState::Normal;
}

inline int blend(int first, int second, float amount)
{
    const auto channel = [amount](int a, int b) {
        return static_cast<int>(static_cast<float>(a) * (1.0f - amount) + static_cast<float>(b) * amount + 0.5f);
    };
    return (channel((first >> 24) & 0xff, (second >> 24) & 0xff) << 24)
        | (channel((first >> 16) & 0xff, (second >> 16) & 0xff) << 16)
        | (channel((first >> 8) & 0xff, (second >> 8) & 0xff) << 8)
        | channel(first & 0xff, second & 0xff);
}

inline Palette makePalette(int fill, int border, int text, int accent, int disabled)
{
    Palette palette;
    palette.fill = fill;
    palette.border = border;
    palette.text = text;
    palette.accent = accent;
    palette.disabled = disabled;
    palette.highlight = blend(border, 0xffffffff, 0.52f);
    palette.shadow = blend(border, 0xff000000, 0.62f);
    palette.hoverFill = blend(fill, accent, 0.12f);
    palette.pressedFill = blend(fill, 0xff000000, 0.20f);
    palette.recessedFill = blend(fill, 0xff000000, 0.12f);
    return palette;
}

inline void fill(visage::Canvas& canvas, const Rect& rect, int color)
{
    if (rect.width <= 0.0f || rect.height <= 0.0f) {
        return;
    }
    canvas.setColor(color);
    canvas.fill(rect.x, rect.y, rect.width, rect.height);
}

inline void drawBorder(visage::Canvas& canvas, const Rect& rect, int color, float thickness = 1.0f)
{
    if (rect.width <= 0.0f || rect.height <= 0.0f || thickness <= 0.0f) {
        return;
    }
    const float edge = std::min(thickness, std::min(rect.width, rect.height) * 0.5f);
    canvas.setColor(color);
    canvas.fill(rect.x, rect.y, rect.width, edge);
    canvas.fill(rect.x, rect.y + rect.height - edge, rect.width, edge);
    canvas.fill(rect.x, rect.y, edge, rect.height);
    canvas.fill(rect.x + rect.width - edge, rect.y, edge, rect.height);
}

inline void drawBevel(visage::Canvas& canvas, const Rect& rect, const Palette& palette,
    bool pressed = false, bool hovered = false, bool enabled = true)
{
    const int baseFill = !enabled ? blend(palette.fill, palette.disabled, 0.55f)
        : pressed ? palette.pressedFill
        : hovered ? palette.hoverFill
        : palette.fill;
    fill(canvas, rect, baseFill);
    drawBorder(canvas, rect, enabled ? palette.border : blend(palette.border, palette.disabled, 0.65f));

    if (rect.width < 3.0f || rect.height < 3.0f) {
        return;
    }
    const int leading = pressed ? palette.shadow : palette.highlight;
    const int trailing = pressed ? palette.highlight : palette.shadow;
    canvas.setColor(leading);
    canvas.fill(rect.x + 1.0f, rect.y + 1.0f, std::max(0.0f, rect.width - 2.0f), 1.0f);
    canvas.fill(rect.x + 1.0f, rect.y + 1.0f, 1.0f, std::max(0.0f, rect.height - 2.0f));
    canvas.setColor(trailing);
    canvas.fill(rect.x + 1.0f, rect.y + rect.height - 2.0f, std::max(0.0f, rect.width - 2.0f), 1.0f);
    canvas.fill(rect.x + rect.width - 2.0f, rect.y + 1.0f, 1.0f, std::max(0.0f, rect.height - 2.0f));
}

inline void drawBevel(visage::Canvas& canvas, const Rect& rect, const Palette& palette, const State& state)
{
    const BaseState baseState = resolveBaseState(state);
    Palette resolvedPalette = palette;
    if (baseState == BaseState::CheckedOrSelected) {
        resolvedPalette.fill = blend(palette.fill, palette.accent, 0.18f);
        resolvedPalette.hoverFill = blend(resolvedPalette.fill, palette.accent, 0.12f);
        resolvedPalette.pressedFill = blend(resolvedPalette.fill, 0xff000000, 0.20f);
    }
    drawBevel(canvas, rect, resolvedPalette,
        baseState == BaseState::Pressed,
        baseState == BaseState::Hovered,
        baseState != BaseState::Disabled);
    if (state.focused && state.enabled) {
        drawBorder(canvas, { rect.x - 1.0f, rect.y - 1.0f, rect.width + 2.0f, rect.height + 2.0f }, palette.accent);
    }
}

inline void drawRecessed(visage::Canvas& canvas, const Rect& rect, const Palette& palette,
    bool focused = false, bool enabled = true)
{
    fill(canvas, rect, enabled ? palette.recessedFill : blend(palette.recessedFill, palette.disabled, 0.55f));
    drawBorder(canvas, rect, focused && enabled ? palette.accent : palette.border);
    if (rect.width >= 3.0f && rect.height >= 3.0f) {
        canvas.setColor(palette.shadow);
        canvas.fill(rect.x + 1.0f, rect.y + 1.0f, std::max(0.0f, rect.width - 2.0f), 1.0f);
        canvas.fill(rect.x + 1.0f, rect.y + 1.0f, 1.0f, std::max(0.0f, rect.height - 2.0f));
        canvas.setColor(palette.highlight);
        canvas.fill(rect.x + 1.0f, rect.y + rect.height - 2.0f, std::max(0.0f, rect.width - 2.0f), 1.0f);
        canvas.fill(rect.x + rect.width - 2.0f, rect.y + 1.0f, 1.0f, std::max(0.0f, rect.height - 2.0f));
    }
}

inline void drawRecessed(visage::Canvas& canvas, const Rect& rect, const Palette& palette, const State& state)
{
    Palette resolvedPalette = palette;
    if (state.readOnly) {
        resolvedPalette.recessedFill = blend(resolvedPalette.recessedFill, resolvedPalette.disabled, 0.28f);
    }
    if (state.checkedOrSelected || state.active) {
        resolvedPalette.recessedFill = blend(resolvedPalette.recessedFill, resolvedPalette.accent, 0.14f);
    }
    drawRecessed(canvas, rect, resolvedPalette, false, state.enabled);
    if (state.focused && state.enabled) {
        drawBorder(canvas, { rect.x - 1.0f, rect.y - 1.0f, rect.width + 2.0f, rect.height + 2.0f }, palette.accent);
    }
}

inline void drawFocus(visage::Canvas& canvas, const Rect& rect, const Palette& palette)
{
    drawBorder(canvas, { rect.x - 1.0f, rect.y - 1.0f, rect.width + 2.0f, rect.height + 2.0f }, palette.accent);
}

inline int stateTextColor(const Palette& palette, bool enabled)
{
    return enabled ? palette.text : blend(palette.text, palette.disabled, 0.68f);
}

} // namespace visiform::ui::visual_style
