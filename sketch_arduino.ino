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
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include "RestClient.h"
#include <QList.h>
#include "QList.cpp"
#include <ArduinoJson.h>
#include <EEPROM.h>

// EEPROM Memory Size and Location
#define MEM_ALOC_SIZE 512

#define SSID_SIZE_ADDRESS 0
#define PASS_SIZE_ADDRESS 1
#define IDENTIFIER_SIZE_ADDRESS 2

#define START_AT_SSID_ADDRESS 20
#define START_AT_PASS_ADDRESS 70
#define START_AT_IDENTIFIER_ADDRESS 120


// Class that represents the log object
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

IPAddress ap_local_IP(192,168,1,1);
IPAddress ap_gateway(192,168,1,255);
IPAddress ap_subnet(255,255,255,0);

const char *ap_ssid = "ESP8266WIFI_SoftAP";
const char *ap_pass = "12345678";

String configSsid;
String configPassword;
String configIdentifier;

const int setupPin = D0;
boolean softApModeStarted = false;
boolean isWifiConnected = false;

const int STATUS_OK = 200;
const int STATUS_CREATED = 201;
long lastDateSyncronized = 0;
long millisOnDateSyncronized = 0;

int stationsConnected = 0;

ESP8266WebServer server(80);

void setup() {
    pinMode(setupPin, INPUT);

    Serial.begin(115200);
    Serial.println();

    readConfigurations();
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
        if(isWifiConnected) {
            collectData();
        }
    }

    delay(1000);
}

void readConfigurations() {
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

void logInformations() {

}

///////// Function to Send data to WebService Rest to log the informations///////////
void collectData() {
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
}
///// End of Function to Send data to WebService Rest to log the informations////////

/////////////////////// Functions to Connect to Wifi configured////////////////////////
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
/////////////////////// End of Functions to Connect to Wifi configured/////////////////


////////////////////// Functions to Handle DateTime////////////////////////////////////
void synchronizeDateTime() {
    Serial.println("\nDate Syncronizing ... ");
    while(lastDateSyncronized ==0) {
        RestClient client = RestClient("tecnolapis.com", 443, 1);

        String response = "";
        int statusCode = client.get("/projetos/WaterSpentReport/time.php", &response);
        Serial.printf("Request Time Status: %d, \nRequest Time Response: %s\n", statusCode, &response);
        if(statusCode == STATUS_OK) {
            lastDateSyncronized = response.toInt();
            millisOnDateSyncronized = millis();
            Serial.printf("Date Sync: %d\n", getDateMillis());
            break;
        }

        if(!digitalRead(setupPin)){
            break;
        }
    }
}

long getDateMillis() {
    long millisNow = millis();
    long millisDiff = millisNow - millisNow;
    long dateMilis = lastDateSyncronized + millisDiff;
    return dateMilis;
}
////////////////////// End of Functions to Handle DateTime//////////////////////////////

//////////////// Functions to Create and Disconnect to Soft AP /////////////////////////
void startSoftAP() {
    if(!softApModeStarted) {
        Serial.print("\n\nSetting Soft-AP Config ... ");
        //boolean softApConfigResult = WiFi.softAPConfig(ap_local_IP, ap_gateway, ap_subnet);
        boolean softApConfigResult=  true;
        if(softApConfigResult) {
            Serial.printf("Soft-AP creating with ssid: %s and pass: %s ... \n", ap_ssid, ap_pass);
            boolean softApResult = WiFi.softAP(ap_ssid, ap_pass);

            if(softApResult) {
                Serial.println("Soft-AP create Ready");
                IPAddress myIP = WiFi.softAPIP();
                Serial.print("AP IP address: ");
                Serial.println(myIP);
                softApModeStarted = true;

                stationsConnected = WiFi.softAPgetStationNum();
                Serial.printf("Stations connected = %d\n", stationsConnected);

            } else {
                Serial.println("Failed!");
                softApModeStarted = false;
            }
        } else {
            Serial.print("Setting Soft-AP Config Failed! ");
            softApModeStarted = false;
        }
    }


    if(stationsConnected != WiFi.softAPgetStationNum()) {
        stationsConnected = WiFi.softAPgetStationNum();
        Serial.printf("Stations connected = %d\n", stationsConnected);
    }
}

void disconnectSofAp() {
    if(softApModeStarted) {
        WiFi.softAPdisconnect(true);
        softApModeStarted = false;
        Serial.print("\nSoft-AP disconnected\n");
    }
}
///////// End of Functions to Create and Disconnect to Soft AP /////////////////////////


///////////////////////////// Webserver AP /////////////////////////////////////////////
boolean serverConfigStarted =  false;
void startConfigServer() {
    if(softApModeStarted && !serverConfigStarted) {

        server.on("/", handleRoot);
        server.on("/config", handleConfigRequest);
        server.on("/config/set", handleConfigSet);

        server.onNotFound(handleNotFound);
        server.begin();

        serverConfigStarted = true;
        Serial.println("HTTP server started");
    }

    if(softApModeStarted && serverConfigStarted) {
        server.handleClient();
    }
}

void handleRoot() {
    Serial.println("handleRoot");
    server.send(200, "text/html", "<h1>Config Page</h1>");
}

void handleConfigRequest() {
    Serial.println("handleConfigRequest");
    server.send(200, "text/html", getConfigPage());
}

void handleConfigSet() {
    Serial.println("\nhandleConfigSet");

    configSsid = server.arg("ssid");
    configPassword =server.arg("pass");
    configIdentifier = server.arg("identifier");

    Serial.print("Config Reading: \n");
    Serial.printf("SSID: %s\n", const_cast<char*>(configSsid.c_str()));
    Serial.printf("PASS: %s\n", const_cast<char*>(configPassword.c_str()));
    Serial.printf("IDENTIFIER: %s\n\n", const_cast<char*>(configIdentifier.c_str()));

    Serial.print("Storing Configs:");
    storeEeprom (START_AT_SSID_ADDRESS, configSsid);
    storeEeprom (SSID_SIZE_ADDRESS, configSsid.length());

    storeEeprom (START_AT_PASS_ADDRESS, configPassword);
    storeEeprom (PASS_SIZE_ADDRESS, configPassword.length());

    storeEeprom (START_AT_IDENTIFIER_ADDRESS, configIdentifier);
    storeEeprom (IDENTIFIER_SIZE_ADDRESS, configIdentifier.length());

    server.send(200, "text/html", "<h1>ConfigSet Success</h1>");
    Serial.print("\n\n");
}

void handleNotFound() {
    Serial.println("handleNotFound");
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i=0; i<server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}
///////////////////////////// End of Webserver AP /////////////////////////////////////


///////////////////////////// Functions to Manipulate EEPROM///////////////////////////
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

///////////////////////////// END of Functions to Manipulate EEPROM///////////////////


