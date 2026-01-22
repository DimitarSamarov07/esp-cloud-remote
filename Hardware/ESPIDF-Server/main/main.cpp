#include "Arduino.h"
#include "ble_service/ble_communication.h"



extern "C" {
    #include "mqtt_service.h"
    #include "wifi_control.h"
}

static const char* TAG = "MQTT_APP";




void setup()
{
    wifi_init_setup();
    wifi_init_sta("Arabadzhievi", "16042325");

    while (IS_WIFI_CONNECTED == 0){
        delay(1000);
        ESP_LOGI(TAG, "Still waiting for connection...");
    }

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }
    if (!mqtt_credentials_exist())
    {
        ESP_LOGI(TAG, "Setup not done. Registering...");
        mqtt_register_device();
        mqtt_service_init();
        mqtt_service_start();
    }
    else
    {
        ESP_LOGI(TAG, "Setup already completed. Logging in...");
        mqtt_service_init();
        mqtt_service_start();
    }
    startBLE();

}


void loop()
{
    delay(500);
}
