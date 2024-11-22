#pragma once
#include <esp_log.h>
#include <functional>
#include <nvs_flash.h>
#include <esp_bt_defs.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_gatt_defs.h>
#include <esp_bt_main.h>
#include <esp_bt_device.h>
#include <esp_hidd.h>

#include "tag.hpp"
#include "esp_hid_gap.h"
#include "timer.hpp"

using namespace TMR;

namespace ble
{
    using ConnectionListener = std::function<void(bool)>;

    class BleModule
    {
    private:
        ConnectionListener connectionListener;
        esp_hidd_dev_t *device_handle;
        uint8_t protocol_mode;
        uint8_t buffer[16];
        bool is_suspended;
        bool is_started;
        bool is_authenticated;
        void onConnected(bool isConnected);
        void onSuspended(bool isSuspended);
        void onStarted(bool isStarted);
        esp_err_t sendReset();
        Timer timer;
        TimeoutListener timeoutListener;
        void onTimeout(Timer *timer);

    public:
        BleModule();
        ~BleModule();
        void init(ConnectionListener listener);
        void start(uint8_t battery_level);
        void stop();
        void onAuthenticated(bool isAuthenticated);
        bool isStarted() { return is_started; };
        bool isSuspended() { return is_suspended; };
        bool isAuthenticated() { return is_authenticated; };
        bool isConnected();
        esp_err_t sendBatteryCharge(uint8_t charge);
        esp_err_t sendButtonPress(uint8_t button, bool isPressed);
        void hidEventCallback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
    };

    static char *con(const char *first, const char *second)
    {
        int l1 = 0, l2 = 0;
        const char *f = first, *l = second;

        // find lengths (you can also use strlen)
        while (*f++)
            ++l1;
        while (*l++)
            ++l2;

        // allocate a buffer including terminating null char
        char *result = new char[l1 + l2 + 1];

        // then concatenate
        for (int i = 0; i < l1; i++)
            result[i] = first[i];
        for (int i = l1; i < l1 + l2; i++)
            result[i] = second[i - l1];

        // finally, "cap" result with terminating null char
        result[l1 + l2] = '\0';
        return result;
    }
    
  constexpr unsigned char mediaReportMap[] = {
        0x05, 0x01,					  // Usage Page (Generic Desktop Ctrls)
        0x09, 0x05,					  // Usage (Game Pad)
        0xA1, 0x01,					  // Collection (Application)
        0x85, 0x03,					  //   Report ID (3)
        0x05, 0x01,					  //   Usage Page (Generic Desktop Ctrls)
        0x75, 0x04,					  //   Report Size (4)
        0x95, 0x01,					  //   Report Count (1)
        0x25, 0x07,					  //   Logical Maximum (7)
        0x46, 0x3B, 0x01,			  //   Physical Maximum (315)
        0x65, 0x14,					  //   Unit (System: English Rotation, Length: Centimeter)
        0x09, 0x39,					  //   Usage (Hat switch)
        0x81, 0x42,					  //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
        0x45, 0x00,					  //   Physical Maximum (0)
        0x65, 0x00,					  //   Unit (None)
        0x75, 0x01,					  //   Report Size (1)
        0x95, 0x04,					  //   Report Count (4)
        0x81, 0x01,					  //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x09,					  //   Usage Page (Button)
        0x15, 0x00,					  //   Logical Minimum (0)
        0x25, 0x01,					  //   Logical Maximum (1)
        0x75, 0x01,					  //   Report Size (1)
        0x95, 0x0F,					  //   Report Count (15)
        0x09, 0x12,					  //   Usage (0x12)
        0x09, 0x11,					  //   Usage (0x11)
        0x09, 0x14,					  //   Usage (0x14)
        0x09, 0x13,					  //   Usage (0x13)
        0x09, 0x0D,					  //   Usage (0x0D)
        0x09, 0x0C,					  //   Usage (0x0C)
        0x09, 0x0B,					  //   Usage (0x0B)
        0x09, 0x0F,					  //   Usage (0x0F)
        0x09, 0x0E,					  //   Usage (0x0E)
        0x09, 0x08,					  //   Usage (0x08)
        0x09, 0x07,					  //   Usage (0x07)
        0x09, 0x05,					  //   Usage (0x05)
        0x09, 0x04,					  //   Usage (0x04)
        0x09, 0x02,					  //   Usage (0x02)
        0x09, 0x01,					  //   Usage (0x01)
        0x81, 0x02,					  //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x75, 0x01,					  //   Report Size (1)
        0x95, 0x01,					  //   Report Count (1)
        0x81, 0x01,					  //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x01,					  //   Usage Page (Generic Desktop Ctrls)
        0x15, 0x01,					  //   Logical Minimum (1)
        0x26, 0xFF, 0x00,			  //   Logical Maximum (255)
        0x09, 0x01,					  //   Usage (Pointer)
        0xA1, 0x00,					  //   Collection (Physical)
        0x09, 0x30,					  //     Usage (X)
        0x09, 0x31,					  //     Usage (Y)
        0x75, 0x08,					  //     Report Size (8)
        0x95, 0x02,					  //     Report Count (2)
        0x81, 0x02,					  //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,						  //   End Collection
        0x09, 0x01,					  //   Usage (Pointer)
        0xA1, 0x00,					  //   Collection (Physical)
        0x09, 0x32,					  //     Usage (Z)
        0x09, 0x35,					  //     Usage (Rz)
        0x75, 0x08,					  //     Report Size (8)
        0x95, 0x02,					  //     Report Count (2)
        0x81, 0x02,					  //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,						  //   End Collection
        0x05, 0x02,					  //   Usage Page (Sim Ctrls)
        0x75, 0x08,					  //   Report Size (8)
        0x95, 0x02,					  //   Report Count (2)
        0x15, 0x00,					  //   Logical Minimum (0)
        0x26, 0xFF, 0x00,			  //   Logical Maximum (255)
        0x09, 0xC5,					  //   Usage (Brake)
        0x09, 0xC4,					  //   Usage (Accelerator)
        0x81, 0x02,					  //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x05, 0x0C,					  //   Usage Page (Consumer)
        0x15, 0x00,					  //   Logical Minimum (0)
        0x25, 0x01,					  //   Logical Maximum (1)
        0x09, 0xE9,					  //   Usage (Volume Increment)
        0x09, 0xEA,					  //   Usage (Volume Decrement)
        0x75, 0x01,					  //   Report Size (1)
        0x95, 0x02,					  //   Report Count (2)
        0x81, 0x02,					  //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x09, 0xCD,					  //   Usage (Play/Pause)
        0x95, 0x01,					  //   Report Count (1)
        0x81, 0x02,					  //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x05,					  //   Report Count (5)
        0x81, 0x01,					  //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x85, 0x05,					  //   Report ID (5)
        0x06, 0x0F, 0x00,			  //   Usage Page (PID Page)
        0x09, 0x97,					  //   Usage (0x97)
        0x75, 0x10,					  //   Report Size (16)
        0x95, 0x02,					  //   Report Count (2)
        0x27, 0xFF, 0xFF, 0x00, 0x00, //   Logical Maximum (65534)
        0x91, 0x02,					  //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0xC0,		
    }; 

    static esp_hid_raw_report_map_t ble_report_maps[] = {
        {.data = mediaReportMap, .len = sizeof(mediaReportMap)}};

    static esp_hid_device_config_t ble_hid_config = {
        .vendor_id = 0x18d1,
        .product_id = 0x9400,
        .version = 0x100,
        .device_name = con("Metalbox ", CONFIG_DEVICE_ID),
        .manufacturer_name = "Metalbox",
        .serial_number = CONFIG_DEVICE_ID,
        .report_maps = ble_report_maps,
        .report_maps_len = 1,
        .battery_level = 20
        };
    static BleModule *singletonBleModule;
    
}
