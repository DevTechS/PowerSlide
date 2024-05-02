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

String controls = "------------";

void reconnect() {
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

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 12; i++) {
    controls[i] = (char)128;
  }

  reconnect();
}

String SendUpdate() {
  http.begin(client, "http://192.168.4.1/update");

  http.addHeader("Control", controls);
  for (int i = 0; i < 12; i++) {
    Serial.print((int)controls[i]);
    Serial.print(" ");
  }
  Serial.println();
  // Serial.println(controls);

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

void loop() {
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    String Return_str = SendUpdate();
  }
  else {
    Serial.println("WiFi Disconnected");
    reconnect();
  }
  while (Serial.available() >= 3) {
    int start_byte = Serial.read();
    if (start_byte == 0) {
      int field = Serial.read();
      uint8_t value = Serial.read();

      controls[field-1] = value;
      // Serial.print("got ");
      // Serial.println(field);
    } else {
      // Serial.println(start_byte);
    }
  }
  // delay(10);
}
