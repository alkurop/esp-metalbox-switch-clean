#pragma once
#include <functional>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "tag.hpp"

#define BUTTON_DEBOUNCE_MILLIS 30

namespace ButtonHandler
{
    void button_handler(void *arg);
}

namespace App
{
    using ButtonListener = std::function<void(uint8_t, bool)>;
    class Button
    {
    private:
        ButtonListener listener;
        TickType_t next;
        gpio_num_t pin;
        uint8_t number;
        bool state;
        std::function<void(void)> margin;

    public:
        void init(gpio_num_t pin, uint8_t number, ButtonListener listener);
        bool getState() { return state; }
        void ping();
        void toggle(bool value);
    };
}
