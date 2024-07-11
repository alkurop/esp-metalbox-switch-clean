#pragma once
#include "esp_log.h"
#include "tag.hpp"
#include <esp_bit_defs.h>
#include <functional>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_sleep.h"

namespace App
{
    using SleeperCallback = std::function<void(void)>;
    using PinCallback = std::function<void(void)>;

    class Sleeper
    {
    private:
        SleeperCallback beforeSleep;
        SleeperCallback afterWake;
        const gpio_num_t *wakeUpPins;
        uint8_t pin_count;

        uint64_t lastInteraction;

    public:
        void init(SleeperCallback beforeSleep, SleeperCallback afterWake, const gpio_num_t *wakeUpPin, uint8_t pin_count)
        {
            this->afterWake = afterWake;
            this->beforeSleep = beforeSleep;
            this->wakeUpPins = wakeUpPins;
            this->pin_count = pin_count;
        };
        esp_err_t cycle();

        // void ping();
        // void recordInteraction();

        static void sleeper_task() {};
    };
}
