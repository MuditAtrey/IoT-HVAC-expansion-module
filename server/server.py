from flask import Flask, request, jsonify, render_template
from threading import Lock, Thread
from collections import deque
from datetime import datetime
import logging
import time
import random

app = Flask(__name__)

# Keep a rolling buffer of the latest posts (max 100)
data_lock = Lock()
recent = deque(maxlen=100)

# Outdoor simulation values (auto-fluctuating)
outdoor_temp = 36.0
outdoor_hum = 75.0

logging.basicConfig(level=logging.INFO)


def outdoor_simulator():
    """Background thread that simulates outdoor temperature/humidity fluctuations."""
    global outdoor_temp, outdoor_hum
    while True:
        time.sleep(1)  # Update every second
        
        # Fluctuate temperature: 36°C ± 1°C with 0.1 precision
        change_temp = random.uniform(-0.1, 0.1)
        outdoor_temp += change_temp
        outdoor_temp = max(35.0, min(37.0, outdoor_temp))  # Keep within 36±1
        
        # Fluctuate humidity: 75% ± 1% with 0.1 precision
        change_hum = random.uniform(-0.1, 0.1)
        outdoor_hum += change_hum
        outdoor_hum = max(74.0, min(76.0, outdoor_hum))  # Keep within 75±1
        
        # Store outdoor values in recent buffer
        entry = {
            'type': 'encoder',  # Keep as 'encoder' for UI compatibility
            'payload': {
                'temperature': round(outdoor_temp, 1),
                'humidity': round(outdoor_hum, 1)
            },
            'remote_addr': 'auto-sim',
            'timestamp': datetime.now().isoformat()
        }
        with data_lock:
            recent.appendleft(entry)


# Start the outdoor simulator thread
simulator_thread = Thread(target=outdoor_simulator, daemon=True)
simulator_thread.start()


@app.route('/')
def index():
    return render_template('index.html')


@app.route('/indoor', methods=['POST'])
def receive_indoor_temp():
    """Handle indoor temperature and humidity updates from NodeMCU2."""
    if not request.is_json:
        return jsonify({'error': 'expected application/json'}), 400
    payload = request.get_json()
    
    # Validate temperature is within bounds
    temp = payload.get('temperature')
    if temp is not None:
        try:
            temp_float = float(temp)
            if not (-10 <= temp_float <= 40):
                return jsonify({'error': 'temperature out of bounds'}), 400
        except ValueError:
            return jsonify({'error': 'invalid temperature value'}), 400
    
    # Validate humidity is within bounds
    hum = payload.get('humidity')
    if hum is not None:
        try:
            hum_float = float(hum)
            if not (0 <= hum_float <= 100):
                return jsonify({'error': 'humidity out of bounds'}), 400
        except ValueError:
            return jsonify({'error': 'invalid humidity value'}), 400
    
    entry = {
        'type': 'indoor',
        'payload': payload,
        'remote_addr': request.remote_addr,
        'timestamp': datetime.now().isoformat()
    }
    with data_lock:
        recent.appendleft(entry)
    logging.info('Received indoor data from %s: %s', request.remote_addr, payload)
    return jsonify({'status': 'ok'}), 201


@app.route('/indoor/button', methods=['POST'])
def receive_indoor_button():
    """Handle button press events from NodeMCU2."""
    if not request.is_json:
        return jsonify({'error': 'expected application/json'}), 400
    payload = request.get_json()
    entry = {
        'type': 'button',
        'payload': payload,
        'remote_addr': request.remote_addr
    }
    with data_lock:
        recent.appendleft(entry)
    logging.info('Received button press from %s: %s', request.remote_addr, payload)
    return jsonify({'status': 'ok'}), 201


@app.route('/data/latest', methods=['GET'])
def latest():
    """Return the most recent entry (or empty) as JSON."""
    with data_lock:
        if not recent:
            return jsonify({'found': False, 'data': None})
        return jsonify({'found': True, 'data': recent[0]})


@app.route('/data/all', methods=['GET'])
def all_data():
    with data_lock:
        return jsonify({'count': len(recent), 'data': list(recent)})


if __name__ == '__main__':
    # Allow configuring host and port via environment variables or CLI args.
    import argparse
    import os

    parser = argparse.ArgumentParser(description='ESP data receiver')
    parser.add_argument('--host', default=os.environ.get('SERVER_HOST', '0.0.0.0'))
    parser.add_argument('--port', type=int, default=int(os.environ.get('SERVER_PORT', '5000')))
    parser.add_argument('--debug', action='store_true')
    args = parser.parse_args()

    bind_host = args.host
    bind_port = args.port
    debug_mode = args.debug

    logging.info('Starting server on %s:%d (debug=%s)', bind_host, bind_port, debug_mode)
    try:
        app.run(host=bind_host, port=bind_port, debug=debug_mode)
    except OSError as e:
        logging.error('Failed to start server: %s', e)
        raise
