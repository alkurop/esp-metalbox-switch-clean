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
#include "timer.hpp"

using namespace App;
using namespace ble;
using namespace button;
using namespace sleeper;
using namespace bchk;
using namespace TMR;

#define B1_PIN GPIO_NUM_3
#define B2_PIN GPIO_NUM_4
#define B3_PIN GPIO_NUM_5
#define L1_PIN GPIO_NUM_1
#define SEND_OFF_PIN GPIO_NUM_6
#define BATTERY_CHECK_ENABLE_PIN GPIO_NUM_7
#define BATTERY_CHECK_PIN GPIO_NUM_0

#define PIN_SIZE 3
#define SLEEPER_TIMEOUT_SECONDS 10 * 60
#define BATTERY_CHECKER_TIMEOUT_SECONDS 60 * 2
#define LAMP_OFF_TIME_SECONDS 10

gpio_num_t wakeUpPins[] = {B1_PIN, B2_PIN, B3_PIN};

static Button button1;
static Button button2;
static Button button3;
static Led led1;
static Sleeper sleeper1(SLEEPER_TIMEOUT_SECONDS);
static BatteryChecker batteryChecker(BATTERY_CHECK_ENABLE_PIN, BATTERY_CHECK_PIN, BATTERY_CHECKER_TIMEOUT_SECONDS);
static BleModule ble1;
static Timer lampOffTimer;

auto lampOffCallback = [](Timer *timer)
{
    led1.set(false);
};

auto buttonPressListener = [](uint8_t number, bool state)
{
    led1.set(!state);
    sleeper1.recordInteraction();
    ble1.sendButtonPress(number, state);
};

auto beforeSleep = []()
{
    button1.uninstall();
    button2.uninstall();
    button3.uninstall();
    batteryChecker.stop();
    ble1.stop();
};

auto afterWake = []()
{
    button1.install();
    button2.install();
    button3.install();
    batteryChecker.start();
    ble1.start(batteryChecker.getBatteryLevel());
};

auto onBatteryChecker = [](uint8_t value)
{
    ESP_LOGI(TAG, "Battery value %d", value);
    ble1.sendBatteryCharge(value);
};

auto connectionListener = [](bool connected)
{
    ESP_LOGI(TAG, "Ble Connected %d", connected);
};

extern "C" void app_main(void)
{
    led1.init(L1_PIN);
    led1.set(true);

    lampOffTimer.init(lampOffCallback);
    lampOffTimer.startOneShot(LAMP_OFF_TIME_SECONDS);
    
    ESP_ERROR_CHECK(button3.init(B3_PIN, 3, buttonPressListener));
    ESP_ERROR_CHECK(button1.init(B1_PIN, 1, buttonPressListener));
    ESP_ERROR_CHECK(button2.init(B2_PIN, 2, buttonPressListener));

    sleeper1.init(beforeSleep, afterWake, wakeUpPins, PIN_SIZE);
    sleeper1.start();
    auto result = batteryChecker.init(onBatteryChecker);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Battery checker init failed with status %d", result);
    }
    batteryChecker.start();

    ble1.init(connectionListener);
    ble1.start(batteryChecker.getBatteryLevel());

    ESP_LOGI(TAG, ">>>>>>> ACTIVE");
}
