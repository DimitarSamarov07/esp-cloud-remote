#include "mqtt_internal.h"
#include "mqtt_topics.h"
#include "esp_log.h"
#include <string.h>
#include "wifi_control.h"

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
    char msg[128] = {0};
    if (length >= sizeof(msg)) length = sizeof(msg) - 1;
    memcpy(msg, message, length);
    msg[length] = '\0';

    ac_command_t cmd = {.type = AC_CMD_UNKNOWN};

    if (strcmp(msg, "TURN_LED_ON") == 0) {
        cmd.type = AC_CMD_TURN_ON;
        ESP_LOGI(TAG, "AC Command: TURN ON");
    } 
    else if (strcmp(msg, "TURN_LED_OFF") == 0) {
        cmd.type = AC_CMD_TURN_OFF;
        ESP_LOGI(TAG, "AC Command: TURN OFF");
    }
    else {
        ESP_LOGW(TAG, "Unknown AC command: %s", msg);
        return;
    }

    if (g_mqtt_ctx.ac_callback) {
        g_mqtt_ctx.ac_callback(&cmd);
    }

    char ack_topic[128];
    snprintf(ack_topic, sizeof(ack_topic), 
             MQTT_TOPIC_AC_STATUS, g_mqtt_ctx.username);
    
    const char *ack_msg = (cmd.type == AC_CMD_TURN_ON) ? "AC turned ON" : "AC turned OFF";
    esp_mqtt_client_publish(g_mqtt_ctx.client, ack_topic, 
                           ack_msg, 0, MQTT_QOS_STATUS, 1);
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
    } else {
        ESP_LOGW(TAG, "Invalid WiFi config format");
    }
}