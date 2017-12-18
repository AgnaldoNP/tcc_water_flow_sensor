/* 
*  EEPROM - http://pedrominatel.com.br/pt/esp8266/utilizando-eeprom-do-esp8266/
          https://circuits4you.com/2016/12/16/esp8266-internal-eeprom-arduino/
*/
#include <EEPROM.h>

#define MEM_ALOC_SIZE 512
#define SSID_SIZE_ADDRESS 0
#define PASS_SIZE_ADDRESS 1
#define IDENTIFIER_SIZE_ADDRESS 2

#define START_AT_SSID_ADDRESS 20
#define START_AT_PASS_ADDRESS 70
#define START_AT_IDENTIFIER_ADDRESS 120

void storeEeprom(int startat, String store) {
    Serial.print("\nStoring: ");
    EEPROM.begin(MEM_ALOC_SIZE);
    for(int i=0; i<store.length(); i++) {
        Serial.print(store[i]);
        EEPROM.write(i+startat,(uint8_t)store[i]);
    }
    EEPROM.commit();
    EEPROM.end();
}

void storeEeprom(int address, int store) {
    EEPROM.begin(MEM_ALOC_SIZE);
    EEPROM.write(address,store);
    EEPROM.commit();
    EEPROM.end();
}

String readEeprom(int startAt, int valueSize) {
    Serial.print("\nReading: ");
    EEPROM.begin(MEM_ALOC_SIZE);
    String value="";
    for(int i=0; i<valueSize; i++) {
        char readChar = (char)(EEPROM.read(i+startAt));
        Serial.print(readChar);
        value += readChar;
    }
    EEPROM.end();

    return value;
}

int readEeprom(int address) {
    EEPROM.begin(MEM_ALOC_SIZE);
    int intStored = EEPROM.read(address);
    EEPROM.end();
    return intStored;
}


