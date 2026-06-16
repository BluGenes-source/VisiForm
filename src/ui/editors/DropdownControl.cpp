#include "ui/editors/DropdownControl.h"

#include <algorithm>

namespace visiform::ui::editors {
namespace {

constexpr float kItemHeight = 24.0f;
constexpr float kItemSpacing = 2.0f;
constexpr float kMinimumPopupWidth = 180.0f;
constexpr std::size_t kMaxVisibleItems = 8;

float totalPopupHeight(std::size_t itemCount)
{
    if (itemCount == 0) {
        return 0.0f;
    }

    return static_cast<float>(itemCount) * kItemHeight
        + static_cast<float>(itemCount - 1) * kItemSpacing;
}

} // namespace

void DropdownControl::open(std::string key,
    const Bounds& anchorBounds,
    const Bounds& viewportBounds,
    std::vector<Item> items,
    const std::string& selectedValue)
{
    activeKey_ = std::move(key);
    anchorBounds_ = anchorBounds;
    viewportBounds_ = viewportBounds;
    items_ = std::move(items);
    scrollOffset_ = 0;
    pendingSelection_.reset();
    open_ = !items_.empty();
    selectedIndex_ = 0;

    const auto iterator = std::find_if(items_.begin(), items_.end(), [&selectedValue](const Item& item) {
        return item.value == selectedValue;
    });
    if (iterator != items_.end()) {
        selectedIndex_ = static_cast<std::size_t>(std::distance(items_.begin(), iterator));
    }

    clampScrollOffset();
    if (selectedIndex_ >= scrollOffset_ + visibleItemCount()) {
        scrollOffset_ = selectedIndex_ - visibleItemCount() + 1;
    }
}

void DropdownControl::close()
{
    open_ = false;
    activeKey_.clear();
    items_.clear();
    selectedIndex_ = 0;
    scrollOffset_ = 0;
}

bool DropdownControl::isOpen() const
{
    return open_;
}

bool DropdownControl::isOpenFor(const std::string& key) const
{
    return open_ && activeKey_ == key;
}

const std::string& DropdownControl::activeKey() const
{
    return activeKey_;
}

bool DropdownControl::contains(float x, float y) const
{
    return open_ && popupBounds().contains(x, y);
}

bool DropdownControl::mouseDown(float x, float y)
{
    if (!open_) {
        return false;
    }

    const auto hitIndex = hitTestIndex(x, y);
    if (!hitIndex.has_value()) {
        close();
        return false;
    }

    const auto& item = items_[*hitIndex];
    pendingSelection_ = Selection{ activeKey_, item.value, item.label, item.hint };
    selectedIndex_ = *hitIndex;
    close();
    return true;
}

bool DropdownControl::mouseWheel(float deltaY, float x, float y)
{
    if (!contains(x, y) || items_.size() <= visibleItemCount()) {
        return false;
    }

    if (deltaY > 0.0f && scrollOffset_ > 0) {
        --scrollOffset_;
    }
    else if (deltaY < 0.0f && scrollOffset_ + visibleItemCount() < items_.size()) {
        ++scrollOffset_;
    }
    return true;
}

bool DropdownControl::keyPress(const visage::KeyEvent& event)
{
    if (!open_ || items_.empty()) {
        return false;
    }

    using KeyCode = visage::KeyCode;
    switch (event.keyCode()) {
    case KeyCode::Up:
        if (selectedIndex_ > 0) {
            --selectedIndex_;
            if (selectedIndex_ < scrollOffset_) {
                scrollOffset_ = selectedIndex_;
            }
        }
        return true;
    case KeyCode::Down:
        if (selectedIndex_ + 1 < items_.size()) {
            ++selectedIndex_;
            if (selectedIndex_ >= scrollOffset_ + visibleItemCount()) {
                scrollOffset_ = selectedIndex_ - visibleItemCount() + 1;
            }
        }
        return true;
    case KeyCode::Return: {
        const auto& item = items_[clampedSelectedIndex()];
        pendingSelection_ = Selection{ activeKey_, item.value, item.label, item.hint };
        close();
        return true;
    }
    case KeyCode::Escape:
        close();
        return true;
    default:
        return false;
    }
}

std::optional<DropdownControl::Selection> DropdownControl::consumeSelection()
{
    if (!pendingSelection_.has_value()) {
        return std::nullopt;
    }

    auto selection = pendingSelection_;
    pendingSelection_.reset();
    return selection;
}

std::optional<std::string> DropdownControl::hintAt(float x, float y) const
{
    const auto index = hitTestIndex(x, y);
    if (!index.has_value()) {
        return std::nullopt;
    }

    if (items_[*index].hint.empty()) {
        return std::nullopt;
    }

    return items_[*index].hint;
}

void DropdownControl::draw(visage::Canvas& canvas, const visage::Font& font, bool drawText) const
{
    if (!open_ || items_.empty()) {
        return;
    }

    const Bounds popup = popupBounds();
    if (popup.width <= 0.0f || popup.height <= 0.0f) {
        return;
    }

    canvas.saveState();
    canvas.setClampBounds(popup.x, popup.y, popup.width, popup.height);
    canvas.setColor(0xff1f2630);
    canvas.fill(popup.x, popup.y, popup.width, popup.height);
    canvas.setColor(0xff11141a);
    canvas.fill(popup.x, popup.y, popup.width, 1.0f);
    canvas.fill(popup.x, popup.y + popup.height - 1.0f, popup.width, 1.0f);
    canvas.fill(popup.x, popup.y, 1.0f, popup.height);
    canvas.fill(popup.x + popup.width - 1.0f, popup.y, 1.0f, popup.height);

    const std::size_t visibleCount = visibleItemCount();
    float top = popup.y;
    for (std::size_t visibleIndex = 0; visibleIndex < visibleCount; ++visibleIndex) {
        const std::size_t itemIndex = scrollOffset_ + visibleIndex;
        const bool active = itemIndex == clampedSelectedIndex();
        canvas.setColor(active ? 0xff355382 : 0xff314055);
        canvas.fill(popup.x + 1.0f, top, popup.width - 2.0f, kItemHeight);
        if (drawText) {
            canvas.setColor(0xffeef2f8);
            canvas.text(items_[itemIndex].label, font, visage::Font::kTopLeft,
                popup.x + 8.0f, top + 4.0f, popup.width - 14.0f, kItemHeight - 6.0f);
        }
        top += kItemHeight + kItemSpacing;
    }
    canvas.restoreState();
}

DropdownControl::Bounds DropdownControl::popupBounds() const
{
    const std::size_t count = visibleItemCount();
    const float popupHeight = totalPopupHeight(count);
    const float popupWidth = std::min(std::max(anchorBounds_.width, kMinimumPopupWidth), viewportBounds_.width);
    const float popupX = std::clamp(anchorBounds_.x,
        viewportBounds_.x,
        std::max(viewportBounds_.x, viewportBounds_.x + viewportBounds_.width - popupWidth));
    const float spaceBelow = std::max(0.0f, viewportBounds_.y + viewportBounds_.height - (anchorBounds_.y + anchorBounds_.height));
    const float spaceAbove = std::max(0.0f, anchorBounds_.y - viewportBounds_.y);
    const bool drawAbove = popupHeight > spaceBelow && spaceAbove > spaceBelow;
    const float popupY = drawAbove
        ? std::max(viewportBounds_.y, anchorBounds_.y - popupHeight - 2.0f)
        : std::min(viewportBounds_.y + viewportBounds_.height - popupHeight, anchorBounds_.y + anchorBounds_.height);
    return {
        popupX,
        popupY,
        popupWidth,
        popupHeight
    };
}

std::size_t DropdownControl::visibleItemCount() const
{
    return std::min<std::size_t>(kMaxVisibleItems, items_.size());
}

std::size_t DropdownControl::clampedSelectedIndex() const
{
    if (items_.empty()) {
        return 0;
    }

    return std::min(selectedIndex_, items_.size() - 1);
}

std::optional<std::size_t> DropdownControl::hitTestIndex(float x, float y) const
{
    if (!open_) {
        return std::nullopt;
    }

    const Bounds popup = popupBounds();
    if (!popup.contains(x, y)) {
        return std::nullopt;
    }

    float top = popup.y;
    const std::size_t count = visibleItemCount();
    for (std::size_t visibleIndex = 0; visibleIndex < count; ++visibleIndex) {
        const Bounds rowBounds{ popup.x, top, popup.width, kItemHeight };
        if (rowBounds.contains(x, y)) {
            return scrollOffset_ + visibleIndex;
        }
        top += kItemHeight + kItemSpacing;
    }

    return std::nullopt;
}

void DropdownControl::clampScrollOffset()
{
    const std::size_t visibleCount = visibleItemCount();
    if (items_.size() <= visibleCount) {
        scrollOffset_ = 0;
        return;
    }

    scrollOffset_ = std::min(scrollOffset_, items_.size() - visibleCount);
}

} // namespace visiform::ui::editors
