/*
 * Alex me
 */
#include <esp_log.h>
#include <esp_bit_defs.h>

#include "tag.hpp"
#include "button.hpp"
#include "led.hpp"
#include "sleeper.hpp"
#include "battery_checker.hpp"
#include "ble_module.hpp"

using namespace App;
#define B1_PIN GPIO_NUM_0
#define L1_PIN GPIO_NUM_1
#define BATTERY_CHECK_ENABLE_PIN GPIO_NUM_0
#define BATTERY_CHECK_PIN GPIO_NUM_0

#define PIN_SIZE 1
#define SLEEPER_TIMEOUT_SECONDS 10 * 60
#define BATTERY_CHECKER_TIMEOUT_SECONDS 10 * 60

gpio_num_t wakeUpPins[PIN_SIZE] = {B1_PIN};

static Button button1;
static Led led1;
static Sleeper sleeper(SLEEPER_TIMEOUT_SECONDS);
static BatteryChecker batteryChecker(BATTERY_CHECK_ENABLE_PIN, BATTERY_CHECK_PIN, BATTERY_CHECKER_TIMEOUT_SECONDS);
static BleModule ble;

auto buttonPressListener = [](uint8_t number, bool state)
{
    led1.set(state);
    sleeper.recordInteraction();
    ble.sendButtonPress(number, state);

};

auto beforeSleep = []()
{
    button1.uninstall();
    batteryChecker.stop();
    ble.stop();
};

auto afterWake = []()
{
    button1.install();
    batteryChecker.start();
    ble.start();
};

auto onBatteryChecker = [](uint8_t value)
{
    ESP_LOGI(TAG, "Battery value %d", value);
    ble.sendBatteryCharge(value);
};

auto connectionListener = [](bool connected)
{
    ESP_LOGI(TAG, "Ble Connected %d", connected);
};

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(button1.init(B1_PIN, 1, buttonPressListener));
    led1.init(L1_PIN);
    sleeper.init(beforeSleep, afterWake, wakeUpPins, PIN_SIZE);
    sleeper.start();
    batteryChecker.init(onBatteryChecker);
    batteryChecker.start();
    ble.init(connectionListener);
    ble.start();

    ESP_LOGI(TAG, ">>>>>>> ACTIVE");
}
