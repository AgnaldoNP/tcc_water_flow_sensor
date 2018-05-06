/*links reference:
  ESP8266WiFi - http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
  QList - https://github.com/SloCompTech/QList
*/

#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

boolean isWifiConnected = false;

boolean wifiConnected() {
  return isWifiConnected;
}

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

long oldTimeConnectWifi = 0;
String ret = "";
boolean isConnecting = false;
void connectToWiFi() {
  if (isConnecting) {
    if ((millis() - oldTimeCheckSetupMode) > 500 && !inSetupMode) {

      if (WiFi.status() == WL_CONNECTED) {
        isConnecting = false;
        Serial.println("");
        Serial.println("WiFi connected");
        printOnDisplay(2, "WiFi connected");
        isWifiConnected = true;
        clearDisplay();

        synchronizeDateTime();

        return;
      }

      Serial.print(".");

      if (ret.length() > 20) {
        ret = "";
      }
      ret += ".";
      
      printOnDisplay(1, ret);
      oldTimeConnectWifi = millis();
    }else{
      isConnecting = false;
      disconnectWifi();
    }
    return;
  }

  if (!isWifiConnected && !isConnecting && !inSetupMode) {
    //Serial.printf("\nConnecting to WiFi %s - %s ...", const_cast<char*>(configSsid.c_str()), const_cast<char*>(configPassword.c_str()));
    if (isConfigured()) {
      WiFi.begin(const_cast<char*>(configSsid.c_str()), const_cast<char*>(configPassword.c_str()));
      clearDisplay();
      printOnDisplay(0, "Connecting to WiFi");
      isConnecting = true;
    } else {
      //Serial.println("Wrong Wifi Config.");
      printOnDisplay(2, "Wrong Wifi Config.");
      isWifiConnected = false;
    }
  }
}

void disconnectWifi() {
  if (isWifiConnected) {
    //Procurar
    isWifiConnected = false;
    WiFi.disconnect(true);
  }
}

boolean isConfigured() {
  return configSsid && configSsid != ""
         && configPassword && configPassword != ""
         && configIdentifier && configIdentifier != "";;
}

void collectData() {
  if ((millis() - oldTimeCollect) > 30000) {
    oldTimeCollect = millis();

    if (isWifiConnected) {
      if (logsList.size() > 0) {
        String json = "[";
        int i, logSize = logsList.size();
        for (i = 0; i < logSize; i++) {
          LogObject *logObject = logsList.at(i);
          json += "{\"startDate\": \"" + String(logObject->startDate) + "000\", ";
          json += "\"endDate\": \"" + String(logObject->endDate) + "000\", ";
          json += "\"spent\": \"" + String(logObject->spent) + "\", ";
          json += "\"average\": \"" + String(logObject->average) + "\", ";
          json += "\"identifier\": \"" + logObject->identifier + "\"}";

          if (i < logSize - 1) {
            json += ",";
          }
        }

        json += "]";
        Serial.println(json);

        Serial.println("\nCollecting data and WebService Loging ...");
        Serial.println("logJson: " + json);

        String response = "";
        HTTPClient http;
        http.begin("http://tecnolapis.com/projetos/WaterSpentReport/log.php");
        http.addHeader("Content-Type", "application/json");
        int statusCode = http.POST(json);
        response = http.getString();
        http.end();

        Serial.println("StatusCode: " + String(statusCode) + "  Response Request> " + response);
        if (statusCode == STATUS_OK || statusCode == STATUS_CREATED) {
          //remove items from list
          for (i = 0; i < logSize; i++) {
            logsList.clear(i);
          }
        }


      }
    } else {
      Serial.println("No wifi connected!");
    }
  }
}



