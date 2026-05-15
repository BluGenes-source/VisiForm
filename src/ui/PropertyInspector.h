#pragma once

#pragma once

#include "model/WidgetNode.h"

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
        Bool,
        ReadOnly
    };

    struct PropertyRow {
        std::string key;
        std::string label;
        std::string displayValue;
        PropertyEditKind editKind = PropertyEditKind::ReadOnly;
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
    [[nodiscard]] std::optional<PropertyRow> hitTestRow(const model::WidgetNode* selectedWidget, float x, float y) const;
    [[nodiscard]] bool beginEditing(const model::WidgetNode* selectedWidget, const std::string& key);
    [[nodiscard]] std::optional<PropertyRow> activeRow(const model::WidgetNode* selectedWidget) const;
    [[nodiscard]] std::optional<ValueCellBounds> activeEditorBounds(const model::WidgetNode* selectedWidget) const;
    [[nodiscard]] std::optional<PendingEdit> buildPendingEdit(const std::string& valueText) const;
    void clearEditing();
    void cancelEditing();
    [[nodiscard]] bool isEditing() const;
    void draw(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::WidgetNode* selectedWidget) const;

private:
    [[nodiscard]] std::vector<PropertyRow> buildRows(const model::WidgetNode* selectedWidget) const;

    float x_{};
    float y_{};
    float width_{};
    float height_{};
    std::string activeKey_{};
    std::string editBuffer_{};
    PropertyEditKind activeEditKind_ = PropertyEditKind::ReadOnly;
};

} // namespace visiform::ui
