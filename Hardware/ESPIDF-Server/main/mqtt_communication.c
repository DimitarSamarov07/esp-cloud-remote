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
#include "cJSON.h"



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


/**
 * @brief Handles incoming MQTT messages and processes them based on the topic received.
 * This callback function processes messages received on specific MQTT topics.
 * It differentiates between the topics "ac/control" and "connection/wifi" and performs
 * actions accordingly. It also provides logging for diagnostic purposes.
 * @param topic The MQTT topic on which the message is received.
 * @param message The payload of the MQTT message.
 * @param length The length of the message payload.
 * The function performs the following actions:
 * - For the "ac/control" topic:
 *   - Processes specific commands such as "TURN_LED_ON", "TURN_LED_OFF".
 *   - Publishes response messages or handles recognized last will messages.
 *   - Logs warnings for unrecognized commands.
 * - For the "connection/wifi" topic:
 *   - Logs the received Wi-Fi configuration message contents.
 *
 * Note:
 * - Messages are truncated if their length exceeds 100 characters.
 * - Logs messages at different levels (info, warning) for debugging.
 */
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


/**
 * @brief Handles various MQTT events triggered during client operations.
 *
 * This function processes MQTT events such as connection, disconnection, data reception,
 * and error handling. The operations performed for each event include logging, subscribing
 * to topics, and invoking appropriate callbacks or actions to manage the MQTT client status.
 *
 * The function handles the following events:
 * - **MQTT_EVENT_BEFORE_CONNECT**:
 *   Logs a message indicating that the MQTT client is preparing to connect.
 * - **MQTT_EVENT_SUBSCRIBED**:
 *   Logs a message indicating a successful subscription to topics.
 * - **MQTT_EVENT_PUBLISHED**:
 *   Logs a message indicating that a message has been successfully published.
 * - **MQTT_EVENT_CONNECTED**:
 *   Logs a message indicating successful connection and subscribes to predefined topics.
 * - **MQTT_EVENT_DISCONNECTED**:
 *   Logs a warning about disconnection, delays for reconnection attempts, and restarts the client.
 * - **MQTT_EVENT_ERROR**:
 *   Logs an error message when an issue is encountered.
 * - **MQTT_EVENT_DATA**:
 *   Logs the received topic and message data, and invokes the mqtt_callback to process the data.
 * - **default**:
 *   Logs unhandled events for debugging purposes.
 *
 * @param handler_args A pointer to additional arguments passed to the handler.
 * @param base The base identifier for the event.
 * @param event_id The specific event ID of the triggered MQTT event.
 * @param event_data A pointer to the event data structure containing relevant MQTT information.
 */
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

/**
 * @brief Buffer used for storing response data received during HTTP events.
 * This static character array is allocated to temporarily hold response data
 * such as JSON payloads received from an HTTP server. The buffer ensures that
 * response data can be processed, parsed, or logged without overflow.
 *
 * Characteristics:
 * - Maximum length of 256 bytes.
 * - Ensures data is null-terminated after copying to prevent memory access violations.
 *
 * Usage:
 * - Commonly used in conjunction with the `_http_event_handle` function to store
 *   event payload, such as JSON-formatted strings, for further processing.
 * - Alerts are logged if the incoming data exceeds the buffer's capacity.
 */
static char response_buffer[256];

/**
 * @brief Handles HTTP client events and processes data based on the event type.
 * This function processes a variety of HTTP client events such as connection, data reception, and
 * errors. It handles incoming JSON payloads, extracts specific fields (e.g., username and password),
 * and saves them securely in NVS. Additionally, it logs detailed information for each event.
 *
 * @param evt Pointer to the HTTP client event structure containing event details.
 * - HTTP_EVENT_ERROR: Indicates an error in the HTTP connection.
 * - HTTP_EVENT_ON_CONNECTED: Triggered upon successful connection to the server.
 * - HTTP_EVENT_HEADER_SENT: Occurs when HTTP request headers have been sent.
 * - HTTP_EVENT_ON_HEADER: Triggered when an HTTP response header is received.
 * - HTTP_EVENT_ON_DATA: Fired when part of the response body data is received.
 *   - Processes JSON payload, logs errors, saves credentials to NVS if valid.
 * - HTTP_EVENT_ON_FINISH: Called after the complete HTTP response has been received.
 * - HTTP_EVENT_DISCONNECTED: Indicates the HTTP connection has been closed.
 * - HTTP_EVENT_REDIRECT: Triggered if a redirection HTTP response is received.
 * For unhandled event IDs, it logs a warning.
 *
 * @return Returns `ESP_OK` on successful handling of the event. Logs errors for invalid data
 *         or failures in various operations but always returns `ESP_OK`.
 */
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

/**
 * @brief Saves user credentials to Non-Volatile Storage (NVS).
 * This function stores the given username and password securely in the ESP32's
 * NVS. Upon successful execution, the credentials are committed for persistent storage.
 * Errors during saving or committing to NVS are logged for debugging.
 *
 * @param username The username to save in NVS. Must be a null-terminated string.
 * @param password The password to save in NVS. Must be a null-terminated string.
 * @return
 * - ESP_OK if the credentials were saved successfully.
 * - Appropriate ESP_ERR_* code if an error occurred during the operation (e.g., opening, writing, or committing).
 */
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

/**
 * @brief Loads MQTT credentials (username and password) from NVS storage.
 * This function retrieves MQTT credentials stored in the ESP32's Non-Volatile Storage (NVS).
 * If successful, it populates the provided buffers with the username and password values.
 * Logs error or success messages depending on the status of the operations.
 *
 * @param username_out Pointer to a buffer to store the MQTT username.
 * @param username_size The size of the buffer provided for the MQTT username.
 * @param password_out Pointer to a buffer to store the MQTT password.
 * @param password_size The size of the buffer provided for the MQTT password.
 *
 * Note:
 * - The function only reads from NVS and does not modify it.
 * - Logs errors for any failed operation.
 */
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