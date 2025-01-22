#include "MQTT-communication.h"
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "OPTELA";
const char* password = "";
const char* mqtt_server = "93.155.224.232";
const int port = 5728;

WiFiClient espClient;
PubSubClient client(espClient);

#define mqttACControl "ac/control"

void topicsSubscribe() {
  client.subscribe(mqttACControl);
}

void callback(char* topic, byte* message, unsigned int length) {
  String receivedMessage;
  for (int i = 0; i < length; i++) {
    receivedMessage += (char)message[i];
  }

  if(String(topic) == mqttACControl) {
    Serial.println("Message received from ac/control");
    if(receivedMessage == "TURN_ON") {
      client.publish("ac/report", "Turn ON signal sent to AC");
      Serial.println("Response sent to ac/report");
      flashLED();
      delay(1000);
    } else if (receivedMessage == "TURN_OFF") {
      client.publish("ac/report", "Turn OFF signal sent to AC");
      Serial.println("Response sent to ac/report");
      flashLED();
      delay(1000);
    } else {
      client.publish("ac/report", "Unknown command sent to ac/control");
      Serial.println("Response sent to ac/report");
      flashLED();
      delay(1000);
    }
  }
}

void mqtt_setup() {
  Serial.begin(115200);
  
  pinMode(2, OUTPUT);
  
  Serial.println("Connecting to ");
  Serial.print(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP Adress: ");
  Serial.print(WiFi.localIP());
  Serial.println();

  client.setServer(mqtt_server, port);
  client.setCallback(callback);
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("Connected");
      Serial.println();
      topicsSubscribe();
    } else {
      delay(5000);
    }
  }
  topicsSubscribe();
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

void mqtt_loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 
}
