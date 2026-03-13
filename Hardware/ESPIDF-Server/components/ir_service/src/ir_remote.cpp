#include <IRremoteESP8266.h>
#include <ir_Daikin.h>
#include <string>

const uint16_t kIrLed = 11;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRDaikinESP ac(kIrLed);



void startAcConnection() {
    ac.begin();
    Serial.begin(115200);
    Serial.println("Ac Connected");
}
void sendTempSignal(int temp) {

}
void sendTurnSignal(const std::string& input, const float temp ) {
    ac.setFan(2);
    if (temp >= 25) {
        ac.setMode(kDaikinHeat);
    }
    else {
        ac.setMode(kDaikinCool);
    }
    ac.setTemp(temp);
    ac.setSwingVertical(false);
    ac.setSwingHorizontal(false);
    if (input == "off") {
        ac.off();
        ac.send();
    }
    else if (input == "on") {
        ac.on();
        ac.send();
    }
    else {
        Serial.println("Invalid Input");
    }
}

