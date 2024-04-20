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
  reconnect();
}

void loop() {
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    unsigned long start = micros();
    String Return_str = httpGETRequest("http://192.168.4.1/example");
    unsigned long end = micros();
    Serial.println((end-start)/1000.0);
    // Serial.println(Return_str);
  }
  else {
    Serial.println("WiFi Disconnected");
    reconnect();
  }
}

const float period = 2000.0; // in milliseconds (2 seconds)
const float frequency = 1.0 / period; // in Hz

String httpGETRequest(const char* serverName) {
  http.begin(client, serverName);

  String Header = "---";
  float brightness = (sin(millis() * frequency * 2 * PI) + 1) * 127.5;
  Header[0] = (uint8_t)brightness;

  http.addHeader("Control", Header);

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
