#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "mqtt_comunication.h"
#include "ir_communication.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char* TAG = "MQTT_APP";

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    // wifi_init_sta();
    // configure_gpio(21);
    // mqtt_init();

    xTaskCreate(rmt_rx_task, "rmt_rx_task", 4096, NULL, 10, NULL);
    ESP_LOGI(TAG, "Application started");
}
