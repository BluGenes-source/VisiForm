#include "ui/ProjectTree.h"

#include "utils/FileUtils.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kPadding = 12.0f;
constexpr float kRowHeight = 24.0f;
constexpr float kSectionSpacing = 12.0f;
constexpr float kSectionHeaderHeight = 22.0f;
constexpr float kScrollBarWidth = 16.0f;
constexpr float kScrollBarGap = 6.0f;
constexpr float kMinimumThumbSize = 20.0f;
constexpr float kMouseWheelSensitivity = 28.0f;

struct TreeEntry {
    enum class Kind {
        Widget,
        Section,
        RecentFile
    };

    Kind kind = Kind::Widget;
    std::string widgetId;
    std::string label;
    std::size_t recentIndex = 0;
    int depth = 0;
    float height = kRowHeight;
};

struct TreeLayout {
    TreeEntry entry{};
    float top = 0.0f;
};

struct LayoutBounds {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

[[nodiscard]] bool containsPoint(const LayoutBounds& bounds, float x, float y)
{
    return x >= bounds.x && y >= bounds.y && x <= bounds.x + bounds.width && y <= bounds.y + bounds.height;
}

void appendRows(const model::WidgetNode& widget, int depth, std::vector<TreeEntry>& rows)
{
    const std::string displayName = widget.name.empty() ? widget.id : widget.name;
    rows.push_back({ TreeEntry::Kind::Widget, widget.id, displayName + " [" + widget.typeName() + "]", 0, depth, kRowHeight });
    for (const auto& child : widget.children) {
        appendRows(child, depth + 1, rows);
    }
}

std::vector<TreeEntry> buildEntries(const model::ProjectDocument& document, const std::vector<std::filesystem::path>& recentFiles)
{
    std::vector<TreeEntry> entries;
    appendRows(document.root, 0, entries);
    if (!recentFiles.empty()) {
        entries.push_back({ TreeEntry::Kind::Section, {}, "Recent Files", 0, 0, kSectionSpacing + kSectionHeaderHeight });
        for (std::size_t index = 0; index < recentFiles.size(); ++index) {
            entries.push_back({ TreeEntry::Kind::RecentFile, {}, {}, index, 0, kRowHeight });
        }
    }
    return entries;
}

std::vector<TreeLayout> buildLayouts(float top, const std::vector<TreeEntry>& entries)
{
    std::vector<TreeLayout> layouts;
    layouts.reserve(entries.size());
    float rowTop = top;
    for (const auto& entry : entries) {
        layouts.push_back({ entry, rowTop });
        rowTop += entry.height;
    }
    return layouts;
}

std::string recentFileLabel(const std::filesystem::path& path)
{
    const std::string filename = path.filename().string();
    if (!filename.empty()) {
        return filename + " - " + utils::FileUtils::normalizeSeparators(path.parent_path().string());
    }

    return utils::FileUtils::normalizeSeparators(path.string());
}

} // namespace

void ProjectTree::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
    clampScrollOffset();
}

void ProjectTree::setRecentFiles(std::vector<std::filesystem::path> recentFiles)
{
    recentFiles_ = std::move(recentFiles);
    clampScrollOffset();
}

bool ProjectTree::contains(float x, float y) const
{
    return x >= x_ && y >= y_ && x <= x_ + width_ && y <= y_ + height_;
}

void ProjectTree::updateScrollMetrics(const model::ProjectDocument& document)
{
    visibleHeight_ = std::max(0.0f, height_ - (kHeaderHeight + 16.0f));
    const auto entries = buildEntries(document, recentFiles_);
    contentHeight_ = 0.0f;
    for (const auto& entry : entries) {
        contentHeight_ += entry.height;
    }

    needsVerticalScrollBar_ = contentHeight_ > visibleHeight_ + 0.5f;
    clampScrollOffset();
}

void ProjectTree::clampScrollOffset()
{
    const float maxScroll = std::max(0.0f, contentHeight_ - visibleHeight_);
    scrollOffsetY_ = std::clamp(scrollOffsetY_, 0.0f, maxScroll);
    if (!needsVerticalScrollBar_ || maxScroll <= 0.0f) {
        scrollOffsetY_ = 0.0f;
    }
}

float ProjectTree::rowYWithScroll(float originalY) const
{
    return originalY - scrollOffsetY_;
}

ProjectTree::Bounds ProjectTree::contentBounds() const
{
    return {
        x_ + 8.0f,
        y_ + kHeaderHeight + 8.0f,
        std::max(0.0f, width_ - 16.0f - (needsVerticalScrollBar_ ? (kScrollBarWidth + kScrollBarGap) : 0.0f)),
        visibleHeight_
    };
}

std::optional<ProjectTree::Bounds> ProjectTree::scrollBarBounds() const
{
    if (!needsVerticalScrollBar_ || visibleHeight_ <= 0.0f) {
        return std::nullopt;
    }

    return Bounds{
        x_ + width_ - 8.0f - kScrollBarWidth,
        y_ + kHeaderHeight + 8.0f,
        kScrollBarWidth,
        visibleHeight_
    };
}

std::optional<ProjectTree::Bounds> ProjectTree::scrollBarThumbBounds() const
{
    const auto scrollBar = scrollBarBounds();
    if (!scrollBar.has_value()) {
        return std::nullopt;
    }

    const float arrowSize = std::min(scrollBar->width, 20.0f);
    const float trackTop = scrollBar->y + arrowSize;
    const float trackHeight = std::max(0.0f, scrollBar->height - arrowSize * 2.0f);
    if (trackHeight <= 0.0f || contentHeight_ <= 0.0f) {
        return std::nullopt;
    }

    const float maxScroll = std::max(0.0f, contentHeight_ - visibleHeight_);
    const float normalized = maxScroll <= 0.0f ? 0.0f : scrollOffsetY_ / maxScroll;
    const float thumbHeight = std::clamp(trackHeight * (visibleHeight_ / contentHeight_), kMinimumThumbSize, trackHeight);
    const float thumbY = trackTop + (trackHeight - thumbHeight) * normalized;
    return Bounds{ scrollBar->x + 4.0f, thumbY, std::max(0.0f, scrollBar->width - 8.0f), thumbHeight };
}

bool ProjectTree::isWithinVisibleContent(float x, float y) const
{
    const Bounds bounds = contentBounds();
    return containsPoint({ bounds.x, bounds.y, bounds.width, bounds.height }, x, y);
}

std::optional<std::string> ProjectTree::hitTestWidgetId(const model::ProjectDocument& document, float x, float y)
{
    updateScrollMetrics(document);
    if (!isWithinVisibleContent(x, y)) {
        return std::nullopt;
    }

    const auto entries = buildEntries(document, recentFiles_);
    const auto layouts = buildLayouts(contentBounds().y, entries);
    const Bounds bounds = contentBounds();
    for (const auto& layout : layouts) {
        const float rowTop = rowYWithScroll(layout.top);
        if (rowTop + layout.entry.height < bounds.y || rowTop > bounds.y + bounds.height) {
            continue;
        }

        if (layout.entry.kind == TreeEntry::Kind::Widget && y >= rowTop && y <= rowTop + layout.entry.height) {
            return layout.entry.widgetId;
        }
    }

    return std::nullopt;
}

std::optional<std::size_t> ProjectTree::hitTestRecentFileIndex(const model::ProjectDocument& document, float x, float y)
{
    updateScrollMetrics(document);
    if (!isWithinVisibleContent(x, y)) {
        return std::nullopt;
    }

    const auto entries = buildEntries(document, recentFiles_);
    const auto layouts = buildLayouts(contentBounds().y, entries);
    const Bounds bounds = contentBounds();
    for (const auto& layout : layouts) {
        const float rowTop = rowYWithScroll(layout.top);
        if (rowTop + layout.entry.height < bounds.y || rowTop > bounds.y + bounds.height) {
            continue;
        }

        if (layout.entry.kind == TreeEntry::Kind::RecentFile && y >= rowTop && y <= rowTop + layout.entry.height) {
            return layout.entry.recentIndex;
        }
    }

    return std::nullopt;
}

bool ProjectTree::mouseDown(const model::ProjectDocument& document, float x, float y)
{
    updateScrollMetrics(document);
    const auto scrollBar = scrollBarBounds();
    if (!scrollBar.has_value() || !containsPoint({ scrollBar->x, scrollBar->y, scrollBar->width, scrollBar->height }, x, y)) {
        return false;
    }

    const float arrowSize = std::min(scrollBar->width, 20.0f);
    const auto thumb = scrollBarThumbBounds();
    if (thumb.has_value() && containsPoint({ thumb->x, thumb->y, thumb->width, thumb->height }, x, y)) {
        draggingScrollBarThumb_ = true;
        scrollBarDragOffsetY_ = y - thumb->y;
        return true;
    }

    if (y < scrollBar->y + arrowSize) {
        scrollOffsetY_ -= kRowHeight;
    }
    else if (y > scrollBar->y + scrollBar->height - arrowSize) {
        scrollOffsetY_ += kRowHeight;
    }
    else if (thumb.has_value() && y < thumb->y) {
        scrollOffsetY_ -= std::max(kRowHeight, visibleHeight_ * 0.85f);
    }
    else {
        scrollOffsetY_ += std::max(kRowHeight, visibleHeight_ * 0.85f);
    }

    clampScrollOffset();
    return true;
}

bool ProjectTree::mouseDrag(const model::ProjectDocument& document, float x, float y)
{
    (void)x;
    updateScrollMetrics(document);
    if (!draggingScrollBarThumb_) {
        return false;
    }

    const auto scrollBar = scrollBarBounds();
    const auto thumb = scrollBarThumbBounds();
    if (!scrollBar.has_value() || !thumb.has_value()) {
        draggingScrollBarThumb_ = false;
        return false;
    }

    const float arrowSize = std::min(scrollBar->width, 20.0f);
    const float trackTop = scrollBar->y + arrowSize;
    const float trackHeight = std::max(0.0f, scrollBar->height - arrowSize * 2.0f);
    const float maxThumbTop = trackTop + std::max(0.0f, trackHeight - thumb->height);
    const float thumbTop = std::clamp(y - scrollBarDragOffsetY_, trackTop, maxThumbTop);
    const float maxScroll = std::max(0.0f, contentHeight_ - visibleHeight_);
    if (trackHeight > thumb->height && maxScroll > 0.0f) {
        scrollOffsetY_ = maxScroll * ((thumbTop - trackTop) / (trackHeight - thumb->height));
    }
    else {
        scrollOffsetY_ = 0.0f;
    }

    clampScrollOffset();
    return true;
}

bool ProjectTree::mouseUp()
{
    const bool wasDragging = draggingScrollBarThumb_;
    draggingScrollBarThumb_ = false;
    scrollBarDragOffsetY_ = 0.0f;
    return wasDragging;
}

bool ProjectTree::mouseWheel(const model::ProjectDocument& document, float deltaY, float x, float y)
{
    if (!contains(x, y)) {
        return false;
    }

    updateScrollMetrics(document);
    if (!needsVerticalScrollBar_) {
        return false;
    }

    scrollOffsetY_ += -deltaY * kMouseWheelSensitivity;
    clampScrollOffset();
    return true;
}

void ProjectTree::drawPanel(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::ProjectDocument& document)
{
    if (width_ <= 0.0f || height_ <= 0.0f) {
        return;
    }

    updateScrollMetrics(document);

    canvas.setColor(0xff232833);
    canvas.fill(x_, y_, width_, height_);

    canvas.setColor(0xff2c3240);
    canvas.fill(x_, y_, width_, kHeaderHeight);

    canvas.setColor(0xff11141a);
    canvas.fill(x_, y_, width_, 1.0f);
    canvas.fill(x_, y_ + height_ - 1.0f, width_, 1.0f);
    canvas.fill(x_, y_, 1.0f, height_);
    canvas.fill(x_ + width_ - 1.0f, y_, 1.0f, height_);

    if (drawText) {
        canvas.setColor(0xfff3f5f8);
        canvas.text("Project Tree", font, visage::Font::kTopLeft,
            x_ + kPadding, y_ + 6.0f, width_ - kPadding * 2.0f, kHeaderHeight - 8.0f);
    }

    const auto entries = buildEntries(document, recentFiles_);
    const auto layouts = buildLayouts(contentBounds().y, entries);
    const Bounds bounds = contentBounds();

    canvas.saveState();
    canvas.setClampBounds(bounds.x, bounds.y, bounds.width, bounds.height);
    for (const auto& layout : layouts) {
        const float rowTop = rowYWithScroll(layout.top);
        if (rowTop + layout.entry.height < bounds.y || rowTop > bounds.y + bounds.height) {
            continue;
        }

        switch (layout.entry.kind) {
        case TreeEntry::Kind::Widget: {
            const bool isPrimarySelected = document.selectedWidgetId == layout.entry.widgetId;
            const bool isSecondarySelected = document.isSelected(layout.entry.widgetId) && !isPrimarySelected;
            canvas.setColor(isPrimarySelected ? 0xff355382 : (isSecondarySelected ? 0xff2d4668 : 0xff252b36));
            canvas.fill(bounds.x, rowTop, bounds.width, layout.entry.height - 2.0f);
            if (drawText) {
                canvas.setColor(isPrimarySelected ? 0xfff8fbff : (isSecondarySelected ? 0xffd9ebff : 0xffdde2ea));
                canvas.text(layout.entry.label, font, visage::Font::kTopLeft,
                    bounds.x + 8.0f + layout.entry.depth * 16.0f, rowTop + 4.0f,
                    bounds.width - 20.0f - layout.entry.depth * 16.0f, layout.entry.height - 6.0f);
            }
            break;
        }
        case TreeEntry::Kind::Section:
            if (drawText) {
                canvas.setColor(0xfff3f5f8);
                canvas.text(layout.entry.label, font, visage::Font::kTopLeft,
                    bounds.x + 4.0f, rowTop + kSectionSpacing, bounds.width - 8.0f, kSectionHeaderHeight - 4.0f);
            }
            break;
        case TreeEntry::Kind::RecentFile:
            canvas.setColor(0xff252b36);
            canvas.fill(bounds.x, rowTop, bounds.width, layout.entry.height - 2.0f);
            if (drawText && layout.entry.recentIndex < recentFiles_.size()) {
                canvas.setColor(0xffcfd6e2);
                canvas.text(recentFileLabel(recentFiles_[layout.entry.recentIndex]), font, visage::Font::kTopLeft,
                    bounds.x + 8.0f, rowTop + 4.0f, bounds.width - 16.0f, layout.entry.height - 6.0f);
            }
            break;
        }
    }
    canvas.restoreState();

    const auto scrollBar = scrollBarBounds();
    if (scrollBar.has_value()) {
        const float arrowSize = std::min(scrollBar->width, 20.0f);
        const float trackTop = scrollBar->y + arrowSize;
        const float trackHeight = std::max(0.0f, scrollBar->height - arrowSize * 2.0f);
        const auto thumb = scrollBarThumbBounds();

        canvas.setColor(0xff39414f);
        canvas.fill(scrollBar->x, scrollBar->y, scrollBar->width, scrollBar->height);
        canvas.setColor(0xff11141a);
        canvas.fill(scrollBar->x, scrollBar->y, scrollBar->width, 1.0f);
        canvas.fill(scrollBar->x, scrollBar->y + scrollBar->height - 1.0f, scrollBar->width, 1.0f);
        canvas.fill(scrollBar->x, scrollBar->y, 1.0f, scrollBar->height);
        canvas.fill(scrollBar->x + scrollBar->width - 1.0f, scrollBar->y, 1.0f, scrollBar->height);

        canvas.setColor(0xff2b313d);
        canvas.fill(scrollBar->x, scrollBar->y, scrollBar->width, arrowSize);
        canvas.fill(scrollBar->x, scrollBar->y + scrollBar->height - arrowSize, scrollBar->width, arrowSize);
        canvas.setColor(0xff1d222b);
        canvas.fill(scrollBar->x + 2.0f, trackTop, scrollBar->width - 4.0f, trackHeight);

        canvas.setColor(0xff92b9ff);
        canvas.fill(scrollBar->x + scrollBar->width * 0.5f - 3.0f, scrollBar->y + 6.0f, 6.0f, 3.0f);
        canvas.fill(scrollBar->x + scrollBar->width * 0.5f - 3.0f, scrollBar->y + scrollBar->height - 9.0f, 6.0f, 3.0f);

        if (thumb.has_value()) {
            canvas.setColor(draggingScrollBarThumb_ ? 0xff92b9ff : 0xff4b79bc);
            canvas.fill(thumb->x, thumb->y, thumb->width, thumb->height);
            canvas.setColor(0xff1d2a3c);
            canvas.fill(thumb->x, thumb->y, thumb->width, 1.0f);
            canvas.fill(thumb->x, thumb->y + thumb->height - 1.0f, thumb->width, 1.0f);
            canvas.fill(thumb->x, thumb->y, 1.0f, thumb->height);
            canvas.fill(thumb->x + thumb->width - 1.0f, thumb->y, 1.0f, thumb->height);
        }
    }
}

} // namespace visiform::ui
