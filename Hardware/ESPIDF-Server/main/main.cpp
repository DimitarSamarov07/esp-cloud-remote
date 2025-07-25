#include "Arduino.h"
#include "mqtt_comunication.h"
#include "ble_communication.h"

static const char* TAG = "MQTT_APP";


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

    //TODO: Add a password that is randomly generated and saved to the ESP32
    if (!is_setup_done())
    {
        ESP_LOGI(TAG, "Setup not done. Registering...");
        mqtt_first_init(NULL, "registration_password");
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
	//TODO: Check if this device has already been setup.

    startBLE();

    mqtt_init();
}

void loop()
{
    delay(500);
}
