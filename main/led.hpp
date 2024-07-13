#pragma once
#include <driver/gpio.h>

namespace App
{
    class Led
    {
    private:
        gpio_num_t pin;
        bool state;

    public:
        void init(gpio_num_t _pin)
        {
            this->pin = _pin;
            gpio_config_t config{(1ULL << pin),
                                 GPIO_MODE_OUTPUT,
                                 GPIO_PULLUP_DISABLE,
                                 GPIO_PULLDOWN_DISABLE,
                                 GPIO_INTR_DISABLE};
            gpio_config(&config);
        }
        void set(bool val)
        {
            this->state = val;
            gpio_set_level(pin, static_cast<uint32_t>(val));
        }
        bool getState() { return state; }
    };

} // namespace app
