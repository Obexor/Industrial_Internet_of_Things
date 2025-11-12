#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include <settings.h>
#include <rest_api.h>

// Internal server instance (port configurable via settings.h)
static WebServer g_server(REST_API_PORT);

// Global runtime config with sensible defaults
static DeviceConfig g_cfg;

static void sendCorsHeaders() {
    g_server.sendHeader("Access-Control-Allow-Origin", "*");
    g_server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    g_server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

static void handleOptions() {
    sendCorsHeaders();
    g_server.send(204); // No Content
}

static void handleGetConfig() {
    StaticJsonDocument<384> doc;
    doc["status"] = g_cfg.status;
    doc["sendIntervalMs"] = g_cfg.sendIntervalMs;
    doc["publishTemperature"] = g_cfg.publishTemperature;
    doc["publishHumidity"] = g_cfg.publishHumidity;
    doc["tempSensorId"] = g_cfg.tempSensorId;
    doc["humSensorId"] = g_cfg.humSensorId;

    String out;
    serializeJson(doc, out);
    sendCorsHeaders();
    g_server.send(200, "application/json", out);
}

static void handlePostConfig() {
    if (g_server.hasArg("plain") == false) {
        sendCorsHeaders();
        g_server.send(400, "application/json", "{\"error\":\"Missing body\"}");
        return;
    }

    const String& body = g_server.arg("plain");
    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) {
        sendCorsHeaders();
        g_server.send(400, "application/json", String("{\"error\":\"Bad JSON: ") + err.c_str() + "\"}");
        return;
    }

    bool changed = false;
    // Update fields if provided
    if (doc.containsKey("status") && doc["status"].is<const char*>()) {
        String newStatus = doc["status"].as<String>();
        if (newStatus != g_cfg.status) {
            g_cfg.status = newStatus;
            g_cfg.statusDirty = true;
            changed = true;
        }
    }
    if (doc.containsKey("sendIntervalMs") && doc["sendIntervalMs"].is<uint32_t>()) {
        uint32_t v = doc["sendIntervalMs"].as<uint32_t>();
        // Enforce a safe minimum for DHT11 (~1s)
        if (v < 1000) v = 1000;
        if (v != g_cfg.sendIntervalMs) { g_cfg.sendIntervalMs = v; changed = true; }
    }
    if (doc.containsKey("publishTemperature") && doc["publishTemperature"].is<bool>()) {
        bool v = doc["publishTemperature"].as<bool>();
        if (v != g_cfg.publishTemperature) { g_cfg.publishTemperature = v; changed = true; }
    }
    if (doc.containsKey("publishHumidity") && doc["publishHumidity"].is<bool>()) {
        bool v = doc["publishHumidity"].as<bool>();
        if (v != g_cfg.publishHumidity) { g_cfg.publishHumidity = v; changed = true; }
    }
    if (doc.containsKey("tempSensorId") && doc["tempSensorId"].is<const char*>()) {
        String v = doc["tempSensorId"].as<String>();
        if (v.length() > 0 && v != g_cfg.tempSensorId) { g_cfg.tempSensorId = v; changed = true; }
    }
    if (doc.containsKey("humSensorId") && doc["humSensorId"].is<const char*>()) {
        String v = doc["humSensorId"].as<String>();
        if (v.length() > 0 && v != g_cfg.humSensorId) { g_cfg.humSensorId = v; changed = true; }
    }

    // Respond with the effective config
    StaticJsonDocument<384> outDoc;
    outDoc["status"] = g_cfg.status;
    outDoc["sendIntervalMs"] = g_cfg.sendIntervalMs;
    outDoc["publishTemperature"] = g_cfg.publishTemperature;
    outDoc["publishHumidity"] = g_cfg.publishHumidity;
    outDoc["tempSensorId"] = g_cfg.tempSensorId;
    outDoc["humSensorId"] = g_cfg.humSensorId;

    String out;
    serializeJson(outDoc, out);
    sendCorsHeaders();
    g_server.send(changed ? 200 : 200, "application/json", out);
}

void initRestApi() {
    // Defaults seeded from compile-time settings
    g_cfg.status = REST_DEFAULT_STATUS; // setup() may publish its own online message
    g_cfg.statusDirty = false;
    g_cfg.sendIntervalMs = REST_DEFAULT_SEND_INTERVAL_MS; // match prior behavior
    g_cfg.publishTemperature = (REST_DEFAULT_PUBLISH_TEMPERATURE != 0);
    g_cfg.publishHumidity = (REST_DEFAULT_PUBLISH_HUMIDITY != 0);
    g_cfg.tempSensorId = SENSOR_ID;
    g_cfg.humSensorId = HUM_SENSOR_ID;

    // Routes
    g_server.on(REST_API_CONFIG_PATH, HTTP_OPTIONS, handleOptions);
    g_server.on(REST_API_CONFIG_PATH, HTTP_GET, handleGetConfig);
    g_server.on(REST_API_CONFIG_PATH, HTTP_POST, handlePostConfig);

    g_server.begin();
    Serial.print("REST API listening on http://");
    Serial.print(WiFi.localIP());
    Serial.print(":" );
    Serial.print(REST_API_PORT);
    Serial.print(REST_API_CONFIG_PATH);
    Serial.println();
}

void restApiLoop() {
    g_server.handleClient();
}

DeviceConfig& getDeviceConfig() {
    return g_cfg;
}
