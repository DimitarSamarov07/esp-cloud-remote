#ifndef MQTT_INTERNAL_H
#define MQTT_INTERNAL_H

#include "mqtt_client.h"
#include "mqtt_types.h"

typedef struct {
    esp_mqtt_client_handle_t client;
    char username[64];
    char password[64];
    bool is_connected;
    
    mqtt_ac_command_callback_t ac_callback;
    mqtt_wifi_config_callback_t wifi_callback;
    mqtt_connected_callback_t connected_callback;
} mqtt_context_t;

extern mqtt_context_t g_mqtt_ctx;

void mqtt_internal_handle_connected(esp_mqtt_event_handle_t event);
void mqtt_internal_handle_disconnected(void);
void mqtt_internal_handle_data(const char *topic, const char *data, size_t length);

#endif