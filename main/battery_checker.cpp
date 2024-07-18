#include "battery_checker.hpp"

using namespace App;

static void batteryCheckerTask(void *arg)
{
    auto batteryChecker = static_cast<BatteryChecker *>(arg);
    uint8_t battery_level = batteryChecker->checkBatteryLevel();
    batteryChecker->batteryListener(battery_level);
    vTaskDelete(NULL);
}

BatteryChecker::BatteryChecker(gpio_num_t enablePin, gpio_num_t checkPin, uint16_t timeoutSeconds)
{
    this->enablePin = enablePin;
    this->checkPin = checkPin;
    this->timeoutSeconds = timeoutSeconds;
    this->timeoutListener = [this](Timer *timer)
    {
        xTaskCreate(batteryCheckerTask, "batteryCheckerTask", 3 * 1024, this, configMAX_PRIORITIES - 2, taskHandle);
    };
};

void BatteryChecker::init(BatteryListener listener)
{
    this->batteryListener = listener;
    this->timer.init(this->timeoutListener);
    batteryLevel = checkBatteryLevel();
};

void BatteryChecker::stop()
{
    timer.stop();
};
static uint8_t mockBatteryValue = 10;

void BatteryChecker::start()
{
    timer.startPeriodic(timeoutSeconds);
};

uint8_t BatteryChecker::checkBatteryLevel()
{
    // pretend that it's working
    ESP_LOGI(TAG, "Battery checker task started");
    vTaskDelay(pdMS_TO_TICKS(500));

    mockBatteryValue += 10;
    if (mockBatteryValue > 100)
    {
        mockBatteryValue = 1;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    return mockBatteryValue;
};
