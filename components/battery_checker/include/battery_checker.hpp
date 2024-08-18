#pragma once

#include <esp_log.h>
#include <esp_check.h>
#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>

#include "timer.hpp"
#include "tag.hpp"

#define BCH_ADC_ATTEN ADC_ATTEN_DB_11
#define BCH_ADC_BW ADC_BITWIDTH_9
#define BCH_ADC_UNIT ADC_UNIT_1

using namespace TMR;

namespace bchk
{
    using BatteryListener = std::function<void(uint8_t)>;

    class BatteryChecker

    {
    private:
        Timer timer;
        uint16_t timeoutSeconds;
        gpio_num_t enablePin;
        gpio_num_t checkPin;
        TimeoutListener timeoutListener;
        uint8_t batteryLevel;
        TaskHandle_t *taskHandle;

        adc_oneshot_unit_handle_t adc_handle;
        adc_cali_handle_t adc_cali_handle;
        adc_channel_t adc_channel;

        esp_err_t initAdc();
        esp_err_t deinitAdc();
        esp_err_t adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);

    public:
        BatteryListener batteryListener;

        BatteryChecker(gpio_num_t enablePin, gpio_num_t checkPin, uint16_t timeoutSeconds);
        esp_err_t init(BatteryListener listener);
        esp_err_t deinit();
        void start();
        void stop();
        uint8_t getBatteryLevel() { return batteryLevel; };
        uint8_t checkBatteryLevel();
    };

} // namespace App
