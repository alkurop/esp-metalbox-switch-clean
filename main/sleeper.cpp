#include "sleeper.hpp"

esp_err_t App::Sleeper::cycle()
{
    this->beforeSleep();
    for (int i = 0; i < this->pin_count; i++)
    {
        ESP_RETURN_ON_ERROR(esp_deep_sleep_enable_gpio_wakeup(BIT(wakeUpPins[i]), ESP_GPIO_WAKEUP_GPIO_LOW), TAG, "enable gpio wakeup");
    }
    ESP_LOGI(TAG, "Deep sleep start");
    esp_deep_sleep_start();
    auto wakeup_reason = esp_sleep_get_wakeup_cause();
    for (int i = 0; i < this->pin_count; i++)
    {
        ESP_RETURN_ON_ERROR(gpio_wakeup_disable(wakeUpPins[i]), TAG, "disable gpio wakeup");
    }

    ESP_RETURN_ON_ERROR(esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO), TAG, "disable gpio wakeup source");
    ESP_LOGI(TAG, "Wake up with reason %d\n", wakeup_reason);
    this->afterWake();
    return ESP_OK;
};
