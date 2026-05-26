#pragma once

#pragma once

#include "model/ProjectDocument.h"

#include <visage/graphics.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace visiform::ui {

namespace resources {
class ImageResourceCache;
}

class DesignerCanvas {
public:
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
    void setShowGrid(bool showGrid);
    void setSnapToGrid(bool snapToGrid);
    void setGridSize(int gridSize);
    void setMajorGridSize(int majorGridSize);
    [[nodiscard]] bool showGrid() const;
    [[nodiscard]] bool snapToGrid() const;
    [[nodiscard]] int gridSize() const;
    [[nodiscard]] int majorGridSize() const;
    [[nodiscard]] bool contains(float x, float y) const;
    [[nodiscard]] std::optional<std::string> hitTestWidgetId(const model::ProjectDocument& document, float x, float y) const;
    [[nodiscard]] std::optional<InteractionHit> hitTestInteraction(const model::ProjectDocument& document, float x, float y, const std::string& selectedWidgetId) const;
    [[nodiscard]] std::optional<FormPoint> toFormPoint(const model::ProjectDocument& document, float x, float y) const;
    [[nodiscard]] model::Rect moveBounds(const model::Rect& originalBounds, const FormPoint& dragStart, const FormPoint& currentPoint) const;
    [[nodiscard]] model::Rect resizeBounds(const model::Rect& originalBounds, HitRegion region, const FormPoint& dragStart, const FormPoint& currentPoint) const;
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

    float x_{};
    float y_{};
    float width_{};
    float height_{};
    bool showGrid_ = true;
    bool showMinorGrid_ = true;
    bool snapToGrid_ = true;
    int gridSize_ = 10;
    int majorGridSize_ = 50;
    float resizeHandleVisualSize_ = 10.0f;
    float resizeHandleHitSize_ = 16.0f;
    float smallWidgetHitPadding_ = 4.0f;
    float minimumWidgetSize_ = 20.0f;
};

} // namespace visiform::ui
