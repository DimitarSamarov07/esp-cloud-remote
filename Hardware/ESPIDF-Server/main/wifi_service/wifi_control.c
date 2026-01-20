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

// You must use this if you want to disconnect on purpose. First set it and THEN call the disconnect method
static bool is_user_initiated_disconnect = false;

int IS_WIFI_CONNECTED = 0;

//TODO: Add connections without password.
#define WIFI_SSID "Arabadzhievi"
#define WIFI_PASSWORD "16042325"


/**
 * @brief Event handler for WiFi and IP events
 * 
 * Handles various WiFi and IP events including disconnections and IP address assignments.
 * Manages automatic reconnection attempts when disconnected, unless the disconnect was user-initiated.
 *
 * @param arg          Event handler argument (unused)
 * @param event_base   Base of the event (WIFI_EVENT or IP_EVENT) 
 * @param event_id     ID of the specific event that occurred
 * @param event_data   Data associated with the event
 */
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
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_BEACON_TIMEOUT) {
        ESP_LOGW(TAG, "WiFi Beacon Timeout! Signal might be too weak.");
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        wifi_event_sta_disconnected_t* disconn = event_data;
        ESP_LOGW(TAG, "WiFi disconnected, reason: %d", disconn->reason);

        IS_WIFI_CONNECTED = 0;

        if (!is_user_initiated_disconnect)
        {
            ESP_LOGI(TAG, "Reconnecting to WiFi...");
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_wifi_connect();
        }
        else
        {
            ESP_LOGI(TAG, "Disconnect was user-initiated; not reconnecting.");
            is_user_initiated_disconnect = false; // Reset flag
        }
    }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        IS_WIFI_CONNECTED = 1;
    }
}

/**
 * @brief Initializes the WiFi system
 * 
 * Sets up the network interface, event loop, and WiFi configuration.
 * Registers event handlers for WiFi and IP events.
 * Creates the default WiFi station interface.
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
 * 
 * Configures the WiFi station with SSID and password.
 * Sets the authentication mode and starts the WiFi connection.
 * Attempts to connect to the configured access point.
 */
void wifi_init_sta(char* ssid, char* password)
{
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialized, connecting to SSID: %s", ssid);
    ESP_ERROR_CHECK(esp_wifi_connect());
}
/**
 * @brief Changes the current WiFi credentials and reconnects
 * * @param new_ssid     The new SSID to connect to
 * @param new_password The new password
 */
void change_wifi(char* new_ssid, char* new_password)
{
    ESP_LOGI(TAG, "Changing WiFi to SSID: %s", new_ssid);

    is_user_initiated_disconnect = true;

    esp_wifi_disconnect();
    esp_wifi_stop();

    wifi_init_sta(new_ssid, new_password);
}
