#pragma once

#include "model/ProjectDocument.h"
#include "utils/AppSettings.h"

#include <visage/graphics.h>

#include <optional>
#include <string>
#include <vector>

namespace visiform::ui {

class PropertyInspector {
public:
    enum class PropertyEditKind {
        Text,
        Integer,
        Float,
        Slider,
        Color,
        Choice,
        Bool,
        ReadOnly
    };

    struct PropertyChoice {
        std::string value;
        std::string label;
        std::string hint;
    };

    struct PropertyRow {
        std::string key;
        std::string label;
        std::string hint;
        std::string displayValue;
        PropertyEditKind editKind = PropertyEditKind::ReadOnly;
        bool isSection = false;
        std::vector<PropertyChoice> choices{};
        float minimumValue = 0.0f;
        float maximumValue = 0.0f;
        float stepValue = 1.0f;
        std::string actionText{};
    };

    struct PendingEdit {
        std::string key;
        std::string valueText;
        PropertyEditKind editKind = PropertyEditKind::ReadOnly;
    };

    struct ValueCellBounds {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    void setBounds(float x, float y, float width, float height);
    [[nodiscard]] bool contains(float x, float y) const;
    [[nodiscard]] std::optional<PropertyRow> hitTestRow(const model::ProjectDocument& document, const utils::AppSettings& settings, float x, float y);
    [[nodiscard]] std::optional<std::string> hitTestColorSwatch(const model::ProjectDocument& document, const utils::AppSettings& settings, float x, float y);
    [[nodiscard]] bool mouseDown(const model::ProjectDocument& document, const utils::AppSettings& settings, float x, float y);
    [[nodiscard]] bool mouseDrag(const model::ProjectDocument& document, const utils::AppSettings& settings, float x, float y);
    [[nodiscard]] bool mouseUp();
    [[nodiscard]] bool mouseWheel(const model::ProjectDocument& document, const utils::AppSettings& settings, float deltaY, float x, float y);
    [[nodiscard]] bool beginEditing(const model::ProjectDocument& document, const utils::AppSettings& settings, const std::string& key);
    [[nodiscard]] std::optional<PropertyRow> activeRow(const model::ProjectDocument& document, const utils::AppSettings& settings) const;
    [[nodiscard]] std::optional<ValueCellBounds> activeEditorBounds(const model::ProjectDocument& document, const utils::AppSettings& settings);
    [[nodiscard]] std::optional<PendingEdit> buildPendingEdit(const std::string& valueText) const;
    [[nodiscard]] std::optional<PendingEdit> consumeInteractionEdit();
    [[nodiscard]] bool consumeScrollInteraction();
    void clearEditing();
    void cancelEditing();
    [[nodiscard]] bool isEditing() const;
    void draw(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::ProjectDocument& document, const utils::AppSettings& settings, std::size_t selectionCount = 0);

private:
    [[nodiscard]] std::vector<PropertyRow> buildRows(const model::ProjectDocument& document, const utils::AppSettings& settings) const;
    void updateScrollMetrics(const std::vector<PropertyRow>& rows);
    void clampScrollOffset();
    [[nodiscard]] float rowYWithScroll(float originalY) const;
    [[nodiscard]] ValueCellBounds contentBounds() const;
    [[nodiscard]] std::optional<ValueCellBounds> scrollBarBounds() const;
    [[nodiscard]] std::optional<ValueCellBounds> scrollBarThumbBounds() const;
    [[nodiscard]] std::optional<ValueCellBounds> colorSwatchBoundsForRow(const PropertyRow& row, float rowTop) const;
    [[nodiscard]] std::optional<ValueCellBounds> sliderTrackBoundsForRow(const PropertyRow& row, float rowTop) const;
    [[nodiscard]] std::optional<ValueCellBounds> sliderThumbBoundsForRow(const PropertyRow& row, float rowTop, float value) const;
    [[nodiscard]] bool isWithinVisibleContent(float x, float y) const;
    [[nodiscard]] float labelColumnWidth() const;
    [[nodiscard]] float valueCellWidth() const;
    [[nodiscard]] std::optional<PendingEdit> sliderEditAtPoint(const std::vector<PropertyRow>& rows, float x, float y);

    float x_{};
    float y_{};
    float width_{};
    float height_{};
    float scrollOffsetY_ = 0.0f;
    float contentHeight_ = 0.0f;
    float visibleHeight_ = 0.0f;
    bool needsVerticalScrollBar_ = false;
    bool draggingScrollBarThumb_ = false;
    float scrollBarDragOffsetY_ = 0.0f;
    bool draggingSlider_ = false;
    std::string draggingSliderKey_{};
    std::string activeKey_{};
    std::string editBuffer_{};
    PropertyEditKind activeEditKind_ = PropertyEditKind::ReadOnly;
    std::optional<PendingEdit> pendingInteractionEdit_{};
    bool pendingScrollInteraction_ = false;
};

} // namespace visiform::ui
