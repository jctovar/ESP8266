/**
   Datalogger para el ESP8266 NodeMCU V3 para usar el DHT22 y subir los datos de temperatura y humedad por POST a un servidor HTTPS

    Created on: 03.08.2020
    jctovar@iztacala.unam.mx
*/
// load DHT library
#include "DHTesp.h"
// load more library
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

DHTesp dht;
ESP8266WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  dht.setup(14, DHTesp::DHT22); // Connect DHT sensor to GPIO 14

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  // Cambiar datos para AP, SSID y contraseña
  WiFiMulti.addAP("INTRANET", "Passw0rd");
}

void loop() {
  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    // client->setFingerprint(fingerprint);
    client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    // Hacer conexion https y pasar cadena POST
    if (https.begin(*client, "https://132.247.66.122/data/datalogger.php?temp=" + String(temperature) + "&humidity=" + String(humidity) + "&device=F3667BAA")) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  Serial.println("Wait 60s before next round...");
  delay(60000);
}
