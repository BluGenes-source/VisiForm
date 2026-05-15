#pragma once

#pragma once

#include <memory>
#include <string>

namespace visiform::ui {
class MainWindow;
}

namespace visiform {

class App {
public:
    static constexpr const char* ProjectFileExtension = ".vfb.json";

    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    [[nodiscard]] std::string applicationName() const;
    int run();

private:
    void startup();
    void shutdown() noexcept;

    std::unique_ptr<ui::MainWindow> mainWindow_{};
};

} // namespace visiform
