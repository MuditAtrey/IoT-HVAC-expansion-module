# System Architecture Documentation

## Overview

The IoT HVAC Control System uses a distributed architecture with three main components:
1. **Arduino R4 Minima** - Local interface and sensor hub
2. **ESP8266 NodeMCU** - WiFi gateway and IR controller
3. **Flask Server** - Web interface and data aggregation

## Component Details

### 1. Arduino Uno R4 Minima (Local Controller)

**Role**: Provides physical interface and real-time sensor monitoring

**Hardware**:
- Dual SSD1306 OLED displays (128x64, SPI)
- DHT22 temperature/humidity sensor
- Rotary encoder with button
- Dedicated power button
- Hardware Serial1 for ESP communication

**Software Responsibilities**:
- Read DHT22 every 2 seconds
- Update OLED displays every 100ms
- Handle rotary encoder input
- Manage local menu system
- Send sensor + HVAC data to ESP via Serial1 every 1 second
- Receive HVAC updates from ESP via Serial1

**Data Format (TX to ESP)**:
```json
{
  "roomTemp": 25.5,
  "roomHumidity": 60.2,
  "hvac": {
    "power": "on",
    "setTemp": 24,
    "mode": "cool",
    "fanSpeed": "medium",
    "timer": 0,
    "swing": "on"
  }
}
```

**Data Format (RX from ESP)**:
```json
{
  "hvac": {
    "power": "on",
    "setTemp": 22,
    "mode": "cool",
    "fanSpeed": "high",
    "timer": 0,
    "swing": "on"
  }
}
```

**Serial Communication**:
- Baud Rate: 9600
- Protocol: JSON over UART
- Hardware: Serial1 (D0=RX, D1=TX)

### 2. ESP8266 NodeMCU (WiFi Hub & IR Controller)

**Role**: Bridge between Arduino, web server, and AC unit

**Hardware**:
- ESP8266 WiFi module
- IR LED (940nm) + NPN transistor driver
- Hardware Serial for Arduino communication

**Software Responsibilities**:
- Connect to WiFi network
- Receive data from Arduino via Serial every 250ms check
- Post sensor data to server every 2 seconds
- Poll server for web commands every 500ms
- Check schedule status every 30 seconds
- Send IR commands to AC unit when settings change
- Forward web commands to Arduino via Serial

**Communication Protocols**:

**Serial (Arduino ↔ ESP8266)**:
- Baud Rate: 9600
- Format: JSON
- Direction: Bidirectional

**HTTP (ESP8266 ↔ Server)**:
- Protocol: HTTP/1.1
- Format: JSON
- Endpoints:
  - POST `/api/data` - Upload sensor + HVAC data
  - GET `/api/hvac/command` - Fetch web commands
  - GET `/api/schedule/status` - Check schedule

**IR (ESP8266 → AC Unit)**:
- Protocol: Daikin (configurable)
- Library: IRremoteESP8266
- Pin: GPIO4 (D2)

**Timing Intervals**:
```cpp
serverUpdateInterval     = 2000ms   // POST data to server
commandCheckInterval     = 500ms    // GET commands from server
scheduleCheckInterval    = 30000ms  // Check schedule status
arduinoSendInterval      = 250ms    // Push updates to Arduino
```

**Immediate Push Events**:
- Arduino changes → Immediate POST to server
- Web changes → Immediate TX to Arduino

### 3. Flask Server (Web Backend)

**Role**: Web interface, API, and data storage

**Technology Stack**:
- Flask 3.0+ (Python web framework)
- Jinja2 templates
- CSV file storage
- In-memory caching

**Endpoints**:
- `GET /` - Dashboard HTML
- `GET /control` - Control panel HTML
- `POST /api/data` - Receive sensor data from ESP
- `GET /api/current` - Get latest sensor readings
- `GET /api/hvac` - Get current HVAC settings
- `POST /api/hvac/update` - Update HVAC settings from web
- `GET /api/hvac/command` - Polled by ESP for commands
- `GET /api/schedule` - Get schedule settings
- `POST /api/schedule/update` - Update schedule
- `GET /api/schedule/status` - Check if AC should be on/off
- `GET /api/history` - Get historical data (last 50 entries)

**Data Storage**:
- **In-Memory**: Latest sensor readings and HVAC settings
- **Persistent**: CSV file (`sensor_data.csv`)
- **Thread-Safe**: Uses threading.Lock for CSV writes

**Source Tracking**:
```python
hvac_settings['source'] = 'arduino'   # Set by ESP when Arduino changes
hvac_settings['source'] = 'web'       # Set by web interface
hvac_settings['source'] = 'schedule'  # Set by schedule automation
hvac_settings['source'] = 'synced'    # After successful sync
```

## Data Flow Diagrams

### Sensor Data Flow (Arduino → Server)

```
┌─────────┐    Serial     ┌─────────┐    HTTP POST    ┌─────────┐
│ DHT22   │─────┐         │         │                 │         │
└─────────┘     ▼         │         │   /api/data     │ Flask   │
           ┌────────┐     │ ESP8266 │────────────────►│ Server  │
┌─────────┐│        │     │         │   JSON          │         │
│ Arduino ││ Serial │────►│         │                 │         │
│ R4      ││  JSON  │9600 │         │                 │         │
└─────────┘│        │     └─────────┘                 └─────────┘
           └────────┘
              1Hz                        2Hz
```

### Control Command Flow (Web → AC Unit)

```
┌─────────┐    HTTP POST   ┌─────────┐    HTTP GET    ┌─────────┐
│ Web     │                │         │                 │         │
│ Browser │───────────────►│ Flask   │◄────────────────│ ESP8266 │
│         │ /api/hvac/     │ Server  │ /api/hvac/     │         │
└─────────┘   update       └─────────┘   command      └─────────┘
                                              │ Poll 500ms    │
                                              │               │
                                              │               ▼
                                              │          ┌────────┐
                                              │          │   IR   │
                                              │          │Transmit│
                                              │          └───┬────┘
                                              │              │
                                              │              ▼
                                              │          ┌────────┐
                                              │          │   AC   │
                                              │          │  Unit  │
                                              │          └────────┘
                                              │
                                              ▼
                                         ┌─────────┐
                                         │ Arduino │
                                         │   UI    │
                                         └─────────┘
                                           Serial
```

### Schedule Automation Flow

```
┌─────────┐
│  Flask  │
│ Server  │
└────┬────┘
     │
     │ GET /api/schedule/status (every 30s)
     │
     ▼
┌─────────┐      Check Time      ┌──────────┐
│ ESP8266 │────────────────────►│ Schedule │
│         │                      │  Logic   │
└────┬────┘                      └─────┬────┘
     │                                 │
     │                                 ▼
     │                          should_be_on?
     │                                 │
     │                           ┌─────┴─────┐
     │                           │ Yes   No  │
     │                           └─────┬─────┘
     │                                 │
     ▼                                 ▼
Turn AC On/Off                   No Change
```

## Synchronization Strategy

### Problem: Race Conditions
Multiple control sources (Arduino, Web, Schedule) can conflict.

### Solution: Source Tracking + Timestamp
Each HVAC update includes:
- `source`: Origin of change (arduino/web/schedule)
- `timestamp`: When change occurred

### Conflict Resolution Rules:
1. **Arduino changes**: Marked as `source='arduino'`, immediately posted to server
2. **Web changes**: Marked as `source='web'`, polled by ESP every 500ms
3. **Schedule changes**: Marked as `source='schedule'`, checked every 30 seconds
4. **Sync completion**: After successful propagation, marked as `source='synced'`

### Propagation Path:
- **Arduino → Server**: Serial (1Hz) → HTTP POST (2Hz)
- **Web → Arduino**: HTTP POST → ESP polls (500ms) → Serial (immediate)
- **Schedule → All**: Server decides → ESP acts → propagates to Arduino/AC

## Performance Characteristics

### Latency Measurements:
- **Arduino → Web display**: ~2-4 seconds
- **Web → Arduino display**: ~0.5-1 second
- **Arduino → AC unit**: ~1-2 seconds
- **Schedule trigger**: ~30-60 seconds (check interval)

### Network Traffic:
- **ESP → Server**: ~120 requests/minute (POST every 2s + GET every 500ms)
- **Web → Server**: 12 requests/minute per client (5s polling)
- **Data volume**: ~1KB per request, ~120KB/minute total

### Memory Usage:
- **Arduino**: ~8KB RAM (displays + buffers)
- **ESP8266**: ~25KB RAM (WiFi + JSON + IR)
- **Server**: ~50MB (Flask + data cache)

## Scalability Considerations

### Current Limitations:
- Single AC unit per system
- No authentication/authorization
- In-memory data (resets on server restart)
- CSV append-only (no data management)

### Potential Improvements:
1. **Multi-unit support**: Add unit ID to all communications
2. **Database backend**: Replace CSV with SQLite/PostgreSQL
3. **Authentication**: Add user accounts and API keys
4. **MQTT protocol**: Replace HTTP polling with pub/sub
5. **Mobile app**: Native iOS/Android applications
6. **Cloud integration**: Remote access via cloud bridge

## Security Considerations

### Current Security:
- ⚠️ No authentication
- ⚠️ No encryption (HTTP)
- ⚠️ No input validation
- ⚠️ No rate limiting

### Recommended Mitigations:
1. Run on isolated local network
2. Use firewall rules to restrict access
3. Enable HTTPS with self-signed certificates
4. Add basic authentication to Flask routes
5. Validate all JSON inputs
6. Implement rate limiting on API endpoints

## Debugging & Monitoring

### Serial Output (Arduino):
```
Arduino R4 Minima HVAC Controller
ESP Serial: RX=D0, TX=D1 @ 9600 baud
-> ESP: {"roomTemp":25.5,"roomHumidity":60.2,"hvac":{...}}
<- ESP: {"hvac":{...}}
```

### Serial Output (ESP8266):
```
ESP8266 HVAC Hub
WiFi connected!
IP address: 192.168.1.100
Sending to server: {"temperature":25.5,"humidity":60.2,"hvac":{...}}
Server response: 200
Settings updated from web
Updating AC...
AC updated successfully
```

### Server Logs:
```
192.168.1.100 - - [24/Oct/2025 04:00:00] "POST /api/data HTTP/1.1" 200 -
192.168.1.100 - - [24/Oct/2025 04:00:00] "GET /api/hvac/command HTTP/1.1" 200 -
127.0.0.1 - - [24/Oct/2025 04:00:00] "GET /api/current HTTP/1.1" 200 -
```

## Testing Strategy

### Unit Testing:
- Arduino: Test display, encoder, DHT22 separately
- ESP8266: Test WiFi, Serial, IR separately
- Server: Test each API endpoint with curl/Postman

### Integration Testing:
1. Arduino → ESP Serial communication
2. ESP → Server HTTP communication
3. Server → ESP command propagation
4. End-to-end: Arduino change → Web display
5. End-to-end: Web change → Arduino display
6. Schedule trigger → AC control

### Performance Testing:
- Network latency measurements
- Memory usage profiling
- Long-term stability testing (24+ hours)
- Multiple client load testing

---

For implementation details, see the source code in `firmware/src/main.cpp` and `server/server.py`.
