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
    this->adc_cali_handle = NULL;
    this->adc_handle = NULL;
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

    ESP_RETURN_ON_ERROR(initAdc(), TAG, "Init adc");

    batteryLevel = checkBatteryLevel();
    return ESP_OK;
};

esp_err_t BatteryChecker::initAdc()
{

    this->adc_channel = (adc_channel_t)this->checkPin;
    ESP_LOGI(TAG, "ADC_CHANNEL %d", this->adc_channel);

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
    ESP_RETURN_ON_ERROR(adc_calibration_init(), TAG, "ADC calibration failed");
    return ESP_OK;
};

esp_err_t BatteryChecker::deinit()
{
    if (!adc_cali_handle)
        return ESP_OK;
    ESP_RETURN_ON_ERROR(adc_cali_delete_scheme_curve_fitting(adc_cali_handle), TAG, "Cali deinit");
    return adc_oneshot_del_unit(adc_handle);
};

void BatteryChecker::stop()
{
    timer.stop();
};

void BatteryChecker::start()
{
    timer.startPeriodic(timeoutSeconds);
};

uint8_t BatteryChecker::checkBatteryLevel()
{
    ESP_LOGI(TAG, "Battery checker task started");

    int calibrated;
    esp_err_t result;
    int sum = 0;
    for (int i = 0; i < BCH_TAKES_NUM; i++)
    {
        int raw = 0;
        vTaskDelay(pdMS_TO_TICKS(BCH_TAKES_DELAY_MILLIS));
        gpio_set_level(enablePin, 1);
        vTaskDelay(pdMS_TO_TICKS(20));
        result = adc_oneshot_read(this->adc_handle, this->adc_channel, &raw);
        if (result != ESP_OK)
        {
            ESP_LOGE(TAG, "Adc read failed with code %d", result);
            return 0;
        }
        sum += raw;
        gpio_set_level(enablePin, 0);
    }
    int raw = sum / BCH_TAKES_NUM;

    result = adc_cali_raw_to_voltage(this->adc_cali_handle, raw, &calibrated);

    if (result != ESP_OK)
    {
        ESP_LOGE(TAG, "Adc calibration convert failed with code %d", result);
        return 0;
    }

    ESP_LOGI(TAG, "RAW %d", raw);
    ESP_LOGI(TAG, "CALIBRATED %d", calibrated);
    uint8_t v = calibrated * 100 / 1450;

    return v;
};

esp_err_t BatteryChecker::adc_calibration_init()
{
    ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = BCH_ADC_UNIT,
        .chan = adc_channel,
        .atten = BCH_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    return adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle);
}
