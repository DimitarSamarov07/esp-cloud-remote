#include "mqtt_service.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "esp_log.h"
#include <string.h>

#include "esp_mac.h"
#include "mbedtls/sha256.h"

static const char *TAG = "mqtt_register";
static char response_buffer[512];
const char *charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";

typedef struct {
    char username[33];
    char password[65];
} mqtt_credentials_t;

extern esp_err_t mqtt_credentials_save(const char *username, const char *password);

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (evt->data_len < sizeof(response_buffer)) {
                memcpy(response_buffer, evt->data, evt->data_len);
                response_buffer[evt->data_len] = '\0';

                cJSON *json = cJSON_Parse(response_buffer);
                if (json) {
                    cJSON *status = cJSON_GetObjectItem(json, "status");

                    if (cJSON_IsString(status) && strcmp(status->valuestring, "Device linked successfully")) {
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

mqtt_credentials_t generate_mqtt_credentials(const size_t pass_length) {
    mqtt_credentials_t creds;
    uint8_t mac[6];
    uint8_t random_seed[16];
    uint8_t hash_output[32];

    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    esp_fill_random(random_seed, sizeof(random_seed));

    snprintf(creds.username, sizeof(creds.username), "mqtt_pc_%02x%02x%02x",
             mac[3], mac[4], mac[5]);

    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    mbedtls_sha256_update(&ctx, mac, sizeof(mac));
    mbedtls_sha256_update(&ctx, random_seed, sizeof(random_seed));
    mbedtls_sha256_finish(&ctx, hash_output);
    mbedtls_sha256_free(&ctx);

    size_t actual_pass_len = (pass_length > 64) ? 64 : pass_length;
    const size_t charset_len = strlen(charset);

    for (size_t i = 0; i < actual_pass_len; i++) {
        creds.password[i] = charset[hash_output[i % 32] % charset_len];
    }
    creds.password[actual_pass_len] = '\0';

    return creds;
}

esp_err_t mqtt_register_device() {
    ESP_LOGI(TAG, "Starting device registration...");
    mqtt_credentials_t creds = generate_mqtt_credentials( 16);
    esp_http_client_config_t config = {
        .url = "http://90.154.171.96:8690/register/device",
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
    cJSON_AddStringToObject(root, "deviceID", creds.username );
    cJSON_AddStringToObject(root, "password", creds.password);
    char *post_data = cJSON_PrintUnformatted(root);
    mqtt_credentials_save(creds.username, creds.password);

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

    return err;
}