#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "Adafruit_TCS34725.h"

Adafruit_TCS34725 tcs = Adafruit_TCS34725();

#define tcs_addr 0x29

  uint16_t r, g, b, c;
bool tcs_began = false;

void tcs_loop() {

  // https://cdn-shop.adafruit.com/datasheets/TCS34725.pdf
  tcs.setGain(TCS34725_GAIN_16X);
  tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_300MS);
  delay(300); 
  tcs.getRawData(&r, &g, &b, &c);
  
  // Create JSON object
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["R"] = r;
  jsonDoc["G"] = g;
  jsonDoc["B"] = b;
  jsonDoc["C"] = c;


  String jsonData;
  serializeJson(jsonDoc, jsonData);


  HTTPClient http;
  String serverUrl = "http://192.168.51.5:8000";

  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonData);

  if (httpResponseCode > 0) {
    String response = http.getString();
  } 

  http.end(); 

  Serial.print("[TCS]"); 
  // Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");
}