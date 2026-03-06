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

extern void update_wifi_status(int status);


/**
 * @brief Performs a blocking WiFi scan safely.
 */
std::string perform_wifi_scan() {
    wifi_mode_t mode;
    if (esp_wifi_get_mode(&mode) != ESP_OK) {
        return "WiFi not initialized";
    }

    // 1. GAG THE EVENT HANDLER
    is_user_initiated_disconnect = true;

    std::string ssid_list = "";
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE
    };

    ESP_LOGI(TAG, "Starting Safe WiFi Scan...");

    // 2. The Retry Loop (Persistent but safe)
    esp_err_t err = ESP_FAIL;
    int retry_count = 0;

    // Try up to 5 times to get past the "System Busy" state
    while (retry_count < 5) {
        // Hammer the disconnect command to force it out of the CONNECTING state
        esp_wifi_disconnect();

        // Give it time to process the disconnect. It still errors sometimes but that's ok
        vTaskDelay(pdMS_TO_TICKS(1000));

        err = esp_wifi_scan_start(&scan_config, true); // true = blocking

        if (err == ESP_OK) {
            break; // Success! It scanned.
        } else if (err == ESP_ERR_WIFI_STATE) {
            ESP_LOGW(TAG, "WiFi still connecting. Forcing disconnect... (%d/5)", retry_count + 1);
            retry_count++;
        } else {
            ESP_LOGE(TAG, "Scan failed with unexpected error: %d", err);
            break;
        }
    }

    // If it STILL failed after 5 retries, give up *gracefully*
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not force WiFi into IDLE state for scan.");
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
        // Skip hidden networks (empty SSIDs)
        if (strlen(reinterpret_cast<char*>(ap_info[i].ssid)) == 0) {
            continue;
        }

        std::string next_ssid = reinterpret_cast<char*>(ap_info[i].ssid);
        next_ssid += "\n";

        // ENFORCE BLE LIMIT: Stop adding to the string if it will exceed 500 bytes.
        // We use 500 to safely stay under the 512-byte absolute maximum limit.
        if (ssid_list.length() + next_ssid.length() > 500) {
            ESP_LOGW(TAG, "Hit BLE 512-byte limit. Truncating remaining networks.");
            break;
        }

        ssid_list += next_ssid;
    }

    // 5. UN-GAG the handler and resume connection
    ESP_LOGI(TAG, "Scan complete, found %d networks. Resuming connection...", ap_count);
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
    if (event_base == WIFI_EVENT && event_id != WIFI_EVENT_WIFI_READY) {
        ESP_LOGI(TAG, "WiFi event: %ld", event_id);
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        auto* disconn = static_cast<wifi_event_sta_disconnected_t *>(event_data);

        IS_WIFI_CONNECTED = 0;

        // If the user didn't intentionally stop the Wi-Fi...
        if (!is_user_initiated_disconnect) {
            bool should_reconnect = true;

            // --- THE MAGIC: PARSE THE ESP32 REASON CODES ---

            // Reason 201: NO_AP_FOUND
            if (disconn->reason == WIFI_REASON_NO_AP_FOUND) {
                ESP_LOGE(TAG, "FAIL: Network not found.");
                update_wifi_status(2); // BLE Status 2
                should_reconnect = false; // Don't infinite loop on a dead SSID
            }
            // Reasons 2, 14, 15, 204: Various Handshake/Auth timeouts usually meaning Wrong Password
            else if (disconn->reason == WIFI_REASON_AUTH_EXPIRE ||
                     disconn->reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT ||
                     disconn->reason == WIFI_REASON_NO_AP_FOUND_W_COMPATIBLE_SECURITY ||
                     disconn->reason == WIFI_REASON_HANDSHAKE_TIMEOUT ||
                     disconn->reason == WIFI_REASON_MIC_FAILURE) {
                ESP_LOGE(TAG, "FAIL: Password Incorrect.");
                update_wifi_status(3); // BLE Status 3
                should_reconnect = false; // Don't infinite loop with a bad password
            }
            // Any other drop (Router rebooted, interference, etc.)
            else {
                ESP_LOGW(TAG, "Dropped. Reason code: %d", disconn->reason);
                // We keep it at status '0' (connecting) or set to '4' if you want a generic error
                // We DO want to automatically retry here, because routers are flaky.
            }

            if (should_reconnect) {
                ESP_LOGI(TAG, "Retrying connection...");
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_wifi_connect();
            } else {
                ESP_LOGW(TAG, "Fatal setup error. Pausing auto-reconnect. Waiting for BLE fix.");
            }

        } else {
            ESP_LOGI(TAG, "Disconnect was intentionally initiated; auto-reconnect paused.");
        }
    }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        IS_WIFI_CONNECTED = 1;
        ESP_LOGI(TAG, "Got IP address. Wi-Fi Connected!");

        // Update BLE app to show Success!
        update_wifi_status(1);
    }
}
/**
 * @brief Initializes the WiFi system
 */
void wifi_init_setup()
{
    ESP_ERROR_CHECK(esp_netif_init());

    // Create event loop if it doesn't exist
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err);
    }

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "WiFi hardware started and in READY state.");
}

/**
 * @brief Initializes and starts the WiFi station
 */
void wifi_init_sta(char* ssid, const char* password)
{
    wifi_config_t wifi_config = {};
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    strlcpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    // Mode is already set in setup, just update the config and connect
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_LOGI(TAG, "Connecting to SSID: %s", ssid);
    esp_wifi_connect();
}
/**
 * @brief Saves WiFi credentials to NVS
 */
void save_wifi_credentials(const char* ssid, const char* pass) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("wifi_store", NVS_READWRITE, &my_handle);
    if (err == ESP_OK) {
        nvs_set_str(my_handle, "ssid", ssid);
        nvs_set_str(my_handle, "pass", pass);
        nvs_commit(my_handle);
        nvs_close(my_handle);
        ESP_LOGI(TAG, "WiFi credentials saved to NVS");
    } else {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
}

/**
 * @brief Loads WiFi credentials from NVS
 * @return true if credentials were found
 */
bool load_wifi_credentials(char* ssid_out, char* pass_out, size_t max_len) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("wifi_store", NVS_READONLY, &my_handle);
    if (err != ESP_OK) return false;

    size_t ssid_len = max_len;
    size_t pass_len = max_len;

    esp_err_t err_ssid = nvs_get_str(my_handle, "ssid", ssid_out, &ssid_len);
    esp_err_t err_pass = nvs_get_str(my_handle, "pass", pass_out, &pass_len);

    nvs_close(my_handle);
    return (err_ssid == ESP_OK && err_pass == ESP_OK);
}


void change_wifi(char* new_ssid, const char* new_password)
{
    ESP_LOGI(TAG, "Changing WiFi to SSID: %s", new_ssid);

    // Reset BLE status to "Connecting"
    update_wifi_status(0);

    // Stop auto-reconnect
    is_user_initiated_disconnect = true;

    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(200));
    is_user_initiated_disconnect = false;

    wifi_init_sta(new_ssid, new_password);
}
#include "nvs.h"