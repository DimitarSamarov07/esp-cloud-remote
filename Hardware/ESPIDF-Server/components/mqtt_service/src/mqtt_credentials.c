#include <string.h>
#include "esp_log.h"
#include "mqtt_internal.h"
#include "mqtt_service.h"
#include "nvs_flash.h"

static const char *TAG = "mqtt_creds";

esp_err_t mqtt_credentials_save(const char *username, const char *password) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("mqtt_creds", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS");
        return err;
    }

    err = nvs_set_str(nvs_handle, "username", username);
    if (err != ESP_OK) goto cleanup;
    
    err = nvs_set_str(nvs_handle, "password", password);
    if (err != ESP_OK) goto cleanup;
    
    err = nvs_set_u8(nvs_handle, "setup_done", 1);
    if (err != ESP_OK) goto cleanup;
    
    err = nvs_commit(nvs_handle);

cleanup:
    nvs_close(nvs_handle);
    return err;
}

esp_err_t mqtt_credentials_load(char *username_out, size_t username_size,
                                char *password_out, size_t password_size) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("mqtt_creds", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS not found, first boot?");
        mqtt_register_device();
        return err;
    }

    err = nvs_get_str(nvs_handle, "username", username_out, &username_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read username");
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_get_str(nvs_handle, "password", password_out, &password_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read password");
    }

    nvs_close(nvs_handle);
    return err;
}

bool mqtt_credentials_exist(void) {
    nvs_handle_t nvs_handle;
    uint8_t setup_done = 0;
    
    if (nvs_open("mqtt_creds", NVS_READONLY, &nvs_handle) == ESP_OK) {
        nvs_get_u8(nvs_handle, "setup_done", &setup_done);
        nvs_close(nvs_handle);
    }
    
    return (setup_done == 1);
}

esp_err_t mqtt_credentials_clear(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("mqtt_creds", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return err;

    nvs_erase_all(nvs_handle);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    
    ESP_LOGI(TAG, "Credentials cleared");
    return ESP_OK;
}