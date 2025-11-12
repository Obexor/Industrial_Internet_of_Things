#include <Arduino.h>
#include <DHT.h>
#include <settings.h>
#include <dht_sensor.h>

// Internal sensor instance (Adafruit DHT library)
static DHT g_dht(DHT11_PIN, DHT11);

void setupDht11() {
    // Initialize the DHT11 on the configured GPIO
    g_dht.begin();
}

bool readDht11(float& temperatureC, float& humidityPercent) {
    // Note: DHT11 updates roughly once per second; callers should avoid calling faster than that.
    float t = g_dht.readTemperature(); // Celsius by default
    float h = g_dht.readHumidity();
    if (isnan(t) || isnan(h)) {
        return false;
    }
    temperatureC = t;
    humidityPercent = h;
    return true;
}
