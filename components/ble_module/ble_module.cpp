#include "ble_module.hpp"

using namespace ble;
using namespace BLE_TAG;

BleModule::BleModule() : buffer{0}
{
    singletonBleModule = this;
    this->resetTimeoutListener = [this](Timer *timer)
    { this->onResetTimeout(timer); };
}
BleModule::~BleModule() { singletonBleModule = nullptr; }
void BleModule::init(EventListener connectionListener, EventListener suspendListener)
{
    this->connectionListener = connectionListener;
    this->suspendListener = suspendListener;
    this->resetTimer.init(this->resetTimeoutListener);
};
void BleModule::onConnected(bool isConnected)
{
    connectionListener(isConnected);
    if (!isConnected)
        esp_hid_ble_gap_adv_start();
};

void BleModule::onSuspended(bool isSuspended)
{
    this->is_suspended = isSuspended;
    this->suspendListener(isSuspended);
};

void BleModule::onResetTimeout(Timer *timer)
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
        this->resetTimer.startOneShot(SEND_RESET_AFTER_AUTH_TIMEOUT_SECONDS);
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

uint8_t BleModule::encodeButtonState(uint8_t currentState, uint8_t button, bool flag)
{
    // Check the current state of each button
    bool leftPressed = (currentState == 1 || currentState == 3 || currentState == 2);
    bool topPressed = (currentState == 1 || currentState == 7 || currentState == 0);
    bool rightPressed = (currentState == 7 || currentState == 5 || currentState == 6);
    bool bottomPressed = (currentState == 3 || currentState == 5 || currentState == 4);

    // Update the state based on the input

    // Update the state based on the input
    if (flag)
    {
        switch (button)
        {
        case 0:
            topPressed = true;
            break;
        case 1:
            rightPressed = true;
            break;
        case 2:
            bottomPressed = true;
            break;
        case 3:
            leftPressed = true;
            break;
        }
    }
    else
    {
        switch (button)
        {
        case 0:
            topPressed = false;
            break;
        case 1:
            rightPressed = false;
            break;
        case 2:
            bottomPressed = false;
            break;
        case 3:
            leftPressed = false;
            break;
        }
    }

    // Combine the states to produce the new bitmask
    if (topPressed && leftPressed)
        return 1; // Top Left
    if (leftPressed && bottomPressed)
        return 3; // Bottom Left
    if (bottomPressed && rightPressed)
        return 5; // Bottom Right
    if (rightPressed && topPressed)
        return 7; // Top Right
    if (topPressed)
        return 0; // Top
    if (rightPressed)
        return 6; // Right
    if (bottomPressed)
        return 4; // Bottom
    if (leftPressed)
        return 2; // Left

    return 8; // Center (default)
}

esp_err_t BleModule::sendReset()
{
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = 127; // axis 0 left joystick left of right - to zero
    buffer[4] = 127; // axis 1  left joystick up or down - to zero
    buffer[5] = 127; // axis 2  right joystick left or right - to zero
    buffer[6] = 127; // axis 3 right joystick up or down - to zero
    buffer[7] = 0;   // axis 5 left rudder -  to min
    buffer[8] = 0;   // axis 4  right rudder - to min
    buffer[0] = 8;
    /*
     * 0 - 0000 top
     * 1 - 0001 top right
     * 2 - 0010 right
     * 3 - 0011 bottom right
     * 4 - 0100 bottom
     * 5 - 0101 bottom left
     * 6 - 0110 left
     * 7 - 0111 top left
     * 8 - 1000 center
     */
    esp_err_t res = esp_hidd_dev_input_set(device_handle, 0, 0x03, buffer, sizeof(buffer));
    ESP_LOGI(TAG, "Send reset %d, buffer %d%d%d%d%d%d%d%d", res, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
    return res;
}

esp_err_t BleModule::sendButtonPress(uint8_t button, bool isPressed)
{

    if (isPressed)
    {
        switch (button)
        {
        case 0:
            buffer[2] |= BIT(6);
            break;
        case 1:
            buffer[2] |= BIT(5);
            break;
        case 2:
            buffer[2] |= BIT(4);
            break;
        }
    }
    else
    {
        switch (button)
        {
        case 0:
            buffer[2] &= ~BIT(6);
            // buffer[0] = encodeButtonState(buffer[0], 0, false); // top released
            break;
        case 1:
            buffer[2] &= ~BIT(5);
            // buffer[0] = encodeButtonState(buffer[0], 1, false); // right released
            break;
        case 2:
            buffer[2] &= ~BIT(4);
            break;
        }
    }
    esp_err_t res = esp_hidd_dev_input_set(device_handle, 0, 0x03, buffer, sizeof(buffer));
    ESP_LOGI(TAG, "Send button result %d, buffer %d%d%d%d%d%d%d%d", res, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
    return res;
};
