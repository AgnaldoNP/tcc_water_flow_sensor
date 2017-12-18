/*links reference:
 ESP8266WiFi - http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
 QList - https://github.com/SloCompTech/QList
 ArduinoJson - https://bblanchon.github.io/ArduinoJson/doc/encoding/
*/

#include <ArduinoJson.h>
#include <WiFiClient.h>
#include "RestClient.h"

boolean isWifiConnected = false;

void readWiFiConfigurations() {
    Serial.print("\nRead Configurations ...");
    int ssidSize = readEeprom(SSID_SIZE_ADDRESS);
    configSsid = readEeprom(START_AT_SSID_ADDRESS, ssidSize);
    Serial.printf("\nSSID: %s", const_cast<char*>(configSsid.c_str()));

    int passSize = readEeprom(PASS_SIZE_ADDRESS);
    configPassword = readEeprom(START_AT_PASS_ADDRESS, passSize);
    Serial.printf("\nSSID_PASS: %s", const_cast<char*>(configPassword.c_str()));

    int identifierSize = readEeprom(IDENTIFIER_SIZE_ADDRESS);
    configIdentifier = readEeprom(START_AT_IDENTIFIER_ADDRESS);
    Serial.printf("\nIDENTIFIER: %s", const_cast<char*>(configIdentifier.c_str()));

    Serial.print("\n\n");
}

void connectToWiFi() {
    if(!isWifiConnected) {
        Serial.printf("\nConnecting to WiFi %s - %s ...", const_cast<char*>(configSsid.c_str()), const_cast<char*>(configPassword.c_str()));
        if(isConfigured()) {
            WiFi.begin(const_cast<char*>(configSsid.c_str()), const_cast<char*>(configPassword.c_str()));
            while (WiFi.status() != WL_CONNECTED && !digitalRead(setupPin)) {
                delay(500);
                Serial.print(".");
            }
            if(WiFi.status() == WL_CONNECTED) {
                Serial.println("");
                Serial.println("WiFi connected");
                isWifiConnected = true;

                synchronizeDateTime();
            }
        } else {
            Serial.println("Wrong Wifi Config.");
            isWifiConnected = false;
        }
    }
}

void disconnectWifi() {
    if(isWifiConnected) {
        //Procurar
        isWifiConnected = false;
    }
}

boolean isConfigured() {
    return configSsid && configSsid != ""
           && configPassword && configPassword != ""
           && configIdentifier && configIdentifier != "";;
}

void collectData() {
  if(isWifiConnected) {
    Serial.println("\nCollecting data and WebService Loging ...");
    if(logsList.size() >0) {
        DynamicJsonBuffer jsonBuffer;
        JsonArray& rootArray = jsonBuffer.createArray();
        int i, logSize = logsList.size();
        for(i=0; i<logSize; i++) {
            LogObject logObject = logsList.at(i);

            JsonObject& logObjectJson = jsonBuffer.createObject();
            logObjectJson["startDate"] = logObject.startDate;
            logObjectJson["endDate"] = logObject.endDate;
            logObjectJson["spent"] = logObject.spent;
            logObjectJson["average"] = logObject.average;
            logObjectJson["identifier"] = logObject.identifier;

            rootArray.add(logObjectJson);
        }

        String json = "";
        rootArray.printTo(json);
        Serial.println("logJson: "+json);

        delete(&jsonBuffer);

        String response = "";
        char jsonBody[json.length() + 1];
        json.toCharArray(jsonBody, json.length());
        RestClient client = RestClient("tecnolapis.com", 443, 1);
        int statusCode = client.post("/projetos/WaterSpentReport/log.php", jsonBody, &response);
        Serial.printf("Request CollectData Status: %d, \nRequest CollectData Response: %s\n", statusCode, &response);
        if(statusCode == STATUS_OK || statusCode == STATUS_CREATED) {
            //remove items from list
            for(i=0; i<logSize; i++) {
                logsList.clear(i);
            }
        }

    } else {
        Serial.println("No log to send to WebService!");
    }
  } else {
    Serial.println("No wifi connected!");
  }
}



