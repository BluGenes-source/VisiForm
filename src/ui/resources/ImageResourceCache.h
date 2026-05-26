#pragma once

#include "model/ProjectDocument.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace visiform::ui::resources {

enum class ImageScaleMode {
    Stretch,
    Fit,
    Fill,
    Center
};

struct ImageDrawRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct ResolvedImageSource {
    std::filesystem::path sourcePath{};
    std::string displayText{};
    bool hasImage = false;
    bool missingResource = false;
};

class ImageResourceCache {
public:
    struct CachedImageInfo {
        bool available = false;
        int width = 0;
        int height = 0;
        std::string error{};
    };

    struct CachedImageData {
        CachedImageInfo info{};
        std::shared_ptr<std::vector<unsigned char>> encodedBytes{};
    };

    [[nodiscard]] const CachedImageData& getOrLoad(const std::filesystem::path& sourcePath) const;
    [[nodiscard]] const CachedImageData& getOrLoad(const std::string& sourcePath) const;
    void invalidate(const std::filesystem::path& sourcePath);
    void clear();

    [[nodiscard]] static ResolvedImageSource resolveWidgetImageSource(const model::ProjectDocument& document, const model::WidgetNode& widget);
    [[nodiscard]] static ImageScaleMode parseScaleMode(const std::string& value);
    [[nodiscard]] static ImageDrawRect computeDrawRect(float x,
        float y,
        float width,
        float height,
        int imageWidth,
        int imageHeight,
        ImageScaleMode scaleMode);

private:
    [[nodiscard]] static CachedImageData loadImage(const std::filesystem::path& sourcePath);
    [[nodiscard]] static std::string cacheKey(const std::filesystem::path& sourcePath);

    mutable std::unordered_map<std::string, CachedImageData> cache_{};
    mutable CachedImageData empty_{};
};

} // namespace visiform::ui::resources
