#include <Arduino.h>
#include <WiFi.h>              // For WiFi connection (use ESP8266WiFi.h for ESP8266)
#include <PubSubClient.h>      // MQTT Library
#include "settings.h"          // Include the settings.h configuration file

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void connectWiFi() {
    WiFi.disconnect();
    Serial.print("Connecting to WiFi...");
    printf("SSID: %s\n", WIFI_SSID);
    printf("Password: %s\n", WIFI_PASSWORD);
    //WiFi.mode(WIFI_STA); // Set WiFi to station mode (client mode)
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int retryCount = 0;
    const int maxRetries = 30;  // Set a maximum number of connection attempts
    // Retry loop
    while (WiFi.status() != WL_CONNECTED && retryCount < maxRetries) {
        delay(5000); // 1-second delay to allow the watchdog to reset
        Serial.print(".");
        retryCount++;

        // Print connection status codes (optional debugging)
        Serial.print(" WiFi.status(): ");
        Serial.println(WiFi.status());
    }

    // Check if connection was successful
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to WiFi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi after maximum retries.");
        Serial.println("Restarting...");
        delay(5000); // Give time for serial output before restarting
        ESP.restart(); // Restart the ESP
    }
}


void setup() {
    Serial.begin(115200);
    // Initialize serial communication
    delay(100);           // Small delay to stabilize
    connectWiFi();        // Attempt to connect to WiFi

}

void loop() {
    // Connect to WiFi
    delay(1000);
}