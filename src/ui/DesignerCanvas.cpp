#include "ui/DesignerCanvas.h"

#include "ui/DesignerCanvas.h"

#include "model/LookAndFeelRegistry.h"
#include "model/WidgetItemUtils.h"
#include "ui/WidgetMetrics.h"
#include "ui/resources/ImageResourceCache.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kPadding = 16.0f;
constexpr float kPreviewPadding = 18.0f;
constexpr float kTitleBarHeight = 28.0f;

struct PanelRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    [[nodiscard]] bool contains(float px, float py) const
    {
        return px >= x && py >= y && px <= x + width && py <= y + height;
    }

    [[nodiscard]] bool isValid() const
    {
        return width > 0.0f && height > 0.0f;
    }
};

void fillCircleApprox(visage::Canvas& canvas, float centerX, float centerY, float radius, int color);

void fillRoundedRect(visage::Canvas& canvas, float x, float y, float width, float height, float radius, int color)
{
    if (width <= 0.0f || height <= 0.0f) {
        return;
    }

    if (radius <= 1.0f) {
        canvas.setColor(color);
        canvas.fill(x, y, width, height);
        return;
    }

    const float r = std::min(radius, std::min(width, height) * 0.5f);
    canvas.setColor(color);
    // center rect
    canvas.fill(x + r, y, std::max(0.0f, width - r * 2.0f), height);
    // left and right rects
    canvas.fill(x, y + r, r, std::max(0.0f, height - r * 2.0f));
    canvas.fill(x + width - r, y + r, r, std::max(0.0f, height - r * 2.0f));
    // corner circles
    fillCircleApprox(canvas, x + r, y + r, r, color);
    fillCircleApprox(canvas, x + width - r, y + r, r, color);
    fillCircleApprox(canvas, x + r, y + height - r, r, color);
    fillCircleApprox(canvas, x + width - r, y + height - r, r, color);
}

void drawRoundedRectBorder(visage::Canvas& canvas, float x, float y, float width, float height, float radius, int borderColor, float thickness, int fillColor)
{
    if (thickness <= 0.0f) {
        return;
    }

    if (radius <= 1.0f) {
        // Fallback to rectangular border
        canvas.setColor(borderColor);
        canvas.fill(x, y, width, thickness);
        canvas.fill(x, y + height - thickness, width, thickness);
        canvas.fill(x, y, thickness, height);
        canvas.fill(x + width - thickness, y, thickness, height);
        return;
    }

    const float r = std::min(radius, std::min(width, height) * 0.5f);
    // draw edge bars
    canvas.setColor(borderColor);
    canvas.fill(x + r, y, std::max(0.0f, width - r * 2.0f), thickness); // top
    canvas.fill(x + r, y + height - thickness, std::max(0.0f, width - r * 2.0f), thickness); // bottom
    canvas.fill(x, y + r, thickness, std::max(0.0f, height - r * 2.0f)); // left
    canvas.fill(x + width - thickness, y + r, thickness, std::max(0.0f, height - r * 2.0f)); // right

    // draw corner caps as filled circles
    fillCircleApprox(canvas, x + r, y + r, r, borderColor);
    fillCircleApprox(canvas, x + width - r, y + r, r, borderColor);
    fillCircleApprox(canvas, x + r, y + height - r, r, borderColor);
    fillCircleApprox(canvas, x + width - r, y + height - r, r, borderColor);

    // carve inner area back out to create border thickness
    const float innerX = x + thickness;
    const float innerY = y + thickness;
    const float innerW = std::max(0.0f, width - thickness * 2.0f);
    const float innerH = std::max(0.0f, height - thickness * 2.0f);
    const float innerR = std::max(0.0f, r - thickness);
    fillRoundedRect(canvas, innerX, innerY, innerW, innerH, innerR, fillColor);
}

struct PreviewLayout {
    PanelRect preview{};
    PanelRect form{};
    float scale = 1.0f;
};

struct WidgetScreenInfo {
    const model::WidgetNode* widget = nullptr;
    PanelRect bounds{};
};

struct ResolvedWidgetStyle {
    int panelColor = 0xff1f242d;
    int fillColor = 0xff2b313d;
    int textColor = 0xffeef2f8;
    int borderColor = 0xff97a3b7;
    int accentColor = 0xff2d7ff9;
    int disabledColor = 0xff6c7788;
    float borderThickness = 1.0f;
    float cornerRadius = 0.0f;
    float fontSize = 16.0f;
};

struct GridColors {
    int minorLineColor = 0xff252a33;
    int majorLineColor = 0xff303744;
};

std::string imageWidgetPlaceholderText(const resources::ResolvedImageSource& resolvedSource,
    const resources::ImageResourceCache::CachedImageData* cachedImage = nullptr)
{
    if (resolvedSource.missingResource) {
        return "Missing image resource";
    }
    if (!resolvedSource.hasImage) {
        return "Image";
    }
    if (cachedImage == nullptr) {
        return resolvedSource.displayText.empty() ? std::string{ "Image" } : "Image: " + resolvedSource.displayText;
    }
    if (cachedImage->info.available && cachedImage->encodedBytes != nullptr && !cachedImage->encodedBytes->empty()) {
        return {};
    }
    if (cachedImage->info.error == "Image source file does not exist.") {
        return "Missing image file";
    }
    if (!cachedImage->info.error.empty()) {
        return "Image load failed";
    }
    return "Image";
}

PanelRect expandRect(const PanelRect& rect, float padding)
{
    return { rect.x - padding, rect.y - padding, rect.width + padding * 2.0f, rect.height + padding * 2.0f };
}

PanelRect selectionRectToScreenRect(const PreviewLayout& layout, const DesignerCanvas::SelectionRect& rect)
{
    return {
        layout.form.x + rect.x * layout.scale,
        layout.form.y + rect.y * layout.scale,
        rect.width * layout.scale,
        rect.height * layout.scale
    };
}

void drawBorder(visage::Canvas& canvas, const PanelRect& bounds, int color, float thickness = 1.0f)
{
    if (!bounds.isValid() || thickness <= 0.0f) {
        return;
    }

    canvas.setColor(color);
    canvas.fill(bounds.x, bounds.y, bounds.width, thickness);
    canvas.fill(bounds.x, bounds.y + bounds.height - thickness, bounds.width, thickness);
    canvas.fill(bounds.x, bounds.y, thickness, bounds.height);
    canvas.fill(bounds.x + bounds.width - thickness, bounds.y, thickness, bounds.height);
}

int blendColor(int colorA, int colorB, float amount)
{
    const auto blendChannel = [amount](int first, int second) {
        return static_cast<int>(std::round(static_cast<float>(first) * (1.0f - amount) + static_cast<float>(second) * amount));
    };

    const int a = blendChannel((colorA >> 24) & 0xff, (colorB >> 24) & 0xff);
    const int r = blendChannel((colorA >> 16) & 0xff, (colorB >> 16) & 0xff);
    const int g = blendChannel((colorA >> 8) & 0xff, (colorB >> 8) & 0xff);
    const int b = blendChannel(colorA & 0xff, colorB & 0xff);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

GridColors computeGridColors(int backgroundColor)
{
    const float luminance = 0.2126f * static_cast<float>((backgroundColor >> 16) & 0xff)
        + 0.7152f * static_cast<float>((backgroundColor >> 8) & 0xff)
        + 0.0722f * static_cast<float>(backgroundColor & 0xff);
    const int contrastTarget = luminance < 128.0f ? 0xffffffff : 0xff000000;

    GridColors colors;
    colors.minorLineColor = blendColor(backgroundColor, contrastTarget, 0.08f);
    colors.majorLineColor = blendColor(backgroundColor, contrastTarget, 0.18f);
    if (colors.majorLineColor == colors.minorLineColor) {
        colors.majorLineColor = blendColor(backgroundColor, contrastTarget, 0.28f);
    }

    return colors;
}

void fillCircleApprox(visage::Canvas& canvas, float centerX, float centerY, float radius, int color)
{
    if (radius <= 0.0f) {
        return;
    }

    canvas.setColor(color);
    const int radiusPixels = std::max(1, static_cast<int>(std::ceil(radius)));
    for (int offsetY = -radiusPixels; offsetY <= radiusPixels; ++offsetY) {
        const float dy = static_cast<float>(offsetY);
        const float halfWidth = std::sqrt(std::max(0.0f, radius * radius - dy * dy));
        canvas.fill(centerX - halfWidth, centerY + dy, halfWidth * 2.0f + 1.0f, 1.0f);
    }
}

std::optional<std::string> resolveFontPath(const std::string& family, bool bold, bool italic)
{
    std::string normalizedFamily = family;
    std::transform(normalizedFamily.begin(), normalizedFamily.end(), normalizedFamily.begin(),
        [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

    std::vector<std::string> candidates;
    if (normalizedFamily == "arial") {
        if (bold && italic) {
            candidates.push_back("C:/Windows/Fonts/arialbi.ttf");
        }
        if (bold) {
            candidates.push_back("C:/Windows/Fonts/arialbd.ttf");
        }
        if (italic) {
            candidates.push_back("C:/Windows/Fonts/ariali.ttf");
        }
        candidates.push_back("C:/Windows/Fonts/arial.ttf");
    }
    else if (normalizedFamily == "tahoma") {
        if (bold) {
            candidates.push_back("C:/Windows/Fonts/tahomabd.ttf");
        }
        candidates.push_back("C:/Windows/Fonts/tahoma.ttf");
    }
    else {
        if (bold && italic) {
            candidates.push_back("C:/Windows/Fonts/segoeuiz.ttf");
        }
        if (bold) {
            candidates.push_back("C:/Windows/Fonts/segoeuib.ttf");
        }
        if (italic) {
            candidates.push_back("C:/Windows/Fonts/segoeuii.ttf");
        }
        candidates.push_back("C:/Windows/Fonts/segoeui.ttf");
        candidates.push_back("C:/Windows/Fonts/tahoma.ttf");
        candidates.push_back("C:/Windows/Fonts/arial.ttf");
    }

    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }

    return std::nullopt;
}

float resolvedFontSize(const model::WidgetNode& widget, const ResolvedWidgetStyle& style)
{
    return std::clamp(widget.getFloatProperty("fontSize", style.fontSize), 8.0f, 72.0f);
}

const visage::Font& resolvedWidgetFont(const model::WidgetNode& widget,
    const ResolvedWidgetStyle& style,
    const visage::Font& fallback,
    visage::Font& fontStorage)
{
    const float fontSize = resolvedFontSize(widget, style);
    const std::string fontFamily = widget.getStringProperty("fontFamily", "Default");
    const bool fontBold = widget.getBoolProperty("fontBold", false);
    const bool fontItalic = widget.getBoolProperty("fontItalic", false);

    const bool useFallbackFont = (fontFamily.empty() || fontFamily == "Default")
        && !fontBold
        && !fontItalic
        && std::abs(fontSize - defaultDesignerFontSize()) < 0.01f;
    if (useFallbackFont) {
        return fallback;
    }

    const auto fontPath = resolveFontPath(fontFamily, fontBold, fontItalic);
    if (!fontPath.has_value()) {
        return fallback;
    }

    fontStorage = visage::Font(fontSize, *fontPath);
    return fontStorage.packedFont() != nullptr ? fontStorage : fallback;
}

std::string widgetLabel(const model::WidgetNode& widget)
{
    if (!widget.name.empty()) {
        return widget.name;
    }

    if (!widget.id.empty()) {
        return widget.id;
    }

    return widget.typeName();
}

std::optional<float> tryParseFloatText(const std::string& text)
{
    if (text.empty() || text == "<unset>") {
        return std::nullopt;
    }

    std::istringstream stream(text);
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

float getNumericProperty(const model::WidgetNode& widget, const std::string& key, float defaultValue)
{
    const auto* property = widget.getProperty(key);
    if (property == nullptr) {
        return defaultValue;
    }

    if (property->isFloat()) {
        return property->asFloat(defaultValue);
    }
    if (property->isInt()) {
        return static_cast<float>(property->asInt(static_cast<int>(defaultValue)));
    }
    if (property->isString()) {
        if (const auto parsedValue = tryParseFloatText(property->asString({}))) {
            return *parsedValue;
        }
    }

    return defaultValue;
}

std::string getStringProperty(const model::WidgetNode& widget, const std::string& key, const std::string& defaultValue)
{
    return widget.getStringProperty(key, defaultValue);
}

std::string getDisplayTextOrFallback(const model::WidgetNode& widget, const std::string& key, const std::string& fallback)
{
    const std::string value = getStringProperty(widget, key, {});
    return value.empty() ? fallback : value;
}

bool getBoolProperty(const model::WidgetNode& widget, const std::string& key, bool defaultValue)
{
    return widget.getBoolProperty(key, defaultValue);
}

std::vector<std::string> splitCommaSeparatedValues(const std::string& text)
{
    std::vector<std::string> values;
    std::istringstream stream(text);
    std::string item;
    while (std::getline(stream, item, ',')) {
        const auto first = std::find_if_not(item.begin(), item.end(), [](unsigned char character) {
            return std::isspace(character) != 0;
        });
        const auto last = std::find_if_not(item.rbegin(), item.rend(), [](unsigned char character) {
            return std::isspace(character) != 0;
        }).base();
        if (first < last) {
            values.emplace_back(first, last);
        }
    }

    if (values.empty()) {
        values.push_back("OK");
    }

    return values;
}

std::vector<std::string> tabLabels(const model::WidgetNode& widget)
{
    if (widget.type == model::WidgetType::TabControl && widget.tabPageCount() > 0) {
        std::vector<std::string> labels;
        labels.reserve(widget.tabPageCount());
        for (const auto& child : widget.children) {
            if (child.type == model::WidgetType::TabPage) {
                labels.push_back(child.tabTitle());
            }
        }
        if (!labels.empty()) {
            return labels;
        }
    }

    std::vector<std::string> labels;
    std::istringstream stream(getStringProperty(widget, "tabs", "Tab 1,Tab 2"));
    std::string item;
    while (std::getline(stream, item, ',')) {
        const auto first = std::find_if_not(item.begin(), item.end(), [](unsigned char character) {
            return std::isspace(character) != 0;
        });
        const auto last = std::find_if_not(item.rbegin(), item.rend(), [](unsigned char character) {
            return std::isspace(character) != 0;
        }).base();
        if (first < last) {
            labels.emplace_back(first, last);
        }
    }

    if (labels.empty()) {
        labels.push_back("Tab 1");
    }

    return labels;
}

int selectedTabIndex(const model::WidgetNode& widget)
{
    if (widget.type == model::WidgetType::TabControl && widget.tabPageCount() > 0) {
        return widget.selectedTabIndex();
    }

    const std::vector<std::string> labels = tabLabels(widget);
    return std::clamp(widget.getIntProperty("selectedTabIndex", widget.getIntProperty("selectedTab", 0)), 0, std::max(0, static_cast<int>(labels.size()) - 1));
}

bool isChildVisibleInParent(const model::WidgetNode& parent, const model::WidgetNode& child)
{
    if (parent.type != model::WidgetType::TabControl) {
        return true;
    }

    if (parent.tabPageCount() > 0) {
        const auto* selectedPage = parent.tabPageAt(parent.selectedTabIndex());
        return child.type == model::WidgetType::TabPage && selectedPage != nullptr && child.id == selectedPage->id;
    }

    return child.getIntProperty("tabIndex", 0) == selectedTabIndex(parent);
}

float normalizedSliderValue(const model::WidgetNode& widget)
{
    const float minimum = getNumericProperty(widget, "min", 0.0f);
    const float maximum = getNumericProperty(widget, "max", 100.0f);
    const float value = getNumericProperty(widget, "value", 50.0f);
    if (maximum <= minimum) {
        return 0.5f;
    }

    const float clampedValue = std::clamp(value, minimum, maximum);
    return std::clamp((clampedValue - minimum) / (maximum - minimum), 0.0f, 1.0f);
}

float normalizedRangeValue(const model::WidgetNode& widget, const std::string& valueKey = "value")
{
    const float minimum = getNumericProperty(widget, "min", 0.0f);
    const float maximum = getNumericProperty(widget, "max", 100.0f);
    const float value = getNumericProperty(widget, valueKey, minimum);
    if (maximum <= minimum) {
        return 0.0f;
    }

    return std::clamp((value - minimum) / (maximum - minimum), 0.0f, 1.0f);
}

std::string progressBarDisplayText(const model::WidgetNode& widget)
{
    if (!getBoolProperty(widget, "showText", true)) {
        return {};
    }

    const std::string explicitText = getDisplayTextOrFallback(widget, "text", {});
    if (!explicitText.empty()) {
        return explicitText;
    }

    const float minimum = getNumericProperty(widget, "min", 0.0f);
    const float maximum = std::max(minimum, getNumericProperty(widget, "max", 100.0f));
    const float safeMaximum = maximum <= minimum ? minimum + 1.0f : maximum;
    const float value = std::clamp(getNumericProperty(widget, "value", minimum), minimum, safeMaximum);
    const float normalized = safeMaximum <= minimum ? 0.0f : std::clamp((value - minimum) / (safeMaximum - minimum), 0.0f, 1.0f);
    return std::to_string(static_cast<int>(std::round(normalized * 100.0f))) + "%";
}

int parseColorOrDefault(const std::string& value, int defaultColor)
{
    if (value.empty() || value.front() != '#') {
        return defaultColor;
    }

    try {
        const std::string digits = value.substr(1);
        const std::uint32_t parsed = static_cast<std::uint32_t>(std::stoul(digits, nullptr, 16));
        if (digits.size() == 6) {
            return static_cast<int>(0xff000000u | parsed);
        }
        if (digits.size() == 8) {
            return static_cast<int>(parsed);
        }
    }
    catch (...) {
    }

    return defaultColor;
}

ResolvedWidgetStyle resolveWidgetStyle(const model::ProjectDocument& document, const model::WidgetNode& widget)
{
    const model::LookAndFeelRegistry& registry = model::LookAndFeelRegistry::instance();
    const std::string widgetLookAndFeelId = widget.getStringProperty("lookAndFeelId", {});
    const model::LookAndFeelDefinition* definition = !widgetLookAndFeelId.empty()
        ? registry.findById(widgetLookAndFeelId)
        : registry.findById(document.lookAndFeelId);
    if (definition == nullptr) {
        definition = &registry.defaultDefinition();
    }

    ResolvedWidgetStyle style;
    style.panelColor = parseColorOrDefault(definition->panelColor, style.panelColor);
    style.fillColor = parseColorOrDefault(definition->controlFillColor, style.fillColor);
    style.textColor = parseColorOrDefault(definition->controlTextColor, style.textColor);
    style.borderColor = parseColorOrDefault(definition->controlBorderColor, style.borderColor);
    style.accentColor = parseColorOrDefault(definition->accentColor, style.accentColor);
    style.disabledColor = parseColorOrDefault(definition->disabledColor, style.disabledColor);
    style.borderThickness = std::clamp(widget.getFloatProperty("borderThickness", definition->borderThickness), 0.0f, 20.0f);
    style.cornerRadius = std::clamp(widget.getFloatProperty("cornerRadius", definition->cornerRadius), 0.0f, 50.0f);
    style.fontSize = std::clamp(widget.getFloatProperty("fontSize", definition->fontSize), 8.0f, 72.0f);

    style.fillColor = parseColorOrDefault(widget.getStringProperty("fillColor", {}), style.fillColor);
    style.textColor = parseColorOrDefault(widget.getStringProperty("textColor", {}), style.textColor);
    style.borderColor = parseColorOrDefault(widget.getStringProperty("borderColor", {}), style.borderColor);
    style.accentColor = parseColorOrDefault(widget.getStringProperty("accentColor", {}), style.accentColor);

    if (widget.type == model::WidgetType::FormWindow
        || widget.type == model::WidgetType::Frame
        || widget.type == model::WidgetType::GroupBox
        || widget.type == model::WidgetType::Panel) {
        style.fillColor = parseColorOrDefault(widget.getStringProperty("backgroundColor", {}),
            widget.type == model::WidgetType::FormWindow ? style.panelColor : style.fillColor);
    }

    return style;
}

PreviewLayout calculatePreviewLayout(float x, float y, float width, float height, const model::ProjectDocument& document)
{
    PreviewLayout layout;
    layout.preview = {
        x + kPadding,
        y + kHeaderHeight + 12.0f,
        std::max(0.0f, width - kPadding * 2.0f),
        std::max(0.0f, height - (kHeaderHeight + 24.0f))
    };

    if (!layout.preview.isValid() || !document.root.bounds.isValid()) {
        return layout;
    }

    const float availableWidth = std::max(0.0f, layout.preview.width - kPreviewPadding * 2.0f);
    const float availableHeight = std::max(0.0f, layout.preview.height - kPreviewPadding * 2.0f);
    if (availableWidth <= 0.0f || availableHeight <= 0.0f) {
        return layout;
    }

    layout.scale = std::min(availableWidth / document.root.bounds.width, availableHeight / document.root.bounds.height);
    if (layout.scale <= 0.0f) {
        return layout;
    }

    const float formWidth = document.root.bounds.width * layout.scale;
    const float formHeight = document.root.bounds.height * layout.scale;
    layout.form = {
        layout.preview.x + (layout.preview.width - formWidth) * 0.5f,
        layout.preview.y + (layout.preview.height - formHeight) * 0.5f,
        formWidth,
        formHeight
    };
    return layout;
}

void drawSelectionOutline(visage::Canvas& canvas, const PanelRect& bounds)
{
    drawBorder(canvas, { bounds.x - 2.0f, bounds.y - 2.0f, bounds.width + 4.0f, bounds.height + 4.0f }, 0xff2d7ff9, 1.5f);
}

void drawSecondarySelectionOutline(visage::Canvas& canvas, const PanelRect& bounds)
{
    drawBorder(canvas, { bounds.x - 1.0f, bounds.y - 1.0f, bounds.width + 2.0f, bounds.height + 2.0f }, 0xffd9473f, 1.25f);
}

PanelRect handleRect(const PanelRect& bounds, DesignerCanvas::HitRegion region, float handleSize)
{
    const float halfHandle = handleSize * 0.5f;
    switch (region) {
    case DesignerCanvas::HitRegion::TopLeftHandle:
        return { bounds.x - halfHandle, bounds.y - halfHandle, handleSize, handleSize };
    case DesignerCanvas::HitRegion::TopRightHandle:
        return { bounds.x + bounds.width - halfHandle, bounds.y - halfHandle, handleSize, handleSize };
    case DesignerCanvas::HitRegion::BottomLeftHandle:
        return { bounds.x - halfHandle, bounds.y + bounds.height - halfHandle, handleSize, handleSize };
    case DesignerCanvas::HitRegion::BottomRightHandle:
        return { bounds.x + bounds.width - halfHandle, bounds.y + bounds.height - halfHandle, handleSize, handleSize };
    case DesignerCanvas::HitRegion::None:
    case DesignerCanvas::HitRegion::Body:
        return {};
    }

    return {};
}

DesignerCanvas::HitRegion hitHandle(const PanelRect& bounds, float x, float y, float handleSize)
{
    constexpr std::array<DesignerCanvas::HitRegion, 4> kHandles = {
        DesignerCanvas::HitRegion::TopLeftHandle,
        DesignerCanvas::HitRegion::TopRightHandle,
        DesignerCanvas::HitRegion::BottomLeftHandle,
        DesignerCanvas::HitRegion::BottomRightHandle
    };

    for (DesignerCanvas::HitRegion handle : kHandles) {
        if (handleRect(bounds, handle, handleSize).contains(x, y)) {
            return handle;
        }
    }

    return DesignerCanvas::HitRegion::None;
}

void drawSelectionHandles(visage::Canvas& canvas, const PanelRect& bounds, float visualHandleSize)
{
    constexpr std::array<DesignerCanvas::HitRegion, 4> kHandles = {
        DesignerCanvas::HitRegion::TopLeftHandle,
        DesignerCanvas::HitRegion::TopRightHandle,
        DesignerCanvas::HitRegion::BottomLeftHandle,
        DesignerCanvas::HitRegion::BottomRightHandle
    };

    for (DesignerCanvas::HitRegion handle : kHandles) {
        const PanelRect handleBounds = handleRect(bounds, handle, visualHandleSize);
        canvas.setColor(0xffffffff);
        canvas.fill(handleBounds.x, handleBounds.y, handleBounds.width, handleBounds.height);
        drawBorder(canvas, handleBounds, 0xff2d7ff9);
    }
}

std::optional<std::string> hitTestWidgetScreenId(const model::WidgetNode& widget,
    float formScreenX,
    float formScreenY,
    float parentLocalX,
    float parentLocalY,
    float scale,
    float x,
    float y,
    float smallWidgetHitPadding)
{
    const float widgetLocalX = parentLocalX + widget.bounds.x;
    const float widgetLocalY = parentLocalY + widget.bounds.y;
    const PanelRect bounds{
        formScreenX + widgetLocalX * scale,
        formScreenY + widgetLocalY * scale,
        std::max(1.0f, widget.bounds.width * scale),
        std::max(1.0f, widget.bounds.height * scale)
    };

    // Z-order convention:
    // - children[0] is backmost
    // - children.back() is frontmost
    // Hit testing walks children from front to back so the topmost overlap wins.
    for (auto iterator = widget.children.rbegin(); iterator != widget.children.rend(); ++iterator) {
        if (!isChildVisibleInParent(widget, *iterator)) {
            continue;
        }
        if (auto match = hitTestWidgetScreenId(*iterator, formScreenX, formScreenY, widgetLocalX, widgetLocalY,
                scale, x, y, smallWidgetHitPadding)) {
            if (widget.type == model::WidgetType::TabControl
                && iterator->type == model::WidgetType::TabPage
                && *match == iterator->id) {
                continue;
            }
            return match;
        }
    }

    PanelRect hitBounds = bounds;
    if (bounds.width < 14.0f || bounds.height < 14.0f) {
        hitBounds = expandRect(bounds, smallWidgetHitPadding);
    }

    if (hitBounds.contains(x, y)) {
        return widget.id;
    }

    return std::nullopt;
}

void drawGrid(visage::Canvas& canvas, const PanelRect& bounds, float scale, bool showMinorGrid, int gridSize, int majorGridSize, int backgroundColor)
{
    const float scaledGridSize = gridSize * scale;
    if (scaledGridSize < 4.0f) {
        return;
    }

    const float scaledMajorGridSize = majorGridSize * scale;

    const float contentTop = bounds.y + std::min(kTitleBarHeight, bounds.height);
    const float contentHeight = bounds.height - std::min(kTitleBarHeight, bounds.height);
    if (contentHeight <= 0.0f) {
        return;
    }

    const GridColors gridColors = computeGridColors(backgroundColor);

    if (showMinorGrid) {
        canvas.setColor(gridColors.minorLineColor);
        for (float x = bounds.x + scaledGridSize; x < bounds.x + bounds.width; x += scaledGridSize) {
            canvas.fill(x, contentTop, 1.0f, contentHeight);
        }
        for (float y = contentTop + scaledGridSize; y < bounds.y + bounds.height; y += scaledGridSize) {
            canvas.fill(bounds.x, y, bounds.width, 1.0f);
        }
    }

    if (scaledMajorGridSize >= scaledGridSize + 1.0f) {
        canvas.setColor(gridColors.majorLineColor);
        for (float x = bounds.x + scaledMajorGridSize; x < bounds.x + bounds.width; x += scaledMajorGridSize) {
            canvas.fill(x, contentTop, 1.0f, contentHeight);
        }
        for (float y = contentTop + scaledMajorGridSize; y < bounds.y + bounds.height; y += scaledMajorGridSize) {
            canvas.fill(bounds.x, y, bounds.width, 1.0f);
        }
    }
}

std::optional<WidgetScreenInfo> findWidgetScreenInfo(const model::WidgetNode& widget,
    const std::string& widgetId,
    float formScreenX,
    float formScreenY,
    float parentLocalX,
    float parentLocalY,
    float scale)
{
    const float widgetLocalX = parentLocalX + widget.bounds.x;
    const float widgetLocalY = parentLocalY + widget.bounds.y;
    const PanelRect bounds{
        formScreenX + widgetLocalX * scale,
        formScreenY + widgetLocalY * scale,
        std::max(1.0f, widget.bounds.width * scale),
        std::max(1.0f, widget.bounds.height * scale)
    };

    if (widget.id == widgetId) {
        return WidgetScreenInfo{ &widget, bounds };
    }

    for (const auto& child : widget.children) {
        if (!isChildVisibleInParent(widget, child)) {
            continue;
        }
        if (auto match = findWidgetScreenInfo(child, widgetId, formScreenX, formScreenY, widgetLocalX, widgetLocalY, scale)) {
            return match;
        }
    }

    return std::nullopt;
}

void drawWidget(visage::Canvas& canvas,
    const visage::Font& font,
    bool drawText,
    const model::ProjectDocument& document,
    resources::ImageResourceCache* imageCache,
    bool simplifySelectedImages,
    const model::WidgetNode& widget,
    float formScreenX,
    float formScreenY,
    float parentLocalX,
    float parentLocalY,
    float scale,
    const std::string& selectedWidgetId,
    float visualHandleSize,
    bool showGrid,
    bool showMinorGrid,
    int gridSize,
    int majorGridSize)
{
    const float widgetLocalX = parentLocalX + widget.bounds.x;
    const float widgetLocalY = parentLocalY + widget.bounds.y;
    const PanelRect bounds{
        formScreenX + widgetLocalX * scale,
        formScreenY + widgetLocalY * scale,
        std::max(1.0f, widget.bounds.width * scale),
        std::max(1.0f, widget.bounds.height * scale)
    };
    const ResolvedWidgetStyle style = resolveWidgetStyle(document, widget);
    visage::Font widgetFontStorage{};
    const visage::Font& widgetFont = resolvedWidgetFont(widget, style, font, widgetFontStorage);
    const float fontSize = resolvedFontSize(widget, style);

    switch (widget.type) {
    case model::WidgetType::FormWindow: {
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        if (showGrid) {
            drawGrid(canvas, bounds, scale, showMinorGrid, gridSize, majorGridSize, style.fillColor);
        }
        canvas.setColor(style.panelColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, std::min(kTitleBarHeight, bounds.height));
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);

        if (drawText) {
            canvas.setColor(style.textColor);
            canvas.text(getStringProperty(widget, "title", widgetLabel(widget)), widgetFont, visage::Font::kTopLeft,
                bounds.x + 10.0f, bounds.y + 4.0f, std::max(0.0f, bounds.width - 20.0f), 22.0f);
        }
        break;
    }
    case model::WidgetType::MenuBar: {
        const auto items = model::splitItems(getStringProperty(widget, "items", {}));
        const std::string selectedIndexKey = std::string(model::selectedItemIndexPropertyKey(widget.type));
        const int selectedIndex = model::sanitizeSelectedIndex(items,
            widget.getIntProperty(selectedIndexKey, items.empty() ? -1 : 0));
        const float itemHeight = std::max(0.0f, bounds.height - 8.0f);
        float itemLeft = bounds.x + 6.0f;
        canvas.setColor(blendColor(style.panelColor, style.fillColor, 0.35f));
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);
        for (std::size_t index = 0; index < items.size() && itemLeft < bounds.x + bounds.width - 6.0f; ++index) {
            const std::string& item = items[index];
            const float idealWidth = std::max(56.0f, estimateDesignerTextWidth(item, fontSize) + 12.0f);
            const float itemWidth = std::min(idealWidth, std::max(0.0f, bounds.x + bounds.width - 6.0f - itemLeft));
            const bool selected = static_cast<int>(index) == selectedIndex;
            if (selected) {
                canvas.setColor(blendColor(style.accentColor, style.fillColor, 0.32f));
                canvas.fill(itemLeft, bounds.y + 4.0f, itemWidth, itemHeight);
            }
            if (drawText) {
                canvas.setColor(style.textColor);
                canvas.text(item, widgetFont, visage::Font::kCenter,
                    itemLeft + 4.0f, bounds.y + 4.0f, std::max(0.0f, itemWidth - 8.0f), itemHeight);
            }
            itemLeft += itemWidth + 4.0f;
        }
        if (items.empty() && drawText) {
            canvas.setColor(style.textColor);
            canvas.text("<empty>", widgetFont, visage::Font::kCenter, bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }
    case model::WidgetType::ToolBar: {
        const auto items = model::splitItems(getStringProperty(widget, "items", {}));
        const std::string selectedIndexKey = std::string(model::selectedItemIndexPropertyKey(widget.type));
        const int selectedIndex = model::sanitizeSelectedIndex(items,
            widget.getIntProperty(selectedIndexKey, items.empty() ? -1 : 0));
        const float itemHeight = std::max(0.0f, bounds.height - 10.0f);
        float itemLeft = bounds.x + 6.0f;
        canvas.setColor(blendColor(style.panelColor, style.fillColor, 0.22f));
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);
        for (std::size_t index = 0; index < items.size() && itemLeft < bounds.x + bounds.width - 6.0f; ++index) {
            const std::string& item = items[index];
            const float idealWidth = std::max(64.0f, estimateDesignerTextWidth(item, fontSize) + 18.0f);
            const float itemWidth = std::min(idealWidth, std::max(0.0f, bounds.x + bounds.width - 6.0f - itemLeft));
            const bool selected = static_cast<int>(index) == selectedIndex;
            canvas.setColor(selected ? blendColor(style.accentColor, style.fillColor, 0.34f) : style.fillColor);
            canvas.fill(itemLeft, bounds.y + 5.0f, itemWidth, itemHeight);
            drawBorder(canvas, { itemLeft, bounds.y + 5.0f, itemWidth, itemHeight }, style.borderColor, style.borderThickness);
            if (drawText) {
                canvas.setColor(style.textColor);
                canvas.text(item, widgetFont, visage::Font::kCenter,
                    itemLeft + 4.0f, bounds.y + 5.0f, std::max(0.0f, itemWidth - 8.0f), itemHeight);
            }
            itemLeft += itemWidth + 6.0f;
        }
        if (items.empty() && drawText) {
            canvas.setColor(style.textColor);
            canvas.text("<empty>", widgetFont, visage::Font::kCenter, bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }
    case model::WidgetType::StatusBar: {
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);

        int fields = static_cast<int>(getNumericProperty(widget, "fields", 1.0f));
        fields = std::clamp(fields, 1, 4);
        const float fieldWidth = bounds.width / static_cast<float>(fields);
        const float fieldInset = std::min(10.0f, std::max(6.0f, bounds.height * 0.16f));
        const float textTop = centeredTextTop(bounds.y + 1.0f, std::max(0.0f, bounds.height - 2.0f), fontSize);
        const float textHeight = std::max(0.0f, bounds.height - 4.0f);
        for (int i = 0; i < fields; ++i) {
            const float fx = bounds.x + fieldWidth * static_cast<float>(i);
            const float fw = fieldWidth;
            if (drawText) {
                const std::string key = std::string("text") + std::to_string(i);
                const std::string text = getDisplayTextOrFallback(widget, key, i == 0 ? "Ready" : "");
                canvas.setColor(style.textColor);
                canvas.text(text, widgetFont, visage::Font::kTopLeft,
                    fx + fieldInset, textTop, std::max(0.0f, fw - fieldInset * 2.0f), textHeight);
            }
            if (i + 1 < fields) {
                canvas.setColor(style.borderColor);
                canvas.fill(fx + fw - 1.0f, bounds.y + fieldInset * 0.5f, 1.0f, std::max(0.0f, bounds.height - fieldInset));
            }
        }
        break;
    }
    case model::WidgetType::ProgressBar: {
        // Draw a bordered progress bar with fill based on min/max/value
        const float normalized = normalizedRangeValue(widget, "value");
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);

        const float fillWidth = std::max(0.0f, bounds.width * normalized);
        canvas.setColor(style.accentColor);
        canvas.fill(bounds.x, bounds.y, fillWidth, bounds.height);

        const std::string text = progressBarDisplayText(widget);
        if (drawText && !text.empty()) {
            canvas.setColor(normalized >= 0.5f ? 0xfff8fbff : style.textColor);
            canvas.text(text, widgetFont, visage::Font::kCenter, bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }
    case model::WidgetType::ColorPicker: {
        const std::string colorValue = getStringProperty(widget, "value", "#2D7DFF");
        const bool showText = getBoolProperty(widget, "showText", true);
        const float swatchSize = std::max(16.0f, bounds.height - 12.0f);
        const float swatchX = bounds.x + 6.0f;
        const float swatchY = bounds.y + (bounds.height - swatchSize) * 0.5f;
        const float textX = swatchX + swatchSize + 10.0f;
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);
        canvas.setColor(parseColorOrDefault(colorValue, style.accentColor));
        canvas.fill(swatchX, swatchY, swatchSize, swatchSize);
        drawBorder(canvas, { swatchX, swatchY, swatchSize, swatchSize }, style.borderColor, style.borderThickness);
        if (drawText) {
            canvas.setColor(style.textColor);
            const std::string label = showText ? getDisplayTextOrFallback(widget, "text", "Color") : colorValue;
            const std::string text = showText ? label + "  " + colorValue : colorValue;
            canvas.text(text, font, visage::Font::kTopLeft,
                textX, bounds.y + bounds.height * 0.5f - 10.0f,
                std::max(0.0f, bounds.width - (textX - bounds.x) - 8.0f), std::max(0.0f, bounds.height - 8.0f));
        }
        break;
    }
    case model::WidgetType::ModalDialog: {
        canvas.setColor(0xff0f1318);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);

        const PanelRect dialogBounds{
            bounds.x + std::max(10.0f, bounds.width * 0.08f),
            bounds.y + std::max(10.0f, bounds.height * 0.08f),
            std::max(120.0f, bounds.width - std::max(20.0f, bounds.width * 0.16f)),
            std::max(80.0f, bounds.height - std::max(20.0f, bounds.height * 0.16f))
        };

        canvas.setColor(style.fillColor);
        canvas.fill(dialogBounds.x, dialogBounds.y, dialogBounds.width, dialogBounds.height);
        canvas.setColor(style.panelColor);
        canvas.fill(dialogBounds.x, dialogBounds.y, dialogBounds.width, std::min(28.0f, dialogBounds.height));
        drawBorder(canvas, dialogBounds, style.borderColor, style.borderThickness);

        if (drawText) {
            canvas.setColor(style.textColor);
            canvas.text(getStringProperty(widget, "title", "Dialog"), widgetFont, visage::Font::kTopLeft,
                dialogBounds.x + 8.0f, dialogBounds.y + 4.0f, std::max(0.0f, dialogBounds.width - 16.0f), 20.0f);
            canvas.text(getStringProperty(widget, "message", "Message text"), widgetFont, visage::Font::kTopLeft,
                dialogBounds.x + 10.0f, dialogBounds.y + 40.0f, std::max(0.0f, dialogBounds.width - 20.0f), std::max(20.0f, dialogBounds.height - 90.0f));
        }

        const auto buttons = splitCommaSeparatedValues(getStringProperty(widget, "buttons", "OK"));
        const float buttonWidth = std::max(56.0f, std::min(96.0f, dialogBounds.width * 0.22f));
        const float buttonHeight = 24.0f;
        const float buttonSpacing = 8.0f;
        const float totalButtonWidth = static_cast<float>(buttons.size()) * buttonWidth
            + static_cast<float>(std::max<std::size_t>(0, buttons.size() - 1)) * buttonSpacing;
        float buttonX = dialogBounds.x + std::max(8.0f, (dialogBounds.width - totalButtonWidth) * 0.5f);
        const float buttonY = dialogBounds.y + dialogBounds.height - buttonHeight - 10.0f;
        for (const auto& button : buttons) {
            canvas.setColor(style.accentColor);
            canvas.fill(buttonX, buttonY, buttonWidth, buttonHeight);
            drawBorder(canvas, { buttonX, buttonY, buttonWidth, buttonHeight }, style.borderColor, style.borderThickness);
            if (drawText) {
                canvas.setColor(style.textColor);
                canvas.text(button, widgetFont, visage::Font::kCenter, buttonX, buttonY, buttonWidth, buttonHeight);
            }
            buttonX += buttonWidth + buttonSpacing;
        }
        break;
    }
    case model::WidgetType::Frame:
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);
        if (drawText) {
            canvas.setColor(style.textColor);
            canvas.text(getStringProperty(widget, "title", widgetLabel(widget)), widgetFont, visage::Font::kTopLeft,
                bounds.x + 8.0f, bounds.y + 6.0f, std::max(0.0f, bounds.width - 16.0f), 20.0f);
        }
        break;
    case model::WidgetType::GroupBox: {
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y + 10.0f, bounds.width, std::max(0.0f, bounds.height - 10.0f));
        drawBorder(canvas, { bounds.x, bounds.y + 10.0f, bounds.width, std::max(0.0f, bounds.height - 10.0f) }, style.borderColor, style.borderThickness);
        if (drawText) {
            const std::string title = getStringProperty(widget, "title", "Group");
            const float titleWidth = std::min(bounds.width - 20.0f, std::max(48.0f, estimateDesignerTextWidth(title, fontSize)));
            canvas.setColor(style.fillColor);
            canvas.fill(bounds.x + 12.0f, bounds.y, titleWidth + 12.0f, 20.0f);
            canvas.setColor(style.textColor);
            canvas.text(title, widgetFont, visage::Font::kTopLeft,
                bounds.x + 18.0f, bounds.y + 1.0f, std::max(0.0f, bounds.width - 28.0f), 18.0f);
        }
        break;
    }
    case model::WidgetType::Panel:
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);
        break;
    case model::WidgetType::TabControl: {
        const std::vector<std::string> labels = tabLabels(widget);
        const int selectedTab = selectedTabIndex(widget);
        const float headerHeight = std::min(32.0f, std::max(24.0f, bounds.height * 0.18f));
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        canvas.setColor(style.panelColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, headerHeight);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);
        const float tabWidth = bounds.width / static_cast<float>(std::max<std::size_t>(1, labels.size()));
        for (std::size_t index = 0; index < labels.size(); ++index) {
            const PanelRect tabBounds{
                bounds.x + tabWidth * static_cast<float>(index),
                bounds.y,
                tabWidth,
                headerHeight
            };
            canvas.setColor(static_cast<int>(index) == selectedTab ? style.fillColor : blendColor(style.panelColor, style.fillColor, 0.22f));
            canvas.fill(tabBounds.x, tabBounds.y, tabBounds.width, tabBounds.height);
            drawBorder(canvas, tabBounds, style.borderColor, 1.0f);
            if (drawText) {
                canvas.setColor(style.textColor);
                canvas.text(labels[index], widgetFont, visage::Font::kCenter,
                    tabBounds.x, tabBounds.y, tabBounds.width, tabBounds.height);
            }
        }
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x + 1.0f, bounds.y + headerHeight, std::max(0.0f, bounds.width - 2.0f), std::max(0.0f, bounds.height - headerHeight - 1.0f));
        break;
    }
    case model::WidgetType::TabPage:
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, blendColor(style.borderColor, style.fillColor, 0.35f), 1.0f);
        break;
    case model::WidgetType::Label:
        if (drawText) {
            canvas.setColor(style.textColor);
            canvas.text(getDisplayTextOrFallback(widget, "text", "Label"), widgetFont, visage::Font::kTopLeft,
                bounds.x + 6.0f, centeredTextTop(bounds.y, bounds.height, fontSize),
                std::max(0.0f, bounds.width - 12.0f), std::max(0.0f, bounds.height - 8.0f));
        }
        break;
    case model::WidgetType::Button:
    {
        const bool pressedState = widget.getBoolProperty("toggleMode", false) && widget.getBoolProperty("checked", false);
        const std::string text = getStringProperty(widget, "text", {});
        const std::string configuredNormalText = getStringProperty(widget, "normalText", {});
        const std::string configuredPressedText = getStringProperty(widget, "pressedText", {});
        const std::string normalText = !configuredNormalText.empty()
            ? configuredNormalText
            : (!text.empty() ? text : std::string{ "Button" });
        const std::string pressedText = !configuredPressedText.empty() ? configuredPressedText : normalText;
        const int normalFillColor = parseColorOrDefault(getStringProperty(widget, "normalFillColor", {}), style.fillColor);
        const int pressedFillColor = parseColorOrDefault(getStringProperty(widget, "pressedFillColor", {}), blendColor(style.fillColor, style.accentColor, 0.18f));
        const float radius = std::clamp(style.cornerRadius, 0.0f, std::min(bounds.width, bounds.height) * 0.5f);
        canvas.setColor(pressedState ? pressedFillColor : normalFillColor);
        fillRoundedRect(canvas, bounds.x, bounds.y, bounds.width, bounds.height, radius, pressedState ? pressedFillColor : normalFillColor);
        drawRoundedRectBorder(canvas, bounds.x, bounds.y, bounds.width, bounds.height, radius, style.borderColor, style.borderThickness, pressedState ? pressedFillColor : normalFillColor);
        if (drawText) {
            canvas.setColor(style.textColor);
            canvas.text(pressedState ? pressedText : normalText, widgetFont, visage::Font::kCenter,
                bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }
    case model::WidgetType::TextBox:
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);
        if (drawText) {
            canvas.setColor(style.textColor);
            canvas.text(getStringProperty(widget, "text", ""), widgetFont, visage::Font::kTopLeft,
                bounds.x + 8.0f, centeredTextTop(bounds.y, bounds.height, fontSize),
                std::max(0.0f, bounds.width - 16.0f), std::max(0.0f, bounds.height - 8.0f));
        }
        break;
    case model::WidgetType::ComboBox: {
        const auto items = model::splitItems(getStringProperty(widget, "items", {}));
        const int selectedIndex = model::sanitizeSelectedIndex(items, widget.getIntProperty("selectedIndex", items.empty() ? -1 : 0));
        const std::string selectedText = model::getSelectedItemText(items, selectedIndex);
        const float arrowWidth = std::min(26.0f, std::max(20.0f, bounds.width * 0.18f));
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);
        canvas.setColor(blendColor(style.panelColor, style.fillColor, 0.22f));
        canvas.fill(bounds.x + bounds.width - arrowWidth, bounds.y, arrowWidth, bounds.height);
        drawBorder(canvas, { bounds.x + bounds.width - arrowWidth, bounds.y, arrowWidth, bounds.height }, style.borderColor, style.borderThickness);
        canvas.setColor(style.borderColor);
        canvas.fill(bounds.x + bounds.width - arrowWidth * 0.5f - 4.0f, bounds.y + bounds.height * 0.5f - 1.0f, 8.0f, 2.0f);
        if (drawText) {
            canvas.setColor(style.textColor);
            canvas.text(selectedText.empty() ? std::string{ "<empty>" } : selectedText, widgetFont, visage::Font::kTopLeft,
                bounds.x + 8.0f, centeredTextTop(bounds.y, bounds.height, fontSize),
                std::max(0.0f, bounds.width - arrowWidth - 14.0f), std::max(0.0f, bounds.height - 8.0f));
        }
        break;
    }
    case model::WidgetType::ListBox: {
        const auto items = model::splitItems(getStringProperty(widget, "items", {}));
        const int selectedIndex = model::sanitizeSelectedIndex(items, widget.getIntProperty("selectedIndex", items.empty() ? -1 : 0));
        const float rowHeight = std::max(18.0f, fontSize * 1.5f);
        const float listTop = bounds.y + 4.0f;
        const float visibleHeight = std::max(0.0f, bounds.height - 8.0f);
        const std::size_t visibleCount = std::max<std::size_t>(1, static_cast<std::size_t>(std::floor(visibleHeight / rowHeight)));
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);
        float rowTop = listTop;
        for (std::size_t index = 0; index < std::min<std::size_t>(visibleCount, items.size()); ++index) {
            const bool selected = static_cast<int>(index) == selectedIndex;
            canvas.setColor(selected ? blendColor(style.accentColor, style.fillColor, 0.32f) : (index % 2 == 0 ? style.fillColor : blendColor(style.panelColor, style.fillColor, 0.18f)));
            canvas.fill(bounds.x + 4.0f, rowTop, std::max(0.0f, bounds.width - 14.0f), rowHeight - 1.0f);
            if (drawText) {
                canvas.setColor(style.textColor);
                canvas.text(items[index], widgetFont, visage::Font::kTopLeft,
                    bounds.x + 10.0f, rowTop + std::max(2.0f, (rowHeight - fontSize * 1.4f) * 0.5f),
                    std::max(0.0f, bounds.width - 22.0f), std::max(0.0f, rowHeight - 4.0f));
            }
            rowTop += rowHeight;
        }
        if (items.size() > visibleCount) {
            canvas.setColor(style.panelColor);
            canvas.fill(bounds.x + bounds.width - 8.0f, bounds.y + 4.0f, 4.0f, std::max(0.0f, bounds.height - 8.0f));
            canvas.setColor(style.accentColor);
            canvas.fill(bounds.x + bounds.width - 8.0f, bounds.y + 10.0f, 4.0f, std::max(16.0f, bounds.height * 0.22f));
        }
        if (items.empty() && drawText) {
            canvas.setColor(style.textColor);
            canvas.text("<empty>", widgetFont, visage::Font::kCenter,
                bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }
    case model::WidgetType::TableGrid: {
        const auto columns = model::splitTableColumns(getStringProperty(widget, "columns", {}));
        const auto rows = model::splitTableRows(getStringProperty(widget, "rows", {}));
        const auto selection = model::clampSelectedCell(
            columns,
            rows,
            widget.getIntProperty("selectedRow", rows.empty() ? -1 : 0),
            widget.getIntProperty("selectedColumn", columns.empty() ? -1 : 0));
        const bool showHeader = getBoolProperty(widget, "showHeader", true);
        const bool showGridLines = getBoolProperty(widget, "showGridLines", true);
        const float headerHeight = showHeader ? std::max(18.0f, getNumericProperty(widget, "headerHeight", 30.0f)) : 0.0f;
        const float rowHeight = std::max(16.0f, getNumericProperty(widget, "rowHeight", 28.0f));
        const float contentX = bounds.x + 4.0f;
        const float contentY = bounds.y + 4.0f;
        const float contentWidth = std::max(0.0f, bounds.width - 8.0f);
        const float contentHeight = std::max(0.0f, bounds.height - 8.0f);
        const float columnWidth = contentWidth / static_cast<float>(std::max<std::size_t>(1, columns.empty() ? 1 : columns.size()));
        const float visibleRowsHeight = std::max(0.0f, contentHeight - headerHeight);
        const std::size_t visibleRowCount = std::max<std::size_t>(1, static_cast<std::size_t>(std::floor(visibleRowsHeight / rowHeight)));

        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);

        if (showHeader) {
            canvas.setColor(blendColor(style.panelColor, style.fillColor, 0.18f));
            canvas.fill(contentX, contentY, contentWidth, std::min(headerHeight, contentHeight));
        }

        for (std::size_t columnIndex = 0; columnIndex < std::max<std::size_t>(1, columns.empty() ? 1 : columns.size()); ++columnIndex) {
            const float columnX = contentX + static_cast<float>(columnIndex) * columnWidth;
            const bool selectedColumn = static_cast<int>(columnIndex) == selection.column;
            if (showHeader && selectedColumn) {
                canvas.setColor(blendColor(style.accentColor, style.fillColor, 0.24f));
                canvas.fill(columnX, contentY, std::max(0.0f, columnWidth - 1.0f), std::min(headerHeight, contentHeight));
            }

            if (showGridLines && columnIndex > 0) {
                canvas.setColor(blendColor(style.borderColor, style.fillColor, 0.35f));
                canvas.fill(columnX, contentY, 1.0f, contentHeight);
            }

            if (showHeader && drawText && columnIndex < columns.size()) {
                const std::string& headerText = columns[columnIndex];
                canvas.setColor(style.textColor);
                canvas.text(headerText.empty() ? std::string{ "<empty>" } : headerText, widgetFont, visage::Font::kTopLeft,
                    columnX + 6.0f, contentY + std::max(3.0f, (headerHeight - fontSize * 1.3f) * 0.35f),
                    std::max(0.0f, columnWidth - 12.0f), std::max(0.0f, headerHeight - 6.0f));
            }
        }

        float rowTop = contentY + headerHeight;
        for (std::size_t rowIndex = 0; rowIndex < std::min<std::size_t>(visibleRowCount, rows.size()); ++rowIndex) {
            const bool selectedRow = static_cast<int>(rowIndex) == selection.row;
            for (std::size_t columnIndex = 0; columnIndex < std::max<std::size_t>(1, columns.empty() ? 1 : columns.size()); ++columnIndex) {
                const float columnX = contentX + static_cast<float>(columnIndex) * columnWidth;
                const bool selectedCell = selectedRow && static_cast<int>(columnIndex) == selection.column;
                canvas.setColor(selectedCell ? blendColor(style.accentColor, style.fillColor, 0.30f)
                    : (selectedRow ? blendColor(style.accentColor, style.fillColor, 0.16f)
                                   : (rowIndex % 2 == 0 ? style.fillColor : blendColor(style.panelColor, style.fillColor, 0.14f))));
                canvas.fill(columnX, rowTop, std::max(0.0f, columnWidth - 1.0f), std::max(0.0f, rowHeight - 1.0f));

                if (drawText) {
                    canvas.setColor(style.textColor);
                    canvas.text(model::getCellText(rows, static_cast<int>(rowIndex), static_cast<int>(columnIndex)), widgetFont, visage::Font::kTopLeft,
                        columnX + 6.0f, rowTop + std::max(2.0f, (rowHeight - fontSize * 1.3f) * 0.35f),
                        std::max(0.0f, columnWidth - 12.0f), std::max(0.0f, rowHeight - 4.0f));
                }
            }

            if (showGridLines) {
                canvas.setColor(blendColor(style.borderColor, style.fillColor, 0.35f));
                canvas.fill(contentX, rowTop + rowHeight - 1.0f, contentWidth, 1.0f);
            }

            rowTop += rowHeight;
        }

        if (rows.size() > visibleRowCount) {
            canvas.setColor(style.panelColor);
            canvas.fill(bounds.x + bounds.width - 8.0f, bounds.y + 4.0f, 4.0f, std::max(0.0f, bounds.height - 8.0f));
            canvas.setColor(style.accentColor);
            canvas.fill(bounds.x + bounds.width - 8.0f, bounds.y + 10.0f, 4.0f, std::max(16.0f, bounds.height * 0.22f));
        }

        if (columns.empty() && drawText) {
            canvas.setColor(style.textColor);
            canvas.text("<no columns>", widgetFont, visage::Font::kCenter,
                bounds.x, bounds.y, bounds.width, bounds.height);
        }
        else if (rows.empty() && drawText) {
            canvas.setColor(style.textColor);
            canvas.text("<no rows>", widgetFont, visage::Font::kCenter,
                bounds.x, bounds.y + headerHeight, bounds.width, std::max(0.0f, bounds.height - headerHeight));
        }
        break;
    }
    case model::WidgetType::TreeView: {
        const std::string nodesText = getStringProperty(widget, "nodes", {});
        const bool showRoot = getBoolProperty(widget, "showRoot", true);
        const bool showLines = getBoolProperty(widget, "showLines", true);
        const std::string expandedNodePaths = getStringProperty(widget, "expandedNodePaths", {});
        const auto visibleNodes = model::flattenVisibleTreeNodes(nodesText, showRoot, expandedNodePaths);
        const std::string selectedNodePath = model::clampSelectedTreeNode(
            nodesText,
            getStringProperty(widget, "selectedNodePath", {}),
            showRoot,
            expandedNodePaths);
        const float rowHeight = std::max(18.0f, fontSize * 1.45f);
        const float indentWidth = 16.0f;
        const float markerSize = 10.0f;
        const float listTop = bounds.y + 4.0f;
        const float visibleHeight = std::max(0.0f, bounds.height - 8.0f);
        const std::size_t visibleCount = std::max<std::size_t>(1, static_cast<std::size_t>(std::floor(visibleHeight / rowHeight)));
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);

        float rowTop = listTop;
        const int lineColor = blendColor(style.borderColor, style.fillColor, 0.35f);
        for (std::size_t index = 0; index < std::min<std::size_t>(visibleCount, visibleNodes.size()); ++index) {
            const auto& node = visibleNodes[index];
            const bool selected = node.path == selectedNodePath;
            const float markerCenterX = bounds.x + 12.0f + static_cast<float>(node.visualDepth) * indentWidth;
            const float markerCenterY = rowTop + rowHeight * 0.5f;
            const float textX = markerCenterX + (node.hasChildren ? 12.0f : 8.0f);

            canvas.setColor(selected ? blendColor(style.accentColor, style.fillColor, 0.28f) : (index % 2 == 0 ? style.fillColor : blendColor(style.panelColor, style.fillColor, 0.14f)));
            canvas.fill(bounds.x + 4.0f, rowTop, std::max(0.0f, bounds.width - 14.0f), rowHeight - 1.0f);

            if (showLines && node.visualDepth > 0) {
                canvas.setColor(lineColor);
                const float indentX = bounds.x + 12.0f + static_cast<float>(node.visualDepth - 1) * indentWidth;
                canvas.fill(indentX, rowTop, 1.0f, rowHeight);
                canvas.fill(indentX, markerCenterY, markerCenterX - indentX, 1.0f);
            }

            if (node.hasChildren) {
                const PanelRect markerBounds{ markerCenterX - markerSize * 0.5f, markerCenterY - markerSize * 0.5f, markerSize, markerSize };
                canvas.setColor(blendColor(style.panelColor, style.fillColor, 0.22f));
                canvas.fill(markerBounds.x, markerBounds.y, markerBounds.width, markerBounds.height);
                drawBorder(canvas, markerBounds, style.borderColor, 1.0f);
                canvas.setColor(style.borderColor);
                canvas.fill(markerBounds.x + 2.0f, markerCenterY, markerBounds.width - 4.0f, 1.0f);
                if (!node.expanded) {
                    canvas.fill(markerCenterX, markerBounds.y + 2.0f, 1.0f, markerBounds.height - 4.0f);
                }
            }

            if (drawText) {
                canvas.setColor(style.textColor);
                canvas.text(node.text, widgetFont, visage::Font::kTopLeft,
                    textX + 6.0f, rowTop + std::max(2.0f, (rowHeight - fontSize * 1.3f) * 0.5f),
                    std::max(0.0f, bounds.x + bounds.width - textX - 18.0f), std::max(0.0f, rowHeight - 4.0f));
            }

            rowTop += rowHeight;
        }

        if (visibleNodes.size() > visibleCount) {
            canvas.setColor(style.panelColor);
            canvas.fill(bounds.x + bounds.width - 8.0f, bounds.y + 4.0f, 4.0f, std::max(0.0f, bounds.height - 8.0f));
            canvas.setColor(style.accentColor);
            canvas.fill(bounds.x + bounds.width - 8.0f, bounds.y + 10.0f, 4.0f, std::max(16.0f, bounds.height * 0.22f));
        }

        if (visibleNodes.empty() && drawText) {
            canvas.setColor(style.textColor);
            canvas.text("<empty>", widgetFont, visage::Font::kCenter,
                bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }
    case model::WidgetType::CheckBox: {
        const float boxSize = 18.0f;
        const float squareX = bounds.x + 6.0f;
        const float squareY = bounds.y + (bounds.height - boxSize) * 0.5f;
        const float textX = squareX + boxSize + 12.0f;
        canvas.setColor(style.fillColor);
        canvas.fill(squareX, squareY, boxSize, boxSize);
        drawBorder(canvas, { squareX, squareY, boxSize, boxSize }, style.borderColor, style.borderThickness);
        if (getBoolProperty(widget, "checked", false)) {
            canvas.setColor(style.accentColor);
            canvas.fill(squareX + 4.0f, squareY + 4.0f, 10.0f, 10.0f);
        }
        if (drawText) {
            canvas.setColor(style.textColor);
            canvas.text(getDisplayTextOrFallback(widget, "text", "CheckBox"), widgetFont, visage::Font::kTopLeft,
                textX, centeredTextTop(bounds.y, bounds.height, fontSize),
                std::max(0.0f, bounds.x + bounds.width - textX - 8.0f), std::max(0.0f, bounds.height - 8.0f));
        }
        break;
    }
    case model::WidgetType::RadioButton: {
        const float boxSize = 18.0f;
        const float outerX = bounds.x + 6.0f;
        const float outerY = bounds.y + (bounds.height - boxSize) * 0.5f;
        const float centerX = outerX + boxSize * 0.5f;
        const float centerY = outerY + boxSize * 0.5f;
        const float textX = outerX + boxSize + 12.0f;
        fillCircleApprox(canvas, centerX, centerY, boxSize * 0.5f, style.borderColor);
        fillCircleApprox(canvas, centerX, centerY,
            std::max(2.0f, boxSize * 0.5f - std::max(1.0f, style.borderThickness)), style.fillColor);
        if (getBoolProperty(widget, "selected", false)) {
            fillCircleApprox(canvas, centerX, centerY, 4.0f, style.accentColor);
        }
        if (drawText) {
            canvas.setColor(style.textColor);
            canvas.text(getDisplayTextOrFallback(widget, "text", "Radio Button"), widgetFont, visage::Font::kTopLeft,
                textX, centeredTextTop(bounds.y, bounds.height, fontSize),
                std::max(0.0f, bounds.x + bounds.width - textX - 8.0f), std::max(0.0f, bounds.height - 8.0f));
        }
        break;
    }
    case model::WidgetType::Slider: {
        const float normalized = normalizedSliderValue(widget);
        const float trackY = bounds.y + bounds.height * 0.5f - 2.0f;
        const float trackLeft = bounds.x + 8.0f;
        const float trackWidth = std::max(0.0f, bounds.width - 16.0f);
        const float handleCenterX = trackLeft + trackWidth * normalized;
        const float handleX = std::clamp(handleCenterX - 6.0f, trackLeft - 6.0f, trackLeft + trackWidth - 6.0f);
        canvas.setColor(style.borderColor);
        canvas.fill(trackLeft, trackY, trackWidth, 4.0f);
        canvas.setColor(style.accentColor);
        canvas.fill(handleX, bounds.y + bounds.height * 0.5f - 8.0f, 12.0f, 16.0f);
        drawBorder(canvas, { handleX, bounds.y + bounds.height * 0.5f - 8.0f, 12.0f, 16.0f }, style.borderColor, style.borderThickness);
        if (drawText) {
            canvas.setColor(style.textColor);
            canvas.text(getStringProperty(widget, "text", widgetLabel(widget)), widgetFont, visage::Font::kTopLeft,
                bounds.x, bounds.y - 18.0f, bounds.width, 16.0f);
        }
        break;
    }
    case model::WidgetType::ScrollBar: {
        const bool vertical = getStringProperty(widget, "orientation", "Horizontal") == "Vertical";
        const float pageSize = std::max(1.0f, getNumericProperty(widget, "pageSize", 10.0f));
        const float minimum = getNumericProperty(widget, "min", 0.0f);
        const float maximum = std::max(minimum + 1.0f, getNumericProperty(widget, "max", 100.0f));
        const float value = std::clamp(getNumericProperty(widget, "value", minimum), minimum, maximum);
        const float normalized = std::clamp((value - minimum) / (maximum - minimum), 0.0f, 1.0f);
        const float thumbFactor = std::clamp(pageSize / (maximum - minimum + pageSize), 0.18f, 0.55f);
        const float arrowSize = vertical ? std::min(bounds.width, 20.0f) : std::min(bounds.height, 20.0f);
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);
        canvas.setColor(style.borderColor);
        if (vertical) {
            const float trackTop = bounds.y + arrowSize;
            const float trackHeight = std::max(0.0f, bounds.height - arrowSize * 2.0f);
            const float thumbHeight = std::clamp(trackHeight * thumbFactor, 18.0f, std::max(18.0f, trackHeight));
            const float thumbY = trackTop + std::max(0.0f, trackHeight - thumbHeight) * normalized;
            canvas.setColor(style.fillColor);
            canvas.fill(bounds.x, bounds.y, bounds.width, arrowSize);
            canvas.fill(bounds.x, bounds.y + bounds.height - arrowSize, bounds.width, arrowSize);
            drawBorder(canvas, { bounds.x, bounds.y, bounds.width, arrowSize }, style.borderColor, style.borderThickness);
            drawBorder(canvas, { bounds.x, bounds.y + bounds.height - arrowSize, bounds.width, arrowSize }, style.borderColor, style.borderThickness);
            canvas.setColor(style.panelColor);
            canvas.fill(bounds.x + 2.0f, trackTop, bounds.width - 4.0f, trackHeight);
            canvas.setColor(style.accentColor);
            canvas.fill(bounds.x + 4.0f, thumbY, std::max(0.0f, bounds.width - 8.0f), thumbHeight);
            drawBorder(canvas, { bounds.x + 4.0f, thumbY, std::max(0.0f, bounds.width - 8.0f), thumbHeight }, style.borderColor, style.borderThickness);
            canvas.fill(bounds.x + bounds.width * 0.5f - 3.0f, bounds.y + 6.0f, 6.0f, 3.0f);
            canvas.fill(bounds.x + bounds.width * 0.5f - 3.0f, bounds.y + bounds.height - 9.0f, 6.0f, 3.0f);
        }
        else {
            const float trackLeft = bounds.x + arrowSize;
            const float trackWidth = std::max(0.0f, bounds.width - arrowSize * 2.0f);
            const float thumbWidth = std::clamp(trackWidth * thumbFactor, 18.0f, std::max(18.0f, trackWidth));
            const float thumbX = trackLeft + std::max(0.0f, trackWidth - thumbWidth) * normalized;
            canvas.setColor(style.fillColor);
            canvas.fill(bounds.x, bounds.y, arrowSize, bounds.height);
            canvas.fill(bounds.x + bounds.width - arrowSize, bounds.y, arrowSize, bounds.height);
            drawBorder(canvas, { bounds.x, bounds.y, arrowSize, bounds.height }, style.borderColor, style.borderThickness);
            drawBorder(canvas, { bounds.x + bounds.width - arrowSize, bounds.y, arrowSize, bounds.height }, style.borderColor, style.borderThickness);
            canvas.setColor(style.panelColor);
            canvas.fill(trackLeft, bounds.y + 2.0f, trackWidth, bounds.height - 4.0f);
            canvas.setColor(style.accentColor);
            canvas.fill(thumbX, bounds.y + 4.0f, thumbWidth, std::max(0.0f, bounds.height - 8.0f));
            drawBorder(canvas, { thumbX, bounds.y + 4.0f, thumbWidth, std::max(0.0f, bounds.height - 8.0f) }, style.borderColor, style.borderThickness);
            canvas.fill(bounds.x + 6.0f, bounds.y + bounds.height * 0.5f - 3.0f, 3.0f, 6.0f);
            canvas.fill(bounds.x + bounds.width - 9.0f, bounds.y + bounds.height * 0.5f - 3.0f, 3.0f, 6.0f);
        }
        break;
    }
    case model::WidgetType::Image:
    {
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);

        const auto resolvedSource = resources::ImageResourceCache::resolveWidgetImageSource(document, widget);
        const auto scaleMode = resources::ImageResourceCache::parseScaleMode(widget.getStringProperty("scaleMode", "Fit"));
        const int warningBorderColor = 0xfff5c16c;
        const int errorBorderColor = 0xffe17d7d;
        int borderColor = style.borderColor;
        std::string placeholderText = imageWidgetPlaceholderText(resolvedSource);
        const bool simplifyImagePreview = simplifySelectedImages
            && (document.isPrimarySelected(widget.id) || document.isSecondarySelected(widget.id));
        bool drewImage = false;

        if (resolvedSource.missingResource) {
            borderColor = warningBorderColor;
        }
        else if (simplifyImagePreview) {
            const PanelRect imageBounds{
                bounds.x + 4.0f,
                bounds.y + 4.0f,
                std::max(0.0f, bounds.width - 8.0f),
                std::max(0.0f, bounds.height - 8.0f)
            };
            canvas.setColor(blendColor(style.fillColor, style.accentColor, 0.12f));
            canvas.fill(imageBounds.x, imageBounds.y, imageBounds.width, imageBounds.height);
            canvas.setColor(blendColor(style.borderColor, style.accentColor, 0.35f));
            canvas.fill(imageBounds.x, imageBounds.y, imageBounds.width, 1.0f);
            canvas.fill(imageBounds.x, imageBounds.y + imageBounds.height - 1.0f, imageBounds.width, 1.0f);
            canvas.fill(imageBounds.x, imageBounds.y, 1.0f, imageBounds.height);
            canvas.fill(imageBounds.x + imageBounds.width - 1.0f, imageBounds.y, 1.0f, imageBounds.height);

            const float inset = std::min(std::min(imageBounds.width, imageBounds.height) * 0.2f, 12.0f);
            canvas.setColor(style.accentColor);
            canvas.fill(imageBounds.x + inset, imageBounds.y + inset,
                std::max(1.0f, imageBounds.width - inset * 2.0f), 1.0f);
            canvas.fill(imageBounds.x + inset, imageBounds.y + imageBounds.height - inset - 1.0f,
                std::max(1.0f, imageBounds.width - inset * 2.0f), 1.0f);
            if (drawText && !resolvedSource.displayText.empty()) {
                placeholderText = resolvedSource.displayText;
            }
            else {
                placeholderText = "Image";
            }
        }
        else if (resolvedSource.hasImage && imageCache != nullptr) {
            const auto cachedImage = imageCache->getOrLoad(resolvedSource.sourcePath);
            placeholderText = imageWidgetPlaceholderText(resolvedSource, &cachedImage);
            if (cachedImage.info.available && cachedImage.encodedBytes != nullptr && !cachedImage.encodedBytes->empty()) {
                const auto drawRect = resources::ImageResourceCache::computeDrawRect(
                    bounds.x + 4.0f,
                    bounds.y + 4.0f,
                    std::max(0.0f, bounds.width - 8.0f),
                    std::max(0.0f, bounds.height - 8.0f),
                    cachedImage.info.width,
                    cachedImage.info.height,
                    scaleMode);
                canvas.setColor(0xffffffff);
                canvas.image(cachedImage.encodedBytes->data(),
                    static_cast<int>(cachedImage.encodedBytes->size()),
                    drawRect.x,
                    drawRect.y,
                    drawRect.width,
                    drawRect.height);
                drewImage = true;
            }
            else if (placeholderText == "Missing image file") {
                borderColor = warningBorderColor;
            }
            else if (placeholderText == "Image load failed") {
                borderColor = errorBorderColor;
            }
        }

        drawBorder(canvas, bounds, borderColor, style.borderThickness);
        if (drawText && !drewImage) {
            canvas.setColor(borderColor == style.borderColor ? style.textColor : borderColor);
            canvas.text(placeholderText, widgetFont, visage::Font::kCenter,
                bounds.x + 6.0f, bounds.y, std::max(0.0f, bounds.width - 12.0f), bounds.height);
        }
        break;
    }
    case model::WidgetType::Spacer:
        canvas.setColor(style.fillColor);
        canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);
        drawBorder(canvas, bounds, style.borderColor, style.borderThickness);
        if (drawText) {
            canvas.setColor(style.textColor);
            canvas.text("Spacer", widgetFont, visage::Font::kCenter, bounds.x, bounds.y, bounds.width, bounds.height);
        }
        break;
    }

    // Draw children from back to front so later children appear on top.
    for (const auto& child : widget.children) {
        if (!isChildVisibleInParent(widget, child)) {
            continue;
        }
        drawWidget(canvas, font, drawText, document, imageCache, simplifySelectedImages, child, formScreenX, formScreenY, widgetLocalX, widgetLocalY,
            scale, selectedWidgetId, visualHandleSize, showGrid, showMinorGrid, gridSize, majorGridSize);
    }

    if (document.isPrimarySelected(widget.id)) {
        drawSelectionOutline(canvas, bounds);
        if (widget.type != model::WidgetType::FormWindow && widget.type != model::WidgetType::TabPage) {
            drawSelectionHandles(canvas, bounds, visualHandleSize);
        }
    }
    else if (document.isSecondarySelected(widget.id)) {
        drawSecondarySelectionOutline(canvas, bounds);
    }
}

} // namespace

void DesignerCanvas::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

void DesignerCanvas::setShowGrid(bool showGrid)
{
    showGrid_ = showGrid;
}

void DesignerCanvas::setSnapToGrid(bool snapToGrid)
{
    snapToGrid_ = snapToGrid;
}

void DesignerCanvas::setGridSize(int gridSize)
{
    gridSize_ = std::max(1, gridSize);
}

void DesignerCanvas::setMajorGridSize(int majorGridSize)
{
    majorGridSize_ = std::max(1, majorGridSize);
}

bool DesignerCanvas::showGrid() const
{
    return showGrid_;
}

bool DesignerCanvas::snapToGrid() const
{
    return snapToGrid_;
}

int DesignerCanvas::gridSize() const
{
    return gridSize_;
}

int DesignerCanvas::majorGridSize() const
{
    return majorGridSize_;
}

bool DesignerCanvas::contains(float x, float y) const
{
    return x >= x_ && y >= y_ && x <= x_ + width_ && y <= y_ + height_;
}

std::optional<DesignerCanvas::FormPoint> DesignerCanvas::toFormPoint(const model::ProjectDocument& document, float x, float y) const
{
    if (!contains(x, y) || !document.root.bounds.isValid()) {
        return std::nullopt;
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(x_, y_, width_, height_, document);
    if (!previewLayout.form.contains(x, y) || previewLayout.scale <= 0.0f) {
        return std::nullopt;
    }

    return FormPoint{
        (x - previewLayout.form.x) / previewLayout.scale + document.root.bounds.x,
        (y - previewLayout.form.y) / previewLayout.scale + document.root.bounds.y
    };
}

std::optional<std::string> DesignerCanvas::hitTestWidgetId(const model::ProjectDocument& document, float x, float y) const
{
    if (!contains(x, y) || !document.root.bounds.isValid()) {
        return std::nullopt;
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(x_, y_, width_, height_, document);
    if (!previewLayout.form.contains(x, y) || previewLayout.scale <= 0.0f) {
        return std::nullopt;
    }

    if (auto widgetId = hitTestWidgetScreenId(document.root, previewLayout.form.x, previewLayout.form.y,
            -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale, x, y, smallWidgetHitPadding_)) {
        return widgetId;
    }

    return std::nullopt;
}

std::optional<int> DesignerCanvas::hitTestTabHeader(const model::ProjectDocument& document, const std::string& widgetId, float x, float y) const
{
    if (!contains(x, y) || widgetId.empty() || !document.root.bounds.isValid()) {
        return std::nullopt;
    }

    const model::WidgetNode* widget = document.findWidgetById(widgetId);
    if (widget == nullptr || widget->type != model::WidgetType::TabControl) {
        return std::nullopt;
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(x_, y_, width_, height_, document);
    if (!previewLayout.form.isValid()) {
        return std::nullopt;
    }

    const auto widgetInfo = findWidgetScreenInfo(document.root, widgetId, previewLayout.form.x, previewLayout.form.y,
        -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale);
    if (!widgetInfo.has_value()) {
        return std::nullopt;
    }

    const float headerHeight = std::min(32.0f, std::max(24.0f, widgetInfo->bounds.height * 0.18f));
    const PanelRect headerBounds{ widgetInfo->bounds.x, widgetInfo->bounds.y, widgetInfo->bounds.width, headerHeight };
    if (!headerBounds.contains(x, y)) {
        return std::nullopt;
    }

    const std::vector<std::string> labels = tabLabels(*widget);
    const float tabWidth = headerBounds.width / static_cast<float>(std::max<std::size_t>(1, labels.size()));
    const int index = std::clamp(static_cast<int>((x - headerBounds.x) / std::max(1.0f, tabWidth)), 0, static_cast<int>(labels.size()) - 1);
    return index;
}

std::optional<DesignerCanvas::InteractionHit> DesignerCanvas::hitTestInteraction(const model::ProjectDocument& document,
    float x,
    float y,
    const std::string& selectedWidgetId) const
{
    const auto hitWidgetId = hitTestWidgetId(document, x, y);
    if (!hitWidgetId.has_value()) {
        return std::nullopt;
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(x_, y_, width_, height_, document);
    if (!previewLayout.form.isValid()) {
        return std::nullopt;
    }

    const auto widgetInfo = findWidgetScreenInfo(document.root, *hitWidgetId, previewLayout.form.x, previewLayout.form.y,
        -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale);
    if (!widgetInfo.has_value()) {
        return std::nullopt;
    }

    InteractionHit hit{ *hitWidgetId, HitRegion::Body };
    if (const auto* selectedWidget = document.findWidgetById(selectedWidgetId);
        selectedWidget != nullptr && selectedWidget->type == model::WidgetType::TabPage) {
        return std::nullopt;
    }

    if (*hitWidgetId == selectedWidgetId && *hitWidgetId != document.root.id) {
        const HitRegion handle = hitHandle(widgetInfo->bounds, x, y, resizeHandleHitSize_);
        if (handle != HitRegion::None) {
            hit.region = handle;
            return hit;
        }
    }

    if (widgetInfo->bounds.contains(x, y)) {
        return hit;
    }

    return std::nullopt;
}

float DesignerCanvas::snap(float value) const
{
    if (!snapToGrid_ || gridSize_ <= 1) {
        return value;
    }

    return std::round(value / static_cast<float>(gridSize_)) * static_cast<float>(gridSize_);
}

model::Rect DesignerCanvas::moveBounds(const model::Rect& originalBounds, const FormPoint& dragStart, const FormPoint& currentPoint) const
{
    model::Rect updated = originalBounds;
    updated.x = originalBounds.x + (currentPoint.x - dragStart.x);
    updated.y = originalBounds.y + (currentPoint.y - dragStart.y);
    updated.x = snap(updated.x);
    updated.y = snap(updated.y);
    return updated;
}

model::Rect DesignerCanvas::resizeBounds(const model::Rect& originalBounds,
    HitRegion region,
    const FormPoint& dragStart,
    const FormPoint& currentPoint) const
{
    float left = originalBounds.x;
    float top = originalBounds.y;
    float right = originalBounds.x + originalBounds.width;
    float bottom = originalBounds.y + originalBounds.height;

    const float deltaX = currentPoint.x - dragStart.x;
    const float deltaY = currentPoint.y - dragStart.y;

    switch (region) {
    case HitRegion::TopLeftHandle:
        left = snap(left + deltaX);
        top = snap(top + deltaY);
        break;
    case HitRegion::TopRightHandle:
        right = snap(right + deltaX);
        top = snap(top + deltaY);
        break;
    case HitRegion::BottomLeftHandle:
        left = snap(left + deltaX);
        bottom = snap(bottom + deltaY);
        break;
    case HitRegion::BottomRightHandle:
        right = snap(right + deltaX);
        bottom = snap(bottom + deltaY);
        break;
    case HitRegion::None:
    case HitRegion::Body:
        return originalBounds;
    }

    if (right - left < minimumWidgetSize_) {
        if (region == HitRegion::TopLeftHandle || region == HitRegion::BottomLeftHandle) {
            left = right - minimumWidgetSize_;
        }
        else {
            right = left + minimumWidgetSize_;
        }
    }

    if (bottom - top < minimumWidgetSize_) {
        if (region == HitRegion::TopLeftHandle || region == HitRegion::TopRightHandle) {
            top = bottom - minimumWidgetSize_;
        }
        else {
            bottom = top + minimumWidgetSize_;
        }
    }

    return { left, top, std::max(minimumWidgetSize_, right - left), std::max(minimumWidgetSize_, bottom - top) };
}

void DesignerCanvas::draw(visage::Canvas& canvas,
    const visage::Font& font,
    bool drawText,
    const model::ProjectDocument& document,
    resources::ImageResourceCache* imageCache,
    bool simplifySelectedImages,
    const std::optional<SelectionRect>& marqueeRect,
    const std::vector<SmartGuide>& smartGuides) const
{
    (void)simplifySelectedImages;

    if (width_ <= 0.0f || height_ <= 0.0f) {
        return;
    }

    canvas.setColor(0xff1f242d);
    canvas.fill(x_, y_, width_, height_);

    canvas.setColor(0xff2a303a);
    canvas.fill(x_, y_, width_, kHeaderHeight);

    canvas.setColor(0xff101318);
    canvas.fill(x_, y_, width_, 1.0f);
    canvas.fill(x_, y_ + height_ - 1.0f, width_, 1.0f);
    canvas.fill(x_, y_, 1.0f, height_);
    canvas.fill(x_ + width_ - 1.0f, y_, 1.0f, height_);

    if (drawText) {
        canvas.setColor(0xfff3f5f8);
        canvas.text("Designer Canvas", font, visage::Font::kTopLeft,
            x_ + kPadding, y_ + 6.0f, width_ - kPadding * 2.0f, kHeaderHeight - 8.0f);
    }

    const PreviewLayout previewLayout = calculatePreviewLayout(x_, y_, width_, height_, document);
    if (!previewLayout.preview.isValid()) {
        return;
    }

    canvas.setColor(0xff303746);
    canvas.fill(previewLayout.preview.x, previewLayout.preview.y, previewLayout.preview.width, previewLayout.preview.height);
    canvas.setColor(0xff475064);
    canvas.fill(previewLayout.preview.x + 1.0f, previewLayout.preview.y + 1.0f,
        previewLayout.preview.width - 2.0f, previewLayout.preview.height - 2.0f);

    if (!previewLayout.form.isValid()) {
        return;
    }

    drawWidget(canvas, font, drawText, document, imageCache, simplifySelectedImages, document.root, previewLayout.form.x, previewLayout.form.y,
        -document.root.bounds.x, -document.root.bounds.y, previewLayout.scale, document.selectedWidgetId,
        resizeHandleVisualSize_, showGrid_, showMinorGrid_, gridSize_, majorGridSize_);

    if (marqueeRect.has_value()) {
        const PanelRect screenRect = selectionRectToScreenRect(previewLayout, *marqueeRect);
        canvas.setColor(0x226fa9ff);
        canvas.fill(screenRect.x, screenRect.y, screenRect.width, screenRect.height);
        canvas.setColor(0xff6fa9ff);
        canvas.fill(screenRect.x, screenRect.y, screenRect.width, 1.0f);
        canvas.fill(screenRect.x, screenRect.y + screenRect.height - 1.0f, screenRect.width, 1.0f);
        canvas.fill(screenRect.x, screenRect.y, 1.0f, screenRect.height);
        canvas.fill(screenRect.x + screenRect.width - 1.0f, screenRect.y, 1.0f, screenRect.height);
    }

    for (const auto& guide : smartGuides) {
        canvas.setColor(0xffff6b2c);
        if (guide.orientation == GuideOrientation::Vertical) {
            const float x = previewLayout.form.x + guide.position * previewLayout.scale;
            canvas.fill(x, previewLayout.form.y, 1.0f, previewLayout.form.height);
        }
        else {
            const float y = previewLayout.form.y + guide.position * previewLayout.scale;
            canvas.fill(previewLayout.form.x, y, previewLayout.form.width, 1.0f);
        }
    }

    if (drawText) {
        canvas.setColor(0xff243041);
        const std::string selectedLabel = document.hasSelection() && document.selectedWidget() != nullptr
            ? "Selected: " + widgetLabel(*document.selectedWidget())
            : "Selected: None";
        canvas.text(selectedLabel, font, visage::Font::kTopLeft,
            previewLayout.preview.x + 8.0f, previewLayout.preview.y + previewLayout.preview.height - 28.0f,
            previewLayout.preview.width - 16.0f, 22.0f);
    }
}

} // namespace visiform::ui
