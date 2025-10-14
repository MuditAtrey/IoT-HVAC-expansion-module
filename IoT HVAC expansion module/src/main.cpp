// Minimal R4 Minima example: blink onboard LED and drive a 7-pin SPI OLED (SSD1306)
#include <Arduino.h>
#include <U8g2lib.h>
extern const u8g2_cb_t u8g2_cb_r0;

// --- Board: UNO R4 Minima (renesas-ra) ---
#if defined(BOARD_UNOR4)

// LED: use built-in blue LED symbol if provided by the core, otherwise fallback to D13
#if defined(LED_BUILTIN_B)
#define LED_PIN LED_BUILTIN_B
#else
#define LED_PIN 13
#endif

// Wiring / pin mapping for a 7-pin SPI OLED (SSD1306 128x64 assumed)
// OLED pin  ->  UNO R4 pin (digital)
// GND       ->  GND
// VCC       ->  3V3
// D0 (SCK)  ->  D13  (SPI SCK)
// D1 (MOSI) ->  D11  (SPI MOSI)
// RES       ->  D8   (we use digital pin 8 for RESET)
// DC        ->  D9   (Data/Command)
// CS        ->  D10  (Chip Select)

#define OLED_CLK   13
#define OLED_MOSI  11
#define OLED_CS    10
#define OLED_DC    9
#define OLED_RESET 8

// U8g2 software SPI constructor: (rotation, clock, data, cs, dc, reset)
U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(&u8g2_cb_r0, OLED_CLK, OLED_MOSI, OLED_CS, OLED_DC, OLED_RESET);

void setup() {
  // LED
  pinMode(LED_PIN, OUTPUT);

  // OLED reset pin
  pinMode(OLED_RESET, OUTPUT);
  // Hardware reset the display
  digitalWrite(OLED_RESET, LOW);
  delay(10);
  digitalWrite(OLED_RESET, HIGH);
  delay(10);

  // Initialize display
  u8g2.begin();

  // Draw an initial message to confirm operation
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 24, "R4 OLED OK");
  u8g2.sendBuffer();
}

void loop() {
  // Blink LED rapidly
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(100);

  // Optional: refresh a small indicator on the OLED (toggle a pixel)
  static bool dot = false;
  dot = !dot;
  u8g2.setDrawColor(1);
  u8g2.drawBox(110, 0, 16, 8);
  if (!dot) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 24, "R4 OLED OK");
  }
  u8g2.sendBuffer();
  delay(400);
}



#else

#if defined(NODEMCU1)
// NodeMCU 1: WiFi + HTTP GET to localhost server
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "muditatrey1234";
const char* password = "muditmudit";
const char* server_host = "192.168.29.12"; // Change to your computer's IP
const int server_port = 8000;            // Change to your server's port

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
  HTTPClient http;
  WiFiClient client;
  String url = String("http://") + server_host + ":" + server_port + "/nodemcu1";
  http.begin(client, url);
  int httpCode = http.GET();
    Serial.printf("HTTP GET to %s, code: %d\n", url.c_str(), httpCode);
    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
  delay(5000);
}

#elif defined(NODEMCU2)
// NodeMCU 2: WiFi + HTTP GET to localhost server
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "muditatrey1234";
const char* password = "muditmudit";
const char* server_host = "192.168.29.12"; // Change to your computer's IP
const int server_port = 8000;            // Change to your server's port

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
  HTTPClient http;
  WiFiClient client;
  String url = String("http://") + server_host + ":" + server_port + "/nodemcu2";
  http.begin(client, url);
  int httpCode = http.GET();
    Serial.printf("HTTP GET to %s, code: %d\n", url.c_str(), httpCode);
    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
  delay(5000);
}

#else
// Fallback for other boards
void setup() {
  Serial.begin(115200);
  Serial.println("Generic board setup");
}
void loop() {
  Serial.println("Generic board loop");
  delay(1000);
}
#endif

#endif