#include "Arduino.h"
#include "mqtt_comunication.h"
#include "ble_communication.h"

static const char* TAG = "MQTT_APP";

void setup(){
    startBLE();
}

void loop()
{
    delay(500);
}
