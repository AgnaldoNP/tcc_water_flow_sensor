/*links reference:
 RestClient - https://github.com/DaKaZ/esp8266-restclient
*/

#include "RestClient.h"

long lastDateSyncronized = 0;
long millisOnDateSyncronized = 0;

void synchronizeDateTime() {
    Serial.println("\nDate Syncronizing ... ");
    while(lastDateSyncronized == 0) {
        RestClient client = RestClient("www.tecnolapis.com", 443, 1);

        String response = "";
        int statusCode = client.get("/projetos/WaterSpentReport/time.php", &response);
       Serial.printf("Request Time Status: %d\n", statusCode);
        if(statusCode == STATUS_OK) {

            Serial.print("Request Time Response:");
            response.replace("\n", "");
            response.replace("d", "");
            response = response.substring(0, response.length()-2);
            Serial.println(response);
          
            lastDateSyncronized = atol(const_cast<char*>(response.c_str()));
            millisOnDateSyncronized = millis();
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
    long millisNow = millis();
    long millisDiff = millisNow - millisNow;
    long dateMilis = lastDateSyncronized + millisDiff;
    return dateMilis;
}
