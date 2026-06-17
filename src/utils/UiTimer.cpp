#include "utils/UiTimer.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <map>
#include <utility>
#endif

namespace visiform::utils {
namespace {

#ifdef _WIN32
std::map<UINT_PTR, std::function<void()>>& timerCallbacks()
{
    static std::map<UINT_PTR, std::function<void()>> callbacks;
    return callbacks;
}

void CALLBACK uiTimerProc(HWND, UINT, UINT_PTR timerId, DWORD)
{
    const auto iterator = timerCallbacks().find(timerId);
    if (iterator != timerCallbacks().end() && iterator->second) {
        iterator->second();
    }
}
#endif

} // namespace

UiTimer::~UiTimer()
{
    stop();
}

bool UiTimer::start(unsigned int intervalMilliseconds, std::function<void()> callback)
{
    stop();
    if (intervalMilliseconds == 0 || !callback) {
        return false;
    }

#ifdef _WIN32
    const UINT_PTR timerId = SetTimer(nullptr, 0, intervalMilliseconds, uiTimerProc);
    if (timerId == 0) {
        return false;
    }

    timerCallbacks()[timerId] = std::move(callback);
    timerId_ = static_cast<unsigned long long>(timerId);
    return true;
#else
    (void)callback;
    return false;
#endif
}

void UiTimer::stop()
{
    if (timerId_ == 0) {
        return;
    }

#ifdef _WIN32
    const auto timerId = static_cast<UINT_PTR>(timerId_);
    KillTimer(nullptr, timerId);
    timerCallbacks().erase(timerId);
#endif
    timerId_ = 0;
}

bool UiTimer::isRunning() const
{
    return timerId_ != 0;
}

} // namespace visiform::utils
