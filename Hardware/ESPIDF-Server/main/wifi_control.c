#include <esp_random.h>
#include <nvs_flash.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "nvs.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_http_client.h"

static const char* TAG = "WIFI_CONTROL";

static bool is_user_initiated_disconnect = false;

int IS_WIFI_CONNECTED = 0;

//TODO: Add connections without password.
#define WIFI_SSID "Fortune 1_2.4Ghz"
#define WIFI_PASSWORD "23111980g"


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT)
    {
        ESP_LOGI(TAG, "WiFi event: %ld", event_id);
    }
    if (event_base == IP_EVENT)
    {
        ESP_LOGI(TAG, "IP event: %ld", event_id);
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *disconn = event_data;
        ESP_LOGW(TAG, "WiFi disconnected, reason: %d", disconn->reason);

        IS_WIFI_CONNECTED = 0;

        if (!is_user_initiated_disconnect) {
            ESP_LOGI(TAG, "Reconnecting to WiFi...");
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_wifi_connect();
        } else {
            ESP_LOGI(TAG, "Disconnect was user-initiated; not reconnecting.");
            is_user_initiated_disconnect = false; // Reset flag
        }
    }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        IS_WIFI_CONNECTED = 1;
    }
}

void wifi_init_setup()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

/**
 * @brief Initializes and configures Wi-Fi in Station (STA) mode for the ESP32.
 * This function sets up the Wi-Fi interface, initializes the default event loop,
 * and configures the Wi-Fi connection with predefined SSID and password.
 * It ensures the device connects to a specified Wi-Fi network.
 *
 * The initialization process includes the following steps:
 * - Initializes the network interface with `esp_netif_init`.
 * - Creates the default Wi-Fi station (STA) interface.
 * - Configures Wi-Fi using default initialization configuration and credentials.
 * - Sets Wi-Fi mode to STA.
 * - Starts the Wi-Fi driver and connects to the configured Wi-Fi network.
 *
 * Diagnostic messages are logged to provide insights during the initialization and
 * connection process.
 *
 * @note This function uses predefined macros for SSID and password
 *       (WIFI_SSID and WIFI_PASSWORD).
 * @note An authentication threshold is set to enforce WPA2-PSK security.
 * @note If any function in the configuration process returns an error,
 *       the program execution will stop as the error will be checked using `ESP_ERROR_CHECK`.
 */
void wifi_init_sta()
{
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialized, connecting to SSID: %s", WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_connect());
}
