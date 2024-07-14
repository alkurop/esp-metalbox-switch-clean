#include "button.hpp"
using namespace App;

IRAM_ATTR void ButtonHandler::button_handler(void *arg)
{
    auto button = static_cast<Button *>(arg);
    button->ping();
};

esp_err_t Button::init(gpio_num_t pin, uint8_t number, ButtonListener listener)
{
    this->state = false;
    this->listener = listener;
    this->pin = pin;
    auto config = createConfig(pin);

    ESP_RETURN_ON_ERROR(gpio_config(&config), TAG, "config button");

    ESP_LOGI(TAG, "gpio wakeup source is ready");
    return this->install();
};

esp_err_t Button::install()
{
    ESP_RETURN_ON_ERROR(gpio_install_isr_service(0), TAG, "install button isr");

    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(pin, ButtonHandler::button_handler, this), TAG, "gpio_isr_handler_add");
    return ESP_OK;
};

esp_err_t Button::uninstall()
{
    gpio_uninstall_isr_service();
    return ESP_OK;
};

void Button::toggle(bool value)
{
    state = !state;
    listener(number, state);
};

void Button::ping()
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
