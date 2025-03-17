#include "Arduino.h"
#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>
extern "C"{
#include "mqtt_comunication.h"
}
#include "ir_communication.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



static const char* TAG = "MQTT_APP";

extern "C" void app_main()
{
  initArduino();
  Serial.begin(115200);

  // wait for the Serial Monitor to be open
  while (!Serial) {
    delay(100);
  }

  Serial.println("\r\nStarting...\r\n");
  ESP_LOGI(TAG, "Application started");

  ESP_ERROR_CHECK(nvs_flash_init());
  wifi_init_sta();
  // configure_gpio(21);
  mqtt_init();

  //xTaskCreate(rmt_rx_task, "rmt_rx_task", 4096, NULL, 10, NULL);
}
