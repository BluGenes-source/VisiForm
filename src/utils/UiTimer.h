#pragma once

#include <functional>

namespace visiform::utils {

class UiTimer {
public:
    UiTimer() = default;
    UiTimer(const UiTimer&) = delete;
    UiTimer& operator=(const UiTimer&) = delete;
    UiTimer(UiTimer&&) = delete;
    UiTimer& operator=(UiTimer&&) = delete;
    ~UiTimer();

    bool start(unsigned int intervalMilliseconds, std::function<void()> callback);
    void stop();
    [[nodiscard]] bool isRunning() const;

private:
    unsigned long long timerId_ = 0;
};

} // namespace visiform::utils
