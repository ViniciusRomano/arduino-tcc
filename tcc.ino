//Programa : Medidor de corrente com Arduino e SCT-013 100A
//Autor : FILIPEFLOP

//Baseado no programa exemplo da biblioteca EmonLib

//Carrega as bibliotecas
#include "EmonLib.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

EnergyMonitor emon1;



#define VOLTS 127;
#define BURDEN_RESISTOR_5V 25.8;
#define BURDEN_RESISTOR_3V 16.1;

int calibration_radio = 2000 / BURDEN_RESISTOR_5V;
int port = A0;

float getAmp(float value) {
  return value > 0.15 ? value - 0.15 : 0;
}

void setup() {
  Serial.begin(9600);
  //Pino, calibracao - Cur Const= Ratio/BurdenR. 2000/33 = 60
  emon1.current(port, calibration_radio);

  WiFi.begin("", "");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("!!!!!!!!!!!!!!!!!!");

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {


  WiFiClient client;

  HTTPClient http;
  //Calcula a corrente
  double Irms = emon1.calcIrms(1480);
  //Mostra o valor da corrente no serial monitor e display
  Serial.print("Corrente : ");
  Serial.println(getAmp(Irms));  // Irms


  // Wifi

  Serial.println(WiFi.localIP());
  if (WiFi.status() == WL_CONNECTED) {


    http.begin(client, "http://api-arduino.viniciusromano.live/api/sensor-values");  // HTTP
    http.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST("{\"sv_value\":\"" + String(getAmp(Irms))+ " \"}");

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  delay(2000);
}

