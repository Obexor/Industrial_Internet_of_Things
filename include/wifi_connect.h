#pragma once

#include <Arduino.h>

// Connect to Wi-Fi using provided credentials. Blocks up to a timeout while attempting.
void connectToWiFi(const char* ssid, const char* password);

// Ensure Wi-Fi stays connected; reconnects automatically using WIFI_SSID/WIFI_PASSWORD from settings.h
void handleWiFiReconnect();
