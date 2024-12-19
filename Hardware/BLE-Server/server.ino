#include <Preferences.h>

#include <BLEDevice.h>
#include <BLEServer.h>

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

BLECharacteristic *pSSIDSet = NULL;
BLECharacteristic *pPassSet = NULL;
BLECharacteristic *pSSIDScan = NULL;
BLECharacteristic *pShouldScan = NULL;
BLECharacteristic *pStatusConnectionSet = NULL;
BLEServer *pServer = NULL;

String oldValue = DEFAULT_VALUE;



class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    BLEDevice::startAdvertising();
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);
  permanent_storage.begin("esp-cloud", false);

  bool doesPreviousPassExist = permanent_storage.isKey("Password");
  bool doesPreviousSSIDExist = permanent_storage.isKey("SSID");

  Serial.println(doesPreviousSSIDExist);
  Serial.println(doesPreviousPassExist);
  if (doesPreviousSSIDExist && doesPreviousPassExist) {
    previousSSID = permanent_storage.getString("SSID");
    previousPass = permanent_storage.getString("Password");
    int isConnected = connectWifi(previousSSID, previousPass);
    if (isConnected == 0) {
      Serial.println("Previous connection was not succesful. Entering pairing mode.");
    } else {
      Serial.println("Connected to previous network with SSID " + previousSSID + ". " + "BLE will not be enabled.");
      isBLENecessery = false;
      return;
    }
  }

  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP Cloud Remote Device");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *setService = pServer->createService(SET_SERVICE_UUID);
  pSSIDSet = setService->createCharacteristic(WIFI_SSID_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pPassSet = setService->createCharacteristic(WIFI_PASS_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pStatusConnectionSet = setService->createCharacteristic(WIFI_CONNECTION_STATUS_UUID, BLECharacteristic::PROPERTY_READ);


  pSSIDSet->setValue(DEFAULT_VALUE);
  pPassSet->setValue(DEFAULT_VALUE);
  pStatusConnectionSet->setValue(DEFAULT_VALUE);
  setService->start();

  BLEService *scanService = pServer->createService(SCAN_SERVICE_UUID);
  pSSIDScan = scanService->createCharacteristic(SCAN_WIFI_SSID_LIST_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  pShouldScan = scanService->createCharacteristic(SCAN_WIFI_SHOULD_SCAN_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pSSIDScan->setValue(DEFAULT_VALUE);
  pShouldScan->setValue(DEFAULT_VALUE);
  scanService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SET_SERVICE_UUID);
  pAdvertising->addServiceUUID(SCAN_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  Serial.println("May the force be with you...");
}

String scanWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  String stringToReturn = "";
  delay(100);
  Serial.println("Now let's see if the scan fucks us....");
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
      // Print SSID and RSSI for each network found
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
      stringToReturn += ";";
      Serial.println();
    }
  }
  WiFi.mode(WIFI_MODE_NULL);
  return stringToReturn;
}

int connectWifi(String SSID, String password) {
  bool shouldSave = false;
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  if (password == DEFAULT_VALUE) {
    WiFi.begin(SSID);
  } else {
    WiFi.begin(SSID, password);
  }

  if (previousSSID != SSID || previousPass != password) {
    shouldSave = true;
  }

  int tryDelay = 500;
  int numberOfTries = 20;

  // Wait for the WiFi event
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
        if (shouldSave) { saveWiFiToStorage(SSID, password); }
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

void saveWiFiToStorage(String ssid, String password) {
  Serial.println("Saving new wifi credentials to storage..");
  permanent_storage.putString("SSID", ssid);
  permanent_storage.putString("Password", password);
}

void loop() {
  if (isBLENecessery) {
    if (deviceConnected) {
      String ssidSetValue = pSSIDSet->getValue();
      String shouldScanStr = pShouldScan->getValue();
      if (shouldScanStr != DEFAULT_VALUE) {
        Serial.println("Producing WiFi glory..");
        String wifiString = scanWifi();
        pSSIDScan->setValue(wifiString);
        Serial.println(wifiString);
        pShouldScan->setValue(DEFAULT_VALUE);
      }
      if (ssidSetValue != DEFAULT_VALUE) {
        String passSetValue = pPassSet->getValue();

        int result = connectWifi(ssidSetValue, passSetValue);
        pStatusConnectionSet->setValue(result);
        pSSIDSet->setValue(DEFAULT_VALUE);
        pPassSet->setValue(DEFAULT_VALUE);
      }
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
      delay(500);                   // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising();  // restart advertising
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
    }
  }

  delay(1000);
}
