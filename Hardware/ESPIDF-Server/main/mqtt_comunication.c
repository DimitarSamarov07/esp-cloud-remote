#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

#define MQTT_AC_CONTROL_TOPIC "ac/control"
#define MQTT_WIFI_CONFIG_TOPIC "connection/wifi"
#define LED_PIN GPIO_NUM_2
#define MQTT_QOS 1

#define WIFI_SSID "Dummy"
#define WIFI_PASSWORD "karuca12345"

static const char* TAG = "MQTT_APP";
static esp_mqtt_client_handle_t client;

//Configuring GPIO pin to flash the LED
void configure_gpio(gpio_num_t pin)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

//Flashing the LED on the specified pin every 2 secs
void flash_led()
{
    gpio_set_level(LED_PIN, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(LED_PIN, 0);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level(LED_PIN, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(LED_PIN, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

//Callback method that is called every time the ESP is connected to a broker
void mqtt_callback(const char* topic, const char* message, size_t length)
{
    char received_message[100] = {0};
    strncpy(received_message, message, length);
    received_message[length] = '\0';

    if (strncmp(topic, MQTT_AC_CONTROL_TOPIC, strlen(MQTT_AC_CONTROL_TOPIC)) == 0)
    {
        ESP_LOGI(TAG, "Received AC control message: %s", received_message);

        if (strcmp(received_message, "TURN_ON") == 0)
        {
            esp_mqtt_client_publish(client, MQTT_AC_CONTROL_TOPIC, "AC turned ON", 0, MQTT_QOS, 0);
            flash_led();
        }
        else if (strcmp(received_message, "TURN_OFF") == 0)
        {
            esp_mqtt_client_publish(client, MQTT_AC_CONTROL_TOPIC, "AC turned OFF", 0, MQTT_QOS, 0);
            flash_led();
        }
        else
        {
            ESP_LOGW(TAG, "Unknown AC command: %s", received_message);
        }
    }
    else if (strncmp(topic, MQTT_WIFI_CONFIG_TOPIC, strlen(MQTT_WIFI_CONFIG_TOPIC)) == 0)
    {
        ESP_LOGI(TAG, "Received Wi-Fi config message: %s", received_message);
    }
}

//Event handler method that is called on every setup (Basically like Arduino's setup())
void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;

    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT Connected also Angel is the best");
        esp_mqtt_client_subscribe(client, MQTT_AC_CONTROL_TOPIC, MQTT_QOS);
        esp_mqtt_client_subscribe(client, MQTT_WIFI_CONFIG_TOPIC, MQTT_QOS);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Received message on topic: %.*s", event->topic_len, event->topic);
        mqtt_callback(event->topic, event->data, event->data_len);
        break;

    default:
        ESP_LOGI(TAG, "Unhandled MQTT event: %ld", event_id);
        break;
    }
}

//Initiating a WIFI connection with the specified SSID and password
void wifi_init_sta()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, 
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialized, connecting to SSID: %s", WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_connect());
}

//Connect to MQTT broker
void mqtt_init()
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://93.155.224.232:5728",
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
}
