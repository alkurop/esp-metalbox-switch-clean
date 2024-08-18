#pragma once

#include <esp_log.h>
#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include "timer.hpp"
#include "tag.hpp"

using namespace TMR;

namespace bchk
{
    using BatteryListener = std::function<void(uint8_t)>;

    class BatteryChecker

    {
    private:
        Timer timer;
        uint16_t timeoutSeconds;
        gpio_num_t enablePin;
        gpio_num_t checkPin;
        TimeoutListener timeoutListener;
        uint8_t batteryLevel;
        TaskHandle_t *taskHandle;

    public:
        BatteryListener batteryListener;

        BatteryChecker(gpio_num_t enablePin, gpio_num_t checkPin, uint16_t timeoutSeconds);
        void init(BatteryListener listener);
        void start();
        void stop();
        uint8_t getBatteryLevel() { return batteryLevel; };
        uint8_t checkBatteryLevel();
    };

} // namespace App
