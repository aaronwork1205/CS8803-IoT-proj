#include <WiFi.h>          
#include "helpers.h"
#include "veml.h"
#include "tcs.h"

int ledMode = 0;
unsigned long discoverLoopLast = 0;

// Wi-Fi credentials
const char* ssid = "Leishen-yxxF";
const char* password = "zhunikaixin";

void setup() {
  Wire.begin();
  
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  connectToWiFi();

  discoverLoopLast = millis();
}

void connectToWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  
  int retries = 0;
  const int maxRetries = 20;

  while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi.");

  }
}

// Local data loop
void ctlLoop() {
  if(millis() - discoverLoopLast < 1000) return;
  discoverLoopLast = millis();
  Serial.println("Ctrl loop");

  if (i2c_exists(veml_addr)) {
    if (veml_began) {
      veml_loop();
    } else {
      veml_began = veml.begin();
      if (!veml_began) {
        Serial.println("start VEML failed!");
      }
    }
  } else {
    Serial.println("no VEML");
  }

  if (i2c_exists(tcs_addr)) {
    if (tcs_began) {
      tcs_loop();
    } else {
      tcs_began = tcs.begin();
      if (!tcs_began) {
        Serial.println("start TCS failed!");
      }
    }
  } else {
    Serial.println("no TCS");
  }
}

// Breathing light to check if chip is still alive
void ledLoop() {
  unsigned long c1 = millis() % (255 * 2);
  unsigned long c2 = millis() % (255);
  analogWrite(LED_BUILTIN, c1 != c2 ? (255 * 2) - c1 : c2);
}

void loop() {
  ctlLoop(); 
  ledLoop();
}