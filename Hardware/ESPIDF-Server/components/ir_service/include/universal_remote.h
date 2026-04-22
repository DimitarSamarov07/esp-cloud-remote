#pragma once

#include <Arduino.h>
#include <IRac.h>

/**
 * @brief Initialize default AC parameters on boot and configures the IR pin.
 */
void ac_init();

/**
 * @brief Translates user input into an IR blast.
 * * @param brand_str  Null-terminated string of the brand (e.g., "SAMSUNG", "DAIKIN").
 * @param power_on   True to turn the AC ON, False to turn it OFF.
 * @param temp_c     Target temperature in degrees Celsius.
 * @param mode       The operating mode (e.g., stdAc::opmode_t::kCool, kHeat, kAuto).
 */
void set_ac_state(const char* brand_str, bool power_on, float temp_c, stdAc::opmode_t mode);

/**
 * @brief Cycles through models 1 to 8 for a specific brand, sending a "Power On" 
 * blast for each to help the user identify their correct model number.
 * * @param brand_str  Null-terminated string of the brand to discover.
 */
void discover_ac_model(const char* brand_str);

/**
 * @brief Utility function. Prints a complete list of all brand strings supported 
 * by the Universal AC controller to the ESP log.
 */
void printSupportedACBrands();