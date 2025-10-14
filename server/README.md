# Local ESP Data Receiver

This small Flask app accepts JSON POSTs from ESP8266/NodeMCU devices on your LAN and displays the recent payloads in a web page.

Quick start

1. Create and activate a Python virtualenv (optional but recommended):

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

2. Run the server:

```bash
python server.py
```

3. Open the page in a browser on the same machine or another device on your LAN:

http://<host-ip>:5000/

NodeMCU example

See `nodemcu_example.ino` for a minimal sketch that sends JSON to the server.

Testing with curl

```bash
curl -X POST -H "Content-Type: application/json" -d '{"temp":23.5,"hum":45}' http://localhost:5000/data
```

Port already in use?

If port 5000 is already used by another program, you have options:

- Run the server on a different port:

```bash
python server.py --port 8000
# or set an environment variable
SERVER_PORT=8000 python server.py
```

- Find which process is using port 5000 and stop it (macOS / Linux):

```bash
# show process using port 5000
lsof -i :5000

# kill the process (replace <PID> with the process id shown)
kill <PID>
# or force kill if needed
kill -9 <PID>
```

On macOS you can also check Activity Monitor or use `ps` to find and stop the process.

