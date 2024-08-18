#include "battery_checker.hpp"

using namespace TMR;
using namespace bchk;
using namespace BCHK_TAG;

static void batteryCheckerTask(void *arg)
{
    auto batteryChecker = static_cast<BatteryChecker *>(arg);
    uint8_t battery_level = batteryChecker->checkBatteryLevel();
    batteryChecker->batteryListener(battery_level);
    vTaskDelete(NULL);
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

esp_err_t BatteryChecker::init(BatteryListener listener)
{
    this->batteryListener = listener;
    this->timer.init(this->timeoutListener);

    gpio_config_t senderConfig{(1ULL << enablePin),
                               GPIO_MODE_OUTPUT,
                               GPIO_PULLUP_DISABLE,
                               GPIO_PULLDOWN_ENABLE,
                               GPIO_INTR_DISABLE};
    gpio_config(&senderConfig);

    initAdc();

    batteryLevel = checkBatteryLevel();
    return ESP_OK;
};

esp_err_t BatteryChecker::initAdc()
{

    if (this->checkPin != GPIO_NUM_4)
        return ESP_ERR_INVALID_ARG;
    this->adc_channel = ADC_CHANNEL_4;
    this->adc_cali_handle = NULL;

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };

    adc_oneshot_chan_cfg_t config = {
        .atten = BCH_ADC_ATTEN,
        .bitwidth = BCH_ADC_BW,
    };

    ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&init_config, &adc_handle), TAG, "ADC one shot new init failed");
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(adc_handle, adc_channel, &config), TAG, "ADC one shot channel config failed");
    ESP_RETURN_ON_ERROR(adc_calibration_init(BCH_ADC_UNIT, adc_channel, BCH_ADC_ATTEN, &adc_cali_handle), TAG, "ADC calibration failed");
    return ESP_OK;
};

esp_err_t BatteryChecker::deinit()
{
    return adc_cali_delete_scheme_curve_fitting(adc_cali_handle);
};

void BatteryChecker::stop()
{
    timer.stop();
};
static uint8_t mockBatteryValue = 10;

void BatteryChecker::start()
{
    timer.startPeriodic(timeoutSeconds);
};

uint8_t BatteryChecker::checkBatteryLevel()
{
    ESP_LOGI(TAG, "Battery checker task started");
    gpio_set_level(enablePin, 1);
    vTaskDelay(pdMS_TO_TICKS(2000));

    mockBatteryValue += 10;
    if (mockBatteryValue > 100)
    {
        mockBatteryValue = 1;
    }
    gpio_set_level(enablePin, 0);

    return mockBatteryValue;
};

esp_err_t BatteryChecker::adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = unit,
        .chan = channel,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    return adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
}
