#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "nvs.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "wifi_service/wifi_control.h"

#define MQTT_AC_CONTROL_TOPIC "ac/control"
#define MQTT_WIFI_CONFIG_TOPIC "connection/wifi"
#define LED_PIN GPIO_NUM_21
#define MQTT_QOS 1

//TODO: Add connections without password.
#define WIFI_SSID "Angel"
#define WIFI_PASSWORD "16042325"

static const char* SUBSCRIPTION_TOPICS[] = {
    "ac/control/%s",
    "connection/wifi/%s",
};

#define TOPIC_COUNT (sizeof(SUBSCRIPTION_TOPICS) / sizeof(SUBSCRIPTION_TOPICS[0]))
static const char *TAG = "ESP32";
static esp_mqtt_client_handle_t mqtt_client;
static char response_buffer[512];
char username[64] = {0};

/**
 * @brief Handles incoming MQTT messages using dynamic topic detection
 */
void mqtt_callback(const char *topic, const char *message, size_t length) {
    char received_message[128] = {0};
    if (length >= sizeof(received_message)) length = sizeof(received_message) - 1;
    memcpy(received_message, message, length);
    received_message[length] = '\0';

    ESP_LOGI(TAG, "Processing message on topic: %s", topic);


    if (strstr(topic, "ac/control") != NULL) {
        if (strcmp(received_message, "TURN_LED_ON") == 0) {
            ESP_LOGI(TAG, "Action: Turning AC ON");
            esp_mqtt_client_publish(mqtt_client, topic, "AC turned ON", 0, MQTT_QOS, 1);
        }
        else if (strcmp(received_message, "TURN_LED_OFF") == 0) {
            ESP_LOGI(TAG, "Action: Turning AC OFF");
            esp_mqtt_client_publish(mqtt_client, topic, "AC turned OFF", 0, MQTT_QOS, 1);
        }
        else if (strstr(received_message, "turned") != NULL) {
            return;
        }
        else {
            ESP_LOGW(TAG, "Unknown command in ac/control: %s", received_message);
        }
    }

    else if (strstr(topic, "connection/wifi") != NULL) {
        ESP_LOGI(TAG, "WiFi Config received: %s", received_message);
        char *ssid = strtok(received_message, "/");
        char *pass = strtok(NULL, "/");
        if (ssid != NULL && pass != NULL) {
            change_wifi(ssid, pass);
        }
    }

    else {
        ESP_LOGW(TAG, "Message received on unregistered topic: %s", topic);
    }
}

/**
 * @brief MQTT Event Handler
 */
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    mqtt_client = event->client;

    switch (event->event_id) {
        case MQTT_EVENT_BEFORE_CONNECT:
            ESP_LOGI(TAG, "MQTT is preparing to connect...");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT subscribed");
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT published");
            break;
        case MQTT_EVENT_CONNECTED:
            char full_topic[128];

            for (int i = 0; i < TOPIC_COUNT; i++) {
                // Construct the unique topic: e.g., "ac/control/my_username_123"
                snprintf(full_topic, sizeof(full_topic), SUBSCRIPTION_TOPICS[i], username);

                int msg_id = esp_mqtt_client_subscribe(event->client, full_topic, MQTT_QOS);
                ESP_LOGI(TAG, "Sent subscribe subscribe topic: %s, msg_id=%d", full_topic, msg_id);
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT Disconnected! Library auto-reconnect logic started.");
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT encountered an error");
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Received message on topic: %.*s", event->topic_len, event->topic);
            mqtt_callback(event->topic, event->data, event->data_len);
            break;
        default:
            break;
    }
}

/**
 * @brief NVS Helpers (FIXED: Unified namespace to "mqtt_creds")
 */
esp_err_t save_credentials_nvs(const char *username, const char *password) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("mqtt_creds", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return err;

    nvs_set_str(nvs_handle, "username", username);
    nvs_set_str(nvs_handle, "password", password);
    nvs_set_u8(nvs_handle, "setup_done", 1);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    return ESP_OK;
}

/**
 * @brief HTTP Event Handle for Registration
 */
esp_err_t _http_event_handle(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (evt->data_len < sizeof(response_buffer)) {
                memcpy(response_buffer, evt->data, evt->data_len);
                response_buffer[evt->data_len] = '\0';

                cJSON *json = cJSON_Parse(response_buffer);
                if (json) {
                    cJSON *username = cJSON_GetObjectItem(json, "username");
                    cJSON *password = cJSON_GetObjectItem(json, "password");

                    if (cJSON_IsString(username) && cJSON_IsString(password)) {
                        save_credentials_nvs(username->valuestring, password->valuestring);
                        ESP_LOGI(TAG, "Credentials saved from registration.");
                    }
                    cJSON_Delete(json);
                }
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}


void load_credentials_nvs(char *username_out, size_t username_size, char *password_out, size_t password_size) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("mqtt_creds", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) return;

    nvs_get_str(nvs_handle, "username", username_out, &username_size);
    nvs_get_str(nvs_handle, "password", password_out, &password_size);
    nvs_close(nvs_handle);
}

bool is_setup_done() {
    nvs_handle_t nvs;
    uint8_t flag = 0;
    if (nvs_open("mqtt_creds", NVS_READONLY, &nvs) == ESP_OK) {
        nvs_get_u8(nvs, "setup_done", &flag);
        nvs_close(nvs);
    }
    return (flag == 1);
}

/**
 * @brief HTTP POST to /register
 */
void mqtt_first_init(const char *password) {
    ESP_LOGI(TAG, "Starting Registration...");
    const char *url = "http://90.154.171.96:8690/register";

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .event_handler = _http_event_handle,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "email", "example@gmail.com");
    cJSON_AddStringToObject(root, "password", password);
    char *post_data = cJSON_PrintUnformatted(root);

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Registration request sent. Status: %d", esp_http_client_get_status_code(client));
    }

    esp_http_client_cleanup(client);
    free(post_data);
    cJSON_Delete(root);
}

/**
 * @brief Initializes MQTT with loaded credentials
 */
void mqtt_init() {

    char password[64] = {0};
    char lwt_topic[128];

    load_credentials_nvs(username, sizeof(username), password, sizeof(password));

    if (strlen(username) == 0) {
        ESP_LOGE(TAG, "No username found in NVS. Cannot start MQTT.");
        return;
    }
    snprintf(lwt_topic, sizeof(lwt_topic), "ac/status/%s", username);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://90.154.171.96:5728",
        .credentials.username = username,
        .credentials.authentication.password = password,
        .session.keepalive = 60,
        .session.disable_clean_session = true,
        .session.last_will.topic = lwt_topic,
        .session.last_will.msg = "offline",
        .session.last_will.qos = 1,
        .session.last_will.retain = true,
        .session.protocol_ver = MQTT_PROTOCOL_V_5
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}
