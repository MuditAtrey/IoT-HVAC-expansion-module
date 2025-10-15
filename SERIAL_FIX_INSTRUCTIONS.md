# NodeMCU2 Serial Communication Fix

## The Problem
NodeMCU2 is sending data to USB Serial (Serial Monitor) instead of to the R4 Minima via the TX pin.

## The Solution
On ESP8266, there are two serial ports:
- **Serial** (UART0): Used for USB/Serial Monitor
- **Serial1** (UART1): TX-only, uses GPIO2 (D4 pin)

## Required Changes to main.cpp

### 1. In setup() function (around line 217):
```cpp
void setup() {
  Serial.begin(115200);   // USB Serial for debug
  Serial1.begin(115200);  // UART1 TX (GPIO2/D4) for sending to R4
  
  WiFi.begin(ssid, password);
  // ... rest of setup
}
```

### 2. In sendEncoderState() function (around line 281):
Change this line:
```cpp
Serial.printf("T:%.1f,H:%.1f\n", currentTemp, currentHum);
```

To:
```cpp
Serial1.printf("T:%.1f,H:%.1f\n", currentTemp, currentHum);  // Send to R4 via TX pin
Serial.printf("Sent to R4: T:%.1f,H:%.1f\n", currentTemp, currentHum);  // Debug to USB
```

### 3. In loop() periodic send (around line 333):
Change this line:
```cpp
Serial.printf("T:%.1f,H:%.1f\n", currentTemp, currentHum);
```

To:
```cpp
Serial1.printf("T:%.1f,H:%.1f\n", currentTemp, currentHum);  // Send to R4 via TX pin
Serial.printf("Periodic send to R4: T:%.1f,H:%.1f\n", currentTemp, currentHum);  // Debug to USB
```

## Updated Wiring

**IMPORTANT**: Serial1 on ESP8266 uses **GPIO2 (D4 pin)**, NOT the default TX pin!

Connect:
```
NodeMCU2 D4 (GPIO2) → R4 Minima RX (pin 0)
NodeMCU2 GND        → R4 Minima GND
```

## Verification Steps

1. Upload the modified code to NodeMCU2
2. Open Serial Monitor at 115200 baud - you should see debug messages like:
   - "WiFi connected!"
   - "Periodic send to R4: T:20.0,H:50.0"
3. The R4 OLED should update within 2 seconds showing the temperature and humidity

## Alternative: Use Software Serial on R4 Instead

If you want to keep using the default TX pin on NodeMCU2, you could use SoftwareSerial on the R4 Minima on different pins. But using Serial1 on NodeMCU2 is the cleaner solution.
