#include "BLE-Server.h"
#include "Preferences.h"

#include "BLEDevice.h"
#include "BLEServer.h"

#include "WiFi.h"


#define SCAN_SERVICE_UUID "31175f53-389f-4d19-8f69-6da32a28d50e"
#define SCAN_WIFI_SSID_LIST_CHARACTERISTIC_UUID "22946000-e86b-4831-adc8-27ab2cdb85fb"
#define SCAN_WIFI_SHOULD_SCAN_CHARACTERISTIC_UUID "3a6b1432-c481-45a7-a798-45ecd7d1ef41"

#define SET_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define WIFI_SSID_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define WIFI_PASS_CHARACTERISTIC_UUID "ed9e4092-2c12-4a94-9d86-696058d16573"
#define WIFI_CONNECTION_STATUS_UUID "4d5ea626-4ac9-4a71-9392-d0764ac7cefe"
#define DEFAULT_VALUE "waiting"


Preferences permanent_storage;

String previousSSID = "";
String previousPass = "";

bool deviceConnected = false;
bool oldDeviceConnected = false;
bool isBLENecessery = true;

// Define characteristics
BLECharacteristic *pSSIDSet = NULL;
BLECharacteristic *pPassSet = NULL;
BLECharacteristic *pSSIDScan = NULL;
BLECharacteristic *pShouldScan = NULL;
BLECharacteristic *pStatusConnectionSet = NULL;
BLEServer *pServer = NULL;

String oldValue = DEFAULT_VALUE;

// Callbacks for the BLE server
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    BLEDevice::startAdvertising();
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};

void ble_setup() {
  permanent_storage.begin("esp-cloud", false);  // initialize the storage in RW mode

  bool doesPreviousPassExist = permanent_storage.isKey("Password");
  bool doesPreviousSSIDExist = permanent_storage.isKey("SSID");

  // Try to reconnect with the last known good WiFi connection
  if (doesPreviousSSIDExist && doesPreviousPassExist) {
    previousSSID = permanent_storage.getString("SSID");
    previousPass = permanent_storage.getString("Password");
    int isConnected = connectWifi(previousSSID, previousPass);
    if (isConnected == 0) {
      Serial.println("Previous connection was not succesful. Entering pairing mode.");
    } else {
      Serial.println("Connected to previous network with SSID " + previousSSID + ". " + "BLE will not be enabled.");
      isBLENecessery = false;  // Enter pairing mode
      return;
    }
  }

  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP Cloud Remote Device");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Set service characteristics. Used to set the WiFi SSID and password
  BLEService *setService = pServer->createService(SET_SERVICE_UUID);
  pSSIDSet = setService->createCharacteristic(WIFI_SSID_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pPassSet = setService->createCharacteristic(WIFI_PASS_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pStatusConnectionSet = setService->createCharacteristic(WIFI_CONNECTION_STATUS_UUID, BLECharacteristic::PROPERTY_READ);


  // Set the characteristics to the default value and start the set service
  pSSIDSet->setValue(DEFAULT_VALUE);
  pPassSet->setValue(DEFAULT_VALUE);
  pStatusConnectionSet->setValue(DEFAULT_VALUE);
  setService->start();

  // The scan service and its characteristics are responsible to return the available WiFi connections.
  BLEService *scanService = pServer->createService(SCAN_SERVICE_UUID);
  pSSIDScan = scanService->createCharacteristic(SCAN_WIFI_SSID_LIST_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  pShouldScan = scanService->createCharacteristic(SCAN_WIFI_SHOULD_SCAN_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pSSIDScan->setValue(DEFAULT_VALUE);
  pShouldScan->setValue(DEFAULT_VALUE);
  scanService->start();

  // Advertising settings for compatability
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SET_SERVICE_UUID);
  pAdvertising->addServiceUUID(SCAN_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  Serial.println("May the force be with you...");  // good luck..
}

// Scans for WiFi networks
// The returned string will be in the format NETWORK_NAME/SIGNAL_POWER/AUTHENTICATION_TYPE; repeated for each network
String scanWifi() {
  // Enable the WiFi module and make sure it's disconnected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  String stringToReturn = "";
  Serial.println("Scanning...");

  // WiFi.scanNetworks will return the number of networks found.
  int numberOfNetworksFound = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (numberOfNetworksFound == 0) {
    Serial.println("NONE");
  } else {
    Serial.print(numberOfNetworksFound);
    Serial.println(" networks found");
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    for (int i = 0; i < numberOfNetworksFound; ++i) {
      // Print SSID and RSSI for each network found for debugging purposes
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      String SSID = WiFi.SSID(i).c_str();
      Serial.printf("%-32.32s", SSID);
      stringToReturn = stringToReturn + SSID + "/";
      Serial.print(" | ");
      int RSSI = WiFi.RSSI(i);
      Serial.printf("%4ld", RSSI);
      stringToReturn = stringToReturn + RSSI + "/";
      Serial.print(" | ");
      Serial.printf("%2ld", WiFi.channel(i));
      Serial.print(" | ");
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:
          Serial.print("open");
          stringToReturn += "open";
          break;
        case WIFI_AUTH_WEP:
          Serial.print("WEP");
          stringToReturn += "WEP";
          break;
        case WIFI_AUTH_WPA_PSK:
          Serial.print("WPA");
          stringToReturn += "WPA";
          break;
        case WIFI_AUTH_WPA2_PSK:
          Serial.print("WPA2");
          stringToReturn += "WPA2";
          break;
        case WIFI_AUTH_WPA_WPA2_PSK:
          Serial.print("WPA+WPA2");
          stringToReturn += "WPA+WPA2";
          break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
          Serial.print("WPA2-EAP");
          stringToReturn += "WPA2-EAP";
          break;
        case WIFI_AUTH_WPA3_PSK:
          Serial.print("WPA3");
          stringToReturn += "WPA3";
          break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
          Serial.print("WPA2+WPA3");
          stringToReturn += "WPA2+WPA3";
          break;
        case WIFI_AUTH_WAPI_PSK:
          Serial.print("WAPI");
          stringToReturn += "WPA2+WPA3";
          break;
        default: Serial.print("unknown"); stringToReturn += "?";
      }
      stringToReturn += ";";  // Start the next network entry
      Serial.println();
    }
  }
  WiFi.mode(WIFI_MODE_NULL);  // Turn off the WiFi module
  return stringToReturn;
}

// The function will attempt to connect to a network by the given SSID and password
// 1 will be returned upon success, and 0 on fail
int connectWifi(String SSID, String password) {
  bool shouldSave = false;
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);  // Make sure the WiFi module is in a clean state

  // Initialize the connection
  if (password == DEFAULT_VALUE || password == "") {
    WiFi.begin(SSID);
  } else {
    WiFi.begin(SSID, password);
  }

  // Skip writing to the permanent memory if the credentials are taken from there
  if (previousSSID != SSID || previousPass != password) {
    shouldSave = true;
  }


  // Each try will be done in tryDelay increments for numberOfTries tries. Some networks take longer to connect, but this should be sufficient
  // Modify if some networks turn out even slower
  int tryDelay = 500;
  int numberOfTries = 20;

  // Wait for the WiFi event
  // Log the data for debugging purposes
  while (true) {
    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL:
        Serial.println("[WiFi] SSID " + SSID + " not found");
        return 0;
        break;
      case WL_CONNECT_FAILED:
        Serial.print("[WiFi] Failed - WiFi not connected! Reason: ");
        return 0;
        break;
      case WL_CONNECTION_LOST:
        Serial.println("[WiFi] Connection was lost");
        return 0;
        break;
      case WL_SCAN_COMPLETED: Serial.println("[WiFi] Scan is completed"); break;
      case WL_DISCONNECTED:
        Serial.println("[WiFi] WiFi is disconnected");
        break;
      case WL_CONNECTED:
        Serial.println("[WiFi] WiFi is connected!");
        Serial.print("[WiFi] IP address: ");
        Serial.println(WiFi.localIP());
        if (shouldSave) { saveWiFiToStorage(SSID, password); }  // save the credentials to the permanent storage
        return 1;
        break;
      default:
        Serial.print("[WiFi] WiFi Status: ");
        Serial.println(WiFi.status());
        break;
    }
    delay(tryDelay);

    if (numberOfTries <= 0) {
      Serial.print("[WiFi] Failed to connect to WiFi!");
      // Use disconnect function to force stop trying to connect
      WiFi.disconnect();
      WiFi.mode(WIFI_MODE_NULL);
      return 0;
    } else {
      numberOfTries--;
    }
  }
}

// Write the WiFi credentials to the non-volatile memory
void saveWiFiToStorage(String ssid, String password) {
  Serial.println("Saving new wifi credentials to storage..");
  permanent_storage.putString("SSID", ssid);
  permanent_storage.putString("Password", password);
}

int ble_loop_step() {
  // Enter if BLE is ON
  if (isBLENecessery) {
    if (deviceConnected) {
      String ssidSetValue = pSSIDSet->getValue();
      String shouldScanStr = pShouldScan->getValue();
      if (shouldScanStr != DEFAULT_VALUE) {  // if scan was enabled through BLE
        Serial.println("Producing WiFi glory..");
        String wifiString = scanWifi();
        pSSIDScan->setValue(wifiString);
        Serial.println(wifiString);
        pShouldScan->setValue(DEFAULT_VALUE);
      }
      if (ssidSetValue != DEFAULT_VALUE) {  // if the SSID default value has been changed, this would be considered a request to pair to a WiFi
        String passSetValue = pPassSet->getValue();

        int result = connectWifi(ssidSetValue, passSetValue);
        pStatusConnectionSet->setValue(result);
        pSSIDSet->setValue(DEFAULT_VALUE);
        pPassSet->setValue(DEFAULT_VALUE);

        if(result == 1){
          Serial.println("The ESP is about to restart.");
          delay(3000);
          ESP.restart();
        }
      }
    }
    // Handle device disconnection
    if (!deviceConnected && oldDeviceConnected) {
      delay(500);                   // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising();  // restart advertising
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on first connect
      oldDeviceConnected = deviceConnected;
    }
    return 0;
  }
  return 1;
}
