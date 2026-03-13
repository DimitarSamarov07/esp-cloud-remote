#include <cJSON.h>

#include "mqtt_internal.h"
#include "esp_log.h"
#include <string.h>
#include "wifi_control.h"
#include "ir_mitsubishi_control.h"


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
    ESP_LOGI(TAG, "AC control message received: %s", json_string);


    cJSON *root = cJSON_Parse(json_string);
    free(json_string);

    if (root == NULL) {
        ESP_LOGE(TAG, "Invalid JSON received");
        return;
    }

    cJSON *state_item = cJSON_GetObjectItem(root, "Power");
    cJSON *temp_item  = cJSON_GetObjectItem(root, "Temperature");
    cJSON *mode_item  = cJSON_GetObjectItem(root, "Mode");
    cJSON *fan_speed_item  = cJSON_GetObjectItem(root, "FanSpeed");
    cJSON *swing_item  = cJSON_GetObjectItem(root, "Swing");
    if (!cJSON_IsBool(state_item) || !cJSON_IsNumber(temp_item) || !cJSON_IsString(mode_item) || !cJSON_IsString(fan_speed_item) || !cJSON_IsBool(swing_item)) {
        ESP_LOGE(TAG, "Missing or invalid fields in AC JSON");
        cJSON_Delete(root);
        return;
    }
    startAcConnection();
    sendTurnSignalMitsubishi(state_item->valuestring, temp_item->valuedouble, mode_item->valuestring, fan_speed_item->valuestring, swing_item->valueint);

    cJSON_Delete(root);
}


static void handle_wifi_config_message(const char *message, size_t length) {
    char msg[128] = {0};
    if (length >= sizeof(msg)) length = sizeof(msg) - 1;
    memcpy(msg, message, length);
    msg[length] = '\0';

    ESP_LOGI(TAG, "WiFi config received: %s", msg);

    wifi_credentials_t creds = {};

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