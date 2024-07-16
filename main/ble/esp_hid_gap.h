/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once

#include <esp_err.h>
#include <esp_log.h>

#include <esp_bt.h>
#include <esp_bt_defs.h>
#include <esp_bt_main.h>
#include <esp_gap_bt_api.h>
#include <esp_hid_common.h>
#include <esp_gattc_api.h>
#include <esp_gatt_defs.h>
#include <esp_gap_ble_api.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#define HIDD_IDLE_MODE 0x00
#define HIDD_BLE_MODE 0x01
#define HIDD_BT_MODE 0x02
#define HIDD_BTDM_MODE 0x03

#define HID_DEV_MODE HIDD_BLE_MODE

#ifdef __cplusplus
extern "C"

{
#endif

    typedef void (*auth_callback_t)(bool is_authed); /**< function called when authentication is */

    typedef struct esp_hidh_scan_result_s
    {
        struct esp_hidh_scan_result_s *next;

        esp_bd_addr_t bda;
        const char *name;
        int8_t rssi;
        esp_hid_usage_t usage;
        esp_hid_transport_t transport; // BT, BLE or USB
        union
        {
            struct
            {
                esp_bt_cod_t cod;
                esp_bt_uuid_t uuid;
            } bt;
            struct
            {
                esp_ble_addr_type_t addr_type;
                uint16_t appearance;
            } ble;
        };
    } esp_hid_scan_result_t;

    esp_err_t esp_hid_gap_init(uint8_t mode, auth_callback_t cb);

    esp_err_t esp_hid_ble_gap_adv_init(uint16_t appearance, const char *device_name);

    esp_err_t esp_hid_ble_gap_adv_start(void);

#ifdef __cplusplus
}
#endif
