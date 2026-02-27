#include "Arduino.h"
#include "ble_communication.h"
#include <NimBLEDevice.h>
#include "wifi_control.h"

extern "C" {
#include "mqtt_service.h"
#include "nvs_flash.h"
#include "nvs.h"
}

static const char *TAG = "MAIN_APP";

extern volatile bool g_wifi_scan_requested;
extern volatile bool g_wifi_credentials_updated;

extern void update_ssid_list(const std::string& list);

extern char set_ssid_chr_value[32];
extern char set_pass_chr_value[32];

// Track MQTT state so we only start it once
bool mqtt_is_running = false;

void startMQTT() {
    if (!mqtt_credentials_exist()) {
        ESP_LOGI(TAG, "MQTT Credentials not found. Registering...");
        mqtt_register_device();
    } else {
        ESP_LOGI(TAG, "MQTT Credentials found. Logging in...");
    }
    mqtt_service_init();
    mqtt_service_start();
    mqtt_is_running = true; // Mark as running
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

    vTaskDelay(pdMS_TO_TICKS(400));


    // 3. Check NVS for saved credentials (NO MORE HARDCODING)
    nvs_handle_t my_handle;
    char saved_ssid[32] = {0};
    char saved_pass[64] = {0};
    size_t ssid_len = sizeof(saved_ssid);
    size_t pass_len = sizeof(saved_pass);
    bool has_saved_creds = false;

    esp_err_t err = nvs_open("wifi_store", NVS_READONLY, &my_handle);
    if (err == ESP_OK) {
        if (nvs_get_str(my_handle, "ssid", saved_ssid, &ssid_len) == ESP_OK &&
            nvs_get_str(my_handle, "pass", saved_pass, &pass_len) == ESP_OK) {
            has_saved_creds = true;
        }
        nvs_close(my_handle);
    }

    if (has_saved_creds) {
        ESP_LOGI(TAG, "Found saved WiFi credentials! Connecting to: %s", saved_ssid);
        wifi_init_sta(saved_ssid, saved_pass);
    } else {
        ESP_LOGW(TAG, "No saved WiFi credentials found. Skipping straight to BLE.");
    }

    // 4. Wait for connection with a timeout (Only wait if we actually tried to connect)
    int timeout = 0;
    if (has_saved_creds) {
        while (IS_WIFI_CONNECTED == 0 && timeout < 10) {
            delay(1000);
            ESP_LOGI(TAG, "Connecting... (%d/10s)", timeout + 1);
            timeout++;
        }
    }

    // 5. Logic Switch: WiFi Success vs Fallback to BLE
    if (IS_WIFI_CONNECTED == 1) {
        ESP_LOGI(TAG, "WiFi Connected! Proceeding to MQTT.");
        startMQTT();
    } else {
        ESP_LOGW(TAG, "WiFi not connected. Starting BLE fallback...");
        startBLE();
    }
}

void loop() {
    // 1. Handle Scan Request safely
    if (g_wifi_scan_requested) {
        g_wifi_scan_requested = false;
        std::string results = perform_wifi_scan();
        update_ssid_list(results);
    }

    // 2. Handle Credentials safely AND Save to NVS
    if (g_wifi_credentials_updated) {
        g_wifi_credentials_updated = false;

        // Save new credentials to NVS so they survive a reboot
        nvs_handle_t my_handle;
        if (nvs_open("wifi_store", NVS_READWRITE, &my_handle) == ESP_OK) {
            nvs_set_str(my_handle, "ssid", set_ssid_chr_value);
            nvs_set_str(my_handle, "pass", set_pass_chr_value);
            nvs_commit(my_handle);
            nvs_close(my_handle);
            ESP_LOGI(TAG, "New WiFi credentials permanently saved to NVS!");
        } else {
            ESP_LOGE(TAG, "Failed to open NVS to save credentials.");
        }

        // Apply the new credentials
        change_wifi(set_ssid_chr_value, set_pass_chr_value);
    }

    // 3. THE HANDOFF: Trigger MQTT once WiFi connects via BLE
    if (IS_WIFI_CONNECTED == 1 && !mqtt_is_running) {
        ESP_LOGI(TAG, "WiFi connected via BLE! Starting MQTT...");
        startMQTT();

        // Optional: stopBLE(); // Shut down BLE to save RAM/Power now that we are online
    }

    delay(100);
}