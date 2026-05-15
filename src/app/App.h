#pragma once

#include <string>

namespace visiform {

// Placeholder application bootstrapper for the future form builder.
class App {
public:
    static constexpr const char* ProjectFileExtension = ".vfb.json";

    [[nodiscard]] std::string applicationName() const;
    int run();
};

} // namespace visiform
