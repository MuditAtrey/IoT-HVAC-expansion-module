/* Minimal NodeMCU (ESP8266) example that posts JSON to the Flask server.
 * Update WIFI_SSID, WIFI_PASS and SERVER_IP before uploading.
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASS = "your-pass";
const char* SERVER_IP = "192.168.1.100"; // change to your PC's IP

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("\nConnected");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    return;
  }

  HTTPClient http;
  String url = String("http://") + SERVER_IP + ":5000/data";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["temp"] = 22.5 + random(-50,50)/100.0;
  doc["hum"] = 42;
  String body;
  serializeJson(doc, body);

  int code = http.POST(body);
  Serial.printf("POST %s -> %d\n", url.c_str(), code);
  http.end();

  delay(2000);
}
