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
#include "timer.hpp"

namespace App
{
    using SleeperCallback = std::function<void(void)>;

    class Sleeper
    {
    private:
        // callbacks
        SleeperCallback beforeSleep;
        SleeperCallback afterWake;
        TimeoutListener timeoutListener;
        void onTimeout(Timer *);

        // params
        const gpio_num_t *wakeUpPins;
        uint8_t pinCount;
        uint8_t timeoutSeconds;
        Timer timer;

        // state
        TickType_t lastInteraction;

        void cycle();

    public:
        Sleeper(const gpio_num_t *wakeUpPin, uint8_t pin_count, uint8_t timeoutSeconds);
        void init(SleeperCallback beforeSleep, SleeperCallback afterWake);

        void start() ;
        void recordInteraction();
    };
}
