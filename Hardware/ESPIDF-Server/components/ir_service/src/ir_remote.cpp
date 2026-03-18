#include <IRremoteESP8266.h>
#include <ir_Daikin.h>
#include <string_view>

constexpr uint16_t kIrLed = 11;
IRDaikinESP ac(kIrLed);

void startAcConnection() {
    ac.begin();
    Serial.begin(115200);
    Serial.println("Ac Connected");
}

static void applyMode(const std::string_view mode) {
    if (mode == "cool") ac.setMode(kDaikinCool);
    else if (mode == "heat") ac.setMode(kDaikinHeat);
    else if (mode == "dry") ac.setMode(kDaikinDry);
    else if (mode == "auto") ac.setMode(kDaikinAuto);
    else
        Serial.printf("Invalid mode: %s\n", mode.data());
}

static void applyFanSpeed(const std::string_view fanSpeed) {
    if (fanSpeed == "auto") {
        ac.setFan(kDaikinFanAuto);
    } else if (fanSpeed == "quiet") {
        ac.setFan(1);
        ac.setQuiet(true);
    } else if (fanSpeed == "low") {
        ac.setFan(2);
    } else if (fanSpeed == "medium") {
        ac.setFan(3);
    } else if (fanSpeed == "high") {
        ac.setFan(4);
    } else if (fanSpeed == "turbo") {
        ac.setFan(5);
        ac.setPowerful(true);
    } else {
        Serial.printf("Invalid fan speed: %s\n", fanSpeed.data());
    }
}

static void applySwing(const bool swing) {
    ac.setSwingVertical(swing);
    ac.setSwingHorizontal(swing);
}

void sendTurnSignal(const bool state, const float temp, const std::string_view mode, const std::string_view fanSpeed, const bool swing) {
    applyMode(mode);
    applyFanSpeed(fanSpeed);
    applySwing(swing);
    ac.setTemp(temp);
    if (state == 0) {
        ESP_LOGI("AC", "Turning off");
        ac.off();
    }
    else if (state == 1) {
        ESP_LOGI("AC", "Turning on");
        ac.on();
    }
    ac.send();
}
