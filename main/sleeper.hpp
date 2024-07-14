#pragma once
#include <esp_log.h>
#include <esp_bit_defs.h>
#include <functional>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_check.h>
#include <esp_sleep.h>

#include "tag.hpp"
#include "timer.hpp"

using namespace std;
namespace App
{
    using SleeperCallback = function<void(void)>;

    class Sleeper
    {
    private:
        // callbacks
        SleeperCallback beforeSleep;
        SleeperCallback afterWake;
        TimeoutListener timeoutListener;
        void onTimeout(Timer *);

        // params
        gpio_num_t *wakeUpPins;
        uint8_t pinCount;
        uint32_t timeoutSeconds;
        Timer timer;

        // state
        int16_t lastInteractionSeconds;

        void goToSleep();

    public:
        Sleeper(uint32_t timeoutSeconds);
        void init(
            SleeperCallback beforeSleep,
            SleeperCallback afterWake,
            gpio_num_t *wakeUpPins,
            uint8_t pinCount);

        void start();
        void recordInteraction();
    };
}
