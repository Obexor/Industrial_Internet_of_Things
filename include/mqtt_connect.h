#pragma once

#include <Arduino.h>

// Forward declaration to avoid forcing PubSubClient include on all users of this header
class PubSubClient;

// Initializes the global MQTT client with the provided broker address and port.
// Must be called after Wi-Fi is connected (uses WiFiClient under the hood).
void setupMqttClient(const char* broker, uint16_t port);

// Attempts to connect to the MQTT broker with the given clientId and optional credentials.
// Returns true on success, false on failure after a short timeout.
bool connectToMqtt(const char* clientId, const char* username = nullptr, const char* password = nullptr);

// Call periodically (e.g., in loop()). If MQTT is disconnected (and Wi‑Fi is up),
// it will attempt to reconnect using the provided credentials with a simple backoff.
void handleMqttReconnect(const char* clientId, const char* username = nullptr, const char* password = nullptr);

// Must be called regularly in loop() to keep the MQTT connection alive and to receive messages.
void mqttLoop();

// Access the underlying PubSubClient instance to publish/subscribe, set callbacks, etc.
PubSubClient& getMqttClient();
