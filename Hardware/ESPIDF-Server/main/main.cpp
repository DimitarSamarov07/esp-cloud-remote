#include "Arduino.h"
#include "mqtt_service/mqtt_communication.h"
#include "ble_serice/ble_communication.h"
#include <cstdio>
#include <wifi_service/wifi_control.h>

#include "esp_mac.h"


static const char* TAG = "MQTT_APP";

constexpr char charset[] = "abcdefghijklmnopqrstuvwxyz"
                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                       "0123456789"
                       "!@#$%^&*()-_=+";

void get_mac_string(char* buf) {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    sprintf(buf, "%02X%02X%02X%02X%02X%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

char* generate_secure_password(size_t length) {
    if (length < 1) return nullptr;

    char* password = static_cast<char*>(malloc(length + 1));
    if (!password) return nullptr;

    uint8_t rand_byte;
    for (size_t i = 0; i < length; i++) {
        esp_fill_random(&rand_byte, 1);
        password[i] = charset[rand_byte % (sizeof(charset) - 1)];
    }

    password[length] = '\0';
    return password;
}
void setup()
{
    ESP_LOGI(TAG, "Initializing WiFi");
    wifi_init_setup();
    wifi_init_sta();

    while (IS_WIFI_CONNECTED == 0){
        delay(1000);
        ESP_LOGI(TAG, "Still waiting for connection...");
    }

    ESP_LOGI(TAG, "Connected to WiFi");
    ESP_LOGI(TAG, "Connecting to MQTT");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }
    if (!is_setup_done())
    {
        ESP_LOGI(TAG, "Setup not done. Registering...");
        char* password = generate_secure_password(16);
        mqtt_first_init(password);
        free(password);
        mqtt_init();
    }
    else
    {
        ESP_LOGI(TAG, "Setup already completed. Logging in...");
        mqtt_init();
    }
    startBLE();

}


void loop()
{
    delay(500);
}
