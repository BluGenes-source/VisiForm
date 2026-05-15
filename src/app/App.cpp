#include "app/App.h"

#include "model/ProjectDocument.h"
#include "ui/MainWindow.h"

#include <iostream>

namespace visiform {

int App::run()
{
    ui::MainWindow mainWindow;
    model::ProjectDocument document{"Untitled Project"};

    std::cout << applicationName() << " placeholder started.\n";
    std::cout << "Project extension: " << model::ProjectDocument::projectFileExtension() << '\n';
    std::cout << "Workspace shell: " << mainWindow.title() << '\n';
    std::cout << "Current document: " << document.name() << '\n';

    return 0;
}

std::string App::applicationName() const
{
    return "VisiForm";
}

} // namespace visiform
