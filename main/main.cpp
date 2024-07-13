/*
 * Alex me
 */
#include "esp_log.h"
#include "tag.hpp"
#include "button.hpp"
#include "led.hpp"
#include "sleeper.hpp"
#include <esp_bit_defs.h>

using namespace App;
#define B1_PIN GPIO_NUM_0
#define L1_PIN GPIO_NUM_1

#define PIN_SIZE 1
const gpio_num_t wakeUpPins[PIN_SIZE] = {B1_PIN};

static Button b1;
static Led l1;
static Sleeper s1;

auto buttonPressListener = [](uint8_t number, bool state)
{
    esp_rom_printf("button %d changed to %d\n", number, state);
    l1.set(state);
    s1.recordInteraction();
};

auto beforeSleep = []()
{
    b1.uninstall();
};

auto afterWake = []()
{
    b1.install();
};

extern "C" void app_main(void)
{
    esp_err_t result = b1.init(B1_PIN, 1, buttonPressListener);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Init button failed with code %d", result);
    }
    l1.init(L1_PIN);
    s1.init(beforeSleep, afterWake, wakeUpPins, PIN_SIZE);
    ESP_LOGI(TAG, ">>>>>>> connected");
}
