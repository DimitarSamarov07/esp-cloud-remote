//
// Created by bruteforce on 22.04.26 г..
//

#include "../include/universal_remote.h"

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRutils.h>
#include "esp_log.h"

static const char* AC_TAG = "AC_CONTROL";

// Define the IR LED pin
const uint16_t IR_LED_PIN = 11;

// Create the global universal AC object
IRac acc(IR_LED_PIN);
stdAc::opmode_t str_to_mode(const char* mode) {
    if (strcasecmp(mode, "Cool") == 0) return stdAc::opmode_t::kCool;
    if (strcasecmp(mode, "Heat") == 0) return stdAc::opmode_t::kHeat;
    if (strcasecmp(mode, "Dry") == 0)  return stdAc::opmode_t::kDry;
    if (strcasecmp(mode, "Fan") == 0)  return stdAc::opmode_t::kFan;
    return stdAc::opmode_t::kAuto;
}

// Helper to map fan speed strings
stdAc::fanspeed_t str_to_fan(const char* fan) {
    if (strcasecmp(fan, "High") == 0)   return stdAc::fanspeed_t::kMax;
    if (strcasecmp(fan, "Medium") == 0) return stdAc::fanspeed_t::kMedium;
    if (strcasecmp(fan, "Low") == 0)    return stdAc::fanspeed_t::kMin;
    return stdAc::fanspeed_t::kAuto;
}

/**
 * @brief Initialize default AC parameters on boot
 */
void ac_init() {
    // Set some safe default values so the object isn't empty
    acc.next.celsius = true;
    acc.next.fanspeed = stdAc::fanspeed_t::kAuto;
    acc.next.swingv = stdAc::swingv_t::kOff;
    acc.next.swingh = stdAc::swingh_t::kOff;
    acc.next.light = false;
    acc.next.beep = false;
    acc.next.power = false;

    ESP_LOGI(AC_TAG, "Universal AC Control Initialized on pin %d", IR_LED_PIN);
}

/**
 * @brief Translates user input into an IR blast
 * * @param brand_str  The brand string (e.g., "SAMSUNG", "DAIKIN", "LG")
 * @param power_on   True to turn ON, False to turn OFF
 * @param temp_c     Target temperature in Celsius
 * @param mode       The standard operating mode (0=Auto, 1=Cool, 2=Heat, etc.)
 * @param light
 */
void set_ac_state(const char* brand_str, bool power_on, float temp_c, const char* mode, const char* fanspeed, stdAc::swingv_t swingv, stdAc::swingh_t swingh, bool light, bool beep) {

    // 1. Convert the user's string into the library's protocol enum
    decode_type_t protocol = strToDecodeType(brand_str);

    // 2. Validate that the string was recognized AND supported by the universal class
    if (protocol == decode_type_t::UNKNOWN || !acc.isProtocolSupported(protocol)) {
        ESP_LOGE(AC_TAG, "Protocol '%s' is not supported by the universal AC class!", brand_str);
        return;
    }

    // 3. Apply the requested state to the 'next' buffer
    acc.next.protocol = protocol;
    acc.next.power = power_on;
    acc.next.degrees = temp_c;
    acc.next.mode = str_to_mode(mode);

    // Note: If a specific brand requires a 'model' sub-type, it defaults to 1.
    // If a user needs a specific sub-model, you can expose ac.next.model as well.
    acc.next.model = 1;
    acc.next.fanspeed = str_to_fan(fanspeed);
    acc.next.swingv = swingv;
    acc.next.swingh = swingh;
    acc.next.light = light;
    acc.next.beep = beep;

    // 4. Fire the IR blast!
    ESP_LOGI(AC_TAG, "Sending Command -> Brand: %s | Power: %d | Temp: %.1f", brand_str, power_on, temp_c);
    acc.sendAc();
}