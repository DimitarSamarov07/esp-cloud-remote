#include <BLEDevice.h>
#include <BLEServer.h>
#include "WiFi.h"


#define SCAN_SERVICE_UUID "31175f53-389f-4d19-8f69-6da32a28d50e"
#define SCAN_WIFI_SSID_LIST_CHARACTERISTIC_UUID "22946000-e86b-4831-adc8-27ab2cdb85fb"
#define SCAN_WIFI_SHOULD_SCAN_CHARACTERISTIC_UUID "3a6b1432-c481-45a7-a798-45ecd7d1ef41"
#define SET_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define WIFI_SSID_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define WIFI_PASS_CHARACTERISTIC_UUID "ed9e4092-2c12-4a94-9d86-696058d16573"
#define DEFAULT_VALUE  "waiting"


bool deviceConnected = false;
bool oldDeviceConnected = false;
BLECharacteristic *pSSIDSet = NULL;
BLECharacteristic *pPassSet = NULL;
BLECharacteristic *pSSIDScan = NULL;
BLECharacteristic *pShouldScan = NULL;
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
  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP Cloud Remote Device");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *setService = pServer->createService(SET_SERVICE_UUID);
  pSSIDSet = setService->createCharacteristic(WIFI_SSID_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  
  pPassSet = setService->createCharacteristic(WIFI_PASS_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  pSSIDSet->setValue(DEFAULT_VALUE);
  pPassSet->setValue(DEFAULT_VALUE);
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

  Serial.println("If you get an error after this, we are cooked.");
}

String scanWifi(){
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
        case WIFI_AUTH_OPEN:            Serial.print("open"); stringToReturn += "open"; break;
        case WIFI_AUTH_WEP:             Serial.print("WEP"); stringToReturn += "WEP"; break;
        case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); stringToReturn += "WPA"; break;
        case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); stringToReturn += "WPA2"; break;
        case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); stringToReturn += "WPA+WPA2"; break;
        case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); stringToReturn += "WPA2-EAP"; break;
        case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); stringToReturn += "WPA3"; break;
        case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3");stringToReturn += "WPA2+WPA3"; break;
        case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI");stringToReturn += "WPA2+WPA3"; break;
        default:                        Serial.print("unknown"); stringToReturn += "?";
      }
      stringToReturn += ";";
      Serial.println();
    } 
  }
  WiFi.mode(WIFI_MODE_NULL);
  return stringToReturn;
}

bool connectWifi(string SSID, string password){
  
}

void loop() {
  // notify changed valueNeil
  if (deviceConnected) {
    String ssidSetValue = pSSIDSet->getValue();
    String shouldScanStr = pShouldScan->getValue();
    if(shouldScanStr != DEFAULT_VALUE){
      Serial.println("Producing WiFi glory..");
      String wifiString = scanWifi();
      pSSIDScan->setValue(wifiString);
      Serial.println(wifiString);
      pShouldScan->setValue(DEFAULT_VALUE);
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
  delay(1000);
}
