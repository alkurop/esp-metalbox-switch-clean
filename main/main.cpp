/*
 * Alex me
 */
#include "esp_log.h"
#include "tag.hpp"
#include "button.hpp"
#include "led.hpp"

using namespace App;

static Button b1;
static Led l1;

auto buttonPressListener = [](uint8_t number, bool state)
{
    esp_rom_printf("button %d changed to %d\n", number, state);
    l1.set(state);
};

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, ">>>>>>> connected");
    b1.init(GPIO_NUM_1, 1, buttonPressListener);
    l1.init(GPIO_NUM_0);
}
