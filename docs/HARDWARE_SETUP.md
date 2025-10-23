# Hardware Setup Guide

## Component Wiring

### Arduino Uno R4 Minima

#### Display 1 - Room Conditions (SSD1306 128x64 OLED - SPI)
| Display Pin | Arduino Pin | Description |
|-------------|-------------|-------------|
| VCC         | 5V          | Power       |
| GND         | GND         | Ground      |
| MOSI        | D11         | Data        |
| CLK         | D13         | Clock       |
| DC          | D9          | Data/Command|
| CS          | D10         | Chip Select |
| RST         | D8          | Reset       |

#### Display 2 - AC Settings (SSD1306 128x64 OLED - SPI)
| Display Pin | Arduino Pin | Description |
|-------------|-------------|-------------|
| VCC         | 5V          | Power       |
| GND         | GND         | Ground      |
| MOSI        | D11         | Data (shared)|
| CLK         | D13         | Clock (shared)|
| DC          | D7          | Data/Command|
| CS          | D6          | Chip Select |
| RST         | D5          | Reset       |

#### DHT22 Temperature/Humidity Sensor
| DHT22 Pin   | Arduino Pin | Description |
|-------------|-------------|-------------|
| VCC         | 5V          | Power       |
| GND         | GND         | Ground      |
| DATA        | D2          | Data Signal |

**Note**: Add a 10kΩ pull-up resistor between VCC and DATA pin.

#### Rotary Encoder
| Encoder Pin | Arduino Pin | Description |
|-------------|-------------|-------------|
| CLK         | D3          | Clock       |
| DT          | D4          | Data        |
| SW          | A1          | Switch/Button|
| +           | 5V          | Power       |
| GND         | GND         | Ground      |

#### Power Button
| Button      | Arduino Pin | Description |
|-------------|-------------|-------------|
| One side    | A0          | Input       |
| Other side  | GND         | Ground      |

**Note**: Enable internal pull-up resistor in code (INPUT_PULLUP).

#### Serial Connection to ESP8266
| Arduino Pin | ESP8266 Pin | Description |
|-------------|-------------|-------------|
| D0 (RX)     | TX          | Receive     |
| D1 (TX)     | RX          | Transmit    |
| GND         | GND         | Ground      |

⚠️ **WARNING**: Arduino R4 TX outputs 5V, but ESP8266 RX expects 3.3V. Consider using a voltage divider (2kΩ + 1kΩ resistors) or logic level shifter to avoid damage.

**Voltage Divider Circuit** (if needed):
```
Arduino TX (D1) ---[2kΩ]---+--- ESP8266 RX
                           |
                         [1kΩ]
                           |
                          GND
```

### ESP8266 NodeMCU

#### IR LED Circuit
| Component   | Connection  | Description |
|-------------|-------------|-------------|
| GPIO4 (D2)  | Base (NPN)  | Control Signal|
| Collector   | IR LED Anode| Current Path|
| Emitter     | GND         | Ground      |
| IR LED Cathode| GND via 100Ω | Current Limit|
| 5V          | IR LED Anode via Transistor | Power |

**Circuit Diagram**:
```
        5V
         |
         |
    [IR LED] (Anode to 5V)
         |
         ├─── Collector (NPN Transistor)
         |
    Base ├─── GPIO4 (D2)
         |
    Emitter ─── GND
         |
    [100Ω] ─── GND (from IR LED Cathode)
```

**Recommended Components**:
- NPN Transistor: 2N2222, BC547, or similar
- IR LED: 940nm wavelength
- Resistor: 100Ω (for IR LED current limiting)

#### Serial Connection to Arduino
(See Arduino section above)

#### Power Supply
- USB 5V (during development)
- External 5V supply for final installation

## Assembly Tips

1. **Use a breadboard** for prototyping before soldering
2. **Keep wires short** to minimize electrical noise
3. **Separate power supplies** for high-current devices if needed
4. **Test each component** individually before full assembly
5. **Double-check polarity** for LEDs and electrolytic capacitors
6. **Use heat shrink tubing** or electrical tape for exposed connections

## Testing Procedure

### 1. Test Displays
Upload a simple U8g2 test sketch to verify both displays work independently.

### 2. Test DHT22
Read temperature and humidity values via Serial Monitor.

### 3. Test Rotary Encoder
Print encoder position and button presses to Serial Monitor.

### 4. Test IR LED
Use phone camera to see if IR LED flashes (appears as purple/white light on camera).

### 5. Test Serial Communication
Send JSON data between Arduino and ESP8266, monitor both Serial outputs.

### 6. Full System Test
Upload complete firmware and verify all components work together.

## Troubleshooting Hardware

### Displays Not Working
- Check SPI connections
- Verify 5V power supply
- Try swapping CS/DC/RST pins in code
- Test with I2C version if available

### DHT22 Reading NaN
- Check pull-up resistor (10kΩ)
- Verify 5V power
- Wait 2+ seconds between readings
- Try different DHT library

### Rotary Encoder Bouncing
- Add 0.1μF capacitors between CLK/DT and GND
- Implement software debouncing
- Check wiring connections

### IR LED Not Transmitting
- Verify transistor orientation (NPN: Collector-Base-Emitter)
- Check IR LED polarity (longer leg = anode)
- Test with known working IR remote app
- Confirm GPIO4 output with multimeter

### ESP8266 Not Powering On
- Ensure 5V supply provides sufficient current (500mA+)
- Check USB cable quality
- Verify no short circuits
- Try different USB port/power supply

## Safety Warnings

⚠️ **Electrical Safety**:
- Never connect/disconnect components while powered
- Use proper insulation for all connections
- Keep water away from electronics
- Use appropriate power supplies

⚠️ **Component Protection**:
- Respect voltage limits (3.3V for ESP8266 logic pins)
- Use current-limiting resistors for LEDs
- Don't exceed pin current ratings (typically 20-40mA)
- Avoid static discharge (ESD) when handling boards

---

For additional help, refer to the main [README.md](../README.md) or open an issue on GitHub.
