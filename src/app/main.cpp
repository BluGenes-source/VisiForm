#include "app/App.h"

#include "app/App.h"

#include <exception>
#include <iostream>

int main()
{
    try {
        visiform::App app;
        return app.run();
    }
    catch (const std::exception& exception) {
        std::cerr << "VisiForm failed to start: " << exception.what() << '\n';
    }
    catch (...) {
        std::cerr << "VisiForm failed to start with an unknown error.\n";
    }

    return 1;
}
