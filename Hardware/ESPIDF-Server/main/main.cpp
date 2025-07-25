#include "Arduino.h"
#include "mqtt_comunication.h"
#include "ble_communication.h"
#include <stdio.h>
#include <string.h>


static const char* TAG = "MQTT_APP";
const char charset[] = "abcdefghijklmnopqrstuvwxyz"
                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                       "0123456789"
                       "!@#$%^&*()-_=+";

char *generate_secure_password(size_t length) {
    if (length < 1) return NULL;

    char* password = static_cast<char*>(malloc(length + 1));
    if (!password) return NULL;

    uint8_t rand_byte;

    for (size_t i = 0; i < length; i++) {
        esp_fill_random(&rand_byte, 1);
        password[i] = charset[rand_byte % (sizeof(charset) - 1)];
    }

    password[length] = '\0';
    return password;
}
void setup(){
    ESP_LOGI(TAG, "Initializing MQTT");
    wifi_init_sta();
    ESP_LOGI(TAG, "Connecting to MQTT");
    delay(10000);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }
    if (!is_setup_done())
    {
        ESP_LOGI(TAG, "Setup not done. Registering...");
        mqtt_first_init(NULL, generate_secure_password(16));
        save_setup_flag(true);
    }
    else
    {
        ESP_LOGI(TAG, "Setup already completed. Logging in...");
        char stored_username[64] = {0};
        char stored_password[64] = {0};
        load_credentials_nvs(stored_username, sizeof(stored_username), stored_password, sizeof(stored_password));
        mqtt_first_init(stored_username, stored_password);
    }
    startBLE();

    mqtt_init();
}



void loop()
{
    delay(500);
}
