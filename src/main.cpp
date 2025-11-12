//
#include <esp32-hal.h>
#include <settings.h>
#include <wifi_connect.h>
#include <mqtt_connect.h>
#include <PubSubClient.h>
#include <dht_sensor.h>
#include <time.h>
#include <rest_api.h>

// Wi-Fi helper functions are provided by wifi_connect.h / wifi_connect.cpp
// MQTT helper functions are provided by mqtt_connect.h / mqtt_connect.cpp

// Simple MQTT message callback: prints received payload and echoes ACK to status topic
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    Serial.print("MQTT message on topic: ");
    Serial.println(topic);

    // Print payload as a string (not null-terminated by default)
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Respond with an ACK on the status topic for easy testing
    getMqttClient().publish(MQTT_TOPIC_STATUS, "ack");
}

// Format current UTC time as ISO8601 (e.g., 2025-08-28T10:00:00Z). Returns false if time not ready.
static bool formatIso8601Utc(char* out, size_t outLen) {
    time_t now = time(nullptr);
    if (now < 100000) { // time not set yet
        return false;
    }
    struct tm* tmInfo = gmtime(&now);
    if (!tmInfo) return false;
    size_t n = strftime(out, outLen, "%Y-%m-%dT%H:%M:%SZ", tmInfo);
    return n > 0;
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialize DHT11 sensor (GPIO set in settings.h)
    setupDht11();

    connectToWiFi(WIFI_SSID, WIFI_PASSWORD); // Initial Wi-Fi connection

    // Start lightweight REST API to configure runtime behavior
    initRestApi();

    // Configure NTP time (UTC) so we can publish ISO8601 timestamps
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    // Wait briefly for time to be set
    for (int i = 0; i < 50; ++i) { // up to ~5s
        time_t now = time(nullptr);
        if (now >= 100000) break;
        delay(100);
    }

    // Setup MQTT client with broker settings
    setupMqttClient(MQTT_BROKER, MQTT_PORT);

    // Set message callback and connect
    getMqttClient().setCallback(onMqttMessage);
    if (connectToMqtt(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
        // Subscribe to command topic and announce status
        getMqttClient().subscribe(MQTT_TOPIC_COMMAND);
        getMqttClient().publish(MQTT_TOPIC_STATUS, "online");
    }
}

void loop() {
    handleWiFiReconnect();  // Ensure Wi-Fi stays connected
    handleMqttReconnect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD); // Keep MQTT connected
    mqttLoop(); // Process incoming MQTT packets
    restApiLoop(); // Handle HTTP REST requests

    // Publish a heartbeat every 5 seconds when connected
    static unsigned long lastHeartbeat = 0;
    unsigned long now = millis();
    if (getMqttClient().connected() && now - lastHeartbeat >= 5000) {
        lastHeartbeat = now;
        getMqttClient().publish(MQTT_TOPIC_STATUS, "heartbeat");
    }

    // Read DHT11 based on configurable send interval and publish to MQTT/Serial when available
    static unsigned long lastDhtRead = 0;
    DeviceConfig& cfg = getDeviceConfig();
    uint32_t interval = cfg.sendIntervalMs < 1000 ? 1000 : cfg.sendIntervalMs; // safety lower bound
    if (now - lastDhtRead >= interval) {
        lastDhtRead = now;
        float tC = NAN, h = NAN;
        if (readDht11(tC, h)) {
            // Print to Serial
            // Serial.print("DHT11 -> T=");
            // Serial.print(tC, 1);
            // Serial.print("C  H=");
            // Serial.print(h, 0);
            // Serial.println("%");

            // Publish a simple line to the status topic for easy testing
            if (getMqttClient().connected()) {
                char msg[64];
                snprintf(msg, sizeof(msg), "T=%.1fC,H=%.0f%%", tC, h);
                getMqttClient().publish(MQTT_TOPIC_STATUS, msg);

                // Build TemperatureReading JSON according to the provided schema
                char ts[32];
                bool hasTs = formatIso8601Utc(ts, sizeof(ts));
                char json[192];
                if (cfg.publishTemperature) {
                    if (hasTs) {
                        snprintf(json, sizeof(json),
                                 "{\"timestamp\":\"%s\",\"sensor_id\":\"%s\",\"value\":%.1f,\"unit\":\"%s\",\"status\":\"ok\"}",
                                 ts, cfg.tempSensorId.c_str(), tC, SENSOR_UNIT);
                    } else {
                        // If time isn't ready yet, still publish without timestamp validity guarantee
                        snprintf(json, sizeof(json),
                                 "{\"timestamp\":\"\",\"sensor_id\":\"%s\",\"value\":%.1f,\"unit\":\"%s\",\"status\":\"ok\"}",
                                 cfg.tempSensorId.c_str(), tC, SENSOR_UNIT);
                    }
                    getMqttClient().publish(MQTT_TOPIC_TEMPERATURE_STATE, json);
                }

                // Build HumidityReading JSON with the same structure and publish
                if (cfg.publishHumidity) {
                    if (hasTs) {
                        snprintf(json, sizeof(json),
                                 "{\"timestamp\":\"%s\",\"sensor_id\":\"%s\",\"value\":%.1f,\"unit\":\"%s\",\"status\":\"ok\"}",
                                 ts, cfg.humSensorId.c_str(), h, HUM_SENSOR_UNIT);
                    } else {
                        snprintf(json, sizeof(json),
                                 "{\"timestamp\":\"\",\"sensor_id\":\"%s\",\"value\":%.1f,\"unit\":\"%s\",\"status\":\"ok\"}",
                                 cfg.humSensorId.c_str(), h, HUM_SENSOR_UNIT);
                    }
                    getMqttClient().publish(MQTT_TOPIC_HUMIDITY_STATE, json);
                }
            }
        }
    }

    // If status was changed via REST, publish the new status string once
    if (getMqttClient().connected()) {
        if (cfg.statusDirty) {
            getMqttClient().publish(MQTT_TOPIC_STATUS, cfg.status.c_str());
            cfg.statusDirty = false;
        }
    }

    delay(10);
}