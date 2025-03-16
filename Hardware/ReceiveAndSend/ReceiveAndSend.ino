#include <Arduino.h>
#include "PinDefinitionsAndMore.h" 


#if !defined(RAW_BUFFER_LENGTH)
// For air condition remotes it requires 750. Default is 200.
#  if !((defined(RAMEND) && RAMEND <= 0x4FF) || (defined(RAMSIZE) && RAMSIZE < 0x4FF))
#define RAW_BUFFER_LENGTH  750
#  endif
#endif
#define MARK_EXCESS_MICROS    20    

//#define DEBUG // Activate this for lots of lovely debug output from the decoders.

#include <IRremote.hpp>

int SEND_BUTTON_PIN = 12;

int DELAY_BETWEEN_REPEAT = 50;

struct storedIRDataStruct {
    IRData receivedIRData;
    uint8_t rawCode[RAW_BUFFER_LENGTH]; 
    uint8_t rawCodeLength; 
} sStoredIRData;

bool sSendButtonWasActive;

void storeCode();
void sendCode(storedIRDataStruct *aIRDataToSend);

void setup() {
    pinMode(12, INPUT_PULLUP);

    Serial.begin(115200);
    while (!Serial)
        ; 
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) || defined(USBCON)  \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); 
#endif
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

    
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
    Serial.print(F("Ready to receive IR signals of protocols: "));
    printActiveIRProtocols(&Serial);
    Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));

    IrSender.begin(); // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin
    Serial.print(F("Ready to send IR signals at pin " STR(IR_SEND_PIN) " on press of button at pin "));
    Serial.println(12);
}

void loop() {

    // If button pressed, send the code.
    bool tSendButtonIsActive = (digitalRead(12) == LOW); // Button pin is active LOW

  
    if (tSendButtonIsActive) {
        if (!sSendButtonWasActive) {
            Serial.println(F("Stop receiving"));
            IrReceiver.stop();
        }
  
        Serial.print(F("Button pressed, now sending "));
        if (sSendButtonWasActive == tSendButtonIsActive) {
            Serial.print(F("repeat "));
            sStoredIRData.receivedIRData.flags = IRDATA_FLAGS_IS_REPEAT;
        } else {
            sStoredIRData.receivedIRData.flags = IRDATA_FLAGS_EMPTY;
        }
        Serial.flush(); 
        sendCode(&sStoredIRData);
        delay(DELAY_BETWEEN_REPEAT); 

    } else if (sSendButtonWasActive) {
      
        Serial.println(F("Button released -> start receiving"));
        IrReceiver.start();

    } else if (IrReceiver.decode()) {
        storeCode();
        IrReceiver.resume(); 
    }

    sSendButtonWasActive = tSendButtonIsActive;
    delay(100);
}

void storeCode() {
    if (IrReceiver.decodedIRData.rawDataPtr->rawlen < 4) {
        Serial.print(F("Ignore data with rawlen="));
        Serial.println(IrReceiver.decodedIRData.rawDataPtr->rawlen);
        return;
    }
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
        Serial.println(F("Ignore repeat"));
        return;
    }
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_AUTO_REPEAT) {
        Serial.println(F("Ignore autorepeat"));
        return;
    }
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_PARITY_FAILED) {
        Serial.println(F("Ignore parity error"));
        return;
    }
    sStoredIRData.receivedIRData = IrReceiver.decodedIRData;

    if (sStoredIRData.receivedIRData.protocol == UNKNOWN) {
        Serial.print(F("Received unknown code and store "));
        Serial.print(IrReceiver.decodedIRData.rawDataPtr->rawlen - 1);
        Serial.println(F(" timing entries as raw "));
        IrReceiver.printIRResultRawFormatted(&Serial, true); // Output the results in RAW format
        sStoredIRData.rawCodeLength = IrReceiver.decodedIRData.rawDataPtr->rawlen - 1;
        IrReceiver.compensateAndStoreIRResultInArray(sStoredIRData.rawCode);
    } else {
        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.printIRSendUsage(&Serial);
        sStoredIRData.receivedIRData.flags = 0; 
        Serial.println();
    }
}

void sendCode(storedIRDataStruct *aIRDataToSend) {
    if (aIRDataToSend->receivedIRData.protocol == UNKNOWN /* i.e. raw */) {
        // Assume 38 KHz
        IrSender.sendRaw(aIRDataToSend->rawCode, aIRDataToSend->rawCodeLength, 38);

        Serial.print(F("raw "));
        Serial.print(aIRDataToSend->rawCodeLength);
        Serial.println(F(" marks or spaces"));
    } else {
        IrSender.write(&aIRDataToSend->receivedIRData);
        printIRResultShort(&Serial, &aIRDataToSend->receivedIRData, false);
    }
}

