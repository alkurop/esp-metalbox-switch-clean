#pragma once
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class SendOffSignal
{
private:
    gpio_num_t pin;

public:
    SendOffSignal(gpio_num_t pin);
    void init();
    void sendOff();
};
