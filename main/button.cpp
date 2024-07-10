#include "button.hpp"
#include "esp_log.h"

IRAM_ATTR void ButtonHandler::button_handler(void *arg)
{
    auto button = reinterpret_cast<App ::Button *>(arg);
    button->ping();
};

void App::Button::init(gpio_num_t pin, uint8_t number, App::ButtonListener listener)
{
    this->state = false;
    this->listener = listener;
    this->pin = pin;
    gpio_config_t config{1ULL << pin,
                         GPIO_MODE_INPUT,
                         GPIO_PULLUP_ENABLE,
                         GPIO_PULLDOWN_DISABLE,
                         GPIO_INTR_ANYEDGE};
    gpio_config(&config);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(pin, ButtonHandler::button_handler, this);
};

void App::Button::toggle(bool value)
{
    state = !state;
    listener(number, state);
};

void App::Button::ping()
{
    bool value = !gpio_get_level(pin);
    bool isSameValue = this->state == value;
    if (!isSameValue)
    {
        TickType_t now = xTaskGetTickCount();
        bool debounceOver = now > this->next + BUTTON_DEBOUNCE_MILLIS / portTICK_PERIOD_MS;
        if (debounceOver)
        {
            this->next = now;
            state = value;
            listener(number, state);
        }
    }
}
