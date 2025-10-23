/*
 * HVAC Control System - Unified Code
 * 
 * This file contains code for both:
 * 1. Arduino Uno R4 Minima - Local control interface
 * 2. ESP8266 NodeMCU - Central hub with WiFi and IR
 * 
 * PlatformIO automatically compiles the correct code based on the selected environment:
 * - platformio run --environment uno_r4_minima
 * - platformio run --environment nodemcuv2
 */

#include <Arduino.h>
#include <ArduinoJson.h>

// ============================================================================
// ARDUINO UNO R4 MINIMA CODE
// ============================================================================
#ifdef ARDUINO_UNOR4_MINIMA

/*
 * Arduino Uno R4 Minima - HVAC Control System
 * 
 * Hardware connections:
 * - Display 1 (Room Conditions) - SPI:
 *   MOSI: D11, CLK: D13, DC: D9, CS: D10, RST: D8
 * 
 * - Display 2 (AC Settings) - SPI:
 *   MOSI: D11, CLK: D13, DC: D7, CS: D6, RST: D5
 * 
 * - DHT22 Sensor:
 *   Data: D2
 * 
 * - Rotary Encoder:
 *   CLK: D3, DT: D4, SW (Button): A1
 * 
 * - AC Power Button:
 *   Button: A0 (with pull-up)
 * 
 * - Serial to ESP8266:
 *   TX: D1, RX: D0 (Hardware Serial)
 */

#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>

// Display 1 - Room Conditions (SPI)
#define OLED1_CLK    13
#define OLED1_MOSI   11
#define OLED1_CS     10
#define OLED1_DC     9
#define OLED1_RESET  8

// Display 2 - AC Settings (SPI)
#define OLED2_CLK    13
#define OLED2_MOSI   11
#define OLED2_CS     6
#define OLED2_DC     7
#define OLED2_RESET  5

// DHT22 Sensor
#define DHTPIN 2
#define DHTTYPE DHT22

// Rotary Encoder
#define ENCODER_CLK 3
#define ENCODER_DT  4
#define ENCODER_SW  A1

// AC Power Button
#define POWER_BTN A0

// Create display objects using U8G2 (SPI, 128x64)
// Constructor: U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI(rotation, cs, dc, reset)
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI display1(U8G2_R0, OLED1_CS, OLED1_DC, OLED1_RESET);
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI display2(U8G2_R0, OLED2_CS, OLED2_DC, OLED2_RESET);

// Create DHT sensor object
DHT dht(DHTPIN, DHTTYPE);

// Simple encoder state tracking
volatile int encoderPos = 0;
volatile bool encoderClkLast = HIGH;

// HVAC Settings
struct HVACSettings {
  String power = "on";
  int setTemp = 24;
  String mode = "cool";
  String fanSpeed = "medium";
  int timer = 0;
  String swing = "on";
} hvacSettings;

// Room conditions
float roomTemp = 0.0;
float roomHumidity = 0.0;

// Menu system
enum MenuState {
  MENU_BROWSE,    // Browsing between windows
  MENU_EDIT       // Editing a setting
};

enum SettingWindow {
  WINDOW_TEMP,
  WINDOW_FAN,
  WINDOW_SWING,
  WINDOW_TIMER,
  WINDOW_COUNT
};

MenuState menuState = MENU_BROWSE;
SettingWindow currentWindow = WINDOW_TEMP;
SettingWindow highlightedWindow = WINDOW_TEMP;

// Button debouncing
unsigned long lastEncoderPress = 0;
unsigned long lastPowerPress = 0;
const unsigned long debounceDelay = 200;

// Encoder state
int lastEncoderPos = 0;

// Update intervals
unsigned long lastDHTRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastDataSend = 0;
const unsigned long dhtInterval = 2000;      // Read DHT every 2 seconds
const unsigned long displayInterval = 100;    // Update display every 100ms
const unsigned long dataSendInterval = 1000;  // Send data every 1 second

// Function declarations
void initDisplays();
void updateDisplay1();
void updateDisplay2();
void readDHT();
void handleEncoder();
void handleButtons();
void sendDataToESP();
void receiveDataFromESP();
void drawWindow(U8G2 &u8g2, int x, int y, int w, int h, 
                const char* title, const char* value, bool highlighted, bool selected);

void setup() {
  Serial.begin(115200);  // USB debugging (USB CDC)
  
  // Initialize Hardware Serial1 (pins D0=RX, D1=TX) to talk to ESP8266
  // Use 9600 baud to reduce signal stress when level shifting isn't available
  Serial1.begin(9600);
  
  Serial.println("Arduino R4 Minima HVAC Controller");
  Serial.println("ESP Serial: RX=D0, TX=D1 @ 9600 baud");
  Serial.println("WARNING: R4 TX is 5V and ESP RX is 3.3V – use a level shifter if possible.");
  
  // Initialize displays
  display1.begin();
  display2.begin();
  
  // Clear displays
  display1.clearBuffer();
  display2.clearBuffer();
  
  // Show startup message
  display1.setFont(u8g2_font_6x10_tr);
  display1.drawStr(25, 32, "HVAC System");
  display1.sendBuffer();
  
  display2.setFont(u8g2_font_6x10_tr);
  display2.drawStr(15, 32, "Initializing...");
  display2.sendBuffer();
  
  delay(2000);
  
  // Initialize DHT sensor
  dht.begin();
  
  // Initialize buttons
  pinMode(ENCODER_SW, INPUT_PULLUP);
  pinMode(POWER_BTN, INPUT_PULLUP);
  
  // Initialize encoder pins
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  encoderClkLast = digitalRead(ENCODER_CLK);
  encoderPos = 0;
  lastEncoderPos = 0;
  
  // Final clear after init (not strictly necessary)
  display1.clearBuffer();
  display1.sendBuffer();
  display2.clearBuffer();
  display2.sendBuffer();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Read DHT sensor periodically
  if (currentTime - lastDHTRead >= dhtInterval) {
    lastDHTRead = currentTime;
    readDHT();
  }
  
  // Handle encoder and buttons
  handleEncoder();
  handleButtons();
  
  // Update displays periodically
  if (currentTime - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = currentTime;
    updateDisplay1();
    updateDisplay2();
  }
  
  // Send data to ESP8266 periodically
  if (currentTime - lastDataSend >= dataSendInterval) {
    lastDataSend = currentTime;
    sendDataToESP();
  }
  
  // Check for data from ESP8266
  receiveDataFromESP();
}

void readDHT() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (!isnan(h) && !isnan(t)) {
    roomTemp = t;
    roomHumidity = h;
  }
}

void handleEncoder() {
  // Read encoder pins
  bool encoderClk = digitalRead(ENCODER_CLK);
  bool encoderDt = digitalRead(ENCODER_DT);
  
  // Detect rotation
  if (encoderClk != encoderClkLast) {
    if (encoderDt != encoderClk) {
      encoderPos++;  // Clockwise
    } else {
      encoderPos--;  // Counter-clockwise
    }
    encoderClkLast = encoderClk;
  }
  
  // Check if position changed
  if (encoderPos != lastEncoderPos) {
    int delta = encoderPos - lastEncoderPos;
    lastEncoderPos = encoderPos;
    
    if (menuState == MENU_BROWSE) {
      // Browse between windows
      if (delta > 0) {
        highlightedWindow = (SettingWindow)((highlightedWindow + 1) % WINDOW_COUNT);
      } else if (delta < 0) {
        highlightedWindow = (SettingWindow)((highlightedWindow - 1 + WINDOW_COUNT) % WINDOW_COUNT);
      }
    } else if (menuState == MENU_EDIT) {
      // Edit the selected setting
      switch (currentWindow) {
        case WINDOW_TEMP:
          hvacSettings.setTemp += delta;
          if (hvacSettings.setTemp > 30) hvacSettings.setTemp = 30;
          if (hvacSettings.setTemp < 16) hvacSettings.setTemp = 16;
          break;
          
        case WINDOW_FAN:
          if (delta != 0) {
            if (hvacSettings.fanSpeed == "low") hvacSettings.fanSpeed = "medium";
            else if (hvacSettings.fanSpeed == "medium") hvacSettings.fanSpeed = "high";
            else if (hvacSettings.fanSpeed == "high") hvacSettings.fanSpeed = "auto";
            else hvacSettings.fanSpeed = "low";
          }
          break;
          
        case WINDOW_SWING:
          if (delta != 0) {
            hvacSettings.swing = (hvacSettings.swing == "on") ? "off" : "on";
          }
          break;
          
        case WINDOW_TIMER:
          hvacSettings.timer += delta * 15;  // 15-minute increments
          if (hvacSettings.timer < 0) hvacSettings.timer = 0;
          if (hvacSettings.timer > 720) hvacSettings.timer = 720;
          break;
      }
    }
  }
}

void handleButtons() {
  // Handle encoder button (select/confirm)
  if (digitalRead(ENCODER_SW) == LOW) {
    if (millis() - lastEncoderPress > debounceDelay) {
      lastEncoderPress = millis();
      
      if (menuState == MENU_BROWSE) {
        // Enter edit mode for highlighted window
        menuState = MENU_EDIT;
        currentWindow = highlightedWindow;
      } else {
        // Exit edit mode, return to browse
        menuState = MENU_BROWSE;
      }
    }
  }
  
  // Handle power button
  if (digitalRead(POWER_BTN) == LOW) {
    if (millis() - lastPowerPress > debounceDelay) {
      lastPowerPress = millis();
      hvacSettings.power = (hvacSettings.power == "on") ? "off" : "on";
    }
  }
}

void updateDisplay1() {
  // Display 1: Room Conditions
  display1.clearBuffer();
  
  // Title
  display1.setFont(u8g2_font_6x10_tr);
  display1.drawStr(15, 10, "Room Condition");
  
  // Draw line under title
  display1.drawLine(0, 12, 128, 12);
  
  // Temperature label
  display1.setFont(u8g2_font_6x10_tr);
  display1.drawStr(5, 26, "Temp:");
  
  // Temperature value (larger font)
  display1.setFont(u8g2_font_10x20_tr);
  char tempStr[10];
  dtostrf(roomTemp, 4, 1, tempStr);
  strcat(tempStr, "C");
  display1.drawStr(45, 30, tempStr);
  
  // Humidity label
  display1.setFont(u8g2_font_6x10_tr);
  display1.drawStr(5, 48, "Humidity:");
  
  // Humidity value (larger font)
  display1.setFont(u8g2_font_10x20_tr);
  char humStr[10];
  sprintf(humStr, "%.0f%%", roomHumidity);
  display1.drawStr(65, 52, humStr);
  
  display1.sendBuffer();
}

void updateDisplay2() {
  // Display 2: AC Settings
  display2.clearBuffer();
  
  // Title
  display2.setFont(u8g2_font_6x10_tr);
  char titleStr[30];
  sprintf(titleStr, "AC Settings [%s]", hvacSettings.power == "on" ? "ON" : "OFF");
  display2.drawStr(5, 10, titleStr);
  
  // Draw line under title
  display2.drawLine(0, 12, 128, 12);
  
  // Define window positions and sizes
  int leftX = 2, rightX = 66;
  int topY = 16, bottomY = 42;
  int leftW = 60, rightW = 60;
  int windowH = 24;
  
  // Temperature (Top Left)
  char tempStr[8];
  sprintf(tempStr, "%dC", hvacSettings.setTemp);
  drawWindow(display2, leftX, topY, leftW, windowH, "TEMP", tempStr,
             highlightedWindow == WINDOW_TEMP, 
             menuState == MENU_EDIT && currentWindow == WINDOW_TEMP);
  
  // Fan Speed (Top Right)
  const char* fanStr = hvacSettings.fanSpeed == "low" ? "LOW" :
                       hvacSettings.fanSpeed == "medium" ? "MED" :
                       hvacSettings.fanSpeed == "high" ? "HIGH" : "AUTO";
  drawWindow(display2, rightX, topY, rightW, windowH, "FAN", fanStr,
             highlightedWindow == WINDOW_FAN,
             menuState == MENU_EDIT && currentWindow == WINDOW_FAN);
  
  // Timer (Bottom Left)
  char timerStr[8];
  if (hvacSettings.timer == 0) {
    sprintf(timerStr, "OFF");
  } else {
    sprintf(timerStr, "%dm", hvacSettings.timer);
  }
  drawWindow(display2, leftX, bottomY, leftW, windowH, "TIMER", timerStr,
             highlightedWindow == WINDOW_TIMER,
             menuState == MENU_EDIT && currentWindow == WINDOW_TIMER);
  
  // Swing (Bottom Right)
  const char* swingStr = hvacSettings.swing == "on" ? "ON" : "OFF";
  drawWindow(display2, rightX, bottomY, rightW, windowH, "SWING", swingStr,
             highlightedWindow == WINDOW_SWING,
             menuState == MENU_EDIT && currentWindow == WINDOW_SWING);
  
  display2.sendBuffer();
}

void drawWindow(U8G2 &u8g2, int x, int y, int w, int h,
                const char* title, const char* value, bool highlighted, bool selected) {
  
  if (selected) {
    // Filled background for selected (editing mode) - bright background, dark text
    u8g2.setDrawColor(1);  // White
    u8g2.drawBox(x, y, w, h);
    u8g2.setDrawColor(0);  // Black for text
    
    // Draw title (small, top)
    u8g2.setFont(u8g2_font_5x7_tr);
    int titleLen = u8g2.getStrWidth(title);
    u8g2.drawStr(x + (w - titleLen) / 2, y + 9, title);
    
    // Draw value (centered)
    u8g2.setFont(u8g2_font_6x10_tr);
    int valueLen = u8g2.getStrWidth(value);
    u8g2.drawStr(x + (w - valueLen) / 2, y + 20, value);
    
    // Reset draw color
    u8g2.setDrawColor(1);
    
  } else if (highlighted) {
    // Thicker border for highlighted (menu mode) - 3 pixel thick border
    u8g2.setDrawColor(1);
    u8g2.drawFrame(x, y, w, h);
    u8g2.drawFrame(x+1, y+1, w-2, h-2);
    u8g2.drawFrame(x+2, y+2, w-4, h-4);
    
    // Draw title (small, top)
    u8g2.setFont(u8g2_font_5x7_tr);
    int titleLen = u8g2.getStrWidth(title);
    u8g2.drawStr(x + (w - titleLen) / 2, y + 9, title);
    
    // Draw value (centered)
    u8g2.setFont(u8g2_font_6x10_tr);
    int valueLen = u8g2.getStrWidth(value);
    u8g2.drawStr(x + (w - valueLen) / 2, y + 20, value);
    
  } else {
    // Normal border - single line
    u8g2.setDrawColor(1);
    u8g2.drawFrame(x, y, w, h);
    
    // Draw title (small, top)
    u8g2.setFont(u8g2_font_5x7_tr);
    int titleLen = u8g2.getStrWidth(title);
    u8g2.drawStr(x + (w - titleLen) / 2, y + 9, title);
    
    // Draw value (centered)
    u8g2.setFont(u8g2_font_6x10_tr);
    int valueLen = u8g2.getStrWidth(value);
    u8g2.drawStr(x + (w - valueLen) / 2, y + 20, value);
  }
}

void sendDataToESP() {
  // Create JSON document
  StaticJsonDocument<512> doc;
  
  doc["roomTemp"] = roomTemp;
  doc["roomHumidity"] = roomHumidity;
  
  JsonObject hvac = doc.createNestedObject("hvac");
  hvac["power"] = hvacSettings.power;
  hvac["setTemp"] = hvacSettings.setTemp;
  hvac["mode"] = hvacSettings.mode;
  hvac["fanSpeed"] = hvacSettings.fanSpeed;
  hvac["timer"] = hvacSettings.timer;
  hvac["swing"] = hvacSettings.swing;
  
  // Send via Software Serial
  serializeJson(doc, Serial1);
  Serial1.println();
  
  // Also debug via USB Serial
  Serial.print("-> ESP: ");
  serializeJson(doc, Serial);
  Serial.println();
}

void receiveDataFromESP() {
  if (Serial1.available() > 0) {
    String jsonString = Serial1.readStringUntil('\n');
    
    Serial.print("<- ESP: ");
    Serial.println(jsonString);
    
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    
    if (!error) {
      if (doc.containsKey("hvac")) {
        JsonObject hvac = doc["hvac"];
        
        if (hvac.containsKey("power")) {
          hvacSettings.power = hvac["power"].as<String>();
        }
        if (hvac.containsKey("setTemp")) {
          hvacSettings.setTemp = hvac["setTemp"];
        }
        if (hvac.containsKey("mode")) {
          hvacSettings.mode = hvac["mode"].as<String>();
        }
        if (hvac.containsKey("fanSpeed")) {
          hvacSettings.fanSpeed = hvac["fanSpeed"].as<String>();
        }
        if (hvac.containsKey("timer")) {
          hvacSettings.timer = hvac["timer"];
        }
        if (hvac.containsKey("swing")) {
          hvacSettings.swing = hvac["swing"].as<String>();
        }
      }
    }
  }
}

#endif // ARDUINO_UNOR4_MINIMA

// ============================================================================
// ESP8266 NODEMCU CODE
// ============================================================================
#ifdef ESP8266

/*
 * ESP8266 NodeMCU - HVAC Control Hub
 * 
 * This is the central hub that:
 * 1. Receives data from Arduino R4 Minima via Serial
 * 2. Sends IR commands to AC unit
 * 3. Communicates with web server via WiFi
 * 4. Syncs settings between Arduino, AC, and Web
 * 
 * Hardware connections:
 * - IR LED (3-pin):
 *   Signal: D2 (GPIO4)
 *   VCC: 5V (via transistor/driver)
 *   GND: GND
 * 
 * - Serial to Arduino R4 Minima:
 *   RX: RX (GPIO3)
 *   TX: TX (GPIO1)
 *   GND: GND
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Daikin.h>  // Change this based on your AC brand

// WiFi credentials
const char* ssid = "Anshul_2";
const char* password = "mudit@00012";

// Server URL (replace with your computer's IP address)
const char* serverUrl = "http://192.168.29.64:5001/api/data";
const char* commandUrl = "http://192.168.29.64:5001/api/hvac/command";
const char* updateUrl = "http://192.168.29.64:5001/api/hvac/update";

// IR Transmitter setup
const uint16_t kIrLed = 4;  // GPIO4 (D2)
IRDaikinESP ac(kIrLed);     // Change to your AC brand (IRDaikinESP, IRMitsubishiAC, etc.)

WiFiClient wifiClient;

// HVAC Settings
struct HVACSettings {
  String power = "on";
  int setTemp = 24;
  String mode = "cool";
  String fanSpeed = "medium";
  int timer = 0;
  String swing = "on";
  String source = "arduino";  // Track where change came from
} hvacSettings;

// Room conditions from Arduino
float roomTemp = 0.0;
float roomHumidity = 0.0;

// Timing
unsigned long lastServerUpdate = 0;
unsigned long lastCommandCheck = 0;
unsigned long lastArduinoSend = 0;
unsigned long lastScheduleCheck = 0;
const unsigned long serverUpdateInterval = 2000;    // Send to server every 2 seconds (faster sync)
const unsigned long commandCheckInterval = 500;     // Check server commands every 0.5 seconds
const unsigned long arduinoSendInterval = 250;      // Push to Arduino quickly when needed
const unsigned long scheduleCheckInterval = 30000;  // Check schedule every 30 seconds

// Flags
bool settingsChanged = false;
bool needsACUpdate = false;

// Function declarations
void connectWiFi();
void receiveFromArduino();
void sendToArduino();
void updateAC();
void sendToServer();
void checkServerCommands();
void checkSchedule();
void applyACSettings();
String modeToString(uint8_t mode);
String fanSpeedToString(uint8_t speed);
uint8_t stringToMode(String mode);
uint8_t stringToFanSpeed(String speed);

void setup() {
  Serial.begin(9600);  // Match Arduino's baud rate (was 115200)
  delay(10);
  
  Serial.println("\nESP8266 HVAC Hub");
  
  // Initialize IR transmitter
  ac.begin();
  Serial.println("IR Transmitter initialized");
  
  // Connect to WiFi
  connectWiFi();
  
  // Initialize AC with default settings
  applyACSettings();
  
  Serial.println("System ready!");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Receive data from Arduino
  receiveFromArduino();
  
  // Send updates to Arduino if needed
  if (currentTime - lastArduinoSend >= arduinoSendInterval) {
    lastArduinoSend = currentTime;
    sendToArduino();
  }
  
  // Update AC if settings changed
  if (needsACUpdate) {
    needsACUpdate = false;
    updateAC();
  }
  
  // Send data to server periodically
  if (currentTime - lastServerUpdate >= serverUpdateInterval) {
    lastServerUpdate = currentTime;
    sendToServer();
  }
  
  // Check for commands from server
  if (currentTime - lastCommandCheck >= commandCheckInterval) {
    lastCommandCheck = currentTime;
    checkServerCommands();
  }
  
  // Check schedule status
  if (currentTime - lastScheduleCheck >= scheduleCheckInterval) {
    lastScheduleCheck = currentTime;
    checkSchedule();
  }
}

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Server URL: ");
    Serial.println(serverUrl);
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

void receiveFromArduino() {
  if (Serial.available() > 0) {
    String jsonString = Serial.readStringUntil('\n');
    
    // Parse JSON from Arduino
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    
    if (!error) {
      // Update room conditions
      if (doc.containsKey("roomTemp")) {
        roomTemp = doc["roomTemp"];
      }
      if (doc.containsKey("roomHumidity")) {
        roomHumidity = doc["roomHumidity"];
      }
      
      // Update HVAC settings if changed from Arduino
      if (doc.containsKey("hvac")) {
        JsonObject hvac = doc["hvac"];
        bool changed = false;
        
        if (hvac.containsKey("power")) {
          String newPower = hvac["power"].as<String>();
          if (newPower != hvacSettings.power) {
            hvacSettings.power = newPower;
            changed = true;
          }
        }
        
        if (hvac.containsKey("setTemp")) {
          int newTemp = hvac["setTemp"];
          if (newTemp != hvacSettings.setTemp) {
            hvacSettings.setTemp = newTemp;
            changed = true;
          }
        }
        
        if (hvac.containsKey("mode")) {
          String newMode = hvac["mode"].as<String>();
          if (newMode != hvacSettings.mode) {
            hvacSettings.mode = newMode;
            changed = true;
          }
        }
        
        if (hvac.containsKey("fanSpeed")) {
          String newFanSpeed = hvac["fanSpeed"].as<String>();
          if (newFanSpeed != hvacSettings.fanSpeed) {
            hvacSettings.fanSpeed = newFanSpeed;
            changed = true;
          }
        }
        
        if (hvac.containsKey("timer")) {
          int newTimer = hvac["timer"];
          if (newTimer != hvacSettings.timer) {
            hvacSettings.timer = newTimer;
            changed = true;
          }
        }
        
        if (hvac.containsKey("swing")) {
          String newSwing = hvac["swing"].as<String>();
          if (newSwing != hvacSettings.swing) {
            hvacSettings.swing = newSwing;
            changed = true;
          }
        }
        
        if (changed) {
          Serial.println("Settings updated from Arduino");
          hvacSettings.source = "arduino";
          needsACUpdate = true;
          settingsChanged = true;
          // Immediately notify server to prevent stale web command override
          sendToServer();
        }
      }
    }
  }
}

void sendToArduino() {
  // Send current HVAC settings back to Arduino
  // (in case they were changed from web)
  if (hvacSettings.source == "web") {
    StaticJsonDocument<512> doc;
    
    JsonObject hvac = doc.createNestedObject("hvac");
    hvac["power"] = hvacSettings.power;
    hvac["setTemp"] = hvacSettings.setTemp;
    hvac["mode"] = hvacSettings.mode;
    hvac["fanSpeed"] = hvacSettings.fanSpeed;
    hvac["timer"] = hvacSettings.timer;
    hvac["swing"] = hvacSettings.swing;
    
    serializeJson(doc, Serial);
    Serial.println();
    
    hvacSettings.source = "synced";  // Mark as synced
  }
}

void updateAC() {
  Serial.println("Updating AC...");
  applyACSettings();
  
  // Send IR command
  ac.send();
  
  Serial.println("AC updated successfully");
  Serial.print("  Power: ");
  Serial.println(hvacSettings.power);
  Serial.print("  Temp: ");
  Serial.println(hvacSettings.setTemp);
  Serial.print("  Mode: ");
  Serial.println(hvacSettings.mode);
  Serial.print("  Fan: ");
  Serial.println(hvacSettings.fanSpeed);
  Serial.print("  Swing: ");
  Serial.println(hvacSettings.swing);
}

void applyACSettings() {
  // Apply settings to IR library
  // Note: Adjust these based on your AC brand's library
  
  // Power
  if (hvacSettings.power == "on") {
    ac.on();
  } else {
    ac.off();
  }
  
  // Temperature
  ac.setTemp(hvacSettings.setTemp);
  
  // Mode
  ac.setMode(stringToMode(hvacSettings.mode));
  
  // Fan Speed
  ac.setFan(stringToFanSpeed(hvacSettings.fanSpeed));
  
  // Swing
  if (hvacSettings.swing == "on") {
    ac.setSwingVertical(true);
  } else {
    ac.setSwingVertical(false);
  }
}

void sendToServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    connectWiFi();
    return;
  }
  
  HTTPClient http;
  
  http.begin(wifiClient, serverUrl);
  http.addHeader("Content-Type", "application/json");
  
  // Create JSON payload with sensor data and HVAC settings
  StaticJsonDocument<512> doc;
  doc["temperature"] = roomTemp;
  doc["humidity"] = roomHumidity;
  
  // Add HVAC settings
  JsonObject hvac = doc.createNestedObject("hvac");
  hvac["power"] = hvacSettings.power;
  hvac["set_temp"] = hvacSettings.setTemp;
  hvac["mode"] = hvacSettings.mode;
  hvac["fan_speed"] = hvacSettings.fanSpeed;
  hvac["timer"] = hvacSettings.timer;
  hvac["swing"] = hvacSettings.swing;
  
  String jsonData;
  serializeJson(doc, jsonData);
  
  Serial.print("Sending to server: ");
  Serial.println(jsonData);
  
  // Send POST request
  int httpResponseCode = http.POST(jsonData);
  
  if (httpResponseCode > 0) {
    Serial.print("Server response: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error sending data: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
}

void checkServerCommands() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  HTTPClient http;
  
  http.begin(wifiClient, commandUrl);
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == 200) {
    String response = http.getString();
    
    // Parse JSON response
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (!error && doc.containsKey("source")) {
      String source = doc["source"].as<String>();
      
      // Only process if changes came from web
      if (source == "web") {
        bool changed = false;
        
        if (doc.containsKey("power")) {
          String newPower = doc["power"].as<String>();
          if (newPower != hvacSettings.power) {
            hvacSettings.power = newPower;
            changed = true;
          }
        }
        
        if (doc.containsKey("set_temp")) {
          int newTemp = doc["set_temp"];
          if (newTemp != hvacSettings.setTemp) {
            hvacSettings.setTemp = newTemp;
            changed = true;
          }
        }
        
        if (doc.containsKey("mode")) {
          String newMode = doc["mode"].as<String>();
          if (newMode != hvacSettings.mode) {
            hvacSettings.mode = newMode;
            changed = true;
          }
        }
        
        if (doc.containsKey("fan_speed")) {
          String newFanSpeed = doc["fan_speed"].as<String>();
          if (newFanSpeed != hvacSettings.fanSpeed) {
            hvacSettings.fanSpeed = newFanSpeed;
            changed = true;
          }
        }
        
        if (doc.containsKey("timer")) {
          int newTimer = doc["timer"];
          if (newTimer != hvacSettings.timer) {
            hvacSettings.timer = newTimer;
            changed = true;
          }
        }
        
        if (doc.containsKey("swing")) {
          String newSwing = doc["swing"].as<String>();
          if (newSwing != hvacSettings.swing) {
            hvacSettings.swing = newSwing;
            changed = true;
          }
        }
        
        if (changed) {
          Serial.println("Settings updated from web");
          hvacSettings.source = "web";
          needsACUpdate = true;
          settingsChanged = true;
          // Immediately forward to Arduino for near-instant UI sync
          sendToArduino();
        }
      }
    }
  }
  
  http.end();
}

void checkSchedule() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  HTTPClient http;
  
  // Use the schedule status endpoint
  String scheduleStatusUrl = "http://192.168.29.64:5001/api/schedule/status";
  http.begin(wifiClient, scheduleStatusUrl);
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == 200) {
    String response = http.getString();
    
    // Parse JSON response
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (!error && doc.containsKey("schedule_active")) {
      bool scheduleActive = doc["schedule_active"];
      
      if (scheduleActive && doc.containsKey("should_be_on")) {
        bool shouldBeOn = doc["should_be_on"];
        String currentPower = hvacSettings.power;
        String requiredPower = shouldBeOn ? "on" : "off";
        
        // Only change if different from current state
        if (currentPower != requiredPower) {
          Serial.print("Schedule triggered: AC should be ");
          Serial.println(requiredPower);
          
          hvacSettings.power = requiredPower;
          hvacSettings.source = "schedule";
          needsACUpdate = true;
          settingsChanged = true;
          
          // Immediately notify server and Arduino
          sendToServer();
          sendToArduino();
        }
      }
    }
  }
  
  http.end();
}

// Helper functions to convert between strings and AC library constants
uint8_t stringToMode(String mode) {
  if (mode == "cool") return kDaikinCool;
  if (mode == "heat") return kDaikinHeat;
  if (mode == "fan") return kDaikinFan;
  if (mode == "dry") return kDaikinDry;
  if (mode == "auto") return kDaikinAuto;
  return kDaikinCool;  // Default
}

uint8_t stringToFanSpeed(String speed) {
  if (speed == "low") return kDaikinFanMin;
  if (speed == "medium") return kDaikinFanMed;
  if (speed == "high") return kDaikinFanMax;
  if (speed == "auto") return kDaikinFanAuto;
  return kDaikinFanAuto;  // Default
}

String modeToString(uint8_t mode) {
  switch (mode) {
    case kDaikinCool: return "cool";
    case kDaikinHeat: return "heat";
    case kDaikinFan: return "fan";
    case kDaikinDry: return "dry";
    case kDaikinAuto: return "auto";
    default: return "cool";
  }
}

String fanSpeedToString(uint8_t speed) {
  switch (speed) {
    case kDaikinFanMin: return "low";
    case kDaikinFanMed: return "medium";
    case kDaikinFanMax: return "high";
    case kDaikinFanAuto: return "auto";
    default: return "auto";
  }
}

#endif // ESP8266
