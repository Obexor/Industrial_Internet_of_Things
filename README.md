# IIoT - MA-MECH-24-BB - Obwexer - Poell

This repository contains a lightweight Industrial Internet of Things (IIoT) firmware and tooling for an ESP32-based lab project. The device reads temperature and humidity from a DHT11 sensor and publishes JSON messages to MQTT topics suitable for downstream processing and dashboards. Runtime behavior (status, send interval, which sensors to publish, and sensor IDs) can be inspected and changed via a simple REST API endpoint on the ESP32.

For quick visualization and local testing, an optional Docker Compose stack is provided (Telegraf → InfluxDB → Grafana) with a prebuilt dashboard. All connectivity, topics, and defaults are configured in include/settings.h so you can adapt the firmware to your Wi‑Fi and MQTT broker in minutes.

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


## Configure include/settings.h (step-by-step)

Before building/flashing, open include/settings.h and set the basic connectivity and topic settings for your environment.

1) Wi‑Fi (required)
- WIFI_SSID and WIFI_PASSWORD

2) MQTT broker (required)
- MQTT_BROKER and MQTT_PORT
- Optionally set MQTT_USERNAME and MQTT_PASSWORD if your broker requires auth
- Optionally change MQTT_CLIENT_ID (must be unique per device)

3) REST API defaults (optional)
- REST_API_PORT and REST_API_CONFIG_PATH
- REST_DEFAULT_* values (status, send interval, publish toggles)

4) Hardware pin
- DHT11_PIN should match your wiring (GPIO number on ESP32)

Example snippet (edit placeholders):

```c
// Wi‑Fi
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"

// MQTT
#define MQTT_BROKER "192.168.1.10"   // or broker.hivemq.com
#define MQTT_PORT xxxx                // default 1883
#define MQTT_USERNAME ""            // leave empty if not required
#define MQTT_PASSWORD ""

// Hardware pin
#define DHT11_PIN 14
```

Notes:
- If you change REST_DEFAULT_SEND_INTERVAL_MS, minimum 1000 ms is enforced at runtime.
- The firmware publishes a one-time status message on MQTT when status changes via REST.
- The HTTP server starts only after Wi‑Fi connects.

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


## Build and Upload (Flash) with Docker
If you don't want to install PlatformIO locally, you can use the provided Dockerfile to build the firmware — and you can also upload the built firmware to your ESP32 directly from the container.

1) Build the image
- Windows PowerShell:
  - docker build -t iiot-esp32 .
- macOS/Linux:
  - docker build -t iiot-esp32 .

2) Build for the default environment (esp32vn-iot-uno)
- Windows PowerShell (mount current directory):
  - docker run --rm -v ${PWD}:/workspace -w /workspace iiot-esp32
  - If ${PWD} does not expand correctly, try: docker run --rm -v %cd%:/workspace -w /workspace iiot-esp32
- macOS/Linux:
  - docker run --rm -v "$PWD":/workspace -w /workspace iiot-esp32

3) Upload (flash) the firmware from Docker
- Linux (typical serial device is /dev/ttyUSB0 or /dev/ttyACM0):
  - docker run --rm \
      --device=/dev/ttyUSB0 \
      -e ACTION=upload \
      -e UPLOAD_PORT=/dev/ttyUSB0 \
      -v "$PWD":/workspace -w /workspace iiot-esp32
- macOS (typical device is /dev/cu.SLAB_USBtoUART or /dev/cu.usbserial-xxxx):
  - docker run --rm \
      --device=/dev/cu.SLAB_USBtoUART \
      -e ACTION=upload \
      -e UPLOAD_PORT=/dev/cu.SLAB_USBtoUART \
      -v "$PWD":/workspace -w /workspace iiot-esp32
- Optional: Serial monitor from Docker (after mapping the device):
  - docker run --rm --device=/dev/ttyUSB0 -e ACTION=monitor -e UPLOAD_PORT=/dev/ttyUSB0 -v "$PWD":/workspace -w /workspace iiot-esp32

Windows note:
- Docker Desktop on Windows does not reliably pass COM ports into Linux containers. For uploading, either:
  1) Use PlatformIO on the host (VS Code + PlatformIO or `pip install platformio` and run `pio run -t upload`).
  2) Use WSL with USB/serial passthrough solutions, or connect the device to a Linux/macOS machine for the flashing step.

Notes:
- Uploading from Docker requires passing the serial device through and appropriate permissions, e.g. on Linux: `--device=/dev/ttyUSB0` (you may also need to add your user to the `dialout` group on the host).
- On Windows, USB serial passthrough to Linux containers is limited; prefer uploading from the host PlatformIO as described above.
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

## Grafana visualization (Docker Compose stack)
This project includes an optional, ready-to-run local stack to visualize MQTT data in Grafana. It uses:
- Mosquitto (MQTT broker)
- Telegraf (ingests MQTT JSON and writes into InfluxDB)
- InfluxDB 2.x (time-series database)
- Grafana (dashboard with temperature and humidity panels)

Files added:
- docker-compose.yml
- ops/mosquitto/mosquitto.conf
- ops/telegraf/telegraf.conf
- ops/grafana/provisioning/datasources/datasource.yml
- ops/grafana/provisioning/dashboards/dashboards.yml
- ops/grafana/dashboards/IIoT DHT11.json

Quick start:
1) Configure the stack to use YOUR MQTT broker (not a local one)
   - Open docker-compose.yml and in the telegraf service set the environment variable MQTT_URL to your broker:
     - Examples:
       - MQTT_URL=tcp://broker.hivemq.com:1883
       - MQTT_URL=tcp://192.168.1.25:1883
       - MQTT_URL=ssl://your-broker.example.com:8883 (for TLS)
     - If your broker requires auth, also set MQTT_USERNAME and MQTT_PASSWORD.
   - Optional: If you are not using the local broker, you can ignore or comment out the mosquitto service in docker-compose.yml.

2) Start the stack
   - Windows PowerShell (from project root):
     - docker compose up -d
   - This will bring up services on the following ports:
     - InfluxDB: http 8086 (http://localhost:8086)
     - Grafana: http 3000 (http://localhost:3000)

3) Point your ESP32 firmware to YOUR broker (same one Telegraf reads from)
   - In include/settings.h set MQTT_BROKER and MQTT_PORT to your broker host and port.
   - If your broker requires auth, set MQTT_USERNAME and MQTT_PASSWORD.
   - Rebuild/flash the firmware.

4) Open Grafana
   - URL: http://localhost:3000
   - Login: admin / admin (from docker-compose.yml)
   - A pre-provisioned InfluxDB datasource is configured.
   - Dashboard: IIoT DHT11 (auto-provisioned). If you don’t see it, go to Dashboards → Browse and open "IIoT DHT11".

5) Data mapping details
   - Telegraf subscribes to the following topics (wildcard group):
     - iiot/group/+/sensor/temperature/state
     - iiot/group/+/sensor/humidity/state
   - It expects the JSON payload emitted by this firmware, e.g.:
     {"timestamp":"2025-01-01T12:00:00Z","sensor_id":"temp-1","value":23.1,"unit":"°C","status":"ok"}
   - It writes to InfluxDB bucket "iiot" with measurement name "reading". Fields: value. Tags: sensor_id, unit, status, topic.
   - The Grafana dashboard queries by unit (°C for temperature, % for humidity) and plots last 6 hours by default.

6) Simulate data without hardware (optional)
   - Publish a sample TemperatureReading:
     - Use a mosquitto_pub or another client against YOUR broker, for example:
       - mosquitto_pub -h <your-broker-host> -p <port> -t iiot/group/test/sensor/temperature/state -m '{"timestamp":"2025-01-01T12:00:00Z","sensor_id":"temp-1","value":23.1,"unit":"°C","status":"ok"}'
   - Publish a sample HumidityReading:
       - mosquitto_pub -h <your-broker-host> -p <port> -t iiot/group/test/sensor/humidity/state -m '{"timestamp":"2025-01-01T12:00:00Z","sensor_id":"hum-1","value":45,"unit":"%","status":"ok"}'
   - After a few seconds, Grafana should show values on the "Latest" panels and begin drawing timeseries.

Notes & tips:
- Ensure your ESP32 can reach your MQTT broker and the machine running Docker can reach the same broker from inside the Telegraf container.
- You can set the Telegraf MQTT settings via environment variables in docker-compose.yml: MQTT_URL, MQTT_USERNAME, MQTT_PASSWORD.
- Default InfluxDB setup credentials and tokens are defined in docker-compose.yml for local development only. Change them for any shared environment.
- To stop the stack: docker compose down (data in InfluxDB and Grafana persists via volumes).


