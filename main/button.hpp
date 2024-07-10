#pragma once
#include <functional>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "../tag.hpp"

#define BUTTON_DEBOUNCE_MILLIS 00

namespace ButtonHandler
{
    void button_handler(void *arg);
}

namespace App
{
    using ButtonListener = std::function<void(uint8_t, bool)>;
    using InterruptListener = std::function<void(void)>;
    class Button
    {
    private:
        ButtonListener listener;
        InterruptListener interruptListener;
        TickType_t next;
        gpio_num_t pin;
        uint8_t number;
        bool state;
        std::function<void(void)> margin;

        void toggle(bool value);

    public:
        void init(gpio_num_t pin,
                  uint8_t number,
                  ButtonListener listener,
                  InterruptListener interruptListener);
        bool getState() { return state; }
        void onInterrupt() { interruptListener(); };
        void ping();
    };
}
