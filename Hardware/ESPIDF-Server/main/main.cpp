#include "Arduino.h"
#include "mqtt_comunication.h"
#include "ble_communication.h"

static const char* TAG = "MQTT_APP";

void setup(){
    ESP_LOGI(TAG, "Initializing MQTT");
    wifi_init_sta();
    ESP_LOGI(TAG, "Connecting to MQTT");
    delay(10000);
	//TODO: Check if this device has already been setup.
    mqtt_first_init();
    startBLE();

    mqtt_init();
}

void loop()
{
    delay(500);
}
