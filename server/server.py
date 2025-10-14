from flask import Flask, request, jsonify, render_template
from threading import Lock
from collections import deque
import logging

app = Flask(__name__)

# Keep a rolling buffer of the latest posts (max 100)
data_lock = Lock()
recent = deque(maxlen=100)

logging.basicConfig(level=logging.INFO)


@app.route('/')
def index():
    return render_template('index.html')


@app.route('/data', methods=['POST'])
def receive_data():
    """Endpoint for NodeMCU/ESP8266 to POST JSON payloads."""
    if not request.is_json:
        return jsonify({'error': 'expected application/json'}), 400
    payload = request.get_json()
    entry = {
        'payload': payload,
        'remote_addr': request.remote_addr
    }
    with data_lock:
        recent.appendleft(entry)
    logging.info('Received data from %s: %s', request.remote_addr, payload)
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
