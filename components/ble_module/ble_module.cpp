#include "ble_module.hpp"

using namespace ble;
using namespace BLE_TAG;

BleModule::BleModule() : buffer{0}
{
    singletonBleModule = this;
    this->timeoutListener = [this](Timer *timer)
    { this->onTimeout(timer); };
}
BleModule::~BleModule() { singletonBleModule = nullptr; }
void BleModule::init(ConnectionListener listener)
{
    this->connectionListener = listener;
    this->timer.init(this->timeoutListener);
};
void BleModule::onConnected(bool isConnected)
{
    connectionListener(isConnected);
    if (!isConnected)
        esp_hid_ble_gap_adv_start();
};

void BleModule::onSuspended(bool isSuspended) { this->is_suspended = isSuspended; };

void BleModule::onTimeout(Timer *timer)
{

    sendReset();
}

void BleModule::onStarted(bool isStarted)
{
    is_started = isStarted;
    if (isStarted)
        esp_hid_ble_gap_adv_start();
};

bool BleModule::isConnected()
{
    if (!device_handle)
        return false;
    else
        return esp_hidd_dev_connected(device_handle);
};

void BleModule::hidEventCallback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    esp_hidd_event_t event = (esp_hidd_event_t)id;
    esp_hidd_event_data_t *param = (esp_hidd_event_data_t *)event_data;
    switch (event)
    {
    case ESP_HIDD_START_EVENT:
    {
        ESP_LOGI(TAG, "START");
        onStarted(true);
        break;
    }
    case ESP_HIDD_CONNECT_EVENT:
    {
        ESP_LOGI(TAG, "CONNECT");
        onConnected(true);
        break;
    }
    case ESP_HIDD_PROTOCOL_MODE_EVENT:
    {
        ESP_LOGI(TAG, "PROTOCOL MODE[%u]: %s", param->protocol_mode.map_index, param->protocol_mode.protocol_mode ? "REPORT" : "BOOT");
        break;
    }
    case ESP_HIDD_CONTROL_EVENT:
    {
        ESP_LOGI(TAG, "CONTROL[%u]: %sSUSPEND", param->control.map_index, param->control.control ? "EXIT_" : "");
        if (param->control.control)
        {
            onSuspended(false);
        }
        else
        {
            onSuspended(true);
        }
        break;
    }
    case ESP_HIDD_OUTPUT_EVENT:
    {
        ESP_LOGI(TAG, "OUTPUT[%u]: %8s ID: %2u, Len: %d, Data:", param->output.map_index, esp_hid_usage_str(param->output.usage), param->output.report_id, param->output.length);
        ESP_LOG_BUFFER_HEX(TAG, param->output.data, param->output.length);
        break;
    }
    case ESP_HIDD_FEATURE_EVENT:
    {
        ESP_LOGI(TAG, "FEATURE[%u]: %8s ID: %2u, Len: %d, Data:", param->feature.map_index, esp_hid_usage_str(param->feature.usage), param->feature.report_id, param->feature.length);
        ESP_LOG_BUFFER_HEX(TAG, param->feature.data, param->feature.length);
        break;
    }
    case ESP_HIDD_DISCONNECT_EVENT:
    {
        ESP_LOGI(TAG, "DISCONNECT: %s", esp_hid_disconnect_reason_str(esp_hidd_dev_transport_get(param->disconnect.dev), param->disconnect.reason));
        onConnected(false);
        break;
    }
    case ESP_HIDD_STOP_EVENT:
    {
        onStarted(false);
        ESP_LOGI(TAG, "STOP");
        break;
    }
    default:
        break;
    }
    return;
};

static void ble_hidd_event_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
    singletonBleModule->hidEventCallback(handler_args, base, id, event_data);
};

static void auth_callback(bool isAuthenticated) { singletonBleModule->onAuthenticated(isAuthenticated); };

void BleModule::onAuthenticated(bool isAuthenticated)
{
    this->is_authenticated = isAuthenticated;
    if (this->is_authenticated)
    {
        ESP_LOGI(TAG, "Start timer");
        this->timer.startOneShot(2);
    }
    ESP_LOGI(TAG, "Is Authenticated %d", isAuthenticated);
};

void BleModule::start(uint8_t battery_level)
{
    esp_err_t ret;

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "setting hid gap, mode:%d", HID_DEV_MODE);
    ret = esp_hid_gap_init(HID_DEV_MODE, auth_callback);
    ESP_ERROR_CHECK(ret);

    ret = esp_hid_ble_gap_adv_init(ESP_HID_APPEARANCE_GAMEPAD, ble_hid_config.device_name);

    ESP_ERROR_CHECK(ret);
    if ((ret = esp_ble_gatts_register_callback(esp_hidd_gatts_event_handler)) != ESP_OK)
    {
        ESP_LOGE(TAG, "GATTS register callback failed: %d", ret);
        return;
    }
    ESP_LOGI(TAG, "setting ble device battery level %d", battery_level);
    ble_hid_config.battery_level = battery_level;
    ESP_ERROR_CHECK(esp_hidd_dev_init(&ble_hid_config, ESP_HID_TRANSPORT_BLE, ble_hidd_event_callback, &device_handle));
};

void BleModule::stop()
{
    ESP_ERROR_CHECK(esp_hidd_dev_deinit(device_handle));
};

esp_err_t BleModule::sendBatteryCharge(uint8_t charge)
{
    esp_err_t res = esp_hidd_dev_battery_set(device_handle, charge);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "Send battery charge with result %d", res);
    }
    return res;
};

esp_err_t BleModule::sendReset()
{
    buffer[1] = 0;
    buffer[2] |= BIT(7);;
    buffer[3] = 127; // axis 0 left joystick left of right - to zero
    buffer[4] = 127; // axis 1  left joystick up or down - to zero
    buffer[5] = 127; // axis 2  right joystick left or right - to zero
    buffer[6] = 127; // axis 3 right joystick up or down - to zero
    buffer[7] = 0;   // axis 5 left rudder -  to min
    buffer[8] = 0;   // axis 4  right rudder - to min
    // buffer[0] = 255; // axis 7  cursor left or right - to zero
                     // axis 6?  cursor left or right - to zero
    // buffer[9] = 255; // axis 6?  cursor left or right - to zero
    // buffer[10] = 255; // axis 6?  cursor left or right - to zero
    // buffer[11] = 255; // axis 6?  cursor left or right - to zero
    // buffer[12] = 255; // axis 6?  cursor left or right - to zero
    // buffer[13] = 255; // axis 6?  cursor left or right - to zero
    // buffer[14] = 255; // axis 6?  cursor left or right - to zero
    // buffer[15] = 255; // axis 6?  cursor left or right - to zero
    buffer[0] = 243; // axis 6?  cursor left or right - to zero

    esp_err_t res = esp_hidd_dev_input_set(device_handle, 0, 0x03, buffer, sizeof(buffer));
    ESP_LOGI(TAG, "Send reset %d, buffer %d%d%d%d%d%d%d%d", res, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
    return res;
}

esp_err_t BleModule::sendButtonPress(uint8_t button, bool isPressed)
{

    uint8_t index = 2;
    uint8_t bit = 6;

    if (button == 0)
    {
        index = 2;
        bit = 6;
    }
    if (button == 1)
    {
        index = 2;
        bit = 5;
    }
    if (button == 2)
    {
        index = 2;
        bit = 4;
    }
    if (isPressed)
    {
        buffer[index] |= BIT(bit);
    }
    else
    {
        buffer[index] &= ~BIT(bit);
    }

    esp_err_t res = esp_hidd_dev_input_set(device_handle, 0, 0x03, buffer, sizeof(buffer));
    ESP_LOGI(TAG, "Send button result %d, buffer %d%d%d%d%d%d%d%d", res, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
    return res;
};
