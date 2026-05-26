#include "ui/resources/ImageResourceCache.h"

#include <algorithm>
#include <bimg/decode.h>
#include <bx/allocator.h>
#include <cmath>
#include <fstream>

namespace visiform::ui::resources {
namespace {

std::string widgetImagePath(const model::WidgetNode& widget)
{
    return widget.getStringProperty("imagePath", widget.getStringProperty("source", {}));
}

std::string imageDisplayText(const std::filesystem::path& sourcePath)
{
    const std::string fileName = sourcePath.filename().string();
    return fileName.empty() ? sourcePath.string() : fileName;
}

bx::DefaultAllocator* allocator()
{
    static bx::DefaultAllocator allocator;
    return &allocator;
}

} // namespace

const ImageResourceCache::CachedImageData& ImageResourceCache::getOrLoad(const std::filesystem::path& sourcePath) const
{
    if (sourcePath.empty()) {
        empty_ = {};
        return empty_;
    }

    const std::string key = cacheKey(sourcePath);
    const auto iterator = cache_.find(key);
    if (iterator != cache_.end()) {
        return iterator->second;
    }

    auto [inserted, created] = cache_.emplace(key, loadImage(sourcePath));
    return inserted->second;
}

const ImageResourceCache::CachedImageData& ImageResourceCache::getOrLoad(const std::string& sourcePath) const
{
    return getOrLoad(std::filesystem::path{ sourcePath });
}

void ImageResourceCache::invalidate(const std::filesystem::path& sourcePath)
{
    if (sourcePath.empty()) {
        return;
    }

    cache_.erase(cacheKey(sourcePath));
}

void ImageResourceCache::clear()
{
    cache_.clear();
}

ResolvedImageSource ImageResourceCache::resolveWidgetImageSource(const model::ProjectDocument& document, const model::WidgetNode& widget)
{
    ResolvedImageSource resolved;

    const std::string resourceId = widget.getStringProperty("resourceId", {});
    if (!resourceId.empty()) {
        const auto* resource = document.findResourceById(resourceId);
        if (resource == nullptr || resource->type != model::ProjectResourceType::Image) {
            resolved.displayText = resourceId;
            resolved.missingResource = true;
            return resolved;
        }

        resolved.sourcePath = std::filesystem::path{ resource->sourcePath };
        resolved.displayText = !resource->displayName.empty() ? resource->displayName : imageDisplayText(resolved.sourcePath);
        resolved.hasImage = !resolved.sourcePath.empty();
        return resolved;
    }

    const std::string imagePath = widgetImagePath(widget);
    if (!imagePath.empty()) {
        resolved.sourcePath = std::filesystem::path{ imagePath };
        resolved.displayText = imageDisplayText(resolved.sourcePath);
        resolved.hasImage = true;
    }

    return resolved;
}

ImageScaleMode ImageResourceCache::parseScaleMode(const std::string& value)
{
    if (value == "Stretch") {
        return ImageScaleMode::Stretch;
    }
    if (value == "Fill") {
        return ImageScaleMode::Fill;
    }
    if (value == "Center") {
        return ImageScaleMode::Center;
    }

    return ImageScaleMode::Fit;
}

ImageDrawRect ImageResourceCache::computeDrawRect(float x,
    float y,
    float width,
    float height,
    int imageWidth,
    int imageHeight,
    ImageScaleMode scaleMode)
{
    ImageDrawRect rect{ x, y, std::max(0.0f, width), std::max(0.0f, height) };
    if (rect.width <= 0.0f || rect.height <= 0.0f) {
        return {};
    }

    if (scaleMode == ImageScaleMode::Stretch || imageWidth <= 0 || imageHeight <= 0) {
        return rect;
    }

    const float sourceWidth = static_cast<float>(imageWidth);
    const float sourceHeight = static_cast<float>(imageHeight);
    const float fitScale = std::min(rect.width / sourceWidth, rect.height / sourceHeight);

    if (scaleMode == ImageScaleMode::Center) {
        const float scale = std::min(1.0f, fitScale);
        const float drawWidth = std::max(1.0f, sourceWidth * scale);
        const float drawHeight = std::max(1.0f, sourceHeight * scale);
        return {
            x + (rect.width - drawWidth) * 0.5f,
            y + (rect.height - drawHeight) * 0.5f,
            drawWidth,
            drawHeight
        };
    }

    const float scale = fitScale;
    const float drawWidth = std::max(1.0f, sourceWidth * scale);
    const float drawHeight = std::max(1.0f, sourceHeight * scale);
    return {
        x + (rect.width - drawWidth) * 0.5f,
        y + (rect.height - drawHeight) * 0.5f,
        drawWidth,
        drawHeight
    };
}

ImageResourceCache::CachedImageData ImageResourceCache::loadImage(const std::filesystem::path& sourcePath)
{
    CachedImageData data;

    if (sourcePath.empty()) {
        data.info.error = "Image source file path is empty.";
        return data;
    }

    if (!std::filesystem::exists(sourcePath)) {
        data.info.error = "Image source file does not exist.";
        return data;
    }

    std::ifstream stream(sourcePath, std::ios::binary);
    if (!stream.good()) {
        data.info.error = "Failed to open image source file.";
        return data;
    }

    auto bytes = std::make_shared<std::vector<unsigned char>>();
    stream.seekg(0, std::ios::end);
    const auto fileSize = stream.tellg();
    stream.seekg(0, std::ios::beg);
    if (fileSize <= 0) {
        data.info.error = "Image source file is empty.";
        return data;
    }

    bytes->resize(static_cast<std::size_t>(fileSize));
    stream.read(reinterpret_cast<char*>(bytes->data()), fileSize);
    if (!stream.good() && !stream.eof()) {
        data.info.error = "Failed to read image source file.";
        return data;
    }

    bimg::ImageContainer* imageContainer = bimg::imageParse(allocator(), bytes->data(), static_cast<uint32_t>(bytes->size()));
    if (imageContainer == nullptr) {
        data.info.error = "Failed to decode image source file.";
        return data;
    }

    data.info.width = imageContainer->m_width;
    data.info.height = imageContainer->m_height;
    bimg::imageFree(imageContainer);

    data.info.available = !bytes->empty();
    data.encodedBytes = std::move(bytes);
    return data;
}

std::string ImageResourceCache::cacheKey(const std::filesystem::path& sourcePath)
{
    return sourcePath.lexically_normal().generic_string();
}

} // namespace visiform::ui::resources
