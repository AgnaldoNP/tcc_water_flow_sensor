/*links reference:
 QList - https://github.com/SloCompTech/QList
 ESP8266WiFi - http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
 ESP8266WiFiAP - https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/ESP8266WiFiAP.h
 ESP8266WebServer - https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/src/ESP8266WebServer.h
 RestClient - https://github.com/DaKaZ/esp8266-restclient
 ArduinoJson - https://bblanchon.github.io/ArduinoJson/doc/encoding/
 EEPROM - http://pedrominatel.com.br/pt/esp8266/utilizando-eeprom-do-esp8266/
          https://circuits4you.com/2016/12/16/esp8266-internal-eeprom-arduino/
*/

#include <ESP8266WiFi.h>
#include <QList.h>
#include "QList.cpp"

//global shared variables
const int STATUS_OK = 200;
const int STATUS_CREATED = 201;

String configSsid;
String configPassword;
String configIdentifier;
//end of global shared variables

class LogObject {
public:
    long startDate;
    long endDate;
    float spent;
    float average;
    String identifier;

    LogObject(String id, long sd, long ed, float s, float a) {
        identifier = id;
        startDate = sd;
        endDate = ed;
        spent = s;
        average = a;
    }
};
QList<LogObject> logsList;

const int setupPin = D0;

void setup() {
    pinMode(setupPin, INPUT);

    Serial.begin(115200);
    Serial.println();

    readWiFiConfigurations();
}

void loop() {

    //Modo de configuração
    if(digitalRead(setupPin)) {
        disconnectWifi();
        startSoftAP();
        startConfigServer();

    } else { //Modo de operação
        disconnectSofAp();
        connectToWiFi();
        collectData();
    }

    delay(1000);
}

void logInformations() {

}





