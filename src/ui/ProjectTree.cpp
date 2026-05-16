#include "ui/ProjectTree.h"

#include "ui/ProjectTree.h"

#include "utils/FileUtils.h"

#include <filesystem>
#include <string>
#include <vector>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kPadding = 12.0f;
constexpr float kRowHeight = 24.0f;
constexpr float kSectionSpacing = 12.0f;

struct TreeRow {
    std::string widgetId;
    std::string label;
    int depth = 0;
};

void appendRows(const model::WidgetNode& widget, int depth, std::vector<TreeRow>& rows)
{
    const std::string displayName = widget.name.empty() ? widget.id : widget.name;
    rows.push_back({ widget.id, displayName + " [" + widget.typeName() + "]", depth });
    for (const auto& child : widget.children) {
        appendRows(child, depth + 1, rows);
    }
}

std::vector<TreeRow> buildRows(const model::ProjectDocument& document)
{
    std::vector<TreeRow> rows;
    appendRows(document.root, 0, rows);
    return rows;
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
}

void ProjectTree::setRecentFiles(std::vector<std::filesystem::path> recentFiles)
{
    recentFiles_ = std::move(recentFiles);
}

bool ProjectTree::contains(float x, float y) const
{
    return x >= x_ && y >= y_ && x <= x_ + width_ && y <= y_ + height_;
}

std::optional<std::string> ProjectTree::hitTestWidgetId(const model::ProjectDocument& document, float x, float y) const
{
    if (!contains(x, y)) {
        return std::nullopt;
    }

    const std::vector<TreeRow> rows = buildRows(document);
    float rowTop = y_ + kHeaderHeight + 8.0f;
    for (const auto& row : rows) {
        if (rowTop + kRowHeight > y_ + height_ - 8.0f) {
            break;
        }

        if (y >= rowTop && y <= rowTop + kRowHeight) {
            return row.widgetId;
        }

        rowTop += kRowHeight;
    }

    return std::nullopt;
}

std::optional<std::size_t> ProjectTree::hitTestRecentFileIndex(const model::ProjectDocument& document, float x, float y) const
{
    if (!contains(x, y) || recentFiles_.empty()) {
        return std::nullopt;
    }

    float rowTop = y_ + kHeaderHeight + 8.0f;
    for (const auto& row : buildRows(document)) {
        (void)row;
        if (rowTop + kRowHeight > y_ + height_ - 8.0f) {
            return std::nullopt;
        }

        rowTop += kRowHeight;
    }

    rowTop += kSectionSpacing + 22.0f;
    for (std::size_t recentIndex = 0; recentIndex < recentFiles_.size(); ++recentIndex) {
        if (rowTop + kRowHeight > y_ + height_ - 8.0f) {
            break;
        }

        if (y >= rowTop && y <= rowTop + kRowHeight) {
            return recentIndex;
        }

        rowTop += kRowHeight;
    }

    return std::nullopt;
}

void ProjectTree::drawPanel(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::ProjectDocument& document) const
{
    if (width_ <= 0.0f || height_ <= 0.0f) {
        return;
    }

    canvas.setColor(0xff232833);
    canvas.fill(x_, y_, width_, height_);

    canvas.setColor(0xff2c3240);
    canvas.fill(x_, y_, width_, kHeaderHeight);

    canvas.setColor(0xff11141a);
    canvas.fill(x_, y_, width_, 1.0f);
    canvas.fill(x_, y_ + height_ - 1.0f, width_, 1.0f);
    canvas.fill(x_, y_, 1.0f, height_);
    canvas.fill(x_ + width_ - 1.0f, y_, 1.0f, height_);

    if (!drawText) {
        return;
    }

    canvas.setColor(0xfff3f5f8);
    canvas.text("Project Tree", font, visage::Font::kTopLeft,
        x_ + kPadding, y_ + 6.0f, width_ - kPadding * 2.0f, kHeaderHeight - 8.0f);

    const std::vector<TreeRow> rows = buildRows(document);
    float rowTop = y_ + kHeaderHeight + 8.0f;
    for (const auto& row : rows) {
        if (rowTop + kRowHeight > y_ + height_ - 8.0f) {
            break;
        }

        const bool isSelected = document.selectedWidgetId == row.widgetId;
        canvas.setColor(isSelected ? 0xff355382 : 0xff252b36);
        canvas.fill(x_ + 8.0f, rowTop, width_ - 16.0f, kRowHeight - 2.0f);

        canvas.setColor(isSelected ? 0xfff8fbff : 0xffdde2ea);
        canvas.text(row.label, font, visage::Font::kTopLeft,
            x_ + 16.0f + row.depth * 16.0f, rowTop + 4.0f,
            width_ - 28.0f - row.depth * 16.0f, kRowHeight - 6.0f);

        rowTop += kRowHeight;
    }

    if (recentFiles_.empty() || rowTop + kSectionSpacing + 22.0f >= y_ + height_ - 8.0f) {
        return;
    }

    rowTop += kSectionSpacing;
    canvas.setColor(0xfff3f5f8);
    canvas.text("Recent Files", font, visage::Font::kTopLeft,
        x_ + kPadding, rowTop, width_ - kPadding * 2.0f, 18.0f);
    rowTop += 22.0f;

    for (std::size_t recentIndex = 0; recentIndex < recentFiles_.size(); ++recentIndex) {
        if (rowTop + kRowHeight > y_ + height_ - 8.0f) {
            break;
        }

        canvas.setColor(0xff252b36);
        canvas.fill(x_ + 8.0f, rowTop, width_ - 16.0f, kRowHeight - 2.0f);
        canvas.setColor(0xffcfd6e2);
        canvas.text(recentFileLabel(recentFiles_[recentIndex]), font, visage::Font::kTopLeft,
            x_ + 16.0f, rowTop + 4.0f, width_ - 28.0f, kRowHeight - 6.0f);
        rowTop += kRowHeight;
    }
}

} // namespace visiform::ui
