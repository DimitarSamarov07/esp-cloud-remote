#include "Arduino.h"
#include "mqtt_comunication.h"


static const char* TAG = "MQTT_APP";

void setup()
{
    wifi_init_sta();
    delay(2000);
    mqtt_init();
}

void loop()
{
    delay(500);
}
