#pragma once

#include <Arduino.h>

// Initialize the DHT11 sensor on the configured pin (DHT11_PIN from settings.h)
void setupDht11();

// Read temperature (Celsius) and humidity (%) from DHT11.
// Returns true on success, false if reading failed.
bool readDht11(float& temperatureC, float& humidityPercent);
