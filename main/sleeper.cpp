#include "sleeper.hpp"

using namespace App;

Sleeper::Sleeper(uint8_t timeoutSeconds)
{

    this->timeoutSeconds = timeoutSeconds;
    this->timeoutListener = [this](Timer *timer)
    { this->onTimeout(timer); };
};

void Sleeper::init(SleeperCallback beforeSleep, SleeperCallback afterWake, gpio_num_t *wakeUpPins, uint8_t pinCount)
{
    this->afterWake = afterWake;
    this->beforeSleep = beforeSleep;
    this->timer.init(this->timeoutListener);
    this->wakeUpPins = wakeUpPins;
    this->pinCount = pinCount;
};

void Sleeper::recordInteraction()
{
    lastInteractionSeconds = MICRO_TO_SEC(esp_timer_get_time());
};

void Sleeper::start()
{
    timer.startOneShot(timeoutSeconds);
};
void Sleeper::onTimeout(Timer *timer)
{

    auto now = MICRO_TO_SEC(esp_timer_get_time());
    auto deltaSeconds = now - lastInteractionSeconds;
    ESP_LOGI(TAG, "On timeout delta seconds %d timeout seconds %d", deltaSeconds, timeoutSeconds);

    if (deltaSeconds >= this->timeoutSeconds)
    {
        goToSleep();
        // happens after wake up
        lastInteractionSeconds = MICRO_TO_SEC(esp_timer_get_time());
        timer->startOneShot(timeoutSeconds);
    }
    else
    {
        auto timeout = timeoutSeconds - deltaSeconds;
        ESP_LOGI(TAG, "Restart timer, with timeout  %d", timeout);
        timer->startOneShot(timeout);
    }
};

static void iterate(gpio_num_t *pins, uint8_t pinCount, function<void(gpio_num_t)> callback)
{
    auto i = pins;
    for (uint8_t k = 0; k < pinCount; k++)
    {
        callback(*i);
        i++;
    }
};

void Sleeper::goToSleep()
{
    this->beforeSleep();
    auto enableWakeup = [](gpio_num_t item)
    {
        ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(BIT(item), ESP_GPIO_WAKEUP_GPIO_LOW));
    };
    iterate(wakeUpPins, pinCount, enableWakeup);

    ESP_LOGI(TAG, "Deep sleep start");
    // esp_deep_sleep_start();
    // auto wakeup_reason = esp_sleep_get_wakeup_cause();
    // ESP_LOGI(TAG, "Wake up with reason %d\n", wakeup_reason);

    auto disableWakeup = [](gpio_num_t item)
    {
        ESP_ERROR_CHECK(gpio_wakeup_disable(item));
    };
    iterate(wakeUpPins, pinCount, disableWakeup);

    ESP_ERROR_CHECK(esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO));

    this->afterWake();
};
