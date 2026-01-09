#include "WString.h"
//
// BLE-Server.h
//

#include <string>

// Setup code
void ble_setup();

// BLE main loop
int ble_loop_step();

// Used to scan for WiFi. Returns a string
String scanWifi();

// Connect to WiFi by given credentials. Returns 0 for failure and 1 for success.
int connectWifi(String SSID, String password);

// Save the WiFi credentials to the non-volatile storage of the ESP
void saveWiFiToStorage(String ssid, String password);




