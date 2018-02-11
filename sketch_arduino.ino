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

///////vaiables for water flow sensor/////////////////
byte sensorInterrupt = 0;  // 0 = digital pin D4
byte sensorPin       = D4;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;
/////////////////////////////////////////////////////



void setup() {
    pinMode(setupPin, INPUT);

    Serial.begin(115200);
    Serial.println();

    readWiFiConfigurations();

    pinMode(sensorPin, INPUT);
    digitalWrite(sensorPin, HIGH);
    
    pulseCount        = 0;
    flowRate          = 0.0;
    flowMilliLitres   = 0;
    totalMilliLitres  = 0;
    oldTime           = 0;
    
    // The Hall-effect sensor is connected to pin D4 which uses interrupt 0.
    // Configured to trigger on a FALLING state change (transition from HIGH
    // state to LOW state)
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
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

   logInformations();
}


void logInformations() {
  if((millis() - oldTime) > 1000){   // Only process counters once per second
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
    
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;

    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print(".");             // Print the decimal point
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC) ;      // Print the fractional part of the variable
    Serial.print("L/min");
    // Print the number of litres flowed in this second
    Serial.print("  Current Liquid Flowing: ");             // Output separator
    Serial.print(flowMilliLitres);
    Serial.print("mL/Sec");
    
    // Print the cumulative total of litres flowed since starting
    Serial.print("  Output Liquid Quantity: ");             // Output separator
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
    
    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
}


void pulseCounter(){
  pulseCount++;
}





