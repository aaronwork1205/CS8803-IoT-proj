#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Adafruit_VEML7700.h>

Adafruit_VEML7700 veml = Adafruit_VEML7700();
float lux = 0;
#define veml_addr 0x10
bool veml_began = false;
void veml_loop() {
  
  if (!veml_began) {
    if (veml.begin()) {
      veml_began = true;
    } else {
      return;
    }
  }

  veml.enable(true);
  veml.interruptEnable(false);
  veml.powerSaveEnable(false);
  

  lux = veml.readLux(VEML_LUX_CORRECTED_NOWAIT);

  StaticJsonDocument<100> jsonDoc;
  jsonDoc["lux"] = lux;

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
}