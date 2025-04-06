#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_uuid.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

// Scan service
// --------

// 31175f53-389f-4d19-8f69-6da32a28d50e
static const ble_uuid128_t ble_scan_service_uuid = BLE_UUID128_INIT(0x0e, 0xd5, 0x28, 0x2a, 0xa3, 0x6d, 0x69, 0x8f,
                                                                    0x19, 0x4d, 0x9f, 0x38, 0x53, 0x5f, 0x17, 0x31);

// 22946000-e86b-4831-adc8-27ab2cdb85fb
static const ble_uuid128_t ble_wifi_ssid_list_chr_uuid = BLE_UUID128_INIT(
    0xfb, 0x85, 0xdb, 0x2c, 0xab, 0x27, 0xc8, 0xad, 0x31, 0x48, 0x6b, 0xe8, 0x00, 0x60, 0x94, 0x22
);

// 3a6b1432-c481-45a7-a798-45ecd7d1ef41
// Empty is considered False, while any data is considered True
static const ble_uuid128_t ble_should_scan_chr_uuid = BLE_UUID128_INIT(
    0x41, 0xef, 0xd1, 0xd7, 0xec, 0x45, 0x98, 0xa7, 0xa7, 0x45, 0x81, 0xc4, 0x32, 0x14, 0x6b, 0x3a
);
// ----------
// Set service
//--------------

//4fafc201-1fb5-459e-8fcc-c5c9c331914b
static const ble_uuid128_t ble_set_service_uuid = BLE_UUID128_INIT(0x4b, 0x91, 0x31, 0xc3, 0xc9, 0xc5, 0xcc, 0x8f, 0x9e,
                                                                   0x45, 0xb5, 0x1f, 0x01, 0xc2, 0xaf, 0x4f);
// beb5483e-36e1-4688-b7f5-ea07361b26a8
static const ble_uuid128_t ble_wifi_ssid_set_chr_uuid = BLE_UUID128_INIT(
    0xa8, 0x26, 0x1b, 0x36, 0x07, 0xea, 0xf5, 0xb7, 0x88, 0x46, 0xe1, 0x36, 0x3e, 0x48, 0xb5, 0xbe
);

// ed9e4092-2c12-4a94-9d86-696058d16573
static const ble_uuid128_t ble_wifi_pass_set_chr_uuid = BLE_UUID128_INIT(
    0x73, 0x65, 0xd1, 0x58, 0x60, 0x69, 0x86, 0x9d, 0x94, 0x4a, 0x12, 0x2c, 0x92, 0x40, 0x9e, 0xed
);

// 4d5ea626-4ac9-4a71-9392-d0764ac7cefe
static const ble_uuid128_t ble_wifi_connection_status_chr_uuid = BLE_UUID128_INIT(
    0xfe, 0xce, 0xc7, 0x4a, 0x76, 0xd0, 0x92, 0x93, 0x71, 0x4a, 0xc9, 0x4a, 0x26, 0xa6, 0x5e, 0x4d
);

// ----------
// Declare services and their characteristics
//--------------------

static const struct ble_gatt_svc_def gatt_server_services[] = {
    {

        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &ble_scan_service_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = &ble_wifi_ssid_list_chr_uuid.u, //!! UUID as given above
                .access_cb = gatt_svr_chr_access,
                //!! Callback function. When ever this characrstic will be accessed by user, this function will execute
                .val_handle = &notification_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                //!! flags set permissions. In this case User can read this characterstic, can write to it,and get notified.
            },
            {
                .uuid = &ble_should_scan_chr_uuid.u, //!! UUID as given above
                .access_cb = gatt_svr_chr_access,
                //!! Callback function. When ever this characrstic will be accessed by user, this function will execute
                .val_handle = &notification_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
                //!! flags set permissions. In this case User can read this characterstic, can write to it,and get notified.
            },
            {
                nullptr
            }
        },
    },
    {

        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &ble_set_service_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                .uuid = &ble_wifi_ssid_set_chr_uuid.u, //!! UUID as given above
                .access_cb = gatt_svr_chr_access,
                //!! Callback function. When ever this characrstic will be accessed by user, this function will execute
                .val_handle = &notification_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                //!! flags set permissions. In this case User can read this characterstic, can write to it,and get notified.
            },
            {
                .uuid = &ble_wifi_pass_set_chr_uuid.u, //!! UUID as given above
                .access_cb = gatt_svr_chr_access,
                //!! Callback function. When ever this characrstic will be accessed by user, this function will execute
                .val_handle = &notification_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
                //!! flags set permissions. In this case User can read this characterstic, can write to it,and get notified.
            },            {
                .uuid = &ble_wifi_connection_status_chr_uuid.u, //!! UUID as given above
                .access_cb = gatt_svr_chr_access,
                //!! Callback function. When ever this characrstic will be accessed by user, this function will execute
                .val_handle = &notification_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                //!! flags set permissions. In this case User can read this characterstic, can write to it,and get notified.
            },
            {
                nullptr
            }
        },
    },
    {
        nullptr
    },
};


void ble_init()
{
}


void startNVS() // Non-volatile storage
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_flash_init();
    }
}
