#include "mqtt_service.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "esp_log.h"
#include <string.h>

#include "esp_mac.h"
#include "mbedtls/sha256.h"

static const char *TAG = "mqtt_register";
static char response_buffer[512];
const char charset[] = "abcdefghijklmnopqrstuvwxyz"
                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                       "0123456789"
                       "!@#$%^&*()-_=+";

extern esp_err_t mqtt_credentials_save(const char *username, const char *password);

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (evt->data_len < sizeof(response_buffer)) {
                memcpy(response_buffer, evt->data, evt->data_len);
                response_buffer[evt->data_len] = '\0';

                cJSON *json = cJSON_Parse(response_buffer);
                if (json) {
                    cJSON *username_json = cJSON_GetObjectItem(json, "username");
                    cJSON *password_json = cJSON_GetObjectItem(json, "password");

                    if (cJSON_IsString(username_json) && cJSON_IsString(password_json)) {
                        mqtt_credentials_save(username_json->valuestring, 
                                            password_json->valuestring);
                        ESP_LOGI(TAG, "Registration successful, credentials saved");
                    } else {
                        ESP_LOGE(TAG, "Invalid response format");
                    }
                    cJSON_Delete(json);
                } else {
                    ESP_LOGE(TAG, "Failed to parse JSON response");
                }
            }
            break;

        case HTTP_EVENT_ERROR:
            ESP_LOGE(TAG, "HTTP request error");
            break;
        default:
            break;
    }
    return ESP_OK;
}

char* generate_secure_password(const size_t length) {
    if (length == 0 || length > 64) return NULL;

    uint8_t mac[6];
    uint8_t random_seed[16];
    uint8_t hash_output[32];

    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    esp_fill_random(random_seed, sizeof(random_seed));

    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    mbedtls_sha256_update(&ctx, mac, sizeof(mac));
    mbedtls_sha256_update(&ctx, random_seed, sizeof(random_seed));
    mbedtls_sha256_finish(&ctx, hash_output);
    mbedtls_sha256_free(&ctx);

    char *password = malloc(length + 1);
    if (!password) return NULL;

    const size_t charset_len = strlen(charset);
    for (size_t i = 0; i < length; i++) {
        password[i] = charset[hash_output[i % 32] % charset_len];
    }
    password[length] = '\0';

    return password;
}

esp_err_t mqtt_register_device() {
    ESP_LOGI(TAG, "Starting device registration...");
    char *password = generate_secure_password(16);
    esp_http_client_config_t config = {
        .url = "http://90.154.171.96:8690/register",
        .method = HTTP_METHOD_POST,
        .event_handler = http_event_handler,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "email", "debeliq@gmail.com");
    cJSON_AddStringToObject(root, "password", password);
    char *post_data = cJSON_PrintUnformatted(root);

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Registration response: HTTP %d", status);
        
        if (status != 200) {
            err = ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    free(post_data);
    cJSON_Delete(root);
    free(password);

    return err;
}