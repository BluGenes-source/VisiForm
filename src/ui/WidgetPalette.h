#pragma once

#pragma once

#include "model/WidgetNode.h"

#include <visage/graphics.h>

#include <optional>
#include <string>

namespace visiform::ui {

class WidgetPalette {
public:
    void setBounds(float x, float y, float width, float height);
    [[nodiscard]] bool contains(float x, float y) const;
    [[nodiscard]] bool mouseDown(float x, float y);
    [[nodiscard]] bool mouseDrag(float x, float y);
    [[nodiscard]] bool mouseUp();
    [[nodiscard]] bool mouseWheel(float deltaY, float x, float y);
    [[nodiscard]] std::optional<model::WidgetType> hitTestWidgetType(float x, float y) const;
    [[nodiscard]] std::optional<std::string> hitTestHint(float x, float y) const;
    void draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const;

private:
    void updateScrollMetrics() const;
    void clampScrollOffset();
    [[nodiscard]] bool setScrollOffsetY(float newScrollOffsetY);
    [[nodiscard]] model::Rect contentBounds() const;
    [[nodiscard]] std::optional<model::Rect> scrollBarBounds() const;
    [[nodiscard]] std::optional<model::Rect> scrollBarThumbBounds() const;
    [[nodiscard]] bool isWithinVisibleContent(float x, float y) const;

    float x_{};
    float y_{};
    float width_{};
    float height_{};
    mutable float scrollOffsetY_ = 0.0f;
    mutable float contentHeight_ = 0.0f;
    mutable float visibleHeight_ = 0.0f;
    mutable bool needsVerticalScrollBar_ = false;
    bool draggingScrollBarThumb_ = false;
    float scrollBarDragOffsetY_ = 0.0f;
};

} // namespace visiform::ui
