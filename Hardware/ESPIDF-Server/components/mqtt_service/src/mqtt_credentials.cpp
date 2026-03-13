#include <string.h>
#include "esp_log.h"
#include "mqtt_internal.h"
#include "mqtt_service.h"
#include "nvs_flash.h"

static const char *TAG = "mqtt_creds";

/**
 * =============================================================================
 * MODULE: MQTT CREDENTIALS & NVS MANAGEMENT
 * =============================================================================
 * Description: Handles the persistent storage of MQTT authentication details
 * across device reboots using the ESP32's Non-Volatile Storage (NVS).
 * * NVS Namespace: "mqtt_creds"
 * Stored Keys:
 * - "username"   (String) : The MQTT client username
 * - "password"   (String) : The MQTT client password/token
 * - "setup_done" (uint8_t): Flag indicating credentials exist (1 = True)
 * =============================================================================
 */


/**
 * @brief Saves the MQTT username and password to flash memory.
 * * @param username Null-terminated string containing the MQTT client username.
 * @param password Null-terminated string containing the MQTT client password.
 * @return esp_err_t ESP_OK on success, or a specific ESP error code on failure.
 */
esp_err_t mqtt_credentials_save(const char *username, const char *password) {
    nvs_handle_t nvs_handle;

    // Open the NVS namespace in Read/Write mode so we can modify it
    esp_err_t err = nvs_open("mqtt_creds", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS");
        return err;
    }

    // Write the username string
    err = nvs_set_str(nvs_handle, "username", username);
    if (err != ESP_OK) goto cleanup; // If writing fails, jump to cleanup to safely close NVS

    // Write the password string
    err = nvs_set_str(nvs_handle, "password", password);
    if (err != ESP_OK) goto cleanup;

    // Set a flag so we can quickly check if setup is done without reading heavy strings later
    err = nvs_set_u8(nvs_handle, "setup_done", 1);
    if (err != ESP_OK) goto cleanup;

    // Commit the changes to the physical flash memory (REQUIRED)
    err = nvs_commit(nvs_handle);

cleanup:
    // Always close the handle to free up memory/resources, even if an error occurred
    nvs_close(nvs_handle);
    return err;
}


/**
 * @brief Loads the MQTT credentials from NVS into the provided buffers.
 * * @param username_out Pointer to the character array where the username will be copied.
 * @param username_size Maximum size of the username_out buffer (in bytes).
 * @param password_out Pointer to the character array where the password will be copied.
 * @param password_size Maximum size of the password_out buffer (in bytes).
 * @return esp_err_t ESP_OK on success, or a specific ESP error code if not found/failed.
 */
esp_err_t mqtt_credentials_load(char *username_out, size_t username_size,
                                char *password_out, size_t password_size) {
    nvs_handle_t nvs_handle;

    // Open in Read-Only mode to protect data and save memory
    esp_err_t err = nvs_open("mqtt_creds", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS not found, first boot?");

        // WARNING: Ensure this doesn't conflict with main.cpp if main.cpp
        // also calls mqtt_register_device() when credentials aren't found!
        mqtt_register_device();
        return err;
    }

    // Read the username into the provided memory buffer
    err = nvs_get_str(nvs_handle, "username", username_out, &username_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read username");
        nvs_close(nvs_handle);
        return err;
    }

    // Read the password into the provided memory buffer
    err = nvs_get_str(nvs_handle, "password", password_out, &password_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read password");
        // Note: Not returning here yet, allowing it to close safely below
    }

    nvs_close(nvs_handle);
    return err;
}


/**
 * @brief Fast check to see if the device has been provisioned.
 * * @return true if the "setup_done" flag is found and equals 1.
 * @return false if the credentials do not exist or NVS fails to open.
 */
bool mqtt_credentials_exist(void) {
    nvs_handle_t nvs_handle;
    uint8_t setup_done = 0;

    // We only need Read-Only access here
    if (nvs_open("mqtt_creds", NVS_READONLY, &nvs_handle) == ESP_OK) {

        // Pull just the 1-byte flag instead of loading the massive string buffers
        nvs_get_u8(nvs_handle, "setup_done", &setup_done);

        nvs_close(nvs_handle);
    }

    // Return true if the flag is 1, false otherwise
    return (setup_done == 1);
}


/**
 * @brief Wipes all MQTT credentials from the device (Factory Reset).
 * * @return esp_err_t ESP_OK on successful erasure, or an error code on failure.
 */
esp_err_t mqtt_credentials_clear(void) {
    nvs_handle_t nvs_handle;

    // Open in Read/Write mode so we can delete things
    esp_err_t err = nvs_open("mqtt_creds", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) return err;

    // Erase everything in the "mqtt_creds" namespace
    nvs_erase_all(nvs_handle);

    // Commit the deletion to physical flash
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    ESP_LOGI(TAG, "Credentials cleared");
    return ESP_OK;
}