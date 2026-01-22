#ifndef MQTT_TYPES_H
#define MQTT_TYPES_H

#include <stdbool.h>
#include "esp_err.h"

typedef enum {
    AC_CMD_TURN_ON,
    AC_CMD_TURN_OFF,
    AC_CMD_SET_TEMP,
    AC_CMD_SET_MODE,
    AC_CMD_UNKNOWN
} ac_command_type_t;

typedef struct {
    ac_command_type_t type;
    uint8_t temperature;
    uint8_t mode;
} ac_command_t;

typedef struct {
    char ssid[32];
    char password[64];
} wifi_credentials_t;

typedef void (*mqtt_ac_command_callback_t)(const ac_command_t *cmd);
typedef void (*mqtt_wifi_config_callback_t)(const wifi_credentials_t *creds);
typedef void (*mqtt_connected_callback_t)(void);

#endif