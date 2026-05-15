#pragma once

#pragma once

#include <visage/app.h>
#include <visage/graphics.h>

#include <string>

namespace visiform::ui {

class MainWindow : public visage::ApplicationWindow {
public:
    MainWindow();
    ~MainWindow() override = default;

    void showWindow();
    void draw(visage::Canvas& canvas) override;

private:
    struct PanelBounds {
        float x;
        float y;
        float width;
        float height;
    };

    void loadLabelFont();
    void drawPanel(visage::Canvas& canvas, const PanelBounds& bounds, int color) const;
    void drawPanelLabel(visage::Canvas& canvas, const PanelBounds& bounds, const char* label) const;
    [[nodiscard]] bool canDrawLabels() const;

    visage::Font labelFont_{};
};

} // namespace visiform::ui
