# Industrial Internet of Things (ESP32 + DHT11 + MQTT + REST)

An ESP32-based reference project that reads temperature and humidity from a DHT11 sensor, publishes data to MQTT, and exposes a lightweight REST API to adjust runtime behavior. Designed for quick lab setups and IIoT coursework.

Key traits:
- ESP32 (Arduino framework, PlatformIO)
- DHT11 temperature and humidity sensor
- MQTT client with reconnect handling and heartbeat
- Simple REST API for runtime configuration (starts only after Wi‑Fi is connected)
- JSON payloads suitable for basic dashboards or backends


## Features
- Wi‑Fi connect and auto‑reconnect helpers
- MQTT client (PubSubClient)
  - Connects with credentials, auto‑reconnects
  - Heartbeat to a status topic every 5 seconds
  - Publishes sensor values and structured JSON readings
- REST API (Arduino WebServer)
  - Endpoint: /config (GET, POST, OPTIONS)
  - CORS enabled (for simple browser tooling)
  - Server start is deferred until Wi‑Fi is connected
- Time sync via NTP for ISO‑8601 timestamps (UTC)


## Hardware
- ESP32 board: esp32vn-iot-uno (configurable in platformio.ini)
- DHT11 sensor
  - VCC → 3.3V
  - GND → GND
  - DATA → GPIO 14 (configurable in include/settings.h via DHT11_PIN)

Note: Some DHT11 breakout boards need a pull‑up resistor (usually included). Consult your module’s pinout.


## Project structure
- include/…
  - settings.h: Wi‑Fi, MQTT, REST defaults, pins, topics
  - rest_api.h, wifi_connect.h, mqtt_connect.h, dht_sensor.h
- src/…
  - main.cpp: setup/loop orchestration
  - wifi_connect.cpp: Wi‑Fi connect/reconnect
  - mqtt_connect.cpp: MQTT client setup/reconnect helpers
  - dht_sensor.cpp: DHT11 readout
  - rest_api.cpp: REST server and runtime config
- platformio.ini: PlatformIO environment and library deps


## Quick start
1) Prerequisites
- Install VS Code + PlatformIO IDE extension, or use CLion with PlatformIO plugin
- Alternatively, install PlatformIO Core (CLI)

2) Clone the project
- Place the folder where your PlatformIO can access it

3) Configure your environment in include/settings.h
- Wi‑Fi: WIFI_SSID, WIFI_PASSWORD
- MQTT: MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD
- Topics: MQTT_BASE_TOPIC, MQTT_GROUP_NAME, MQTT_TOPIC_TEMPERATURE_STATE, MQTT_TOPIC_HUMIDITY_STATE
- REST: port (REST_API_PORT), endpoint path (REST_API_CONFIG_PATH)
- Sensor pins and IDs: DHT11_PIN, SENSOR_ID, HUM_SENSOR_ID, units

4) Build and upload (via PlatformIO)
- From IDE: select env "esp32vn-iot-uno" and click Upload
- From CLI:
  - pio run -e esp32vn-iot-uno
  - pio run -e esp32vn-iot-uno -t upload
- Serial monitor (115200 baud):
  - pio device monitor -b 115200


## Runtime behavior
- On boot:
  - Initializes DHT11 and attempts Wi‑Fi connection (10s timeout)
  - Initializes REST API routes
  - REST server will only start (bind port) once Wi‑Fi is connected
  - Configures NTP and waits briefly for time (to stamp JSON)
  - Sets up MQTT; subscribes to command topic and publishes "online"
- In loop():
  - Auto‑reconnect for Wi‑Fi and MQTT
  - REST server handles HTTP requests when running
  - Heartbeat message to status topic every 5s
  - Periodic DHT11 read; publishes simple status line and JSON readings


## MQTT topics and payloads
- Base topic (configurable): MQTT_BASE_TOPIC (default: iiot/group/test/sensor)
  - Status: {base}/status
  - Command: {base}/cmd
- AsyncAPI-compatible channels (from settings.h):
  - Temperature state: iiot/group/{GROUP}/sensor/temperature/state
  - Humidity state:    iiot/group/{GROUP}/sensor/humidity/state

Examples (published when enabled):
- Simple status line (human-readable):
  - Topic: {base}/status
  - Payload: T=23.1C,H=45%

- TemperatureReading JSON:
  {
    "timestamp": "2025-01-01T12:00:00Z",  
    "sensor_id": "temp-1",                 
    "value": 23.1,                          
    "unit": "°C",                          
    "status": "ok"
  }

- HumidityReading JSON (same structure):
  {
    "timestamp": "2025-01-01T12:00:00Z",
    "sensor_id": "hum-1",
    "value": 45.0,
    "unit": "%",
    "status": "ok"
  }

Notes:
- If NTP time is not yet available, timestamp may be an empty string on first messages.
- SENSOR_ID and HUM_SENSOR_ID are configurable in include/settings.h.


## REST API
- Base URL: http://<esp32-ip>:<REST_API_PORT>
- Endpoint: REST_API_CONFIG_PATH (default: /config)
- Methods: OPTIONS, GET, POST
- CORS: enabled (Access-Control-Allow-Origin: *)

GET /config → 200 application/json
{
  "status": "online",
  "sendIntervalMs": 2000,
  "publishTemperature": true,
  "publishHumidity": true,
  "tempSensorId": "temp-1",
  "humSensorId": "hum-1"
}

POST /config → 200 application/json (echoes effective config)
Accepts any subset of fields:
{
  "status": "maint",
  "sendIntervalMs": 5000,
  "publishTemperature": true,
  "publishHumidity": false,
  "tempSensorId": "lab-temp",
  "humSensorId": "lab-hum"
}

Rules and notes:
- sendIntervalMs minimum enforced: 1000 ms
- Changing status sets an internal flag to publish the new status once on MQTT
- Server only starts after Wi‑Fi connects; until then, requests won’t be served


## Configuration reference (include/settings.h)
- Wi‑Fi: WIFI_SSID, WIFI_PASSWORD
- MQTT: MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD
- Topics: MQTT_BASE_TOPIC, MQTT_TOPIC_STATUS, MQTT_TOPIC_COMMAND
- Group/topic for state channels: MQTT_GROUP_NAME, MQTT_TOPIC_TEMPERATURE_STATE, MQTT_TOPIC_HUMIDITY_STATE
- REST: REST_API_PORT (default 80), REST_API_CONFIG_PATH (default "/config")
- Defaults exposed via REST: REST_DEFAULT_STATUS, REST_DEFAULT_SEND_INTERVAL_MS, REST_DEFAULT_PUBLISH_TEMPERATURE, REST_DEFAULT_PUBLISH_HUMIDITY
- Sensor: DHT11_PIN (default 14), SENSOR_ID, SENSOR_UNIT, HUM_SENSOR_ID, HUM_SENSOR_UNIT


## Build configuration (PlatformIO)
platformio.ini sets:
- platform = espressif32
- board = esp32vn-iot-uno
- framework = arduino
- monitor_speed = 115200
- lib_deps:
  - knolleary/PubSubClient
  - adafruit/DHT sensor library
  - bblanchon/ArduinoJson

To use a different board, change the board in platformio.ini or add a new [env:<name>] section.


## Build with Docker
If you don't want to install PlatformIO locally, you can use the provided Dockerfile to build the firmware.

1) Build the image
- Windows PowerShell:
  - docker build -t iiot-esp32 .
- macOS/Linux:
  - docker build -t iiot-esp32 .

2) Run a build for the default environment (esp32vn-iot-uno)
- Windows PowerShell (mount current directory):
  - docker run --rm -v ${PWD}:/workspace -w /workspace iiot-esp32 pio run -e esp32vn-iot-uno
  - If ${PWD} does not expand correctly, try: docker run --rm -v %cd%:/workspace -w /workspace iiot-esp32 pio run -e esp32vn-iot-uno
- macOS/Linux:
  - docker run --rm -v "$PWD":/workspace -w /workspace iiot-esp32 pio run -e esp32vn-iot-uno

Notes:
- Uploading to the board from inside Docker requires passing the serial device through and appropriate permissions, e.g. on Linux: --device=/dev/ttyUSB0 --group-add dialout. On Windows and macOS Docker Desktop, USB passthrough to Linux containers is limited; typically you build in Docker but upload using PlatformIO on the host.
- The first build will download the ESP32 toolchain and libraries; subsequent builds are faster due to Docker layer caching.


## Testing
This project includes basic PlatformIO Unit Tests (Unity) that validate compile-time settings and REST config defaults.

What’s covered:
- settings.h topic/channel constants expand to the expected strings
- initRestApi() seeds DeviceConfig with defaults from settings.h (server start is deferred until Wi‑Fi connects, so the test runs without network)

Run tests from your host:
- pio test -e esp32vn-iot-uno

Run tests using the Docker image:
- docker run --rm -v ${PWD}:/workspace -w /workspace iiot-esp32 pio test -e esp32vn-iot-uno

Notes:
- Tests are in the test/ folder and use Arduino + Unity test runner.
- They don’t require actual Wi‑Fi, MQTT, or sensors; hardware peripherals are not exercised.


## Troubleshooting
- Wi‑Fi won’t connect:
  - Verify WIFI_SSID/WIFI_PASSWORD in include/settings.h
  - Check serial logs; the sketch retries and prints IP on success
- REST API not reachable:
  - Server starts only after Wi‑Fi connects; check serial for: "REST API listening on http://<ip>:<port>/config"
  - Ensure your computer is on the same network as the ESP32
- No MQTT messages:
  - Confirm broker address/port and credentials
  - Check firewall or broker permissions
  - Watch serial logs for reconnect attempts
- Empty timestamps in JSON:
  - NTP may not have synced yet; wait a few seconds after boot
- Sensor readings are erratic:
  - Verify wiring and power; ensure adequate delays (interval >= 1000 ms)


## License
This project is intended for educational and lab use. Provide attribution if you reuse significant portions.