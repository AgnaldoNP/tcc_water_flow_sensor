/*links reference:
 RestClient - https://github.com/DaKaZ/esp8266-restclient
*/
#include "RestClient.h"


long lastDateSyncronized = 0;
long millisOnDateSyncronized = 0;

void synchronizeDateTime() {
    Serial.println("\nDate Syncronizing ... ");
    
    while(lastDateSyncronized == 0) {
        String response = "";
        RestClient client = RestClient("www.tecnolapis.com", 80);
        int statusCode = client.get("/projetos/WaterSpentReport/time.php", &response);
        Serial.printf("Request Time Status: %d\n", statusCode);
        Serial.printf("Request Response: ");
        Serial.println(response);
        if(statusCode == STATUS_OK) {

            Serial.print("Request Time Response:");
            response.replace("\n", "");
            response.replace("d", "");
            response = response.substring(0, response.length()-3);
            Serial.println(response);

            response = response.substring(0, response.length()-4);
            Serial.println(response);
            lastDateSyncronized = atol(const_cast<char*>(response.c_str()));
            millisOnDateSyncronized = millis()/1000;

            Serial.print("Time converted: ");
            Serial.println(lastDateSyncronized);
            Serial.printf("Date Sync: ");
            Serial.println(getDateMillis());
            break;
        }

        if(!digitalRead(setupPin)){
           // break;
        }
    }
}

long getDateMillis() {
    long millisNow = millis()/1000;
    long millisDiff = millisNow - millisOnDateSyncronized;
    long dateMilis = lastDateSyncronized + millisDiff;
    return dateMilis;
}
