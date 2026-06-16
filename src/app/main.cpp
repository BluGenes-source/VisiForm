#include "app/Startup.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    return visiform::startup::runApplication();
}
#else
int main()
{
    return visiform::startup::runApplication();
}
#endif
