#include "mqtt_service.h"
#include "mqtt_internal.h"
#include "mqtt_topics.h"
#include "esp_log.h"
#include <string.h>

#include "esp_err.h"
#include "mqtt_client.h"

static const char *TAG = "mqtt_service";

mqtt_context_t g_mqtt_ctx = {0};

extern void mqtt_event_handler(void *args, esp_event_base_t base,
                              int32_t id, void *data);
extern esp_err_t mqtt_credentials_load(char *user, size_t user_size,
                                      char *pass, size_t pass_size);

esp_err_t mqtt_service_init(void) {
    esp_err_t err = mqtt_credentials_load(g_mqtt_ctx.username,
                                          sizeof(g_mqtt_ctx.username),
                                          g_mqtt_ctx.password,
                                          sizeof(g_mqtt_ctx.password));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "No credentials found in NVS");
        return err;
    }

    if (strlen(g_mqtt_ctx.username) == 0) {
        ESP_LOGE(TAG, "Empty username, cannot initialize MQTT");
        return ESP_ERR_INVALID_ARG;
    }

    char lwt_topic[128];
    snprintf(lwt_topic, sizeof(lwt_topic), MQTT_TOPIC_LWT, g_mqtt_ctx.username);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://90.154.171.96:5728",
        .credentials.username = g_mqtt_ctx.username,
        .credentials.authentication.password = g_mqtt_ctx.password,
        .session.keepalive = 60,
        .session.disable_clean_session = true,
        .session.last_will = {
            .topic = lwt_topic,
            .msg = "offline",
            .msg_len = 7,
            .qos = 1,
            .retain = true,
        },
        .session.protocol_ver = MQTT_PROTOCOL_V_5,
    };

    g_mqtt_ctx.client = esp_mqtt_client_init(&mqtt_cfg);
    if (!g_mqtt_ctx.client) {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(g_mqtt_ctx.client, ESP_EVENT_ANY_ID,
                                   mqtt_event_handler, NULL);

    ESP_LOGI(TAG, "MQTT service initialized for user: %s", g_mqtt_ctx.username);
    return ESP_OK;
}

esp_err_t mqtt_service_start(void) {
    if (!g_mqtt_ctx.client) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_mqtt_client_start(g_mqtt_ctx.client);
}

esp_err_t mqtt_service_stop(void) {
    if (!g_mqtt_ctx.client) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_mqtt_client_stop(g_mqtt_ctx.client);
}

esp_err_t mqtt_service_register_ac_callback(mqtt_ac_command_callback_t callback) {
    g_mqtt_ctx.ac_callback = callback;
    return ESP_OK;
}

esp_err_t mqtt_service_register_wifi_callback(mqtt_wifi_config_callback_t callback) {
    g_mqtt_ctx.wifi_callback = callback;
    return ESP_OK;
}

esp_err_t mqtt_service_register_connected_callback(mqtt_connected_callback_t callback) {
    g_mqtt_ctx.connected_callback = callback;
    return ESP_OK;
}

bool mqtt_service_is_connected(void) {
    return g_mqtt_ctx.is_connected;
}

const char* mqtt_service_get_username(void) {
    return g_mqtt_ctx.username;
}