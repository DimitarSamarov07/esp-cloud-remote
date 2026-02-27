#include "NimBLEDevice.h"
#include "NimBLEServer.h"
#include "NimBLEUtils.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <string>

// ---------------------------------------------------
// GLOBALS
// ---------------------------------------------------
static const char *tag = "ESP_Cloud_Remote_BLE";

// DEFINITIONS (Allocating memory for these flags here)
volatile bool g_wifi_scan_requested = false;
volatile bool g_wifi_credentials_updated = false;

// Global Data Buffers
char ssid_list_val_chr_value[1000] = "";
char should_scan_chr_value[32] = "";
char set_ssid_chr_value[32] = "";
char set_pass_chr_value[32] = "";
int wifi_connection_status_chr_value = 0;

// Pointers to Characteristics (for updating from main loop)
NimBLECharacteristic *pSsidListChar = nullptr;
NimBLECharacteristic *pStatusChar = nullptr;

// ---------------------------------------------------
// UUID DEFINITIONS
// ---------------------------------------------------

/**
 * =============================================================================
 * SERVICE: WIFI SCAN SERVICE (31175f53-389f-4d19-8f69-6da32a28d50e)
 * =============================================================================
 * * 1. CHAR_WIFI_SSID_LIST (22946000-e86b-4831-adc8-27ab2cdb85fb)
 * - Properties: READ | NOTIFY
 * - Format: UTF-8 String (Newline separated)
 * - Logic: Contains the results of the last successful WiFi scan.
 * Maximum length is capped at ~500 bytes to stay within BLE MTU limits.
 * * 2. CHAR_SHOULD_SCAN (3a6b1432-c481-45a7-a798-45ecd7d1ef41)
 * - Properties: READ | WRITE
 * - Format: UTF-8 String / Boolean
 * - Logic: Writing any value to this characteristic triggers a global flag
 * (g_wifi_scan_requested). The main loop then halts WiFi connection logic
 * to perform a safe hardware scan.
 *

 */

#define SERVICE_SCAN_UUID           "31175f53-389f-4d19-8f69-6da32a28d50e"
#define CHAR_WIFI_SSID_LIST_UUID    "22946000-e86b-4831-adc8-27ab2cdb85fb"
#define CHAR_SHOULD_SCAN_UUID       "3a6b1432-c481-45a7-a798-45ecd7d1ef41"


/**
* * =============================================================================
* SERVICE: WIFI SETTINGS SERVICE (4fafc201-1fb5-459e-8fcc-c5c9c331914b)
* =============================================================================
* * 1. CHAR_SET_SSID (beb5483e-36e1-4688-b7f5-ea07361b26a8)
* - Properties: READ | WRITE
* - Format: UTF-8 String (Max 32 chars)
* - Logic: Sets the target WiFi network name. Triggers g_wifi_credentials_updated.
*
*
* * 2. CHAR_SET_PASS (ed9e4092-2c12-4a94-9d86-696058d16573)
* - Properties: READ | WRITE
* - Format: UTF-8 String (Max 64 chars)
* - Logic: Sets the WiFi password. Triggers g_wifi_credentials_updated.
*
*
* * 3. CHAR_CONN_STATUS (4d5ea626-4ac9-4a71-9392-d0764ac7cefe)
* - Properties: READ | NOTIFY
* - Format: UTF-8 String (e.g., "1: Connected!")
* - Logic: Real-time feedback loop from the WiFi Event Handler.
* - 0: Connecting... (Handshake in progress)
* - 1: Connected! (IP obtained)
* - 2: Error - Network Not Found (SSID out of range or invalid)
* - 3: Error - Incorrect Password (Auth failure)
* - 4: Error - Connection Dropped (Generic failure)
* =============================================================================
*/

#define SERVICE_SET_UUID            "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_SET_SSID_UUID          "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_SET_PASS_UUID          "ed9e4092-2c12-4a94-9d86-696058d16573"
#define CHAR_CONN_STATUS_UUID       "4d5ea626-4ac9-4a71-9392-d0764ac7cefe"

// ---------------------------------------------------
// CALLBACK CLASSES
// ---------------------------------------------------

class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override {
        ESP_LOGI(tag, "Client Connected");
    };

    void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override {
        ESP_LOGI(tag, "Client Disconnected - Reason: %d", reason);
        // Automatically restart advertising on disconnect so you can reconnect
        NimBLEDevice::startAdvertising();
    }
};

/**
 * Scan Request Callback
 * SAFE: Does NOT scan. Just sets a flag.
 */
class ScanRequestCallback : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *pChar, NimBLEConnInfo &connInfo) override {
        std::string value = pChar->getValue();
        if (value.length() > 0) {
            ESP_LOGI(tag, "Scan requested via BLE. Flagging main loop...");

            // CRITICAL FIX: We do NOT call perform_wifi_scan() here.
            // We just set the flag. The main loop will handle the heavy work.
            g_wifi_scan_requested = true;
        }
    }
};

/**
 * Credential Write Callback
 * SAFE: Copies data and sets flag. Does not connect.
 */
class CredentialWriteCallback : public NimBLECharacteristicCallbacks {
    char *targetBuffer;
    size_t maxLen;

public:
    CredentialWriteCallback(char *buffer, size_t len) : targetBuffer(buffer), maxLen(len) {
    }

    void onWrite(NimBLECharacteristic *pChar, NimBLEConnInfo &connInfo) override {
        std::string value = pChar->getValue();

        if (value.length() > 0) {
            // Clear buffer and copy new value
            memset(targetBuffer, 0, maxLen);
            size_t copyLen = (value.length() < maxLen) ? value.length() : (maxLen - 1);
            memcpy(targetBuffer, value.c_str(), copyLen);

            ESP_LOGI(tag, "Received credential write: %s", targetBuffer);

            // Flag main loop that credentials arrived
            g_wifi_credentials_updated = true;
        }
    }
};

// ---------------------------------------------------
// HELPER FUNCTIONS (Called from Main Loop)
// ---------------------------------------------------

void update_ssid_list(const std::string& list) {
    snprintf(ssid_list_val_chr_value, sizeof(ssid_list_val_chr_value), "%s", list.c_str());

    if (pSsidListChar) {
        pSsidListChar->setValue(list.c_str());
        pSsidListChar->notify();
    }
}

void update_wifi_status(int status) {
    wifi_connection_status_chr_value = status;
    if (pStatusChar) {
        pStatusChar->setValue((uint32_t) status);
        pStatusChar->notify();
    }
}

void startNVS() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_flash_init();
    }
}

// ---------------------------------------------------
// START BLE
// ---------------------------------------------------

void startBLE() {
    printf("\n Starting BLE (Final Fixed Version) \n");

    // 1. Initialize NimBLE Device
    NimBLEDevice::init("ESP_Cloud_Remote_BLE");

    // 2. Set Power
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    // 3. Security
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);

    // 4. Create Server
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // ============================================================
    // SERVICE 1: SCAN SERVICE
    // ============================================================
    NimBLEService *pScanService = pServer->createService(SERVICE_SCAN_UUID);

    // Char: SSID List (Read | Notify)
    pSsidListChar = pScanService->createCharacteristic(
        CHAR_WIFI_SSID_LIST_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    pSsidListChar->setValue("Waiting for scan...");

    // Char: Should Scan (Read | Write)
    NimBLECharacteristic *pShouldScanChar = pScanService->createCharacteristic(
        CHAR_SHOULD_SCAN_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pShouldScanChar->setValue(should_scan_chr_value);
    // Attach the safe callback
    pShouldScanChar->setCallbacks(new ScanRequestCallback());

    pScanService->start();

    // ============================================================
    // SERVICE 2: SET SERVICE
    // ============================================================
    NimBLEService *pSetService = pServer->createService(SERVICE_SET_UUID);

    // Char: Set SSID (Read | Write)
    NimBLECharacteristic *pSetSsidChar = pSetService->createCharacteristic(
        CHAR_SET_SSID_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pSetSsidChar->setValue(set_ssid_chr_value);
    pSetSsidChar->setCallbacks(new CredentialWriteCallback(set_ssid_chr_value, sizeof(set_ssid_chr_value)));

    // Char: Set Password (Read | Write)
    NimBLECharacteristic *pSetPassChar = pSetService->createCharacteristic(
        CHAR_SET_PASS_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE
    );
    pSetPassChar->setValue(set_pass_chr_value);
    pSetPassChar->setCallbacks(new CredentialWriteCallback(set_pass_chr_value, sizeof(set_pass_chr_value)));

    // Char: Connection Status (Read | Notify)
    pStatusChar = pSetService->createCharacteristic(
        CHAR_CONN_STATUS_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
    );
    pStatusChar->setValue((uint32_t) wifi_connection_status_chr_value);

    pSetService->start();

    // ============================================================
    // ADVERTISING
    // ============================================================
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();

    pAdvertising->addServiceUUID(SERVICE_SCAN_UUID);
    pAdvertising->addServiceUUID(SERVICE_SET_UUID);

    NimBLEAdvertisementData advData;
    advData.setFlags(0x06); // General Discoverable
    advData.setName("ESP_Cloud_Remote_BLE");
    advData.setCompleteServices(NimBLEUUID(SERVICE_SCAN_UUID));

    pAdvertising->setAdvertisementData(advData);
    pAdvertising->start();

    printf("BLE Started Successfully.\n");
}

void stopBLE() {
    printf("\n Stopping BLE \n");
    NimBLEDevice::deinit(true);
}
