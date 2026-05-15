#include "ui/MainWindow.h"

#include "ui/MainWindow.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <string>

namespace visiform::ui {
namespace {

constexpr auto kWindowTitle = "VisiForm - Visage Form Builder";
constexpr float kToolbarHeight = 56.0f;
constexpr float kStatusBarHeight = 32.0f;
constexpr float kLeftPanelWidth = 260.0f;
constexpr float kRightPanelWidth = 300.0f;
constexpr float kGap = 8.0f;
constexpr float kInnerPadding = 16.0f;

} // namespace

MainWindow::MainWindow()
{
    setTitle(kWindowTitle);
    loadLabelFont();
}

void MainWindow::showWindow()
{
    show(visage::Dimension::logicalPixels(1200), visage::Dimension::logicalPixels(800));
}

void MainWindow::draw(visage::Canvas& canvas)
{
    const float windowWidth = width();
    const float windowHeight = height();

    canvas.setColor(0xff1b1d23);
    canvas.fill(0, 0, windowWidth, windowHeight);

    if (windowWidth <= 0.0f || windowHeight <= 0.0f) {
        return;
    }

    const PanelBounds toolbar{ 0.0f, 0.0f, windowWidth, kToolbarHeight };
    const PanelBounds statusBar{ 0.0f, std::max(0.0f, windowHeight - kStatusBarHeight), windowWidth, kStatusBarHeight };

    const float contentTop = toolbar.height + kGap;
    const float contentBottom = std::max(contentTop, statusBar.y - kGap);
    const float contentHeight = std::max(0.0f, contentBottom - contentTop);
    const float paletteWidth = std::min(kLeftPanelWidth, std::max(180.0f, windowWidth * 0.22f));
    const float inspectorWidth = std::min(kRightPanelWidth, std::max(220.0f, windowWidth * 0.25f));

    const PanelBounds widgetPalette{ kGap, contentTop, paletteWidth, contentHeight };
    const PanelBounds propertyInspector{
        std::max(kGap, windowWidth - inspectorWidth - kGap),
        contentTop,
        inspectorWidth,
        contentHeight
    };

    const float canvasX = widgetPalette.x + widgetPalette.width + kGap;
    const float canvasWidth = std::max(0.0f, propertyInspector.x - canvasX - kGap);
    const PanelBounds designerCanvas{ canvasX, contentTop, canvasWidth, contentHeight };

    drawPanel(canvas, toolbar, 0xff2a2d36);
    drawPanel(canvas, widgetPalette, 0xff252932);
    drawPanel(canvas, designerCanvas, 0xff20242c);
    drawPanel(canvas, propertyInspector, 0xff252932);
    drawPanel(canvas, statusBar, 0xff2a2d36);

    drawPanelLabel(canvas, toolbar, "Toolbar / Menu Placeholder");
    drawPanelLabel(canvas, widgetPalette, "Widget Palette");
    drawPanelLabel(canvas, designerCanvas, "Designer Canvas");
    drawPanelLabel(canvas, propertyInspector, "Property Inspector");
    drawPanelLabel(canvas, statusBar, "Status: Ready");

    if (designerCanvas.width > kInnerPadding * 2.0f && designerCanvas.height > 120.0f) {
        canvas.setColor(0xff353b48);
        const float gridLeft = designerCanvas.x + kInnerPadding;
        const float gridTop = designerCanvas.y + 56.0f;
        const float gridWidth = designerCanvas.width - kInnerPadding * 2.0f;
        const float gridHeight = designerCanvas.height - 72.0f;
        canvas.fill(gridLeft, gridTop, gridWidth, gridHeight);

        canvas.setColor(0xff4a5161);
        canvas.fill(gridLeft + 1.0f, gridTop + 1.0f, gridWidth - 2.0f, gridHeight - 2.0f);
        canvas.setColor(0xff2b313d);
        canvas.fill(gridLeft + 2.0f, gridTop + 2.0f, gridWidth - 4.0f, gridHeight - 4.0f);

        // TODO: Replace this painted mock layout with real docked Visage editor widgets.
    }
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
        if (canDrawLabels()) {
            return;
        }
    }
}

void MainWindow::drawPanel(visage::Canvas& canvas, const PanelBounds& bounds, int color) const
{
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) {
        return;
    }

    canvas.setColor(color);
    canvas.fill(bounds.x, bounds.y, bounds.width, bounds.height);

    canvas.setColor(0xff14161b);
    canvas.fill(bounds.x, bounds.y, bounds.width, 1.0f);
    canvas.fill(bounds.x, bounds.y + bounds.height - 1.0f, bounds.width, 1.0f);
    canvas.fill(bounds.x, bounds.y, 1.0f, bounds.height);
    canvas.fill(bounds.x + bounds.width - 1.0f, bounds.y, 1.0f, bounds.height);
}

void MainWindow::drawPanelLabel(visage::Canvas& canvas, const PanelBounds& bounds, const char* label) const
{
    if (!canDrawLabels() || bounds.width <= 0.0f || bounds.height <= 0.0f) {
        return;
    }

    canvas.setColor(0xfff2f4f8);
    canvas.text(label,
        labelFont_,
        visage::Font::kTopLeft,
        bounds.x + kInnerPadding,
        bounds.y + 8.0f,
        std::max(0.0f, bounds.width - kInnerPadding * 2.0f),
        std::max(0.0f, bounds.height - 12.0f));
}

bool MainWindow::canDrawLabels() const
{
    return labelFont_.packedFont() != nullptr;
}

} // namespace visiform::ui
