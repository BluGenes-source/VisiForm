#include "ui/ProjectTree.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kPadding = 12.0f;
constexpr float kRowVerticalPadding = 12.0f;
constexpr float kIndentWidth = 16.0f;
constexpr float kExpanderSize = 12.0f;
constexpr float kExpanderPadding = 10.0f;
constexpr float kContentLeftPadding = 4.0f;
constexpr float kControlLabelGap = 8.0f;
constexpr float kLabelRightPadding = 6.0f;
constexpr float kTypeWidthRatio = 0.38f;
constexpr float kScrollBarWidth = 16.0f;
constexpr float kScrollBarGap = 6.0f;
constexpr float kMinimumThumbSize = 20.0f;

struct TreeEntry {
    enum class Kind {
        Project,
        Widget
    };

    Kind kind = Kind::Widget;
    std::string widgetId{};
    std::string name{};
    std::string type{};
    int depth = 0;
    bool hasChildren = false;
    bool expanded = false;
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

void appendWidgetRows(
    const model::WidgetNode& widget,
    int depth,
    const std::unordered_set<std::string>& expandedWidgetIds,
    std::vector<TreeEntry>& rows)
{
    const bool hasChildren = !widget.children.empty();
    const bool expanded = hasChildren && expandedWidgetIds.contains(widget.id);
    const std::string displayName = widget.name.empty() ? widget.id : widget.name;
    rows.push_back({ TreeEntry::Kind::Widget, widget.id, displayName, widget.typeName(), depth, hasChildren, expanded });
    if (!expanded) {
        return;
    }

    for (const auto& child : widget.children) {
        appendWidgetRows(child, depth + 1, expandedWidgetIds, rows);
    }
}

[[nodiscard]] std::vector<TreeEntry> buildEntries(
    const model::ProjectDocument& document,
    bool projectRootExpanded,
    const std::unordered_set<std::string>& expandedWidgetIds)
{
    std::vector<TreeEntry> entries;
    const std::string projectName = document.projectName.empty() ? std::string{ "Project" } : document.projectName;
    entries.push_back({ TreeEntry::Kind::Project, {}, projectName, "Project", 0, true, projectRootExpanded });
    if (projectRootExpanded) {
        appendWidgetRows(document.root, 1, expandedWidgetIds, entries);
    }
    return entries;
}

[[nodiscard]] std::vector<TreeLayout> buildLayouts(
    float top,
    float rowHeight,
    const std::vector<TreeEntry>& entries)
{
    std::vector<TreeLayout> layouts;
    layouts.reserve(entries.size());
    float rowTop = top;
    for (const auto& entry : entries) {
        layouts.push_back({ entry, rowTop });
        rowTop += rowHeight;
    }
    return layouts;
}

[[nodiscard]] float expanderX(float contentX, int depth)
{
    return contentX + kContentLeftPadding + static_cast<float>(depth) * kIndentWidth;
}

[[nodiscard]] float measuredTextWidth(const visage::Font& font, const std::string& text)
{
    if (text.empty()) {
        return 0.0f;
    }

    const std::u32string utf32 = visage::String::convertUtf8ToUtf32<std::u32string>(text);
    return font.stringWidth(utf32);
}

[[nodiscard]] std::string elideText(const visage::Font& font, const std::string& text, float availableWidth)
{
    if (text.empty() || availableWidth <= 0.0f) {
        return {};
    }
    if (measuredTextWidth(font, text) <= availableWidth) {
        return text;
    }

    const std::u32string ellipsis = U"\u2026";
    const float ellipsisWidth = font.stringWidth(ellipsis);
    if (ellipsisWidth > availableWidth) {
        return {};
    }

    std::u32string utf32 = visage::String::convertUtf8ToUtf32<std::u32string>(text);
    const int prefixLength = font.widthOverflowIndex(
        utf32.c_str(),
        static_cast<int>(utf32.size()),
        availableWidth - ellipsisWidth);
    utf32.resize(static_cast<std::size_t>(std::max(0, prefixLength)));
    utf32 += ellipsis;
    return visage::String::convertUtf32ToUtf8(utf32);
}

struct FittedLabel {
    std::string name{};
    std::string separator{};
    std::string type{};
};

[[nodiscard]] FittedLabel fitLabel(
    const visage::Font& font,
    const TreeEntry& entry,
    float availableWidth)
{
    static const std::string kSeparator = " : ";
    const float separatorWidth = measuredTextWidth(font, kSeparator);
    const float nameWidth = measuredTextWidth(font, entry.name);
    const float typeWidth = measuredTextWidth(font, entry.type);
    if (nameWidth + separatorWidth + typeWidth <= availableWidth) {
        return { entry.name, kSeparator, entry.type };
    }

    const std::u32string ellipsis = U"\u2026";
    const float minimumPartWidth = font.stringWidth(ellipsis);
    if (availableWidth < separatorWidth + minimumPartWidth * 2.0f) {
        return { elideText(font, entry.name + kSeparator + entry.type, availableWidth), {}, {} };
    }

    const float typeRegionWidth = std::clamp(
        availableWidth * kTypeWidthRatio,
        minimumPartWidth,
        availableWidth - separatorWidth - minimumPartWidth);
    const float nameRegionWidth = availableWidth - separatorWidth - typeRegionWidth;
    return {
        elideText(font, entry.name, nameRegionWidth),
        kSeparator,
        elideText(font, entry.type, typeRegionWidth)
    };
}

void collectWidgetIds(const model::WidgetNode& widget, std::unordered_set<std::string>& ids)
{
    ids.insert(widget.id);
    for (const auto& child : widget.children) {
        collectWidgetIds(child, ids);
    }
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

void ProjectTree::resetForDocument(const model::ProjectDocument& document)
{
    clearHover();
    projectRootExpanded_ = true;
    expandedWidgetIds_.clear();
    expandedWidgetIds_.insert(document.root.id);
    observedRootWidgetId_ = document.root.id;
    observedSelectionId_.clear();
    pendingRevealWidgetId_.clear();
    scrollOffsetY_ = 0.0f;
    revealWidget(document, document.selectedWidgetId);
}

void ProjectTree::revealWidget(const model::ProjectDocument& document, const std::string& widgetId)
{
    if (widgetId.empty() || document.findWidgetById(widgetId) == nullptr) {
        return;
    }

    projectRootExpanded_ = true;
    expandAncestors(document, widgetId);
    pendingRevealWidgetId_ = widgetId;
    observedSelectionId_ = widgetId;
}

bool ProjectTree::contains(float x, float y) const
{
    return x >= x_ && y >= y_ && x <= x_ + width_ && y <= y_ + height_;
}

bool ProjectTree::updateHover(const model::ProjectDocument& document, float x, float y)
{
    updateScrollMetrics(document);

    bool nextProjectRootHovered = false;
    std::string nextHoveredWidgetId;
    if (isWithinVisibleContent(x, y)) {
        const auto entries = buildEntries(document, projectRootExpanded_, expandedWidgetIds_);
        const auto layouts = buildLayouts(contentBounds().y, rowHeight_, entries);
        const Bounds bounds = contentBounds();
        for (const auto& layout : layouts) {
            const float rowTop = rowYWithScroll(layout.top);
            if (rowTop + rowHeight_ <= bounds.y || rowTop >= bounds.y + bounds.height) {
                continue;
            }
            if (y < rowTop || y >= rowTop + rowHeight_) {
                continue;
            }

            if (layout.entry.kind == TreeEntry::Kind::Project) {
                nextProjectRootHovered = true;
            }
            else {
                nextHoveredWidgetId = layout.entry.widgetId;
            }
            break;
        }
    }

    const bool changed = projectRootHovered_ != nextProjectRootHovered
        || hoveredWidgetId_ != nextHoveredWidgetId;
    projectRootHovered_ = nextProjectRootHovered;
    hoveredWidgetId_ = std::move(nextHoveredWidgetId);
    return changed;
}

void ProjectTree::clearHover()
{
    projectRootHovered_ = false;
    hoveredWidgetId_.clear();
}

std::optional<std::string> ProjectTree::hoverHint(const model::ProjectDocument& document) const
{
    if (projectRootHovered_) {
        const std::string projectName = document.projectName.empty() ? std::string{ "Project" } : document.projectName;
        return projectName + " : Project root";
    }

    const model::WidgetNode* widget = document.findWidgetById(hoveredWidgetId_);
    if (widget == nullptr) {
        return std::nullopt;
    }

    const std::string displayName = widget->name.empty() ? widget->id : widget->name;
    std::string hint = displayName + " : " + widget->typeName();
    if (const model::WidgetNode* parent = document.findParentOf(widget->id)) {
        const std::string parentName = parent->name.empty() ? parent->id : parent->name;
        hint += " - Parent: " + parentName;
    }
    return hint;
}

void ProjectTree::pruneExpansionState(const model::ProjectDocument& document)
{
    std::unordered_set<std::string> existingIds;
    collectWidgetIds(document.root, existingIds);
    std::erase_if(expandedWidgetIds_, [&existingIds](const std::string& id) {
        return !existingIds.contains(id);
    });
}

void ProjectTree::expandAncestors(const model::ProjectDocument& document, const std::string& widgetId)
{
    const model::WidgetNode* current = document.findWidgetById(widgetId);
    while (current != nullptr) {
        const model::WidgetNode* parent = document.findParentOf(current->id);
        if (parent == nullptr) {
            break;
        }

        expandedWidgetIds_.insert(parent->id);
        current = parent;
    }
}

void ProjectTree::updateScrollMetrics(const model::ProjectDocument& document)
{
    if (observedRootWidgetId_ != document.root.id) {
        observedRootWidgetId_ = document.root.id;
        projectRootExpanded_ = true;
        expandedWidgetIds_.clear();
        expandedWidgetIds_.insert(document.root.id);
        pendingRevealWidgetId_ = document.selectedWidgetId;
    }

    pruneExpansionState(document);
    if (observedSelectionId_ != document.selectedWidgetId) {
        observedSelectionId_ = document.selectedWidgetId;
        revealWidget(document, document.selectedWidgetId);
    }

    const float rawVisibleHeight = std::max(0.0f, height_ - (kHeaderHeight + 16.0f));
    visibleHeight_ = rawVisibleHeight >= rowHeight_
        ? std::floor(rawVisibleHeight / rowHeight_) * rowHeight_
        : rawVisibleHeight;
    const auto entries = buildEntries(document, projectRootExpanded_, expandedWidgetIds_);
    contentHeight_ = static_cast<float>(entries.size()) * rowHeight_;
    needsVerticalScrollBar_ = contentHeight_ > visibleHeight_ + 0.5f;
    clampScrollOffset();
    revealPendingWidget(document);
}

void ProjectTree::revealPendingWidget(const model::ProjectDocument& document)
{
    if (pendingRevealWidgetId_.empty() || document.findWidgetById(pendingRevealWidgetId_) == nullptr) {
        pendingRevealWidgetId_.clear();
        return;
    }

    const auto entries = buildEntries(document, projectRootExpanded_, expandedWidgetIds_);
    const auto layouts = buildLayouts(contentBounds().y, rowHeight_, entries);
    const Bounds bounds = contentBounds();
    const auto selected = std::find_if(layouts.begin(), layouts.end(), [this](const TreeLayout& layout) {
        return layout.entry.kind == TreeEntry::Kind::Widget
            && layout.entry.widgetId == pendingRevealWidgetId_;
    });
    if (selected == layouts.end()) {
        pendingRevealWidgetId_.clear();
        return;
    }

    const float selectedTop = selected->top - scrollOffsetY_;
    if (selectedTop < bounds.y) {
        scrollOffsetY_ = std::max(0.0f, selected->top - bounds.y);
    }
    else if (selectedTop + rowHeight_ > bounds.y + bounds.height) {
        scrollOffsetY_ += selectedTop + rowHeight_ - (bounds.y + bounds.height);
    }
    clampScrollOffset();
    pendingRevealWidgetId_.clear();
}

void ProjectTree::clampScrollOffset()
{
    const float maxScroll = std::max(0.0f, contentHeight_ - visibleHeight_);
    if (rowHeight_ > 0.0f) {
        scrollOffsetY_ = std::round(scrollOffsetY_ / rowHeight_) * rowHeight_;
    }
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

    const auto entries = buildEntries(document, projectRootExpanded_, expandedWidgetIds_);
    const auto layouts = buildLayouts(contentBounds().y, rowHeight_, entries);
    const Bounds bounds = contentBounds();
    for (const auto& layout : layouts) {
        const float rowTop = rowYWithScroll(layout.top);
        if (rowTop + rowHeight_ <= bounds.y || rowTop >= bounds.y + bounds.height) {
            continue;
        }

        if (layout.entry.kind == TreeEntry::Kind::Widget && y >= rowTop && y < rowTop + rowHeight_) {
            return layout.entry.widgetId;
        }
    }

    return std::nullopt;
}

bool ProjectTree::mouseDown(const model::ProjectDocument& document, float x, float y)
{
    updateScrollMetrics(document);
    const auto scrollBar = scrollBarBounds();
    if (scrollBar.has_value() && containsPoint({ scrollBar->x, scrollBar->y, scrollBar->width, scrollBar->height }, x, y)) {
        const float arrowSize = std::min(scrollBar->width, 20.0f);
        const auto thumb = scrollBarThumbBounds();
        if (thumb.has_value() && containsPoint({ thumb->x, thumb->y, thumb->width, thumb->height }, x, y)) {
            draggingScrollBarThumb_ = true;
            scrollBarDragOffsetY_ = y - thumb->y;
            clearHover();
            return true;
        }

        if (y < scrollBar->y + arrowSize) {
            scrollOffsetY_ -= rowHeight_;
        }
        else if (y > scrollBar->y + scrollBar->height - arrowSize) {
            scrollOffsetY_ += rowHeight_;
        }
        else if (thumb.has_value() && y < thumb->y) {
            scrollOffsetY_ -= std::max(rowHeight_, visibleHeight_ - rowHeight_);
        }
        else {
            scrollOffsetY_ += std::max(rowHeight_, visibleHeight_ - rowHeight_);
        }

        clampScrollOffset();
        clearHover();
        return true;
    }

    if (!isWithinVisibleContent(x, y)) {
        return false;
    }

    const auto entries = buildEntries(document, projectRootExpanded_, expandedWidgetIds_);
    const auto layouts = buildLayouts(contentBounds().y, rowHeight_, entries);
    for (const auto& layout : layouts) {
        const float rowTop = rowYWithScroll(layout.top);
        if (!layout.entry.hasChildren || y < rowTop || y >= rowTop + rowHeight_) {
            continue;
        }

        const float indicatorX = expanderX(contentBounds().x, layout.entry.depth);
        if (x < indicatorX || x > indicatorX + kExpanderSize) {
            continue;
        }

        if (layout.entry.kind == TreeEntry::Kind::Project) {
            projectRootExpanded_ = !projectRootExpanded_;
        }
        else if (layout.entry.expanded) {
            expandedWidgetIds_.erase(layout.entry.widgetId);
        }
        else {
            expandedWidgetIds_.insert(layout.entry.widgetId);
        }
        clearHover();
        updateScrollMetrics(document);
        return true;
    }

    return false;
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
    clearHover();
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

    scrollOffsetY_ += deltaY < 0.0f ? rowHeight_ : -rowHeight_;
    clampScrollOffset();
    clearHover();
    return true;
}

void ProjectTree::drawPanel(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::ProjectDocument& document)
{
    if (width_ <= 0.0f || height_ <= 0.0f) {
        return;
    }

    if (drawText && font.packedFont() != nullptr) {
        rowHeight_ = std::ceil(std::max(font.lineHeight() + kRowVerticalPadding, kExpanderSize + kExpanderPadding));
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

    const auto entries = buildEntries(document, projectRootExpanded_, expandedWidgetIds_);
    const auto layouts = buildLayouts(contentBounds().y, rowHeight_, entries);
    const Bounds bounds = contentBounds();

    canvas.saveState();
    canvas.setClampBounds(bounds.x, bounds.y, bounds.width, bounds.height);
    for (const auto& layout : layouts) {
        const float rowTop = rowYWithScroll(layout.top);
        if (rowTop + rowHeight_ <= bounds.y || rowTop >= bounds.y + bounds.height) {
            continue;
        }

        const bool isProject = layout.entry.kind == TreeEntry::Kind::Project;
        const bool isPrimarySelected = !isProject && document.selectedWidgetId == layout.entry.widgetId;
        const bool isSecondarySelected = !isProject && document.isSelected(layout.entry.widgetId) && !isPrimarySelected;
        canvas.setColor(isPrimarySelected ? 0xff355382 : (isSecondarySelected ? 0xff2d4668 : (isProject ? 0xff29313d : 0xff252b36)));
        canvas.fill(bounds.x, rowTop, bounds.width, rowHeight_);

        for (int guideDepth = 0; guideDepth < layout.entry.depth; ++guideDepth) {
            const float guideX = expanderX(bounds.x, guideDepth) + kExpanderSize * 0.5f;
            canvas.setColor(0xff3a4554);
            canvas.fill(guideX, rowTop, 1.0f, rowHeight_);
        }

        const float indicatorX = expanderX(bounds.x, layout.entry.depth);
        if (layout.entry.hasChildren) {
            const float indicatorY = rowTop + (rowHeight_ - kExpanderSize) * 0.5f;
            canvas.setColor(0xff465365);
            canvas.fill(indicatorX, indicatorY, kExpanderSize, kExpanderSize);
            canvas.setColor(0xffaebed3);
            canvas.fill(indicatorX + 3.0f, indicatorY + 5.0f, kExpanderSize - 6.0f, 2.0f);
            if (!layout.entry.expanded) {
                canvas.fill(indicatorX + 5.0f, indicatorY + 3.0f, 2.0f, kExpanderSize - 6.0f);
            }
        }

        if (drawText) {
            const float textX = indicatorX + kExpanderSize + kControlLabelGap;
            const float textWidth = std::max(0.0f, bounds.x + bounds.width - textX - kLabelRightPadding);
            const FittedLabel fitted = fitLabel(font, layout.entry, textWidth);
            float partX = textX;

            canvas.setColor(isPrimarySelected ? 0xfff8fbff : (isSecondarySelected ? 0xffd9ebff : (isProject ? 0xfff0f4fa : 0xffdde2ea)));
            const float fittedNameWidth = measuredTextWidth(font, fitted.name);
            canvas.text(fitted.name, font, visage::Font::kLeft, partX, rowTop, fittedNameWidth, rowHeight_);
            partX += fittedNameWidth;

            if (!fitted.separator.empty()) {
                const float separatorWidth = measuredTextWidth(font, fitted.separator);
                canvas.text(fitted.separator, font, visage::Font::kLeft, partX, rowTop, separatorWidth, rowHeight_);
                partX += separatorWidth;
            }

            if (!fitted.type.empty()) {
                canvas.setColor(isPrimarySelected ? 0xffc6d7ee : (isSecondarySelected ? 0xffa8c4e8 : 0xff96a0af));
                canvas.text(fitted.type, font, visage::Font::kLeft,
                    partX, rowTop, std::max(0.0f, textX + textWidth - partX), rowHeight_);
            }
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
