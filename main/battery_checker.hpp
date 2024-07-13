#pragma once

#include <esp_log.h>
#include <functional>
#include "timer.hpp"

namespace App
{
        using BatteryListener = std::function<void(uint8_t)>;

    class BatteryChecker

    {
    private:
        Timer timer;
        uint16_t timeout;
        BatteryListener listener;

    public:
        BatteryChecker();
        void init(BatteryListener listener);
        void runBatteryCheckerTask();
    };

} // namespace App
