#include <WiFi.h>
#include <settings.h>

// Function to handle Wi-Fi connection with auto-reconnect
void connectToWiFi(const char* ssid, const char* password) {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.mode(WIFI_STA);  // Set Wi-Fi to station mode
    WiFi.begin(ssid, password);  // Start Wi-Fi connection

    unsigned long startAttemptTime = millis();
    const unsigned long connectionTimeout = 10000;  // 10 seconds timeout

    // Try to connect for max connectionTimeout
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < connectionTimeout) {
        Serial.print(".");
        delay(500);
    }

    // Check if the Wi-Fi connection was successful
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to Wi-Fi");
    }
}

// Function to automatically reconnect if Wi-Fi is disconnected
void handleWiFiReconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi lost. Reconnecting...");
        connectToWiFi(WIFI_SSID, WIFI_PASSWORD);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    connectToWiFi(WIFI_SSID, WIFI_PASSWORD); // Initial Wi-Fi connection
}

void loop() {
    handleWiFiReconnect();  // Ensure Wi-Fi stays connected
    // Your main code here
    delay(1000);
}