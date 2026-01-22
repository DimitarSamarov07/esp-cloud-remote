#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include <stdbool.h>
#include "esp_err.h"
#include "mqtt_types.h"

esp_err_t mqtt_service_init(void);

esp_err_t mqtt_service_start(void);

esp_err_t mqtt_service_stop(void);


esp_err_t mqtt_register_device();


esp_err_t mqtt_service_register_ac_callback(mqtt_ac_command_callback_t callback);

esp_err_t mqtt_service_register_wifi_callback(mqtt_wifi_config_callback_t callback);

esp_err_t mqtt_service_register_connected_callback(mqtt_connected_callback_t callback);

bool mqtt_credentials_exist(void);

esp_err_t mqtt_credentials_clear(void);

bool mqtt_service_is_connected(void);

const char *mqtt_service_get_username(void);

#endif
