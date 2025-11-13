//
// Created by TobiasObwexer on 12.11.2025.
//

#ifndef INDUSTRIAL_INTERNET_OF_THINGS_SETTINGS_H
#define INDUSTRIAL_INTERNET_OF_THINGS_SETTINGS_H

// =====================
// WiFi configuration
// Fill in the placeholders below with your WiFi details and credentials.
// =====================

#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"

// =====================
// MQTT configuration
// Fill in the placeholders below with your broker details and credentials.
// Username and password are optional — leave as empty strings if your broker
// does not require authentication.
// =====================

// Hostname or IP address of your MQTT broker (e.g., "broker.hivemq.com")
#define MQTT_BROKER "0.0.0.0"

// TCP port of the broker (commonly 1883 for unencrypted, 8883 for TLS)
#define MQTT_PORT 1883

// A unique client identifier for this device (must be unique per broker)
#define MQTT_CLIENT_ID "ESP_32_Client"

// Optional credentials (leave as empty strings if not used)
#define MQTT_USERNAME "YOURUSERNAME"
#define MQTT_PASSWORD "YOURPASSWORD"

// Base topic for this device/application. Adjust to your preferred hierarchy
// Example: "iiot/esp32-01" or "home/lab/esp32"
#define MQTT_BASE_TOPIC "iiot/group/test/sensor"

// Common derived topics for quick testing
#define MQTT_TOPIC_STATUS   MQTT_BASE_TOPIC "/status"   // publishes device status/heartbeat
#define MQTT_TOPIC_COMMAND  MQTT_BASE_TOPIC "/cmd"      // subscribe here to receive commands

// =====================
// AsyncAPI-compatible channels for sensor state
// =====================
// Provide your group name as a dash-separated list of last names, e.g. "muster-schmidt-meier"
#define MQTT_GROUP_NAME "Obwexer-Poell"  // TODO: replace with your group members' last names

// Full channels/topics required by the provided payload specification
// Temperature: iiot/group/{nachname1-nachname2-nachname3}/sensor/temperature/state
// Humidity:    iiot/group/{nachname1-nachname2-nachname3}/sensor/humidity/state
#define MQTT_TOPIC_TEMPERATURE_STATE "iiot/group/" MQTT_GROUP_NAME "/sensor/temperature/state"
#define MQTT_TOPIC_HUMIDITY_STATE    "iiot/group/" MQTT_GROUP_NAME "/sensor/humidity/state"

// Sensor identity and units for the TemperatureReading schema
#define SENSOR_ID "temp-1"        // TODO: change to your unique temperature sensor ID
#define SENSOR_UNIT "°C"            // Allowed: "°C" or "K"

// Sensor identity and units for the HumidityReading schema (same structure as temperature)
#define HUM_SENSOR_ID "hum-1"      // TODO: change to your unique humidity sensor ID
#define HUM_SENSOR_UNIT "%"         // Relative humidity in percent

// =====================
// REST API configuration
// =====================
// You can adjust the REST API server port and endpoint path here, as well as
// the default runtime settings the API exposes. These are placeholders — feel
// free to change them to your needs.

// TCP port for the built‑in HTTP server
#define REST_API_PORT 80

// Endpoint path used for getting/setting runtime configuration
#define REST_API_CONFIG_PATH "/config"

// Default device status string exposed via REST and also published to MQTT
#define REST_DEFAULT_STATUS "online"

// Default send interval for sensor messages (milliseconds). Minimum enforced is 1000 ms.
#define REST_DEFAULT_SEND_INTERVAL_MS 2000

// Whether to publish temperature and humidity by default (1 = true, 0 = false)
#define REST_DEFAULT_PUBLISH_TEMPERATURE 1
#define REST_DEFAULT_PUBLISH_HUMIDITY 1

// =====================
// Sensor configuration
// =====================

// GPIO where the DHT11 data pin is connected
// On ESP32 this refers to the GPIO number (G14)
#define DHT11_PIN 14 // TODO: change to your ESP32 GPIO pin

#endif //INDUSTRIAL_INTERNET_OF_THINGS_SETTINGS_H