/*links reference:
 QList - https://github.com/SloCompTech/QList
 ESP8266WiFi - http://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
 ESP8266WiFiAP - https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/ESP8266WiFiAP.h
 ESP8266WebServer - https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/src/ESP8266WebServer.h
*/

#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

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


