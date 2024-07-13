#include "sleeper.hpp"

using namespace App;

Sleeper::Sleeper(const gpio_num_t *wakeUpPin, uint8_t pinCount, uint8_t timeoutSeconds)
{
    this->wakeUpPins = wakeUpPins;
    this->pinCount = pinCount;
    this->timeoutSeconds = timeoutSeconds;
    this->timeoutListener = [this](Timer *timer)
    {
        this->onTimeout(timer);
    };
};

void Sleeper::init(SleeperCallback beforeSleep, SleeperCallback afterWake)
{
    this->afterWake = afterWake;
    this->beforeSleep = beforeSleep;
    this->timer.init(this->timeoutListener);
};

void Sleeper::recordInteraction()
{
    lastInteraction = xTaskGetTickCount();
};

void Sleeper::start()
{
    timer.startOneShot(timeoutSeconds);
};
void Sleeper::onTimeout(Timer *timer)
{
    ESP_LOGI(TAG, "On timeout");
    // this->beforeSleep();
    // this->afterWake();
    timer->startOneShot(timeoutSeconds);
};

void Sleeper::cycle()
{
    this->beforeSleep();
    for (int i = 0; i < this->pinCount; i++)
    {
        ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(BIT(wakeUpPins[i]), ESP_GPIO_WAKEUP_GPIO_LOW));
    }
    ESP_LOGI(TAG, "Deep sleep start");
    esp_deep_sleep_start();
    auto wakeup_reason = esp_sleep_get_wakeup_cause();
    for (int i = 0; i < this->pinCount; i++)
    {
        ESP_ERROR_CHECK(gpio_wakeup_disable(wakeUpPins[i]));
    }

    ESP_ERROR_CHECK(esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO));

    ESP_LOGI(TAG, "Wake up with reason %d\n", wakeup_reason);
    
    this->afterWake();
};
