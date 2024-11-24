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

#define BCH_ADC_ATTEN ADC_ATTEN_DB_6
#define BCH_ADC_BW ADC_BITWIDTH_DEFAULT
#define BCH_ADC_UNIT ADC_UNIT_1
#define BCH_TAKES_NUM 5
#define BCH_TAKES_DELAY_MILLIS 50
#define CALIBRATED_MIN 860
#define CALIBRATED_OFF_MIN 850
#define CALIBRATED_MAX 1300

using namespace TMR;

/**
 Todo:
 - add 100 nf capacitor to input
 - check if it works after putting to sleep
 - calibrate the voltage metering min max (now for 3,3 volt it shows 1625)

*/
namespace bchk
{
    using BatteryLevelListener = std::function<void(uint8_t)>;
    using BatteryTooLowListener = std::function<void(void)>;

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
        esp_err_t adc_calibration_init();
        int calculatePercentage(int input);

    public:
        BatteryLevelListener batteryListener;
        BatteryTooLowListener tooLowListener;

        BatteryChecker(gpio_num_t enablePin, gpio_num_t checkPin, uint16_t timeoutSeconds);
        esp_err_t init(BatteryLevelListener levelListener, BatteryTooLowListener tooLowListener);
        esp_err_t deinit();
        void start();
        void stop();
        uint8_t getBatteryLevel() { return batteryLevel; };
        uint8_t checkBatteryLevel();
    };

} // namespace App
