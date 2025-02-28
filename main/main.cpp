/*
 * Alex me
 */
#include <esp_log.h>
#include <esp_bit_defs.h>
#include "esp_mac.h"

#include "tag.hpp"
#include "button.hpp"
#include "led.hpp"
#include "sleeper.hpp"
#include "battery_checker.hpp"
#include "ble_module.hpp"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "send_off_signal.hpp"

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
#define BLINK_PERIOD_MILLIS 250

gpio_num_t wakeUpPins[] = {B1_PIN, B2_PIN, B3_PIN};

static Button button1;
static Button button2;
static Button button3;
static Led led1;
static Sleeper sleeper1(SLEEPER_TIMEOUT_SECONDS);
static BatteryChecker batteryChecker(BATTERY_CHECK_ENABLE_PIN, BATTERY_CHECK_PIN, BATTERY_CHECKER_TIMEOUT_SECONDS);
static BleModule ble1;
static Timer blinkTimer;
static SendOffSignal sendOffSignal(SEND_OFF_PIN);

void startBlinkTimer()
{
    led1.set(true);
    blinkTimer.startPeriodicMillis(BLINK_PERIOD_MILLIS);
};
void stopBlinkTimer()
{
    led1.set(true);
    blinkTimer.stop();
};

auto onBatteryTooLow = []() {
    // board 3.2 does not support this 
    sendOffSignal.sendOff();
};

auto buttonPressListener = [](uint8_t number, bool state)
{
    ESP_LOGI(TAG, "Button value %d", number);
    led1.set(state);
    sleeper1.recordInteraction();
    ble1.sendButtonPress(number, state);
};

auto beforeSleep = []()
{
    led1.set(false);
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
    startBlinkTimer();
};

auto onBatteryChecker = [](uint8_t value)
{
    ESP_LOGI(TAG, "Battery value %d", value);
    ble1.sendBatteryCharge(value);
};

auto onBlinkTimer = [](Timer *timer)
{
    auto blink = !led1.getState();
    led1.set(blink);
};

auto suspendListener = [](bool suspended)
{
    if (suspended)
        sleeper1.manuallySleep();
};

auto connectionListener = [](bool connected)
{
    ESP_LOGI(TAG, "Ble Connected %d", connected);
    if (connected)
        stopBlinkTimer();
    else
        startBlinkTimer();
};

void generate_mac_address(esp_bd_addr_t rand_addr) {

    // Get the device ID string from sdkconfig
    const char *device_id = CONFIG_DEVICE_ID;
    int len = strlen(device_id);
    rand_addr[3] = (len >= 3) ? device_id[len - 3] : 0x00;
    rand_addr[4] = (len >= 2) ? device_id[len - 2] : 0x00;
    rand_addr[5] = (len >= 1) ? device_id[len - 1] : 0x00;
    ESP_LOGI(TAG, "Generated MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
        rand_addr[0], rand_addr[1], rand_addr[2],
        rand_addr[3], rand_addr[4], rand_addr[5]);
}

extern "C" void app_main(void)
{

    esp_bd_addr_t rand_addr = {0xD9, 0x31, 0x06, 0xa3, 0x35, 0xe2};
    generate_mac_address(rand_addr);
    auto err = esp_iface_mac_addr_set(rand_addr, ESP_MAC_BT);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Setting mac address failed: %d", err);
    }


    led1.init(L1_PIN);
    sendOffSignal.init();
    blinkTimer.init(onBlinkTimer);
    startBlinkTimer();

    ESP_ERROR_CHECK(button3.init(B3_PIN, 2, buttonPressListener));
    ESP_ERROR_CHECK(button1.init(B1_PIN, 0, buttonPressListener));
    ESP_ERROR_CHECK(button2.init(B2_PIN, 1, buttonPressListener));

    sleeper1.init(beforeSleep, afterWake, wakeUpPins, PIN_SIZE);
    sleeper1.start();
    auto result = batteryChecker.init(onBatteryChecker, onBatteryTooLow);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Battery checker init failed with status %d", result);
    }
    batteryChecker.start();

    ble1.init(connectionListener, suspendListener);
    ble1.start(batteryChecker.getBatteryLevel());

    ESP_LOGI(TAG, ">>>>>>> ACTIVE");
}
