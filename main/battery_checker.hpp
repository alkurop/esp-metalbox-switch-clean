#pragma once

#include <esp_log.h>
#include <functional>
#include <driver/gpio.h>

#include "timer.hpp"
#include "tag.hpp"

namespace App
{
    using BatteryListener = std::function<void(uint8_t)>;

    class BatteryChecker

    {
    private:
        Timer timer;
        uint16_t timeoutSeconds;
        BatteryListener listener;
        gpio_num_t enablePin;
        gpio_num_t checkPin;
        TimeoutListener timeoutListener;

        void runBatteryCheckerTask();

    public:
        BatteryChecker(gpio_num_t enablePin, gpio_num_t checkPin, uint16_t timeoutSeconds);
        void init(BatteryListener listener);
        void start();
        void stop();
    };

} // namespace App
