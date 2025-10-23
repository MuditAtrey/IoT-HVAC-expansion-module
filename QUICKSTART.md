# Quick Start Guide

Get your IoT HVAC system up and running in minutes!

## Prerequisites

- Arduino Uno R4 Minima
- ESP8266 NodeMCU
- Required components (see [Hardware Setup](docs/HARDWARE_SETUP.md))
- Computer with Python 3.9+
- WiFi network (2.4GHz)

## Step 1: Hardware Assembly (15 minutes)

1. Connect displays to Arduino (see [Hardware Setup](docs/HARDWARE_SETUP.md))
2. Connect DHT22 sensor to Arduino D2
3. Connect rotary encoder to Arduino D3/D4/A1
4. Connect power button to Arduino A0
5. Wire Arduino Serial1 to ESP8266 Serial
6. Connect IR LED to ESP8266 D2 (GPIO4) via transistor

## Step 2: Configure Firmware (5 minutes)

1. Open `firmware/src/main.cpp`

2. Update WiFi credentials (line ~573):
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

3. Update server IP address (lines ~576-579):
```cpp
const char* serverUrl = "http://YOUR_IP:5001/api/data";
const char* commandUrl = "http://YOUR_IP:5001/api/hvac/command";
// ... update all URLs
```

## Step 3: Upload Firmware (5 minutes)

```bash
cd firmware

# Upload to Arduino R4 Minima
pio run -e uno_r4_minima --target upload

# Upload to ESP8266 NodeMCU  
pio run -e nodemcuv2 --target upload
```

## Step 4: Start Server (2 minutes)

```bash
cd server
pip install Flask
python server.py
```

Server starts at `http://0.0.0.0:5001`

## Step 5: Access Web Interface

Open browser to:
- **Dashboard**: `http://YOUR_IP:5001`
- **Control**: `http://YOUR_IP:5001/control`

## Test the System

1. **Check displays**: Both OLEDs should show data
2. **Verify web**: Dashboard shows temperature/humidity
3. **Test control**: Change temperature from web interface
4. **Watch IR LED**: Should flash when sending commands (visible on phone camera)
5. **Local control**: Use rotary encoder on Arduino

## Common First-Time Issues

### "WiFi connection failed"
- Double-check SSID and password
- Ensure 2.4GHz network (not 5GHz)
- Move ESP8266 closer to router

### "Can't access web interface"
- Verify server is running (`python server.py`)
- Check firewall settings
- Use correct IP address
- Try `localhost:5001` from server machine

### "Displays blank"
- Check all SPI connections
- Verify 5V power
- Try pressing reset on Arduino

### "IR LED not working"
- Check transistor wiring
- Verify GPIO4 connection
- Test IR LED with phone camera
- Confirm AC brand matches code (default: Daikin)

## Next Steps

- Set up automated schedule in Control Panel
- Customize temperature ranges
- Add more sensor logging
- Optimize for your AC brand

---

Need more help? See [Troubleshooting](docs/TROUBLESHOOTING.md) or [Hardware Setup](docs/HARDWARE_SETUP.md).
