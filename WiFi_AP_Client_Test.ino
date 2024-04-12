#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;

const char* ssid = "ESP8266-Access-Point";
const char* password = "123456789";

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

WiFiClient client;
HTTPClient http;

void setup() {
  Serial.begin(115200);
 
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi");
}

void loop() {
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    unsigned long start = micros();
    String temperature = httpGETRequest("http://192.168.4.1/example");
    unsigned long end = micros();
    Serial.println((end-start)/1000.0);
    // Serial.println(temperature);
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}

String httpGETRequest(const char* serverName) {
  http.begin(client, serverName);
  http.addHeader("Test", "253");
  int httpResponseCode = http.GET();
  String payload = "--"; 
  if (httpResponseCode>0) {
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return payload;
}
