#pragma once

#include <Arduino.h>

// Runtime-configurable settings exposed via REST API
struct DeviceConfig {
    String status;              // Free-form device status, also published to MQTT status topic
    uint32_t sendIntervalMs;    // Interval for sending sensor readings
    bool publishTemperature;    // Whether to publish temperature readings
    bool publishHumidity;       // Whether to publish humidity readings
    String tempSensorId;        // Sensor ID for temperature
    String humSensorId;         // Sensor ID for humidity

    // Internal flag to signal that status has changed and should be re-published
    bool statusDirty;
};

// Initialize the REST API HTTP server (port 80) and seed defaults.
// Call after Wi‑Fi is connected.
void initRestApi();

// Call regularly from loop() to handle HTTP requests.
void restApiLoop();

// Access the mutable device configuration.
DeviceConfig& getDeviceConfig();
