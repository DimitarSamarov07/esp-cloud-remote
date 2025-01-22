#include "BLE-Server.h" 
#include_next "MQTT-communication.h"

void setup() {
  Serial.begin(115200);
  ble_setup();
  mqtt_setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  int ble_pass = 0;
  while(ble_pass != 1){
    ble_pass = ble_loop_step();
    delay(100);
  }
  Serial.println("The BLE Server has skipped execution.");
  while(true){
    mqtt_loop_step();
    delay(100);
  }
}