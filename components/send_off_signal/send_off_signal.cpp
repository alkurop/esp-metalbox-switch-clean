#include "send_off_signal.hpp"

SendOffSignal::SendOffSignal(gpio_num_t pin)
{
    this->pin = pin;
};

void SendOffSignal::init()
{
    gpio_config_t config{(1ULL << pin),
                         GPIO_MODE_OUTPUT,
                         GPIO_PULLUP_DISABLE,
                         GPIO_PULLDOWN_ENABLE,
                         GPIO_INTR_DISABLE};
    gpio_config(&config);
};

void SendOffSignal::sendOff()
{
    gpio_set_level(pin, 1);
}
