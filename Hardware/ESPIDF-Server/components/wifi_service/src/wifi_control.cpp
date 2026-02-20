#include <nvs_flash.h>
#include <cstdint>
#include <cstring>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include <string>

static const char* TAG = "WIFI_CONTROL";

static bool is_user_initiated_disconnect = false;

int IS_WIFI_CONNECTED = 0;

//TODO: Add connections without password.
#define WIFI_SSID "Arabadzhievi"
#define WIFI_PASSWORD "16042325"

/**
 * @brief Performs a blocking WiFi scan safely.
 */
std::string perform_wifi_scan() {
    wifi_mode_t mode;
    if (esp_wifi_get_mode(&mode) != ESP_OK) {
        return "WiFi not initialized";
    }

    // 1. GAG THE EVENT HANDLER
    // This stops wifi_event_handler from fighting us.
    is_user_initiated_disconnect = true;

    // 2. Force the radio into an IDLE state for a safe scan
    ESP_LOGW(TAG, "Disconnecting to allow safe scan...");
    esp_wifi_disconnect();

    // Give the background event loop a moment to process the disconnect
    vTaskDelay(pdMS_TO_TICKS(1000));

    std::string ssid_list = "";

    // Leaving scan_time empty uses default timings for BLE Coexistence.
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE
    };

    ESP_LOGI(TAG, "Starting Safe WiFi Scan...");

    // 3. Start scan (Blocking)
    esp_err_t err = esp_wifi_scan_start(&scan_config, true);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Scan failed to start: %d", err);
        // Ungag and resume if it fails
        is_user_initiated_disconnect = false;
        esp_wifi_connect();
        return "Scan failed - System Busy";
    }

    // 4. Retrieve Results
    uint16_t number = 20;
    wifi_ap_record_t ap_info[20];
    uint16_t ap_count = 0;

    esp_wifi_scan_get_ap_num(&ap_count);
    esp_wifi_scan_get_ap_records(&number, ap_info);

    for (int i = 0; i < ap_count; i++) {
        ssid_list += reinterpret_cast<char *>(ap_info[i].ssid);
        ssid_list += "\n";
    }

    // 5. UN-GAG the handler and resume connection
    ESP_LOGI(TAG, "Scan complete, resuming connection...");
    is_user_initiated_disconnect = false;
    esp_wifi_connect();

    return ssid_list.empty() ? "No networks found" : ssid_list;
}



/**
 * @brief Event handler for WiFi and IP events
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        // Reduced log spam here
        if (event_id != WIFI_EVENT_WIFI_READY) {
            ESP_LOGI(TAG, "WiFi event: %ld", event_id);
        }
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_BEACON_TIMEOUT) {
        ESP_LOGW(TAG, "WiFi Beacon Timeout! Signal might be too weak.");
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        auto* disconn = static_cast<wifi_event_sta_disconnected_t *>(event_data);
        ESP_LOGW(TAG, "WiFi disconnected, reason: %d", disconn->reason);

        IS_WIFI_CONNECTED = 0;

        // If a scan or user requested this, DO NOT reconnect automatically
        if (!is_user_initiated_disconnect) {
            ESP_LOGI(TAG, "Reconnecting to WiFi...");
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_wifi_connect();
        } else {
            ESP_LOGI(TAG, "Disconnect was intentionally initiated; auto-reconnect paused.");
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        IS_WIFI_CONNECTED = 1;
        ESP_LOGI(TAG, "Got IP address. Wi-Fi Connected!");
    }
}

/**
 * @brief Initializes the WiFi system
 */
void wifi_init_setup()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

/**
 * @brief Initializes and starts the WiFi station
 */
void wifi_init_sta(char* ssid, const char* password)
{
    wifi_config_t wifi_config = {};

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    strlcpy(reinterpret_cast<char *>(wifi_config.sta.ssid), ssid, sizeof(wifi_config.sta.ssid));
    strlcpy(reinterpret_cast<char *>(wifi_config.sta.password), password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_connect();
}

/**
 * @brief Changes the current WiFi credentials and reconnects
 */
void change_wifi(char* new_ssid, const char* new_password)
{
    ESP_LOGI(TAG, "Changing WiFi to SSID: %s", new_ssid);

    // Stop auto-reconnect
    is_user_initiated_disconnect = true;

    esp_wifi_disconnect();

    // Give the stack a moment to drop the connection
    vTaskDelay(pdMS_TO_TICKS(200));

    // Reset flag so the new connection attempt will auto-retry if needed
    is_user_initiated_disconnect = false;

    wifi_init_sta(new_ssid, new_password);
}