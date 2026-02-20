#include "Arduino.h"
#include "ble_communication.h"
#include <NimBLEDevice.h>
#include "wifi_control.h"


extern "C" {
#include "mqtt_service.h"
#include "nvs_flash.h"
}

static const char *TAG = "MAIN_APP";

// Shared flags from ble_communication.cpp
extern volatile bool g_wifi_scan_requested;
extern volatile bool g_wifi_credentials_updated;

extern void update_ssid_list(const std::string& list);

extern char set_ssid_chr_value[32];
extern char set_pass_chr_value[32];

void startMQTT() {
    if (!mqtt_credentials_exist()) {
        ESP_LOGI(TAG, "MQTT Credentials not found. Registering...");
        mqtt_register_device();
    } else {
        ESP_LOGI(TAG, "MQTT Credentials found. Logging in...");
    }
    mqtt_service_init();
    mqtt_service_start();
}

void setup() {
    initArduino();
    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_flash_init();
    }

    // 2. Setup WiFi infrastructure
    wifi_init_setup();

    // 3. Attempt to connect with "Hardcoded" or Last Known credentials
    ESP_LOGI(TAG, "Attempting WiFi connection...");
    wifi_init_sta("Arabadzhievi", "16042325");

    // 4. Wait for connection with a timeout
    int timeout = 0;
    while (IS_WIFI_CONNECTED == 0 && timeout < 10) {
        delay(1000);
        ESP_LOGI(TAG, "Connecting... (%d/10s)", timeout + 1);
        timeout++;
    }

    // 5. Logic Switch: WiFi Success vs Fallback to BLE
    if (IS_WIFI_CONNECTED == 1) {
        ESP_LOGI(TAG, "WiFi Connected! Proceeding to MQTT.");
        startMQTT();
    } else {
        ESP_LOGW(TAG, "WiFi Connection failed. Starting BLE fallback...");
        startBLE();
    }
}

void loop() {
    // Handle Scan Request safely
    if (g_wifi_scan_requested) {
        g_wifi_scan_requested = false;
        // Call your safe scan function (from wifi_control.cpp)
        std::string results = perform_wifi_scan();
        // Update BLE with results
        update_ssid_list(results);
    }

    // Handle Credentials safely
    if (g_wifi_credentials_updated) {
        g_wifi_credentials_updated = false;
        change_wifi(set_ssid_chr_value, set_pass_chr_value);
    }

    delay(100);
}
