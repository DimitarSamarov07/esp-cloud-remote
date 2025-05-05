#include <esp_random.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "esp_http_client.h"
#include "../components/cJSON/cJSON.h"


#define MQTT_AC_CONTROL_TOPIC "ac/control"
#define MQTT_WIFI_CONFIG_TOPIC "connection/wifi"
#define LED_PIN GPIO_NUM_21
#define MQTT_QOS 1

//TODO: Add connections without password.
#define WIFI_SSID "Arabadzhievi"
#define WIFI_PASSWORD "16042325"


static const char* TAG = "ESP32";
static esp_mqtt_client_handle_t mqtt_client; //Handle for the MQTT client
static esp_http_client_handle_t http_client; //Handle for the HTTP client


//IMPORTANT: Useless code for now so it's commented out. For your sake I hope you do not uncomment it.
//Configuring GPIO pin to flash the LED
// void configure_gpio(gpio_num_t pin)
// {
//     gpio_config_t io_conf = {
//         .pin_bit_mask = (1ULL << pin),
//         .mode = GPIO_MODE_OUTPUT,
//         .pull_up_en = GPIO_PULLUP_DISABLE,
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .intr_type = GPIO_INTR_DISABLE
//     };
//     ESP_ERROR_CHECK(gpio_config(&io_conf));
// }
//
// //Flashing the LED on the specified pin every 2 secs
// void flash_led()
// {
//     gpio_set_level(LED_PIN, 1);
//     vTaskDelay(100 / portTICK_PERIOD_MS);
//     gpio_set_level(LED_PIN, 0);
//     vTaskDelay(50 / portTICK_PERIOD_MS);
//     gpio_set_level(LED_PIN, 1);
//     vTaskDelay(100 / portTICK_PERIOD_MS);
//     gpio_set_level(LED_PIN, 0);
//     vTaskDelay(1000 / portTICK_PERIOD_MS);
// }

//Callback method that is called every time the ESP is connected to a broker
void mqtt_callback(const char* topic, const char* message, size_t length)
{
    char received_message[100] = {0};
    strncpy(received_message, message, length);
    received_message[length] = '\0';

    if (strncmp(topic, MQTT_AC_CONTROL_TOPIC, strlen(MQTT_AC_CONTROL_TOPIC)) == 0)
    {
        if (strcmp(received_message, "TURN_LED_ON") == 0)
        {
            esp_mqtt_client_publish(mqtt_client, MQTT_AC_CONTROL_TOPIC, "AC turned ON", 0, MQTT_QOS, 1);
        }
        else if (strcmp(received_message, "TURN_LED_OFF") == 0)
        {
            esp_mqtt_client_publish(mqtt_client, MQTT_AC_CONTROL_TOPIC, "AC turned OFF", 0, MQTT_QOS, 1);
        }
        else if (strcmp(received_message, "AC turned OFF") == 0 || strcmp(received_message, "AC turned ON") == 0 ||
            strcmp(received_message, "The AC is now OFF") == 0 || strcmp(received_message, "The AC is now ON") == 0)
        {
            received_message[0] = '\0';
        }
        else if (strcmp(received_message, "ESP32 Disconnected") != 0)
        {
            ESP_LOGI(TAG, "Received last will.");
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
    mqtt_client = event->client;

    switch (event->event_id)
    {
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
        ESP_LOGI(TAG, "MQTT Connected!");
        esp_mqtt_client_subscribe(mqtt_client, MQTT_AC_CONTROL_TOPIC, MQTT_QOS);
        esp_mqtt_client_subscribe(mqtt_client, MQTT_WIFI_CONFIG_TOPIC, MQTT_QOS);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "MQTT Disconnected! Attempting to reconnect...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_start(mqtt_client);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT encountered an error");
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Received message on topic: %.*s", event->topic_len, event->topic);
        mqtt_callback(event->topic, event->data, event->data_len);
        break;

    default:
        ESP_LOGI(TAG, "Unhandled MQTT event: %lu", event_id);
        break;
    }
}

static char response_buffer[256];

esp_err_t _http_event_handle(esp_http_client_event_t* evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
        break;

    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;

    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;

    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
        break;

    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);

        if (evt->data_len >= sizeof(response_buffer))
        {
            ESP_LOGE(TAG, "Response too long for buffer");
            break;
        }

    // Copy and null-terminate safely
        memcpy(response_buffer, evt->data, evt->data_len);
        response_buffer[evt->data_len] = '\0';

        ESP_LOGI(TAG, "Received JSON: %s", response_buffer);

        cJSON* json = cJSON_Parse(response_buffer);
        if (!json)
        {
            ESP_LOGE(TAG, "JSON Parse error: %s", cJSON_GetErrorPtr());
            break;
        }

        cJSON* username = cJSON_GetObjectItem(json, "username");
        cJSON* password = cJSON_GetObjectItem(json, "password");

        if (cJSON_IsString(username) && cJSON_IsString(password))
        {
            ESP_LOGI(TAG, "Username: %s", username->valuestring);
            ESP_LOGI(TAG, "Password: %s", password->valuestring);

            nvs_handle_t nvs_handle;
            esp_err_t err = nvs_open("mqtt_creds", NVS_READWRITE, &nvs_handle);
            if (err == ESP_OK)
            {
                nvs_set_str(nvs_handle, "username", username->valuestring);
                nvs_set_str(nvs_handle, "password", password->valuestring);
                nvs_commit(nvs_handle);
                nvs_close(nvs_handle);
                ESP_LOGI(TAG, "Saved credentials to NVS");
            }
            else
            {
                ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
            }
        }
        else
        {
            ESP_LOGE(TAG, "Missing username or password in JSON");
        }

        cJSON_Delete(json);
        break;

    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        break;

    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        break;

    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
        break;

    default:
        ESP_LOGW(TAG, "Unhandled HTTP event id: %d", evt->event_id);
        break;
    }

    return ESP_OK;
}


//Initiating a WI-FI connection with the specified SSID and password
void wifi_init_sta()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,  // Initialize ssid array to zero
            .password = WIFI_PASSWORD,  // Initialize password array to zero
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialized, connecting to SSID: %s", WIFI_SSID);
    ESP_ERROR_CHECK(esp_wifi_connect());
}

esp_err_t save_credentials_nvs(const char* username, const char* password)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS: Error opening NVS: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_str(nvs_handle, "username", username);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS: Error setting username in NVS: %s", esp_err_to_name(err));
        goto close_nvs;
    }

    err = nvs_set_str(nvs_handle, "password", password);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS: Error setting password in NVS: %s", esp_err_to_name(err));
        goto close_nvs;
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS: Error committing NVS: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "NVS: Credentials saved to NVS");
    }

close_nvs:
    nvs_close(nvs_handle);
    return err;
}

//TODO: Make a hasBeenSetup bool that is set to true once the configuration of username and password is done. If it's false it has to do setup.
void mqtt_first_init()
{
    esp_http_client_config_t config = {
        .url = "http://93.155.224.232:8690/auth/getCredentials",
        .method = HTTP_METHOD_GET,
        .event_handler = _http_event_handle,
    };
    http_client = esp_http_client_init(&config);
    if (!http_client)
    {
        ESP_LOGE(TAG, "HTTP client initialization failed");
        return;
    }
    esp_err_t err = esp_http_client_perform(http_client);
    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(http_client);
        ESP_LOGI(TAG, "HTTP GET Status = %d", status_code);
        if (status_code == 200)
        {
            const int content_length = esp_http_client_get_content_length(http_client);
            if (content_length > 0)
            {
                char* response_buffer = (char*)malloc(content_length + 1);
                if (response_buffer != NULL)
                {
                    int read_len = esp_http_client_read_response(http_client, response_buffer, content_length);
                    if (read_len >= 0)
                    {
                        response_buffer[read_len] = '\0';
                        ESP_LOGI(TAG, "HTTP Response Body:\n%s", response_buffer);


                        cJSON* json = cJSON_Parse(response_buffer);
                        if (json == NULL)
                        {
                            const char* error_ptr = cJSON_GetErrorPtr();
                            if (error_ptr != NULL)
                            {
                                ESP_LOGE(TAG, "JSON Parse error: %s", error_ptr);
                            }
                        }
                        else
                        {
                            cJSON* username_item = cJSON_GetObjectItem(json, "username");
                            cJSON* password_item = cJSON_GetObjectItem(json, "password");

                            if (username_item != NULL && cJSON_IsString(username_item) &&
                                password_item != NULL && cJSON_IsString(password_item))
                            {
                                ESP_LOGI(TAG, "Username: %s, Password: %s", username_item->valuestring,
                                         password_item->valuestring);
                                save_credentials_nvs(username_item->valuestring, password_item->valuestring);
                            }
                            else
                            {
                                ESP_LOGW(TAG, "Could not find username and/or password in JSON or they are not strings")
                                ;
                            }
                            cJSON_Delete(json);
                        }
                    }
                    else
                    {
                        ESP_LOGE(TAG, "Failed to read response");
                    }
                    free(response_buffer);
                }
                else
                {
                    ESP_LOGE(TAG, "Failed to allocate memory for response buffer");
                }
            }
            else
            {
                ESP_LOGW(TAG, "Empty response body");
            }
        }
        else
        {
            ESP_LOGE(TAG, "HTTP request failed with status code %d", status_code);
        }
    }
    else
    {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(http_client);
}

void load_credentials_nvs(char* username_out, size_t username_size, char* password_out, size_t password_size)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("mqtt_creds", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error opening NVS: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_get_str(nvs_handle, "username", username_out, &username_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get username: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "Loaded username: %s", username_out);
    }

    err = nvs_get_str(nvs_handle, "password", password_out, &password_size);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get password: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "Loaded password: %s", password_out);
    }

    nvs_close(nvs_handle);
}

//Connect to MQTT broker
void mqtt_init()
{
    char username[64] = {0};
    char password[64] = {0};
    load_credentials_nvs(username, sizeof(username), password, sizeof(password));
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://93.155.224.232:5728",
        .broker.address.uri = "mqtt://93.155.224.232:5728",
        .credentials.username = username,
        .session.keepalive = 20,
        .credentials.authentication.password = password,
        .session.disable_clean_session = true,
        .session.disable_keepalive = false,
        .session.last_will.topic = MQTT_AC_CONTROL_TOPIC,
        .session.last_will.msg = "ESP32 Disconnected",
        .session.last_will.qos = 1,
        .session.last_will.retain = true,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));
}
