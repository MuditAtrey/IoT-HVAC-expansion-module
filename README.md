# 🌡️ IoT HVAC Control System

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-orange.svg)](https://platformio.org/)
[![Python](https://img.shields.io/badge/Python-3.9+-blue.svg)](https://www.python.org/)
[![Flask](https://img.shields.io/badge/Flask-3.0+-green.svg)](https://flask.palletsprojects.com/)

A comprehensive IoT-based HVAC (Heating, Ventilation, and Air Conditioning) control system featuring dual-microcontroller architecture, web interface, and automated scheduling capabilities.

## 📋 Table of Contents

- [Overview](#overview)
- [System Architecture](#system-architecture)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation & Setup](#installation--setup)
- [Usage](#usage)
- [API Documentation](#api-documentation)
- [Project Structure](#project-structure)
- [Configuration](#configuration)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## 🎯 Overview

This project implements a sophisticated HVAC control system that combines:
- **Dual Microcontroller Architecture**: Arduino Uno R4 Minima for local control + ESP8266 NodeMCU for WiFi connectivity
- **Web Interface**: Responsive Flask-based dashboard accessible from any device
- **Real-time Monitoring**: DHT22 sensor for temperature and humidity tracking
- **IR Control**: Direct communication with AC units via infrared transmitter
- **Smart Scheduling**: Time-based automation for energy efficiency
- **Mobile Optimized**: Touch-friendly interface for smartphones and tablets

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         System Overview                          │
└─────────────────────────────────────────────────────────────────┘

    ┌──────────────────┐         ┌──────────────────┐
    │  Arduino R4      │◄───────►│  ESP8266         │
    │  Minima          │  Serial │  NodeMCU         │
    │                  │  9600   │                  │
    │  • Dual OLEDs    │  baud   │  • WiFi Hub      │
    │  • DHT22 Sensor  │         │  • IR Transmitter│
    │  • Rotary Encoder│         │  • HTTP Client   │
    │  • Local Control │         │                  │
    └──────────────────┘         └────────┬─────────┘
            │                             │
            │                             │ WiFi
            │                             │
            │                      ┌──────▼──────┐
            │                      │             │
            │                      │  Flask      │
            │                      │  Web Server │
            │                      │             │
            │                      │  • REST API │
            │                      │  • Web UI   │
            │                      │  • Logging  │
            │                      │  • Schedule │
            │                      └──────┬──────┘
            │                             │
            │                             │ HTTP
            │                             │
            ▼                             ▼
    ┌──────────────────┐         ┌──────────────────┐
    │   AC Unit        │         │  Web Browsers    │
    │   (IR Control)   │         │  • Desktop       │
    │                  │         │  • Mobile        │
    └──────────────────┘         │  • Tablet/Kindle │
                                 └──────────────────┘
```

### Data Flow

1. **Sensor Reading**: Arduino reads DHT22 temperature/humidity → Sends to ESP8266
2. **WiFi Transmission**: ESP8266 posts data to Flask server every 2 seconds
3. **Web Control**: User changes settings on web interface
4. **Command Sync**: ESP8266 polls server every 500ms for commands
5. **IR Transmission**: ESP8266 sends IR codes to AC unit
6. **Display Update**: Arduino updates dual OLED displays with current status
7. **Schedule Check**: ESP8266 checks schedule every 30 seconds for automated control

## ✨ Features

### Core Features
- 🌡️ **Real-time Temperature & Humidity Monitoring** (DHT22 sensor)
- 🎛️ **Full HVAC Control**: Power, Temperature (16-30°C), Mode (Cool/Heat/Fan/Dry/Auto)
- 💨 **Fan Speed Control**: Low/Medium/High/Auto
- 🔀 **Swing Control**: Toggle air direction
- ⏰ **Timer Function**: Auto shut-off after set duration
- 📊 **Historical Data Logging**: CSV-based data storage
- 📈 **Real-time Charts**: Temperature/humidity trends

### Advanced Features
- 📅 **Smart Scheduling**: Set automatic on/off times (e.g., 11 PM to 5 AM)
- 🌙 **Overnight Schedule Support**: Handles schedules that span midnight
- 🔄 **Bidirectional Sync**: Changes from Arduino or web interface sync instantly
- 📱 **Mobile Responsive**: Optimized for phones, tablets, and Kindle devices
- 🚀 **Sub-second Latency**: Fast polling ensures near-instant updates
- 🔐 **Source Tracking**: Prevents setting conflicts between control interfaces

### Display Features
- **Display 1 (Room Conditions)**: Current temperature and humidity
- **Display 2 (AC Settings)**: Power status, set temperature, fan speed, timer, swing
- **Interactive Menu**: Rotary encoder navigation with visual feedback

## 🛠️ Hardware Requirements

### Arduino Uno R4 Minima Setup
- **Board**: Arduino Uno R4 Minima
- **Display 1** (Room Conditions - SPI):
  - MOSI: D11, CLK: D13, DC: D9, CS: D10, RST: D8
- **Display 2** (AC Settings - SPI):
  - MOSI: D11, CLK: D13, DC: D7, CS: D6, RST: D5
- **DHT22 Sensor**: Data → D2
- **Rotary Encoder**: CLK → D3, DT → D4, SW → A1
- **Power Button**: A0 (with pull-up resistor)
- **Serial Connection**: TX (D1) → ESP8266 RX, RX (D0) → ESP8266 TX

### ESP8266 NodeMCU Setup
- **Board**: ESP8266 NodeMCU
- **IR LED (3-pin)**: Signal → D2 (GPIO4), VCC → 5V (via transistor), GND → GND
- **Serial Connection**: RX → Arduino TX, TX → Arduino RX, GND → Arduino GND
- **Power**: 5V via USB or external supply

### Additional Components
- 2x SSD1306 128x64 OLED Displays (SPI)
- 1x DHT22 Temperature/Humidity Sensor
- 1x Rotary Encoder (with button)
- 1x IR LED (940nm recommended)
- 1x NPN Transistor (e.g., 2N2222) for IR LED driver
- 1x 100Ω Resistor (for IR LED)
- 1x Push Button
- Jumper wires and breadboard
- 2x USB cables (for programming)

## 💻 Software Requirements

### Firmware Development
- **PlatformIO**: Latest version
- **Arduino Framework**: Compatible with R4 Minima
- **ESP8266 Framework**: Arduino core for ESP8266

### Required Libraries (Auto-installed by PlatformIO)
- `U8g2` - OLED display driver
- `DHT sensor library` - Temperature/humidity sensor
- `ArduinoJson` - JSON serialization
- `IRremoteESP8266` - IR transmission (Daikin protocol)
- `ESP8266WiFi` - WiFi connectivity
- `ESP8266HTTPClient` - HTTP requests

### Server
- **Python**: 3.9 or higher
- **Flask**: 3.0+
- **pip packages**: See `requirements.txt`

## 📦 Installation & Setup

### 1. Clone the Repository
```bash
git clone https://github.com/yourusername/IOT-HVAC-System.git
cd IOT-HVAC-System
```

### 2. Firmware Setup

#### Install PlatformIO
```bash
# Using pip
pip install platformio

# Or install PlatformIO IDE extension in VS Code
```

#### Configure WiFi Credentials
Edit `firmware/src/main.cpp` (ESP8266 section):
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

#### Update Server IP Address
Replace `192.168.29.64` with your server's IP address in `main.cpp`:
```cpp
const char* serverUrl = "http://YOUR_SERVER_IP:5001/api/data";
const char* commandUrl = "http://YOUR_SERVER_IP:5001/api/hvac/command";
const char* scheduleStatusUrl = "http://YOUR_SERVER_IP:5001/api/schedule/status";
```

#### Upload Firmware
```bash
cd firmware

# Upload to Arduino R4 Minima
pio run -e uno_r4_minima --target upload

# Upload to ESP8266 NodeMCU
pio run -e nodemcuv2 --target upload
```

### 3. Server Setup

#### Install Dependencies
```bash
cd server
pip install -r requirements.txt
```

#### Run the Server
```bash
python server.py
```

The server will start on `http://0.0.0.0:5001`

### 4. Access the Web Interface

Open your browser and navigate to:
- **Dashboard**: `http://YOUR_SERVER_IP:5001/`
- **Control Panel**: `http://YOUR_SERVER_IP:5001/control`

## 📱 Usage

### Local Control (Arduino)
1. **Power**: Press the dedicated power button (A0)
2. **Navigate**: Rotate the encoder to highlight different settings
3. **Edit**: Press the encoder button to enter edit mode
4. **Adjust**: Rotate to change value
5. **Confirm**: Press encoder button again to save

### Web Control
1. **Dashboard** (`/`):
   - View real-time temperature and humidity
   - Monitor current HVAC settings
   - See historical data charts

2. **Control Panel** (`/control`):
   - Toggle AC power on/off
   - Adjust temperature (16-30°C)
   - Change mode (Cool/Heat/Fan/Dry/Auto)
   - Set fan speed (Low/Medium/High/Auto)
   - Configure timer (0-720 minutes)
   - Toggle swing on/off
   - **Schedule** on/off times

### Scheduling
1. Go to Control Panel
2. Scroll to "Schedule (Auto On/Off)" section
3. Enable the schedule
4. Set start time (e.g., 23:00 for 11 PM)
5. Set end time (e.g., 05:00 for 5 AM)
6. Schedule will automatically turn AC on/off at set times

**Note**: Overnight schedules (e.g., 11 PM - 5 AM) are fully supported.

## 🔌 API Documentation

### Base URL
```
http://YOUR_SERVER_IP:5001
```

### Endpoints

#### POST `/api/data`
Submit sensor data and HVAC settings from ESP8266.

**Request Body**:
```json
{
  "temperature": 25.5,
  "humidity": 60.2,
  "hvac": {
    "power": "on",
    "set_temp": 24,
    "mode": "cool",
    "fan_speed": "medium",
    "timer": 0,
    "swing": "on"
  }
}
```

**Response**:
```json
{
  "status": "success",
  "timestamp": "2025-10-24 04:00:00"
}
```

#### GET `/api/current`
Get current sensor readings.

**Response**:
```json
{
  "temperature": 25.5,
  "humidity": 60.2,
  "timestamp": "2025-10-24 04:00:00"
}
```

#### GET `/api/hvac`
Get current HVAC settings.

**Response**:
```json
{
  "power": "on",
  "set_temp": 24,
  "mode": "cool",
  "fan_speed": "medium",
  "timer": 0,
  "swing": "on",
  "source": "arduino",
  "timestamp": "2025-10-24 04:00:00"
}
```

#### POST `/api/hvac/update`
Update HVAC settings from web interface.

**Request Body**:
```json
{
  "power": "on",
  "set_temp": 22,
  "mode": "cool",
  "fan_speed": "high"
}
```

**Response**:
```json
{
  "status": "success",
  "settings": { /* updated settings */ }
}
```

#### GET `/api/hvac/command`
ESP8266 polls this endpoint for web-initiated commands.

**Response**: Same as `/api/hvac`

#### GET `/api/schedule`
Get current schedule settings.

**Response**:
```json
{
  "enabled": true,
  "start_time": "23:00",
  "end_time": "05:00",
  "timestamp": "2025-10-24 04:00:00"
}
```

#### POST `/api/schedule/update`
Update schedule settings.

**Request Body**:
```json
{
  "enabled": true,
  "start_time": "23:00",
  "end_time": "05:00"
}
```

#### GET `/api/schedule/status`
Check if AC should be on or off based on schedule.

**Response**:
```json
{
  "schedule_active": true,
  "should_be_on": false,
  "current_time": "06:30",
  "start_time": "23:00",
  "end_time": "05:00"
}
```

#### GET `/api/history`
Get last 50 historical data points.

**Response**:
```json
[
  {
    "Timestamp": "2025-10-24 04:00:00",
    "Temperature": "25.5",
    "Humidity": "60.2"
  },
  ...
]
```

## 📁 Project Structure

```
IOT-HVAC-System/
├── firmware/                   # Microcontroller code
│   ├── src/
│   │   └── main.cpp           # Unified Arduino/ESP8266 firmware
│   ├── include/               # Header files
│   ├── lib/                   # Local libraries
│   └── platformio.ini         # PlatformIO configuration
│
├── server/                    # Flask web server
│   ├── server.py             # Main server application
│   ├── templates/            # HTML templates
│   │   ├── index.html        # Dashboard
│   │   └── control.html      # Control panel
│   └── requirements.txt      # Python dependencies
│
├── docs/                     # Documentation
│   ├── HARDWARE_SETUP.md    # Hardware wiring guide
│   ├── API.md               # Detailed API documentation
│   └── TROUBLESHOOTING.md   # Common issues and solutions
│
├── .gitignore               # Git ignore rules
├── LICENSE                  # MIT License
└── README.md               # This file
```

## ⚙️ Configuration

### Changing AC Brand
The system currently uses the Daikin IR protocol. To change to another brand:

1. Edit `firmware/src/main.cpp` (ESP8266 section)
2. Replace the IR library includes:
```cpp
#include <ir_Daikin.h>  // Change to your brand (e.g., ir_Samsung.h)
IRDaikinESP ac(kIrLed);  // Change to your brand class
```

3. Update the helper functions accordingly

Supported brands: Daikin, Mitsubishi, Samsung, LG, Panasonic, Hitachi, and more.

### Adjusting Sync Intervals
Edit `main.cpp` (ESP8266 section):
```cpp
const unsigned long serverUpdateInterval = 2000;     // Post to server
const unsigned long commandCheckInterval = 500;      // Check for commands
const unsigned long scheduleCheckInterval = 30000;   // Check schedule
```

### Temperature Range
Default range is 16-30°C. Modify in both Arduino and ESP8266 sections if needed.

## 🔧 Troubleshooting

### ESP8266 Won't Connect to WiFi
- Check SSID and password in `main.cpp`
- Verify WiFi is 2.4GHz (ESP8266 doesn't support 5GHz)
- Check signal strength
- Try power cycling the ESP8266

### Settings Reverting Automatically
- Ensure latest firmware is uploaded (includes source tracking fix)
- Check that ESP8266 polling interval is set to 500ms
- Verify server is responding correctly

### High Latency Between Control Changes
- Confirm ESP8266 firmware has reduced intervals (2s, 500ms)
- Check network congestion
- Verify server is on the same local network

### IR LED Not Working
- Check GPIO4 (D2) connection
- Verify transistor is correctly wired
- Test IR LED with phone camera (should see flashing)
- Confirm IR protocol matches your AC brand

### Displays Not Updating
- Verify SPI connections for both displays
- Check CS, DC, and RST pins match code
- Ensure displays are powered correctly
- Check U8g2 library version

### Serial Communication Issues
- Baud rates must match: Arduino Serial1 = ESP8266 Serial = 9600
- Verify TX/RX crossover connection
- Use a logic level shifter if available (R4 TX is 5V, ESP RX is 3.3V)

### Web Interface Not Loading
- Check server is running (`python server.py`)
- Verify firewall allows port 5001
- Confirm you're accessing correct IP address
- Try accessing from server's localhost first

## 🤝 Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- U8g2 library by olikraus
- DHT sensor library by Adafruit
- IRremoteESP8266 library by crankyoldgit
- Flask framework by Pallets
- PlatformIO development platform

## 📞 Support

For issues, questions, or suggestions:
- Open an issue on GitHub
- Check the [Troubleshooting](#troubleshooting) section
- Review the [API Documentation](#api-documentation)

---

**Made with ❤️ for energy-efficient home automation**
