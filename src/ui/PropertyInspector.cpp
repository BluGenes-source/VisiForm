#include "ui/PropertyInspector.h"

#include "ui/PropertyInspector.h"

#include <iomanip>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace visiform::ui {
namespace {

constexpr float kHeaderHeight = 34.0f;
constexpr float kRowHeight = 30.0f;
constexpr float kPadding = 12.0f;

std::string formatFloat(float value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    std::string text = stream.str();
    while (!text.empty() && text.back() == '0') {
        text.pop_back();
    }
    if (!text.empty() && text.back() == '.') {
        text.pop_back();
    }
    return text.empty() ? "0" : text;
}

} // namespace

void PropertyInspector::setBounds(float x, float y, float width, float height)
{
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;
}

void PropertyInspector::draw(visage::Canvas& canvas, const visage::Font& font, bool drawText, const model::WidgetNode* selectedWidget) const
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

    if (drawText) {
        canvas.setColor(0xfff3f5f8);
        canvas.text("Property Inspector", font, visage::Font::kTopLeft,
            x_ + kPadding, y_ + 6.0f, width_ - kPadding * 2.0f, kHeaderHeight - 8.0f);
    }

    std::vector<std::string> rows;
    if (selectedWidget == nullptr) {
        rows.emplace_back("No selection");
    }
    else {
        rows.push_back("id: " + selectedWidget->id);
        rows.push_back("name: " + selectedWidget->name);
        rows.push_back("type: " + selectedWidget->typeName());
        rows.push_back("x: " + formatFloat(selectedWidget->bounds.x));
        rows.push_back("y: " + formatFloat(selectedWidget->bounds.y));
        rows.push_back("width: " + formatFloat(selectedWidget->bounds.width));
        rows.push_back("height: " + formatFloat(selectedWidget->bounds.height));

        std::set<std::string> drawnProperties;
        auto addProperty = [&](const std::string& key) {
            if (const auto* property = selectedWidget->getProperty(key)) {
                rows.push_back(key + ": " + property->toDisplayString());
                drawnProperties.insert(key);
            }
        };

        addProperty("text");
        addProperty("title");
        addProperty("backgroundColor");

        for (const auto& [key, value] : selectedWidget->properties) {
            if (drawnProperties.contains(key) || value.isEmpty()) {
                continue;
            }

            rows.push_back(key + ": " + value.toDisplayString());
        }
    }

    float rowTop = y_ + kHeaderHeight + 8.0f;
    for (std::size_t index = 0; index < rows.size(); ++index) {
        if (rowTop + kRowHeight > y_ + height_ - 8.0f) {
            break;
        }

        canvas.setColor(index % 2 == 0 ? 0xff2b313d : 0xff262c37);
        canvas.fill(x_ + 8.0f, rowTop, width_ - 16.0f, kRowHeight - 2.0f);

        if (drawText) {
            canvas.setColor(0xffdde2ea);
            canvas.text(rows[index], font, visage::Font::kTopLeft,
                x_ + 18.0f, rowTop + 5.0f, width_ - 30.0f, kRowHeight - 8.0f);
        }

        rowTop += kRowHeight;
    }
}

} // namespace visiform::ui
