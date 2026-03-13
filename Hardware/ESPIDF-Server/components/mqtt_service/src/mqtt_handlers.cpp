#include <cJSON.h>

#include "mqtt_internal.h"
#include "mqtt_topics.h"
#include "esp_log.h"
#include <string.h>
#include "wifi_control.h"
#include "ir_remote.h"


static const char *TAG = "mqtt_handlers";

static void handle_ac_control_message(const char *message, size_t length);
static void handle_wifi_config_message(const char *message, size_t length);

void mqtt_internal_handle_data(const char *topic, const char *data, size_t length) {
    if (strstr(topic, "ac/control") != NULL) {
        handle_ac_control_message(data, length);
    }
    else if (strstr(topic, "connection/wifi") != NULL) {
        handle_wifi_config_message(data, length);
    }
    else {
        ESP_LOGW(TAG, "Unknown topic: %s", topic);
    }
}

static void handle_ac_control_message(const char *message, size_t length) {
    char *json_string = (char *)malloc(length + 1);
    if (json_string == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for JSON string");
        return;
    }
    memcpy(json_string, message, length);
    json_string[length] = '\0';


    // Parse the JSON payload
    cJSON *root = cJSON_Parse(json_string);
    free(json_string);

    cJSON *state_item = cJSON_GetObjectItem(root, "state");
    cJSON *temp_item = cJSON_GetObjectItem(root, "temp");
    sendTurnSignal(state_item->valuestring, temp_item->valueint);

    cJSON_Delete(root);

    if (root == NULL) {
        ESP_LOGE(TAG, "Invalid JSON received");
        return;
    }
}

static void handle_wifi_config_message(const char *message, size_t length) {
    char msg[128] = {0};
    if (length >= sizeof(msg)) length = sizeof(msg) - 1;
    memcpy(msg, message, length);
    msg[length] = '\0';

    ESP_LOGI(TAG, "WiFi config received: %s", msg);

    wifi_credentials_t creds = {0};

    char *ssid = strtok(msg, "/");
    char *pass = strtok(NULL, "/");

    if (ssid && pass) {
        strncpy(creds.ssid, ssid, sizeof(creds.ssid) - 1);
        strncpy(creds.password, pass, sizeof(creds.password) - 1);
        if (g_mqtt_ctx.wifi_callback) {
            g_mqtt_ctx.wifi_callback(&creds);


        }
        change_wifi(ssid,pass);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        if (checkWIFI() == 0) {
            ESP_LOGI(TAG, "WiFi connection lost");
            esp_restart();
        }
    } else {
        ESP_LOGW(TAG, "Invalid WiFi config format");
    }
}