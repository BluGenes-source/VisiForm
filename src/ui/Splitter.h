#pragma once

#include <algorithm>
#include <cmath>

namespace visage {
class Canvas;
}

namespace visiform::ui {

class Splitter {
public:
    enum class Orientation {
        Vertical,
        Horizontal
    };

    struct Bounds {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;

        [[nodiscard]] bool contains(float px, float py) const
        {
            return px >= x && py >= y && px <= x + width && py <= y + height;
        }
    };

    void setBounds(float x, float y, float width, float height)
    {
        bounds_ = { x, y, std::max(0.0f, width), std::max(0.0f, height) };
        clampSplitPosition();
    }

    void setOrientation(Orientation orientation)
    {
        orientation_ = orientation;
        clampSplitPosition();
    }

    [[nodiscard]] Orientation orientation() const
    {
        return orientation_;
    }

    void setMinimumFirstPaneSize(float size)
    {
        minimumFirstPaneSize_ = std::max(0.0f, size);
        clampSplitPosition();
    }

    void setMinimumSecondPaneSize(float size)
    {
        minimumSecondPaneSize_ = std::max(0.0f, size);
        clampSplitPosition();
    }

    void setDividerThickness(float size)
    {
        dividerThickness_ = std::max(1.0f, size);
        clampSplitPosition();
    }

    void setHitThickness(float size)
    {
        hitThickness_ = std::max(dividerThickness_, size);
    }

    void setSplitPosition(float size)
    {
        splitPosition_ = size;
        clampSplitPosition();
    }

    [[nodiscard]] float splitPosition() const
    {
        return splitPosition_;
    }

    [[nodiscard]] float firstPaneSize() const
    {
        return clampedSplitPosition();
    }

    [[nodiscard]] float secondPaneSize() const
    {
        return std::max(0.0f, availableMainAxis() - clampedSplitPosition());
    }

    [[nodiscard]] Bounds bounds() const
    {
        return bounds_;
    }

    [[nodiscard]] Bounds firstPaneBounds() const
    {
        const float firstSize = clampedSplitPosition();
        if (orientation_ == Orientation::Vertical) {
            return { bounds_.x, bounds_.y, firstSize, bounds_.height };
        }

        return { bounds_.x, bounds_.y, bounds_.width, firstSize };
    }

    [[nodiscard]] Bounds secondPaneBounds() const
    {
        const float firstSize = clampedSplitPosition();
        if (orientation_ == Orientation::Vertical) {
            return {
                bounds_.x + firstSize + dividerThickness_,
                bounds_.y,
                std::max(0.0f, bounds_.width - firstSize - dividerThickness_),
                bounds_.height
            };
        }

        return {
            bounds_.x,
            bounds_.y + firstSize + dividerThickness_,
            bounds_.width,
            std::max(0.0f, bounds_.height - firstSize - dividerThickness_)
        };
    }

    [[nodiscard]] Bounds dividerBounds() const
    {
        const float firstSize = clampedSplitPosition();
        if (orientation_ == Orientation::Vertical) {
            return {
                bounds_.x + firstSize,
                bounds_.y,
                dividerThickness_,
                bounds_.height
            };
        }

        return {
            bounds_.x,
            bounds_.y + firstSize,
            bounds_.width,
            dividerThickness_
        };
    }

    [[nodiscard]] Bounds hitBounds() const
    {
        const Bounds divider = dividerBounds();
        const float extra = std::max(0.0f, hitThickness_ - dividerThickness_) * 0.5f;
        if (orientation_ == Orientation::Vertical) {
            return {
                divider.x - extra,
                divider.y,
                divider.width + extra * 2.0f,
                divider.height
            };
        }

        return {
            divider.x,
            divider.y - extra,
            divider.width,
            divider.height + extra * 2.0f
        };
    }

    [[nodiscard]] bool contains(float x, float y) const
    {
        return bounds_.contains(x, y);
    }

    [[nodiscard]] bool isPointOverDivider(float x, float y) const
    {
        return hitBounds().contains(x, y);
    }

    [[nodiscard]] bool isDragging() const
    {
        return dragging_;
    }

    [[nodiscard]] bool setHovered(bool hovered)
    {
        const bool changed = hovered_ != hovered;
        hovered_ = hovered;
        return changed;
    }

    [[nodiscard]] bool mouseDown(float x, float y)
    {
        if (!isPointOverDivider(x, y)) {
            return false;
        }

        dragging_ = true;
        dragPointerOffset_ = leadingCoordinate(x, y) - dividerLeadingCoordinate();
        return true;
    }

    [[nodiscard]] bool mouseDrag(float x, float y)
    {
        if (!dragging_) {
            return false;
        }

        const float before = clampedSplitPosition();
        splitPosition_ = leadingCoordinate(x, y) - leadingOrigin() - dragPointerOffset_;
        clampSplitPosition();
        return std::abs(clampedSplitPosition() - before) > 0.01f;
    }

    [[nodiscard]] bool mouseUp()
    {
        const bool wasDragging = dragging_;
        dragging_ = false;
        dragPointerOffset_ = 0.0f;
        return wasDragging;
    }

    void draw(visage::Canvas& canvas) const;

private:
    [[nodiscard]] float leadingOrigin() const
    {
        return orientation_ == Orientation::Vertical ? bounds_.x : bounds_.y;
    }

    [[nodiscard]] float leadingCoordinate(float x, float y) const
    {
        return orientation_ == Orientation::Vertical ? x : y;
    }

    [[nodiscard]] float dividerLeadingCoordinate() const
    {
        return leadingOrigin() + clampedSplitPosition();
    }

    [[nodiscard]] float totalMainAxis() const
    {
        return orientation_ == Orientation::Vertical ? bounds_.width : bounds_.height;
    }

    [[nodiscard]] float availableMainAxis() const
    {
        return std::max(0.0f, totalMainAxis() - dividerThickness_);
    }

    [[nodiscard]] float minimumSplitPosition() const
    {
        const float available = availableMainAxis();
        const float firstMinimum = std::clamp(minimumFirstPaneSize_, 0.0f, available);
        const float secondMinimum = std::clamp(minimumSecondPaneSize_, 0.0f, available);
        const float maximum = std::max(0.0f, available - secondMinimum);
        return std::min(firstMinimum, maximum);
    }

    [[nodiscard]] float maximumSplitPosition() const
    {
        const float available = availableMainAxis();
        const float secondMinimum = std::clamp(minimumSecondPaneSize_, 0.0f, available);
        return std::max(0.0f, available - secondMinimum);
    }

    [[nodiscard]] float clampedSplitPosition() const
    {
        return std::clamp(splitPosition_, minimumSplitPosition(), maximumSplitPosition());
    }

    void clampSplitPosition()
    {
        splitPosition_ = clampedSplitPosition();
    }

    Orientation orientation_ = Orientation::Vertical;
    Bounds bounds_{};
    float minimumFirstPaneSize_ = 0.0f;
    float minimumSecondPaneSize_ = 0.0f;
    float dividerThickness_ = 6.0f;
    float hitThickness_ = 14.0f;
    float splitPosition_ = 0.0f;
    float dragPointerOffset_ = 0.0f;
    bool dragging_ = false;
    bool hovered_ = false;
};

} // namespace visiform::ui
