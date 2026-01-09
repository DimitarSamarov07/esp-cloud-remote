#include "esp32-hal-gpio.h"
#include "MQTT-communication.h"
#include "BLE-Server.h"
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "OPTELA";
const char* password = "";
const char* mqtt_server = "93.155.224.232";
const int port = 5728;

WiFiClient espClient;
PubSubClient client(espClient);

#define mqttACControl "ac/control"
#define mqttConnectionWifi "connection/wifi"

void topicsSubscribe() {
  client.subscribe(mqttACControl, 1);
  client.subscribe(mqttConnectionWifi, 1);
}

void callback(char* topic, byte* message, unsigned int length) {
  String receivedMessage;
  for (int i = 0; i < length; i++) {
    receivedMessage += (char)message[i];
  }

  if (String(topic) == mqttACControl) {
    Serial.println("Message received from ac/control");
    if (receivedMessage == "TURN_ON") {
      client.publish("ac/report", "Turn ON signal sent to AC");
      flashLED();
    } else if (receivedMessage == "TURN_OFF") {
      client.publish("ac/report", "Turn OFF signal sent to AC");
      flashLED();
    } else if (receivedMessage == "TURN_LED_ON") {
      digitalWrite(2, HIGH);
      delay(3000);
    } else if (receivedMessage == "TURN_LED_OFF") {
      digitalWrite(2, LOW);
      flashLED();
    } else {
      client.publish("ac/report", "Unknown command sent to ac/control");
      flashLED();
    }
  }

  if (String(topic) == mqttConnectionWifi) {
    Serial.println("Message received from connection/wifi");
    int delimeterIndex = receivedMessage.indexOf('/');
    if (delimeterIndex != -1) {
      String newSSID = receivedMessage.substring(0, delimeterIndex);
      String newPass = receivedMessage.substring(delimeterIndex + 1);
      Serial.println(newSSID);
      Serial.println(newPass);
      client.publish("ac/report", "Changed WiFi connection");
      connectWifi(newSSID, newPass);  // TODO: If new connection doesn't exist stay in previous connection
      flashLED();
    } else {
      client.publish("ac/report", "Couldn't change WiFi because it wasn't sent in correct format");
    }
  }

  Serial.println("Response sent to ac/report");
  delay(1000);
}

void mqtt_setup() {
  pinMode(2, OUTPUT);
  // connectWifi(ssid, password);  // currently handled by the BLE Server
  client.setServer(mqtt_server, port);
  client.setCallback(callback);
}

void reconnect() {
  if (client.connect("ESP32Client", "ac/control", 1, true, "Hitler")) {
    Serial.println("Connected");
    Serial.println();
    topicsSubscribe();
  } else {
    delay(5000);
  }
  //topicsSubscribe();
}

void flashLED() {
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);
  delay(50);
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);
  delay(1000);
}

void mqtt_loop_step() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
