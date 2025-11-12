#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#include <mqtt_connect.h>

// Internal globals
static WiFiClient g_wifiClient;
static PubSubClient g_mqttClient(g_wifiClient);

// Remember broker for reconnects
static String g_brokerHost;
static uint16_t g_brokerPort = 1883;

// Simple reconnect backoff
static unsigned long g_nextReconnectAttemptMs = 0;
static const unsigned long RECONNECT_INTERVAL_MS = 5000; // 5 seconds

void setupMqttClient(const char* broker, uint16_t port) {
    g_brokerHost = broker ? broker : "";
    g_brokerPort = port;
    g_mqttClient.setServer(g_brokerHost.c_str(), g_brokerPort);
}

bool connectToMqtt(const char* clientId, const char* username, const char* password) {
    if (g_brokerHost.isEmpty()) {
        Serial.println("MQTT: Broker not set. Call setupMqttClient() first.");
        return false;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("MQTT: Wi-Fi not connected. Skipping MQTT connect.");
        return false;
    }

    Serial.print("MQTT: Connecting to ");
    Serial.print(g_brokerHost);
    Serial.print(":");
    Serial.println(g_brokerPort);

    bool ok;
    if (username && password) {
        ok = g_mqttClient.connect(clientId, username, password);
    } else {
        ok = g_mqttClient.connect(clientId);
    }

    if (ok) {
        Serial.println("MQTT: Connected.");
        return true;
    } else {
        Serial.print("MQTT: Connect failed, rc=");
        Serial.println(g_mqttClient.state());
        return false;
    }
}

void handleMqttReconnect(const char* clientId, const char* username, const char* password) {
    // Only attempt if Wi-Fi is connected
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    if (g_mqttClient.connected()) {
        return;
    }

    unsigned long now = millis();
    if (now < g_nextReconnectAttemptMs) {
        return; // wait until next backoff slot
    }

    if (connectToMqtt(clientId, username, password)) {
        // Successful reconnect; next attempts can happen any time after a disconnect
        g_nextReconnectAttemptMs = now;
    } else {
        // Schedule next attempt
        g_nextReconnectAttemptMs = now + RECONNECT_INTERVAL_MS;
    }
}

void mqttLoop() {
    if (g_mqttClient.connected()) {
        g_mqttClient.loop();
    }
}

PubSubClient& getMqttClient() {
    return g_mqttClient;
}
