#include "utils/FileUtils.h"

namespace visiform::utils {

std::string FileUtils::normalizeSeparators(std::string path)
{
    for (auto& character : path) {
        if (character == '\\') {
            character = '/';
        }
    }

    return path;
}

bool FileUtils::hasProjectExtension(const std::filesystem::path& path)
{
    return path.extension() == std::filesystem::path{L".json"}
        && path.stem().extension() == std::filesystem::path{L".vfb"};
}

} // namespace visiform::utils
