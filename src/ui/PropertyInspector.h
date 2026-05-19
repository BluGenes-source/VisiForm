#pragma once

#include "model/ProjectDocument.h"

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
        Color,
        Choice,
        Bool,
        ReadOnly
    };

    struct PropertyRow {
        std::string key;
        std::string label;
        std::string displayValue;
        PropertyEditKind editKind = PropertyEditKind::ReadOnly;
        bool isSection = false;
        std::vector<std::string> choices{};
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
    [[nodiscard]] std::optional<PropertyRow> hitTestRow(const model::ProjectDocument& document, float x, float y);
    [[nodiscard]] std::optional<std::string> hitTestSuggestion(const model::ProjectDocument& document, float x, float y);
    [[nodiscard]] std::optional<std::string> hitTestColorSwatch(const model::ProjectDocument& document, float x, float y);
    [[nodiscard]] bool mouseDown(const model::ProjectDocument& document, float x, float y);
    [[nodiscard]] bool mouseDrag(const model::ProjectDocument& document, float x, float y);
    [[nodiscard]] bool mouseUp();
    [[nodiscard]] bool mouseWheel(const model::ProjectDocument& document, float deltaY, float x, float y);
    [[nodiscard]] bool beginEditing(const model::ProjectDocument& document, const std::string& key);
    [[nodiscard]] std::optional<PropertyRow> activeRow(const model::ProjectDocument& document) const;
    [[nodiscard]] std::optional<ValueCellBounds> activeEditorBounds(const model::ProjectDocument& document);
    [[nodiscard]] std::optional<PendingEdit> buildPendingEdit(const std::string& valueText) const;
    void clearEditing();
    void cancelEditing();
    [[nodiscard]] bool isEditing() const;
    void draw(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::ProjectDocument& document, std::size_t selectionCount = 0);

private:
    [[nodiscard]] std::vector<PropertyRow> buildRows(const model::ProjectDocument& document) const;
    void rebuildSuggestions(const model::ProjectDocument& document);
    void updateScrollMetrics(const std::vector<PropertyRow>& rows);
    void clampScrollOffset();
    [[nodiscard]] float rowYWithScroll(float originalY) const;
    [[nodiscard]] ValueCellBounds contentBounds() const;
    [[nodiscard]] std::optional<ValueCellBounds> scrollBarBounds() const;
    [[nodiscard]] std::optional<ValueCellBounds> scrollBarThumbBounds() const;
    [[nodiscard]] std::optional<ValueCellBounds> colorSwatchBoundsForRow(const PropertyRow& row, float rowTop) const;
    [[nodiscard]] bool isWithinVisibleContent(float x, float y) const;
    [[nodiscard]] float valueCellWidth() const;

    struct CallbackSuggestionItem {
        std::string value;
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    std::vector<CallbackSuggestionItem> suggestions_{};
    std::string activeCallbackPropertyKey_{};

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
    std::string activeKey_{};
    std::string editBuffer_{};
    PropertyEditKind activeEditKind_ = PropertyEditKind::ReadOnly;
};

} // namespace visiform::ui
