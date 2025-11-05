#include <Arduino.h>
#include <WiFi.h>              // For WiFi connection (use ESP8266WiFi.h for ESP8266)
#include <PubSubClient.h>      // MQTT Library
#include "settings.h"          // Include the settings.h configuration file

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void connectWiFi() {
    Serial.print("Connecting to WiFi...");
    printf("SSID: %s\n", WIFI_SSID);
    printf("Password: %s\n", WIFI_PASSWORD);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int retryCount = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        retryCount++;

        // If we keep failing to connect after a set number of attempts
        if (retryCount > 2) {
            Serial.println("\nWiFi connection failed. Restarting...");
            ESP.restart();
        }
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void connectMQTT() {
    Serial.print("Connecting to MQTT Broker...");

    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    while (!mqttClient.connected()) {
        Serial.println("");
        Serial.print("Attempting MQTT connection...");

        // Try to connect to the broker with credentials
        if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
            Serial.println("Connected to MQTT broker!");
        } else {
            Serial.print("Failed to connect, rc=");
            Serial.print(mqttClient.state());
            Serial.println(". Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void mqttCallback(char* topic, byte* message, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)message[i]);
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);

    // Connect to WiFi
    connectWiFi();
}

void loop() {


}