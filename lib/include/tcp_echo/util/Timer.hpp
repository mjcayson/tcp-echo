#pragma once

#include <chrono>
#include <thread>

namespace tcp_echo::util
{
    // Simple sleep helpers to avoid sprinkling <thread> everywhere.
    inline void SleepForMs(int ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
} // namespace tcp_echo::util
