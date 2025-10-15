from flask import Flask, request, jsonify, render_template
from threading import Lock
from collections import deque
from datetime import datetime
import logging

app = Flask(__name__)

# Keep a rolling buffer of the latest posts (max 100)
data_lock = Lock()
recent = deque(maxlen=100)

logging.basicConfig(level=logging.INFO)


@app.route('/')
def index():
    return render_template('index.html')


@app.route('/nodemcu1', methods=['POST'])
def receive_indoor_temp():
    """Handle indoor temperature and humidity updates from NodeMCU1."""
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


@app.route('/nodemcu2/encoder', methods=['POST'])
def receive_encoder():
    """Handle outdoor temperature and humidity updates from NodeMCU2 encoder."""
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
        'type': 'encoder',
        'payload': payload,
        'remote_addr': request.remote_addr,
        'timestamp': datetime.now().isoformat()
    }
    with data_lock:
        recent.appendleft(entry)
    logging.info('Received outdoor data from %s: %s', request.remote_addr, payload)
    return jsonify({'status': 'ok'}), 201


@app.route('/nodemcu2/button', methods=['POST'])
def receive_button():
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


@app.route('/nodemcu1/button', methods=['POST'])
def receive_button1():
    """Handle button press events from NodeMCU1."""
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
    logging.info('Received button press from NodeMCU1 %s: %s', request.remote_addr, payload)
    return jsonify({'status': 'ok'}), 201


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
