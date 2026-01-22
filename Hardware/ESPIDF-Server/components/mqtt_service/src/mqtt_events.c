#include "mqtt_internal.h"
#include "esp_log.h"

static const char *TAG = "mqtt_events";

extern esp_err_t mqtt_subscribe_all_topics(void);

void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                       int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    g_mqtt_ctx.client = event->client;

    switch (event->event_id) {
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG, "MQTT preparing to connect...");
            break;

        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected!");
            g_mqtt_ctx.is_connected = true;
            mqtt_subscribe_all_topics();
            
            if (g_mqtt_ctx.connected_callback) {
                g_mqtt_ctx.connected_callback();
            }
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT disconnected (auto-reconnect enabled)");
            g_mqtt_ctx.is_connected = false;
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "Subscribed, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "Unsubscribed, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "Published, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Message on topic: %.*s", event->topic_len, event->topic);
            mqtt_internal_handle_data(event->topic, event->data, event->data_len);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error occurred");
            break;

        default:
            ESP_LOGD(TAG, "Unhandled event: %d", event_id);
            break;
    }
}