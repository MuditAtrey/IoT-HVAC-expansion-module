from flask import Flask, render_template, jsonify, request
import csv
import os
from datetime import datetime
from threading import Lock

app = Flask(__name__)

# File to store data
DATA_FILE = 'sensor_data.csv'
csv_lock = Lock()

# In-memory storage for latest values
latest_data = {
    'temperature': None,
    'humidity': None,
    'timestamp': None
}

# In-memory storage for HVAC settings
hvac_settings = {
    'power': None,          # on/off
    'set_temp': None,       # target temperature
    'mode': None,           # cool/heat/fan/dry/auto
    'fan_speed': None,      # low/medium/high/auto
    'timer': None,          # timer in minutes
    'swing': None,          # swing on/off (added feature)
    'timestamp': None
}

# Schedule settings
schedule_settings = {
    'enabled': False,       # whether schedule is active
    'start_time': '23:00',  # time to turn on (24-hour format)
    'end_time': '05:00',    # time to turn off (24-hour format)
    'timestamp': None
}

# Initialize CSV file if it doesn't exist
def init_csv():
    if not os.path.exists(DATA_FILE):
        with open(DATA_FILE, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['Timestamp', 'Temperature', 'Humidity'])

init_csv()

@app.route('/')
def index():
    """Serve the main webpage"""
    return render_template('index.html')

@app.route('/control')
def control():
    """Serve the control webpage"""
    return render_template('control.html')

@app.route('/api/data', methods=['POST'])
def receive_data():
    """Receive data from ESP8266"""
    try:
        data = request.get_json()
        temperature = data.get('temperature')
        humidity = data.get('humidity')
        
        if temperature is None or humidity is None:
            return jsonify({'error': 'Missing temperature or humidity'}), 400
        
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        
        # Update latest data
        latest_data['temperature'] = temperature
        latest_data['humidity'] = humidity
        latest_data['timestamp'] = timestamp
        
        # Update HVAC settings if provided
        if 'hvac' in data:
            hvac = data['hvac']
            hvac_settings['power'] = hvac.get('power')
            hvac_settings['set_temp'] = hvac.get('set_temp')
            hvac_settings['mode'] = hvac.get('mode')
            hvac_settings['fan_speed'] = hvac.get('fan_speed')
            hvac_settings['timer'] = hvac.get('timer')
            hvac_settings['swing'] = hvac.get('swing')
            hvac_settings['timestamp'] = timestamp
            # Mark origin so ESP doesn't re-apply stale web commands
            hvac_settings['source'] = 'arduino'
        
        # Save to CSV (this happens every 15 seconds from ESP8266)
        with csv_lock:
            with open(DATA_FILE, 'a', newline='') as f:
                writer = csv.writer(f)
                writer.writerow([timestamp, temperature, humidity])
        
        return jsonify({'status': 'success', 'timestamp': timestamp}), 200
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/current', methods=['GET'])
def get_current():
    """Get current sensor values (called every 5 seconds from webpage)"""
    return jsonify(latest_data)

@app.route('/api/hvac', methods=['GET'])
def get_hvac():
    """Get current HVAC settings"""
    return jsonify(hvac_settings)

@app.route('/api/hvac/update', methods=['POST'])
def update_hvac():
    """Update HVAC settings from web control"""
    try:
        data = request.get_json()
        
        # Update settings
        if 'power' in data:
            hvac_settings['power'] = data['power']
        if 'set_temp' in data:
            # Validate temperature range
            temp = int(data['set_temp'])
            if 16 <= temp <= 30:
                hvac_settings['set_temp'] = temp
            else:
                return jsonify({'error': 'Temperature must be between 16°C and 30°C'}), 400
        if 'mode' in data:
            hvac_settings['mode'] = data['mode']
        if 'fan_speed' in data:
            hvac_settings['fan_speed'] = data['fan_speed']
        if 'timer' in data:
            hvac_settings['timer'] = data['timer']
        if 'swing' in data:
            hvac_settings['swing'] = data['swing']
        
        hvac_settings['timestamp'] = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        hvac_settings['source'] = 'web'  # Mark that change came from web
        
        return jsonify({'status': 'success', 'settings': hvac_settings}), 200
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/hvac/command', methods=['GET'])
def get_hvac_command():
    """ESP8266 polls this endpoint to get settings (for control from web)"""
    return jsonify(hvac_settings)

@app.route('/api/schedule', methods=['GET'])
def get_schedule():
    """Get current schedule settings"""
    return jsonify(schedule_settings)

@app.route('/api/schedule/update', methods=['POST'])
def update_schedule():
    """Update schedule settings from web"""
    try:
        data = request.get_json()
        
        if 'enabled' in data:
            schedule_settings['enabled'] = bool(data['enabled'])
        if 'start_time' in data:
            # Validate time format (HH:MM)
            try:
                datetime.strptime(data['start_time'], '%H:%M')
                schedule_settings['start_time'] = data['start_time']
            except ValueError:
                return jsonify({'error': 'Invalid start_time format. Use HH:MM (24-hour)'}), 400
        if 'end_time' in data:
            # Validate time format (HH:MM)
            try:
                datetime.strptime(data['end_time'], '%H:%M')
                schedule_settings['end_time'] = data['end_time']
            except ValueError:
                return jsonify({'error': 'Invalid end_time format. Use HH:MM (24-hour)'}), 400
        
        schedule_settings['timestamp'] = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        
        return jsonify({'status': 'success', 'schedule': schedule_settings}), 200
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/schedule/status', methods=['GET'])
def get_schedule_status():
    """Check if AC should be on or off based on schedule"""
    try:
        if not schedule_settings['enabled']:
            return jsonify({
                'schedule_active': False,
                'should_be_on': None,
                'message': 'Schedule is disabled'
            })
        
        now = datetime.now()
        current_time = now.strftime('%H:%M')
        
        start = schedule_settings['start_time']
        end = schedule_settings['end_time']
        
        # Handle overnight schedules (e.g., 23:00 to 05:00)
        if start > end:
            # Overnight schedule
            should_be_on = current_time >= start or current_time < end
        else:
            # Same-day schedule
            should_be_on = start <= current_time < end
        
        return jsonify({
            'schedule_active': True,
            'should_be_on': should_be_on,
            'current_time': current_time,
            'start_time': start,
            'end_time': end
        })
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/history', methods=['GET'])
def get_history():
    """Get historical data from CSV"""
    history = []
    
    with csv_lock:
        if os.path.exists(DATA_FILE):
            with open(DATA_FILE, 'r') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    history.append(row)
    
    # Return last 50 entries, most recent first
    return jsonify(history[-50:][::-1])

if __name__ == '__main__':
    # Run on all interfaces so ESP8266 can connect
    app.run(host='0.0.0.0', port=5001, debug=True)
