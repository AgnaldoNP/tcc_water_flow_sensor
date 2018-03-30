

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
byte sensorPin       = 13; //pin D7

unsigned long pulseCount;  

float flowRate;
float media;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long secondsFlow;
unsigned long oldTime;
/////////////////////////////////////////////////////

void pulseCounter() {
  pulseCount++;
}

void setup() {
    Serial.begin(115200);
    pinMode(BUILTIN_LED, OUTPUT);
    pinMode(setupPin, INPUT);
    pinMode(sensorPin, INPUT);
    attachInterrupt(sensorPin, pulseCounter, RISING);

    pulseCount        = 0;
    flowRate          = 0.0;
    flowMilliLitres   = 0;
    totalMilliLitres  = 0;
    oldTime           = 0;
    secondsFlow       = 0;

    pulseCounter();
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

   logInformations();
}

void logInformations() {
  
  if((millis() - oldTime) > 1000){   // Only process counters once per seconds
    cli();      //Desabilita interrupção
    Serial.println(pulseCount);
     
    flowRate = pulseCount / (float)5.5; //Converte para L/min
    media=media+flowRate; //Soma a vazão para o calculo da media
    totalMilliLitres += flowRate; //Vazão total
    secondsFlow++;
    
    Serial.print(flowRate); //Imprime na serial o valor da vazão
    Serial.print(" L/min - "); //Imprime L/min
    Serial.print(secondsFlow); //Imprime a contagem (segundos)
    Serial.println("s"); //Imprime s indicando que está em segundos
    
    if(secondsFlow%60 ==0)
    {
      media = media/60; //Tira a media dividindo por 60
      Serial.print("\nMedia por minuto = "); //Imprime a frase Media por minuto =
      Serial.print(media); //Imprime o valor da media
      Serial.println(" L/min - "); //Imprime L/min
      media = 0; //Zera a variável media para uma nova contagem
      secondsFlow=0; //Zera a variável i para uma nova contagem
      Serial.println("\n\nInicio\n\n"); //Imprime Inicio indicando que a contagem iniciou
    }

    pulseCount = 0;   //Zera a variável para contar os giros por segundos 
    oldTime = millis();
  }

   sei();      //Habilita interrupção
}


