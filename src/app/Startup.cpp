#include "app/Startup.h"

#include "app/App.h"

#include <exception>
#include <string>
#include <string_view>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <iostream>
#endif

namespace visiform::startup {
namespace {

#ifdef _WIN32
std::wstring widenUtf8(std::string_view text)
{
    if (text.empty()) {
        return {};
    }

    const int requiredLength = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        text.data(),
        static_cast<int>(text.size()),
        nullptr,
        0);
    if (requiredLength <= 0) {
        return std::wstring(text.begin(), text.end());
    }

    std::wstring wideText(static_cast<std::size_t>(requiredLength), L'\0');
    MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        text.data(),
        static_cast<int>(text.size()),
        wideText.data(),
        requiredLength);
    return wideText;
}

void writeDebugMessage(std::string_view message)
{
    std::wstring wideMessage = widenUtf8(message);
    wideMessage.push_back(L'\n');
    OutputDebugStringW(wideMessage.c_str());
}

void showFatalStartupDialog(std::string_view message)
{
    const std::wstring wideMessage = widenUtf8(message);
    MessageBoxW(nullptr,
        wideMessage.c_str(),
        L"VisiForm Startup Error",
        MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_TASKMODAL);
}
#else
void writeDebugMessage(std::string_view message)
{
    std::cerr << message << '\n';
}

void showFatalStartupDialog(std::string_view)
{
}
#endif

void reportFatalStartupFailure(std::string message)
{
    writeDebugMessage(message);
#ifdef _WIN32
    showFatalStartupDialog(message);
#endif
}

} // namespace

int runApplication()
{
    try {
        visiform::App app;
        return app.run();
    }
    catch (const std::exception& exception) {
        reportFatalStartupFailure("VisiForm failed to start:\n\n" + std::string{ exception.what() });
    }
    catch (...) {
        reportFatalStartupFailure("VisiForm failed to start with an unknown error.");
    }

    return 1;
}

} // namespace visiform::startup
