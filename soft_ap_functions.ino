/*
   ESP8266WiFiAP - https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/ESP8266WiFiAP.h
   ESP8266WebServer - https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/src/ESP8266WebServer.h
*/


IPAddress ap_local_IP(192, 168, 1, 1);
IPAddress ap_gateway(192, 168, 1, 255);
IPAddress ap_subnet(255, 255, 255, 0);

const char *ap_ssid = "ESP8266WIFI_AP";
const char *ap_pass = "12345678";

boolean softApModeStarted = true;
int stationsConnected = 0;

void startSoftAP() {
  if (!softApModeStarted) {
    //Serial.print("\n\nSetting Soft-AP Config ... ");
    clearDisplay();
    printOnDisplay(0, "Starting Soft-AP ...");
    boolean softApConfigResult = WiFi.softAPConfig(ap_local_IP, ap_gateway, ap_subnet);
    //boolean softApConfigResult=  true;
    if (softApConfigResult) {
      //Serial.printf("Soft-AP creating with ssid: %s and pass: %s ... \n", ap_ssid, ap_pass);
      boolean softApResult = WiFi.softAP(ap_ssid, ap_pass);

      if (softApResult) {
        //Serial.println("Soft-AP create Ready");
        printOnDisplay(1, "Soft-AP Started");

        IPAddress myIP = WiFi.softAPIP();
        //Serial.print("AP IP address: ");
        //Serial.println(myIP);

        printOnDisplay(2, "Wi-Fi: " + String(ap_ssid));
        printOnDisplay(3, "Pass: " + String(ap_pass));
        printOnDisplay(4, "IP: " + IPAddressToString(myIP));
        softApModeStarted = true;

        stationsConnected = WiFi.softAPgetStationNum();
        //Serial.printf("Stations connected = %d\n", stationsConnected);

      } else {
        //Serial.println("Failed!");
        printOnDisplay(1, "Soft-AP Error!");
        softApModeStarted = false;
      }
    } else {
      //Serial.print("Setting Soft-AP Config Failed! ");
      printOnDisplay(1, "Soft-AP Error!");
      softApModeStarted = false;
    }
  }


  if (stationsConnected != WiFi.softAPgetStationNum()) {
    stationsConnected = WiFi.softAPgetStationNum();
    //Serial.printf("Stations connected = %d\n", stationsConnected);
  }
}

String IPAddressToString(IPAddress ipAddress) {
  static char szRet[20];
  String str = String(ipAddress[0]);
  str += ".";
  str += String(ipAddress[1]);
  str += ".";
  str += String(ipAddress[2]);
  str += ".";
  str += String(ipAddress[3]);
  return str;
}

void disconnectSofAp() {
  if (softApModeStarted) {
    WiFi.softAPdisconnect(true);
    softApModeStarted = false;
    //Serial.print("\nSoft-AP disconnected\n");
  }
}
