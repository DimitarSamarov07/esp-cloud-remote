/*
 * WebSocketClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <WebSocketsClient.h>
#include <Arduino_JSON.h>


WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

	switch(type) {
		case WStype_DISCONNECTED:
			USE_SERIAL.printf("[ESP] Disconnected!\n");
			break;
		case WStype_CONNECTED:
			USE_SERIAL.printf("[ESP] Connected to url: %s\n", payload);

			// send message to server when Connected
			webSocket.sendTXT("Connected");
			break;
		case WStype_TEXT: {
			USE_SERIAL.printf("[ESP] get text: %s\n", payload);
      String string_payload = (char *)payload;

      JSONVar myObject = JSON.parse(string_payload);
      if (JSON.typeof(myObject) == "undefined") {
        USE_SERIAL.println(string_payload);
        return;
      }

      String led_state = (String) myObject["led_state"];
      if (led_state == "OFF") {
          digitalWrite(42, LOW);
          webSocket.sendTXT("OK");
      }
      else if (led_state == "ON"){
          digitalWrite(42, HIGH);
          webSocket.sendTXT("OK");
      }
      else {
          webSocket.sendTXT("error");
      }
			break;
    }
	}

}

void setup() {
	USE_SERIAL.begin(115200);

	USE_SERIAL.setDebugOutput(true);

	USE_SERIAL.println();
	USE_SERIAL.println();
	USE_SERIAL.println();

	for(uint8_t t = 4; t > 0; t--) {
		USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}

	WiFiMulti.addAP("SSID", "PASSWORD");

	//WiFi.disconnect();
	while(WiFiMulti.run() != WL_CONNECTED) {
		delay(100);
	}

	// server address, port and URL
	webSocket.begin("localhost", 8690, "/ws-repeat");

	// event handler
	webSocket.onEvent(webSocketEvent);

	// use HTTP Basic Authorization this is optional remove if not needed
	//webSocket.setAuthorization("user", "Password");

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);

}

void loop() {
	webSocket.loop();
}
