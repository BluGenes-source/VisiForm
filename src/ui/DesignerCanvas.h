#pragma once

#pragma once

#include "model/ProjectDocument.h"

#include <visage/graphics.h>

#include <cstdint>
#include <optional>

namespace visiform::ui {

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

    void setBounds(float x, float y, float width, float height);
    [[nodiscard]] bool contains(float x, float y) const;
    [[nodiscard]] std::optional<std::string> hitTestWidgetId(const model::ProjectDocument& document, float x, float y) const;
    [[nodiscard]] std::optional<InteractionHit> hitTestInteraction(const model::ProjectDocument& document, float x, float y, const std::string& selectedWidgetId) const;
    [[nodiscard]] std::optional<FormPoint> toFormPoint(const model::ProjectDocument& document, float x, float y) const;
    [[nodiscard]] model::Rect moveBounds(const model::Rect& originalBounds, const FormPoint& dragStart, const FormPoint& currentPoint) const;
    [[nodiscard]] model::Rect resizeBounds(const model::Rect& originalBounds, HitRegion region, const FormPoint& dragStart, const FormPoint& currentPoint) const;
    void draw(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::ProjectDocument& document) const;

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
    float handleSize_ = 8.0f;
    float minimumWidgetSize_ = 20.0f;
};

} // namespace visiform::ui
