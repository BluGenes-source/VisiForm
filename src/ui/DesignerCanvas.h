#pragma once

#include "model/ProjectDocument.h"
#include "ui/VisualStyleBaseline.h"

#include <visage/graphics.h>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace visiform::ui {

namespace resources {
class ImageResourceCache;
}

class DesignerCanvas {
public:
    enum class Mode {
        Design,
        Preview
    };

    enum class HitRegion {
        None,
        Body,
        TopLeftHandle,
        TopRightHandle,
        BottomLeftHandle,
        BottomRightHandle
    };

    struct InteractionHit {
        std::string widgetId{};
        HitRegion region = HitRegion::None;
    };

    struct FormPoint {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct ViewPoint {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct SelectionRect {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    enum class GuideOrientation {
        Vertical,
        Horizontal
    };

    struct SmartGuide {
        GuideOrientation orientation = GuideOrientation::Vertical;
        float position = 0.0f;
        std::string reason{};
    };

    void setBounds(float x, float y, float width, float height);
    void setMode(Mode mode);
    bool updatePreviewHover(const model::ProjectDocument& document, float x, float y);
    bool beginPreviewInteraction(const model::ProjectDocument& document, float x, float y);
    bool endPreviewInteraction(const model::ProjectDocument& document, float x, float y);
    void clearPreviewInteraction();
    [[nodiscard]] visual_style::State resolvedVisualState(const model::WidgetNode& widget) const;
    [[nodiscard]] int previewSelectedIndex(const model::WidgetNode& widget, int fallback) const;
    [[nodiscard]] int previewSelectedTab(const model::WidgetNode& widget, int fallback) const;
    void setShowGrid(bool showGrid);
    void setSnapToGrid(bool snapToGrid);
    void setGridSize(int gridSize);
    void setMajorGridSize(int majorGridSize);
    [[nodiscard]] bool showGrid() const;
    [[nodiscard]] Mode mode() const;
    [[nodiscard]] bool snapToGrid() const;
    [[nodiscard]] int gridSize() const;
    [[nodiscard]] int majorGridSize() const;
    [[nodiscard]] float zoom() const;
    [[nodiscard]] int zoomPercent() const;
    [[nodiscard]] bool contains(float x, float y) const;
    [[nodiscard]] bool containsViewport(float x, float y) const;
    [[nodiscard]] ViewPoint viewportCenter() const;
    [[nodiscard]] FormPoint viewToModelPoint(const model::ProjectDocument& document, float x, float y) const;
    [[nodiscard]] ViewPoint modelToViewPoint(const model::ProjectDocument& document, float x, float y) const;
    [[nodiscard]] model::Rect viewToModelRect(const model::ProjectDocument& document, const model::Rect& rect) const;
    [[nodiscard]] model::Rect modelToViewRect(const model::ProjectDocument& document, const model::Rect& rect) const;
    void setZoomAround(const model::ProjectDocument& document, float zoom, float viewX, float viewY);
    void resetView(const model::ProjectDocument& document);
    void fitFormToCanvas(const model::ProjectDocument& document);
    void panBy(float deltaX, float deltaY);
    [[nodiscard]] std::optional<std::string> hitTestWidgetId(const model::ProjectDocument& document, float x, float y) const;
    [[nodiscard]] std::optional<int> hitTestTabHeader(const model::ProjectDocument& document, const std::string& widgetId, float x, float y) const;
    [[nodiscard]] std::optional<InteractionHit> hitTestInteraction(const model::ProjectDocument& document, float x, float y, const std::string& selectedWidgetId) const;
    [[nodiscard]] std::optional<FormPoint> toFormPoint(const model::ProjectDocument& document, float x, float y) const;
    [[nodiscard]] model::Rect moveBounds(const model::Rect& originalBounds, const FormPoint& dragStart, const FormPoint& currentPoint) const;
    [[nodiscard]] model::Rect resizeBounds(const model::Rect& originalBounds, HitRegion region, const FormPoint& dragStart, const FormPoint& currentPoint) const;
    [[nodiscard]] float measureWidgetTextWidth(const model::ProjectDocument& document,
        const model::WidgetNode& widget,
        const std::string& text,
        const visage::Font& fallbackFont) const;
    void draw(visage::Canvas& canvas,
        const visage::Font& font,
        bool drawText,
        const model::ProjectDocument& document,
        resources::ImageResourceCache* imageCache,
        bool simplifySelectedImages = false,
        const std::optional<SelectionRect>& marqueeRect = std::nullopt,
        const std::vector<SmartGuide>& smartGuides = {}) const;

private:
    [[nodiscard]] float snap(float value) const;

    static constexpr float kMinimumZoom = 0.25f;
    static constexpr float kMaximumZoom = 4.0f;
    float x_{};
    float y_{};
    float width_{};
    float height_{};
    Mode mode_ = Mode::Design;
    bool showGrid_ = true;
    bool showMinorGrid_ = true;
    bool snapToGrid_ = true;
    int gridSize_ = 10;
    int majorGridSize_ = 50;
    float zoom_ = 1.0f;
    float panX_ = 0.0f;
    float panY_ = 0.0f;
    float smallWidgetHitPadding_ = 4.0f;
    float minimumWidgetSize_ = 20.0f;
    std::string previewHoveredWidgetId_{};
    std::string previewPressedWidgetId_{};
    std::string previewFocusedWidgetId_{};
    std::unordered_map<std::string, bool> previewChecked_{};
    std::unordered_map<std::string, bool> previewSelected_{};
    std::unordered_map<std::string, int> previewSelectedIndex_{};
    std::unordered_map<std::string, int> previewSelectedTab_{};
};

} // namespace visiform::ui
