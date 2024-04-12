
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>

const char* ssid = "ESP8266-Access-Point";
const char* password = "123456789";

AsyncWebServer server(80);

void setup(){
  Serial.begin(115200);
  Serial.println();
  
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/example", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println(request->header("Test"));
    request->send_P(200, "text/plain", "Output");
  });

  server.begin();
}
 
void loop(){
  Serial.println("Loop");
  delay(100);
}
