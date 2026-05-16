#pragma once

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
    [[nodiscard]] std::optional<std::string> hitTestWidgetId(const model::ProjectDocument& document, float x, float y) const;
    [[nodiscard]] std::optional<std::size_t> hitTestRecentFileIndex(const model::ProjectDocument& document, float x, float y) const;
    void drawPanel(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::ProjectDocument& document) const;

private:
    float x_{};
    float y_{};
    float width_{};
    float height_{};
    std::vector<std::filesystem::path> recentFiles_{};
};

} // namespace visiform::ui
