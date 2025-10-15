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
// NodeMCU 1: WiFi + rotary encoder input -> HTTP POST to server (INDOOR)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "muditatrey1234";
const char* password = "muditmudit";
const char* server_host = "10.54.2.33"; // Change to your computer's IP
const int server_port = 8000;            // Change to your server's port

// Rotary encoder pin mapping (5-pin module: VCC,GND,CLK,DT,SW)
// Default pins (change if you wire differently)
#define ENC_CLK_PIN 14 // D5
#define ENC_DT_PIN  12 // D6
#define ENC_SW_PIN  13 // D7

// Temperature limits and step
const float TEMP_MIN = -10.0;
const float TEMP_MAX = 40.0;
const float TEMP_STEP = 0.5;

// Humidity limits and step
const float HUM_MIN = 0.0;
const float HUM_MAX = 100.0;
const float HUM_STEP = 0.5;

volatile long encoderCount = 0;
volatile bool encoderChanged = false;
float currentTemp = 20.0; // Start at 20°C
float currentHum = 50.0;  // Start at 50%
bool editingTemp = true;  // true = editing temperature, false = editing humidity

// for switch debounce
unsigned long lastSwitchTime = 0;
const unsigned long switchDebounceMs = 300;

// previous state for gray code method
volatile int lastEncoded = 0;

ICACHE_RAM_ATTR void handleEncoder() {
  int MSB = digitalRead(ENC_CLK_PIN); // most significant bit
  int LSB = digitalRead(ENC_DT_PIN);  // least significant bit
  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderCount++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderCount--;
  lastEncoded = encoded;
  encoderChanged = true;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("\nWiFi connected!");

  // encoder pins
  pinMode(ENC_CLK_PIN, INPUT_PULLUP);
  pinMode(ENC_DT_PIN, INPUT_PULLUP);
  pinMode(ENC_SW_PIN, INPUT_PULLUP);

  // read initial
  int MSB = digitalRead(ENC_CLK_PIN);
  int LSB = digitalRead(ENC_DT_PIN);
  lastEncoded = (MSB << 1) | LSB;

  // attach interrupts
  attachInterrupt(digitalPinToInterrupt(ENC_CLK_PIN), handleEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_DT_PIN), handleEncoder, CHANGE);
}

void sendEncoderState() {
  float newTemp = currentTemp;
  float newHum = currentHum;
  
  // Update the value based on current mode
  if (editingTemp) {
    newTemp = currentTemp + (encoderCount * TEMP_STEP);
    newTemp = max(TEMP_MIN, min(TEMP_MAX, newTemp));
  } else {
    newHum = currentHum + (encoderCount * HUM_STEP);
    newHum = max(HUM_MIN, min(HUM_MAX, newHum));
  }
  
  HTTPClient http;
  WiFiClient client;
  String url = String("http://") + server_host + ":" + server_port + "/nodemcu1";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  
  char tempStr[10];
  char humStr[10];
  dtostrf(newTemp, 4, 1, tempStr);
  dtostrf(newHum, 4, 1, humStr);
  String body = String("{\"temperature\":") + tempStr + String(",\"humidity\":") + humStr + String("}");
  
  int code = http.POST(body);
  Serial.printf("POST %s -> %d (Temp: %s°C, Hum: %s%%, Mode: %s)\n", 
                url.c_str(), code, tempStr, humStr, editingTemp ? "TEMP" : "HUM");
  http.end();
  
  // Update current values if successfully sent
  if (code == 201) {
    currentTemp = newTemp;
    currentHum = newHum;
    encoderCount = 0; // Reset counter after applying change
  }
}

void loop() {
  static long lastSentCount = 0;
  // check switch (button) - active LOW
  if (digitalRead(ENC_SW_PIN) == LOW) {
    unsigned long now = millis();
    if (now - lastSwitchTime > switchDebounceMs) {
      lastSwitchTime = now;
      
      // Toggle between temperature and humidity editing mode
      editingTemp = !editingTemp;
      Serial.printf("Encoder mode switched to: %s\n", editingTemp ? "TEMPERATURE" : "HUMIDITY");
      
      // Send mode change notification to server
      HTTPClient http;
      WiFiClient client;
      String url = String("http://") + server_host + ":" + server_port + "/nodemcu1/button";
      http.begin(client, url);
      http.addHeader("Content-Type", "application/json");
      String body = String("{\"mode\":\"") + (editingTemp ? "temperature" : "humidity") + String("\"}");
      int code = http.POST(body);
      Serial.printf("POST %s -> %d\n", url.c_str(), code);
      http.end();
    }
  }

  // if encoder changed, send update
  if (encoderChanged) {
    // disable interrupts briefly to capture the count
    noInterrupts();
    long val = encoderCount;
    encoderChanged = false;
    interrupts();

    if (val != lastSentCount) {
      lastSentCount = val;
      sendEncoderState();
    }
  }

  // small sleep to reduce CPU
  delay(10);
}

#elif defined(NODEMCU2)
// NodeMCU 2: WiFi + rotary encoder input -> HTTP POST to server
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "muditatrey1234";
const char* password = "muditmudit";
const char* server_host = "10.54.2.33"; // Change to your computer's IP
const int server_port = 8000;            // Change to your server's port

// Rotary encoder pin mapping (5-pin module: VCC,GND,CLK,DT,SW)
// Default pins (change if you wire differently)
#define ENC_CLK_PIN 14 // D5
#define ENC_DT_PIN  12 // D6
#define ENC_SW_PIN  13 // D7

// Temperature limits and step
const float TEMP_MIN = -10.0;
const float TEMP_MAX = 40.0;
const float TEMP_STEP = 0.5;

// Humidity limits and step
const float HUM_MIN = 0.0;
const float HUM_MAX = 100.0;
const float HUM_STEP = 0.5;

volatile long encoderCount = 0;
volatile bool encoderChanged = false;
float currentTemp = 20.0; // Start at 20°C
float currentHum = 50.0;  // Start at 50%
bool editingTemp = true;  // true = editing temperature, false = editing humidity

// for switch debounce
unsigned long lastSwitchTime = 0;
const unsigned long switchDebounceMs = 300;

// previous state for gray code method
volatile int lastEncoded = 0;

ICACHE_RAM_ATTR void handleEncoder() {
  int MSB = digitalRead(ENC_CLK_PIN); // most significant bit
  int LSB = digitalRead(ENC_DT_PIN);  // least significant bit
  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderCount++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderCount--;
  lastEncoded = encoded;
  encoderChanged = true;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("\nWiFi connected!");

  // encoder pins
  pinMode(ENC_CLK_PIN, INPUT_PULLUP);
  pinMode(ENC_DT_PIN, INPUT_PULLUP);
  pinMode(ENC_SW_PIN, INPUT_PULLUP);

  // read initial
  int MSB = digitalRead(ENC_CLK_PIN);
  int LSB = digitalRead(ENC_DT_PIN);
  lastEncoded = (MSB << 1) | LSB;

  // attach interrupts
  attachInterrupt(digitalPinToInterrupt(ENC_CLK_PIN), handleEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_DT_PIN), handleEncoder, CHANGE);
}

void sendEncoderState() {
  float newTemp = currentTemp;
  float newHum = currentHum;
  
  // Update the value based on current mode
  if (editingTemp) {
    newTemp = currentTemp + (encoderCount * TEMP_STEP);
    newTemp = max(TEMP_MIN, min(TEMP_MAX, newTemp));
  } else {
    newHum = currentHum + (encoderCount * HUM_STEP);
    newHum = max(HUM_MIN, min(HUM_MAX, newHum));
  }
  
  HTTPClient http;
  WiFiClient client;
  String url = String("http://") + server_host + ":" + server_port + "/nodemcu2/encoder";
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  
  char tempStr[10];
  char humStr[10];
  dtostrf(newTemp, 4, 1, tempStr);
  dtostrf(newHum, 4, 1, humStr);
  String body = String("{\"temperature\":") + tempStr + String(",\"humidity\":") + humStr + String("}");
  
  int code = http.POST(body);
  Serial.printf("POST %s -> %d (Temp: %s°C, Hum: %s%%, Mode: %s)\n", 
                url.c_str(), code, tempStr, humStr, editingTemp ? "TEMP" : "HUM");
  http.end();
  
  // Update current values if successfully sent
  if (code == 201) {
    currentTemp = newTemp;
    currentHum = newHum;
    encoderCount = 0; // Reset counter after applying change
  }
}

void loop() {
  static long lastSentCount = 0;
  // check switch (button) - active LOW
  if (digitalRead(ENC_SW_PIN) == LOW) {
    unsigned long now = millis();
    if (now - lastSwitchTime > switchDebounceMs) {
      lastSwitchTime = now;
      
      // Toggle between temperature and humidity editing mode
      editingTemp = !editingTemp;
      Serial.printf("Encoder mode switched to: %s\n", editingTemp ? "TEMPERATURE" : "HUMIDITY");
      
      // Send mode change notification to server
      HTTPClient http;
      WiFiClient client;
      String url = String("http://") + server_host + ":" + server_port + "/nodemcu2/button";
      http.begin(client, url);
      http.addHeader("Content-Type", "application/json");
      String body = String("{\"mode\":\"") + (editingTemp ? "temperature" : "humidity") + String("\"}");
      int code = http.POST(body);
      Serial.printf("POST %s -> %d\n", url.c_str(), code);
      http.end();
    }
  }

  // if encoder changed, send update
  if (encoderChanged) {
    // disable interrupts briefly to capture the count
    noInterrupts();
    long val = encoderCount;
    encoderChanged = false;
    interrupts();

    if (val != lastSentCount) {
      lastSentCount = val;
      sendEncoderState();
    }
  }

  // small sleep to reduce CPU
  delay(10);
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