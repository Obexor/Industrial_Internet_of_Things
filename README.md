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
1) Start the stack
   - Windows PowerShell (from project root):
     - docker compose up -d
   - This will bring up services on the following ports:
     - Mosquitto: tcp 1883 (localhost:1883)
     - InfluxDB: http 8086 (http://localhost:8086)
     - Grafana: http 3000 (http://localhost:3000)

2) Point your ESP32 firmware to the local broker
   - In include/settings.h set:
     - MQTT_BROKER to the host IP address reachable by your ESP32 (NOT 127.0.0.1). For example, your laptop IP on the same Wi‑Fi, e.g. "192.168.1.25".
     - MQTT_PORT to 1883 (default)
   - Rebuild/flash the firmware.

3) Open Grafana
   - URL: http://localhost:3000
   - Login: admin / admin (from docker-compose.yml)
   - A pre-provisioned InfluxDB datasource is configured.
   - Dashboard: IIoT DHT11 (auto-provisioned). If you don’t see it, go to Dashboards → Browse and open "IIoT DHT11".

4) Data mapping details
   - Telegraf subscribes to the following topics (wildcard group):
     - iiot/group/+/sensor/temperature/state
     - iiot/group/+/sensor/humidity/state
   - It expects the JSON payload emitted by this firmware, e.g.:
     {"timestamp":"2025-01-01T12:00:00Z","sensor_id":"temp-1","value":23.1,"unit":"°C","status":"ok"}
   - It writes to InfluxDB bucket "iiot" with measurement name "reading". Fields: value. Tags: sensor_id, unit, status, topic.
   - The Grafana dashboard queries by unit (°C for temperature, % for humidity) and plots last 6 hours by default.

5) Simulate data without hardware (optional)
   - Publish a sample TemperatureReading:
     - docker exec -it iiot-mosquitto mosquitto_pub -h localhost -t iiot/group/test/sensor/temperature/state -m '{"timestamp":"2025-01-01T12:00:00Z","sensor_id":"temp-1","value":23.1,"unit":"°C","status":"ok"}'
   - Publish a sample HumidityReading:
     - docker exec -it iiot-mosquitto mosquitto_pub -h localhost -t iiot/group/test/sensor/humidity/state -m '{"timestamp":"2025-01-01T12:00:00Z","sensor_id":"hum-1","value":45,"unit":"%","status":"ok"}'
   - After a few seconds, Grafana should show values on the "Latest" panels and begin drawing timeseries.

Notes & tips:
- Ensure your ESP32 and the machine running Docker are on the same network. Use the host IP for MQTT_BROKER.
- Default InfluxDB setup credentials and tokens are defined in docker-compose.yml for local development only. Change them for any shared environment.
- To stop the stack: docker compose down (data in InfluxDB and Grafana persists via volumes).
