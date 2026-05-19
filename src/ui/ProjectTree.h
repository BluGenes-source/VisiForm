#pragma once

#include "model/ProjectDocument.h"

#include <filesystem>
#include <visage/graphics.h>

#include <optional>
#include <vector>

namespace visiform::ui {

class ProjectTree {
public:
    void setBounds(float x, float y, float width, float height);
    void setRecentFiles(std::vector<std::filesystem::path> recentFiles);
    [[nodiscard]] bool contains(float x, float y) const;
    [[nodiscard]] std::optional<std::string> hitTestWidgetId(const model::ProjectDocument& document, float x, float y);
    [[nodiscard]] std::optional<std::size_t> hitTestRecentFileIndex(const model::ProjectDocument& document, float x, float y);
    [[nodiscard]] bool mouseDown(const model::ProjectDocument& document, float x, float y);
    [[nodiscard]] bool mouseDrag(const model::ProjectDocument& document, float x, float y);
    [[nodiscard]] bool mouseUp();
    [[nodiscard]] bool mouseWheel(const model::ProjectDocument& document, float deltaY, float x, float y);
    void drawPanel(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::ProjectDocument& document);

private:
    struct Bounds {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    void updateScrollMetrics(const model::ProjectDocument& document);
    void clampScrollOffset();
    [[nodiscard]] float rowYWithScroll(float originalY) const;
    [[nodiscard]] Bounds contentBounds() const;
    [[nodiscard]] std::optional<Bounds> scrollBarBounds() const;
    [[nodiscard]] std::optional<Bounds> scrollBarThumbBounds() const;
    [[nodiscard]] bool isWithinVisibleContent(float x, float y) const;

    float x_{};
    float y_{};
    float width_{};
    float height_{};
    std::vector<std::filesystem::path> recentFiles_{};
    float scrollOffsetY_ = 0.0f;
    float contentHeight_ = 0.0f;
    float visibleHeight_ = 0.0f;
    bool needsVerticalScrollBar_ = false;
    bool draggingScrollBarThumb_ = false;
    float scrollBarDragOffsetY_ = 0.0f;
};

} // namespace visiform::ui
