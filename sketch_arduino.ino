
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN  //4 
Adafruit_SSD1306 display(OLED_RESET);

#include <ESP8266WiFi.h>
#include <QList.h>
#include "QList.cpp"

#define HTTP_DEBUG

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
QList<LogObject*> logsList = QList<LogObject*>();

const int setupPin = D3;

///////vaiables for water flow sensor/////////////////
byte sensorPin       = 13; //pin D7

unsigned long pulseCount;

float flowRate;
float media;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long secondsFlow;
unsigned long oldTime;
unsigned long oldTimeCollect;
/////////////////////////////////////////////////////

byte oldSwitchState = HIGH;
bool inSetupMode =  false;
float oldTimeCheckSetupMode;

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void pulseCounter() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);

  configDisplay();
  clearDisplay();
  printOnDisplay(0, "Setup ...");

  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(setupPin, INPUT_PULLUP);
  pinMode(sensorPin, INPUT);
  attachInterrupt(sensorPin, pulseCounter, RISING);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;
  secondsFlow       = 0;
  oldTimeCheckSetupMode = 0;
  oldTimeCollect = millis();

  pulseCounter();

  printOnDisplay(1, "Reading WiFi Config ...");
  readWiFiConfigurations();
  clearDisplay();
}

bool checkSetupMode() {
  byte switchState = digitalRead (setupPin);
  if ((millis() - oldTimeCheckSetupMode) > 50 && switchState != oldSwitchState) {
    oldSwitchState =  switchState;
    if (switchState == HIGH) {
      inSetupMode = !inSetupMode;
      oldTimeCheckSetupMode = millis();
      Serial.println(inSetupMode ? "inSetupMode: true" : "inSetupMode: false");
    }
  }
}

void loop() {
  checkSetupMode();

  //Modo de configuração
  if (inSetupMode) {
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

  if ((millis() - oldTime) > 1000) { // Only process counters once per seconds
    cli();      //Desabilita interrupção
    Serial.println(pulseCount);

    flowRate = pulseCount / (float)7.5;//5.5; //Converte para L/min
    media = media + flowRate; //Soma a vazão para o calculo da media
    totalMilliLitres += flowRate; //Vazão total
    secondsFlow++;

    Serial.print(flowRate); //Imprime na serial o valor da vazão
    Serial.print(" L/min - "); //Imprime L/min
    Serial.print(secondsFlow); //Imprime a contagem (segundos)
    Serial.println("s"); //Imprime s indicando que está em segundos
    
    if (wifiConnected()) {
      clearDisplay();
      printOnDisplay(0, String(flowRate) + " L/Min");
      printOnDisplay(1, String(flowRate / 60) + " L/Sec");
      printOnDisplay(2, String(totalMilliLitres) + " Spent (ml)");
    }

    if (flowRate / 60 >= 0.01) {
      long now = getDateMillis();
      LogObject *logObject = new LogObject(configIdentifier, now - 1, now, flowRate / 60, flowRate);
      logsList.push_front(logObject);
    }

    if (secondsFlow % 60 == 0)
    {
      media = totalMilliLitres / 60; //Tira a media dividindo por 60
      Serial.print("\nMedia por minuto = "); //Imprime a frase Media por minuto =
      Serial.print(media); //Imprime o valor da media
      Serial.println(" L/min - "); //Imprime L/min
      media = 0; //Zera a variável media para uma nova contagem
      secondsFlow = 0; //Zera a variável i para uma nova contagem
      Serial.println("\n\nInicio\n\n"); //Imprime Inicio indicando que a contagem iniciou
    }

    pulseCount = 0;   //Zera a variável para contar os giros por segundos
    oldTime = millis();
  }

  sei();      //Habilita interrupção
}


