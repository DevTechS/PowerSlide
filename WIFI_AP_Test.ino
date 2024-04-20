
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>

const char* ssid = "ESP8266-Access-Point";
const char* password = "123456789";

AsyncWebServer server(80);

uint8_t brightness = 30;

void setup(){
  Serial.begin(115200);
  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/example", HTTP_GET, [](AsyncWebServerRequest *request){
    brightness = (uint8_t)request->header("Control")[0];
    Serial.println(brightness);
    request->send_P(200, "text/plain", "Output_here");
  });

  server.begin();
}

void loop(){
  analogWrite(LED_BUILTIN, brightness);
  Serial.println("Loop");
  delay(20);
}
