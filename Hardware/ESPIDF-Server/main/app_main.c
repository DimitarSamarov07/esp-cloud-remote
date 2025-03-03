#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "mqtt_comunication.h"


static const char* TAG = "MQTT_APP";

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
    configure_gpio(2);
    mqtt_init();
    ESP_LOGI(TAG, "Application started");
}
