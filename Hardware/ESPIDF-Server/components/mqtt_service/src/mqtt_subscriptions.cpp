#include "mqtt_internal.h"
#include "mqtt_topics.h"
#include "esp_log.h"
#include <stdio.h>

#include "mqtt_client.h"

static const char *TAG = "mqtt_subs";

static const char *topic_templates[] = {
    MQTT_TOPIC_AC_CONTROL,
    MQTT_TOPIC_WIFI_CONFIG,
};

esp_err_t mqtt_subscribe_all_topics(void) {
    if (!g_mqtt_ctx.client) {
        ESP_LOGE(TAG, "MQTT client not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    char full_topic[128];
    
    for (int i = 0; i < MQTT_TOPIC_COUNT; i++) {
        snprintf(full_topic, sizeof(full_topic), 
                 topic_templates[i], g_mqtt_ctx.username);
        
        int msg_id = esp_mqtt_client_subscribe(g_mqtt_ctx.client, 
                                               full_topic, MQTT_QOS_CONTROL);
        
        if (msg_id < 0) {
            ESP_LOGE(TAG, "Failed to subscribe to: %s", full_topic);
            return ESP_FAIL;
        }
        
        ESP_LOGI(TAG, "Subscribed to: %s (msg_id=%d)", full_topic, msg_id);
    }

    return ESP_OK;
}

esp_err_t mqtt_unsubscribe_all_topics(void) {
    if (!g_mqtt_ctx.client) {
        return ESP_ERR_INVALID_STATE;
    }

    char full_topic[128];
    
    for (int i = 0; i < MQTT_TOPIC_COUNT; i++) {
        snprintf(full_topic, sizeof(full_topic), 
                 topic_templates[i], g_mqtt_ctx.username);
        
        esp_mqtt_client_unsubscribe(g_mqtt_ctx.client, full_topic);
    }

    return ESP_OK;
}