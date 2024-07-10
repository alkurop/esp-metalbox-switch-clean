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

static Button b1;
static Led l1;
static Sleeper s1;

auto buttonPressListener = [](uint8_t number, bool state)
{
    esp_rom_printf("button %d changed to %d\n", number, state);
    l1.set(state);
};

static void light_sleep_task(void *args)
{
    while (true)
    {

        
        // auto config = b1.createConfig(B1_PIN);
        /* Initialize GPIO */
        // gpio_config_t config = {
        //     .pin_bit_mask = BIT64(B1_PIN),
        //     .mode = GPIO_MODE_INPUT,
        //     .pull_up_en = GPIO_PULLUP_ENABLE,
        //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
        //     .intr_type = GPIO_INTR_DISABLE,
        // };

        // auto config = b1.createConfig(B1_PIN);
        // gpio_config(&config);
        /* Enable wake up from GPIO */

        b1.uninstall();
        gpio_wakeup_enable(B1_PIN, GPIO_INTR_LOW_LEVEL);
        printf("Button pin wakeup enabled\n");
        esp_sleep_enable_gpio_wakeup();
        esp_light_sleep_start();

        auto wakeup_reason = esp_sleep_get_wakeup_cause();
        printf("Woke up reason %d\n", wakeup_reason);

        gpio_wakeup_disable(B1_PIN);

        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);
        // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_BT);

        printf("Button pin wakeup disabled\n");
        b1.install();
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }

    vTaskDelete(NULL);
}

extern "C" void app_main(void)
{
    esp_err_t result = b1.init(B1_PIN, 1, buttonPressListener);
    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Init button failed with code %d", result);
    }
    l1.init(L1_PIN);

    xTaskCreate(light_sleep_task, "light_sleep_task", 4096, NULL, 6, NULL);
    ESP_LOGI(TAG, ">>>>>>> connected");
}
