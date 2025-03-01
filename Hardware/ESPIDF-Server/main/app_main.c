#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_log.h"
#include "mqtt_client.h"

#define mqttACControl "ac/control"
#define mqttConnectionWifi "connection/wifi"


const char* SSID = "";
const char* PASSWORD  = "";
int8_t QoS = 1;

static void mqtt5_app_start(void)
{
    
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL
        
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
    esp_mqtt_client_subscribe_single(client, mqttACControl, QoS);
    esp_mqtt_client_subscribe_single(client, mqttConnectionWifi, QoS);
}

void app_main(void)
{
    mqtt5_app_start();
}
