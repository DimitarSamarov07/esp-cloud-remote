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


const char* SSID = "";
const char* PASSWORD  = "";
int8_t QoS = 1;

static esp_mqtt_client_handle_t client;

static void mqtt5_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
        
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}
void topicsSubcride(){
    esp_mqtt_client_subscribe_single(client, mqttACControl, QoS);
    esp_mqtt_client_subscribe_single(client, mqttConnectionWifi, QoS);
}
void callback(char* topic, char* message, unsigned int length ){
    char* recievedMessage = "";
    for(int i = 0; i < length; i++){
        recievedMessage += (char)message[i];    
    }
    if(strcmp(topic, mqttACControl) == 0){
        printf("[ESP} Message receieved on topic ac/control \n");
        if (strcmp(recievedMessage, "TURN_ON") == 0)
        {
            esp_mqtt_client_publish(client, mqttACControl, "[ESP} Turn ON signal sent to AC!", 0, QoS, false);
            flashLED();
        }
        else if(strcmp(recievedMessage, "TURN_OFF") == 0){
            esp_mqtt_client_publish(client, mqttACControl, "[ESP] Turn OFF signal sent to AC!", 0, QoS, false);
            flashLED();
        }
        else if(strcmp(recievedMessage, "TURN_LED_ON") == 0){
            gpio_set_level(GPIO_NUM_2, 1);
            vTaskDelay(3000);
        }
        else if(strcmp(recievedMessage, "TURN_LED_FF") == 0){
            gpio_set_level(GPIO_NUM_2, 0);
            flashLED();
        }
        else{
            esp_mqtt_client_publish(client, mqttACControl, "[ESP] Unknown command sent to ac/control");
            flashLED();
        }
    }
    if(strcmp(mqttConnectionWifi, topic ) == 0){
        printf("Message received from connection/wifi \n");
    }  
}
void reconnect(){
    if(esp_mqtt_client_reconnect != ESP_FAIL){
        printf("[ESP] Connected! \n");
        topicsSubcride();
    }
    else{
        vTaskDelay(5000);
    }
}
void configure_gpio(gpio_num_t pin){
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}
void flashLED(){
    gpio_set_level(GPIO_NUM_2, 1);
    vTaskDelay(100);
    gpio_set_level(GPIO_NUM_2, 0);
    vTaskDelay(50);
    gpio_set_level(GPIO_NUM_2, 2);
    vTaskDelay(100);
    gpio_set_level(GPIO_NUM_2, 0);
    vTaskDelay(1000);
}

void app_main(void)
{
    mqtt5_app_start();
}
