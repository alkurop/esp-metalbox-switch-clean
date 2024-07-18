#include "battery_checker.hpp"

using namespace App;

static void batteryCheckerTask(void *arg)
{
    auto batteryChecker = static_cast<BatteryChecker *>(arg);
    batteryChecker->runBatteryCheckerTask();
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
};

void BatteryChecker::stop()
{
    timer.stop();
};
static uint8_t mockBatteryValue;

void BatteryChecker::start()
{
    timer.startPeriodic(timeoutSeconds);
};

void BatteryChecker::runBatteryCheckerTask()
{
    // pretend that it's working
    ESP_LOGI(TAG, "Battery checker task started");
    vTaskDelay(pdMS_TO_TICKS(2000));
    mockBatteryValue += 10;
    if (mockBatteryValue > 100)
    {
        mockBatteryValue = 1;
    }
    this->batteryListener(mockBatteryValue);
    ESP_LOGI(TAG, "Battery checker task ended in %d seconds", 2);
    vTaskDelete(NULL);
};
