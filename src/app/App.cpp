#include "app/App.h"

#include "app/App.h"

#include "ui/MainWindow.h"

#include <stdexcept>

namespace visiform {

App::App() = default;

App::~App()
{
    shutdown();
}

int App::run()
{
    startup();

    try {
        mainWindow_->showWindow();
        if (!mainWindow_->isShowing()) {
            throw std::runtime_error("The main Visage window did not open.");
        }

        mainWindow_->runEventLoop();
        shutdown();
        return 0;
    }
    catch (...) {
        shutdown();
        throw;
    }
}

void App::startup()
{
    if (mainWindow_) {
        return;
    }

    mainWindow_ = std::make_unique<ui::MainWindow>();
}

void App::shutdown() noexcept
{
    if (mainWindow_) {
        mainWindow_->close();
        mainWindow_.reset();
    }
}

std::string App::applicationName() const
{
    return "VisiForm";
}

} // namespace visiform
