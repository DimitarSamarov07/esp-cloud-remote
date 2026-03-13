
#include <ir_Mitsubishi.h>
#include <string_view>

constexpr uint16_t kIrLed = 11;
IRMitsubishiAC ac(kIrLed);

void startAcConnection() {
    ac.begin();
    Serial.begin(115200);
    Serial.println("Ac Connected");
}

static void applyMode(const std::string_view mode) {
    if (mode == "cool") ac.setMode(kMitsubishiAcCool);
    else if (mode == "heat") ac.setMode(kMitsubishiAcHeat);
    else if (mode == "dry") ac.setMode(kMitsubishiAcDry);
    else if (mode == "auto") ac.setMode(kMitsubishiAcAuto);
    else
        Serial.printf("Invalid mode: %s\n", mode.data());
}

static void applyFanSpeed(const std::string_view fanSpeed) {
    if (fanSpeed == "auto") {
        ac.setFan(kMitsubishiAcFanAuto);
    } else if (fanSpeed == "quiet") {
        ac.setFan(1);
        ac.setMode(kMitsubishiAcFanQuiet);
    } else if (fanSpeed == "low") {
        ac.setFan(2);
    } else if (fanSpeed == "medium") {
        ac.setFan(3);
    } else if (fanSpeed == "high") {
        ac.setFan(4);
    } else if (fanSpeed == "turbo") {
        ac.setFan(5);
        ac.setMode(kMitsubishiAcFanMax);
    } else {
        Serial.printf("Invalid fan speed: %s\n", fanSpeed.data());
    }
}

static void applySwing(const bool swing) {
    ac.setVane(kMitsubishiAcVaneAuto);
}

void sendTurnSignalMitsubishi(const bool state, const float temp, const std::string_view mode, const std::string_view fanSpeed, const bool swing) {
    applyMode(mode);
    applyFanSpeed(fanSpeed);
    applySwing(swing);
    ac.setTemp(temp);
    state ? ac.on() : ac.off();
    ac.send();
}
