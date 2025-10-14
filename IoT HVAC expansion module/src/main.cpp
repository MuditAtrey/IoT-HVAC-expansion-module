#include <Arduino.h>

void setup() {
  // Set the internal LED pin (GPIO 2, D4) as an output
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Toggle the LED state (Note: ESP8266 LED is active LOW)
  digitalWrite(LED_BUILTIN, LOW);   // Turn on the LED
  delay(100);                       // Wait for 100ms
  digitalWrite(LED_BUILTIN, HIGH);  // Turn off the LED
  delay(100);                       // Wait for 100ms
}