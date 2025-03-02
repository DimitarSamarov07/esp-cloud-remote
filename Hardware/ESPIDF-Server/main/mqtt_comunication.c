#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

#define mqttACControl "ac/control"
#define mqttConnectionWifi "connection/wifi"
#define LED_PIN 2

const char *SSID = "";
const char *PASSWORD = "";
int8_t QoS = 1;

static esp_mqtt_client_handle_t client;

void configure_gpio(gpio_num_t pin)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);
}
void flashLED()
{
    gpio_set_level(GPIO_NUM_2, 1);
    vTaskDelay(100);
    gpio_set_level(GPIO_NUM_2, 0);
    vTaskDelay(50);
    gpio_set_level(GPIO_NUM_2, 1);
    vTaskDelay(100);
    gpio_set_level(GPIO_NUM_2, 0);
    vTaskDelay(1000);
}
void topicsSubcride()
{
    esp_mqtt_client_subscribe_single(client, mqttACControl, QoS);
    esp_mqtt_client_subscribe_single(client, mqttConnectionWifi, QoS);
}

void mqtt_init_setup()
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://93.155.224.232:5728",

    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, callback, client );
    esp_mqtt_client_start(client);
    configure_gpio(2);
    topicsSubcride();
}
void callback(char *topic, char *message, unsigned int length)
{
    char *recievedMessage = "";
    for (int i = 0; i < length; i++)
    {
        recievedMessage += (char)message[i];
    }
    if (strcmp(topic, mqttACControl) == 0)
    {
        printf("[ESP} Message receieved on topic ac/control \n");
        if (strcmp(recievedMessage, "TURN_ON") == 0)
        {
            esp_mqtt_client_publish(client, mqttACControl, "[ESP} Turn ON signal sent to AC!", 0, QoS, true);
            flashLED();
        }
        else if (strcmp(recievedMessage, "TURN_OFF") == 0)
        {
            esp_mqtt_client_publish(client, mqttACControl, "[ESP] Turn OFF signal sent to AC!", 0, QoS, true);
            flashLED();
        }
        else if (strcmp(recievedMessage, "TURN_LED_ON") == 0)
        {
            gpio_set_level(GPIO_NUM_2, 1);
            vTaskDelay(3000);
        }
        else if (strcmp(recievedMessage, "TURN_LED_FF") == 0) //strcmp - comares a char array to the desired string and returns 0 if they are the same.
        {
            gpio_set_level(GPIO_NUM_2, 0);
            flashLED();
        }
        else
        {
            esp_mqtt_client_publish(client, mqttACControl, "[ESP] Unknown command sent to ac/control", 0, QoS, true);
            flashLED();
        }
    }
    if (strcmp(mqttConnectionWifi, topic) == 0)
    {
        printf("Message received from connection/wifi \n");
        int delimeterIndex = strcspn(recievedMessage, "/"); //strcspn - returns how much characters are before the first occurance of the desired character. If none are found it returns the length of the string.
        if(delimeterIndex != strlen(recievedMessage)){ //strlen - returns the length of the char array
            char *newSSID;
            char *newPass;
            strncpy(newSSID, delimeterIndex, strlen(recievedMessage) - delimeterIndex);
            strncpy(newPass, delimeterIndex + 1, strlen(recievedMessage));
            printf("[ESP] new SSID: ");
            print(newSSID);
            printf("[ESP] new password: ");
            print(newPass);
            esp_mqtt_client_publish(client, "ac/report", "Changed WiFi connection.", 0, QoS, true);
            flashLED();
        }
    }
    else{
        printf("[ESP] Problem with connecting to WiFi.");
        esp_mqtt_client_publish(client, "ac/report", "Couldn't change WiFi because it wasn't sent in correct format.", 0, QoS, true);
    }
}
void reconnect()
{
    if (esp_wifi_connect != ESP_FAIL)  //TODO: Check for ESP connection
    {
        printf("[ESP] Connected! \n");
        topicsSubcride();
    }
    else
    {
        vTaskDelay(5000);
    }
}