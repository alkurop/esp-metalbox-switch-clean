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

using namespace App;
#define B1_PIN GPIO_NUM_0
#define L1_PIN GPIO_NUM_1
#define BATTERY_CHECK_ENABLE_PIN GPIO_NUM_0
#define BATTERY_CHECK_PIN GPIO_NUM_0

#define PIN_SIZE 1
#define SLEEPER_TIMEOUT_SECONDS 10 * 60
#define BATTERY_CHECKER_TIMEOUT_SECONDS 1 * 20

gpio_num_t wakeUpPins[PIN_SIZE] = {B1_PIN};

static Button b1;
static Led l1;
static Sleeper sleeper(SLEEPER_TIMEOUT_SECONDS);
static BatteryChecker batteryChecker(BATTERY_CHECK_ENABLE_PIN, BATTERY_CHECK_PIN, BATTERY_CHECKER_TIMEOUT_SECONDS);

auto buttonPressListener = [](uint8_t number, bool state)
{
    l1.set(state);
    sleeper.recordInteraction();
};

auto beforeSleep = []()
{
    b1.uninstall();
    batteryChecker.stop();
};

auto afterWake = []()
{
    b1.install();
    batteryChecker.start();
};

auto onBatteryChecker = [](uint8_t value)
{
    ESP_LOGI(TAG, "Battery value %d", value);
};

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(b1.init(B1_PIN, 1, buttonPressListener));
    l1.init(L1_PIN);
    sleeper.init(beforeSleep, afterWake, wakeUpPins, PIN_SIZE);
    sleeper.start();
    batteryChecker.init(onBatteryChecker);
    batteryChecker.start();

    ESP_LOGI(TAG, ">>>>>>> connected");
}
