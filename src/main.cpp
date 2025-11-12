#include <esp32-hal.h>
#include <settings.h>
#include <wifi_connect.h>
#include <mqtt_connect.h>
#include <PubSubClient.h>
#include <dht_sensor.h>


// Wi-Fi helper functions are provided by wifi_connect.h / wifi_connect.cpp
// MQTT helper functions are provided by mqtt_connect.h / mqtt_connect.cpp

// Simple MQTT message callback: prints received payload and echoes ACK to status topic
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    Serial.print("MQTT message on topic: ");
    Serial.println(topic);

    // Print payload as a string (not null-terminated by default)
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Respond with an ACK on the status topic for easy testing
    getMqttClient().publish(MQTT_TOPIC_STATUS, "ack");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialize DHT11 sensor (GPIO set in settings.h)
    setupDht11();

    connectToWiFi(WIFI_SSID, WIFI_PASSWORD); // Initial Wi-Fi connection

    // Setup MQTT client with broker settings
    setupMqttClient(MQTT_BROKER, MQTT_PORT);

    // Set message callback and connect
    getMqttClient().setCallback(onMqttMessage);
    if (connectToMqtt(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
        // Subscribe to command topic and announce status
        getMqttClient().subscribe(MQTT_TOPIC_COMMAND);
        getMqttClient().publish(MQTT_TOPIC_STATUS, "online");
    }
}

void loop() {
    handleWiFiReconnect();  // Ensure Wi-Fi stays connected
    handleMqttReconnect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD); // Keep MQTT connected
    mqttLoop(); // Process incoming MQTT packets

    // Publish a heartbeat every 5 seconds when connected
    static unsigned long lastHeartbeat = 0;
    unsigned long now = millis();
    if (getMqttClient().connected() && now - lastHeartbeat >= 5000) {
        lastHeartbeat = now;
        getMqttClient().publish(MQTT_TOPIC_STATUS, "heartbeat");
    }

    // Read DHT11 no more than once per 2 seconds and publish to MQTT/Serial when available
    static unsigned long lastDhtRead = 0;
    if (now - lastDhtRead >= 2000) {
        lastDhtRead = now;
        float tC = NAN, h = NAN;
        if (readDht11(tC, h)) {
            // Print to Serial
            Serial.print("DHT11 -> T=");
            Serial.print(tC, 1);
            Serial.print("C  H=");
            Serial.print(h, 0);
            Serial.println("%");

            // Publish a simple line to the status topic for easy testing
            if (getMqttClient().connected()) {
                char msg[64];
                snprintf(msg, sizeof(msg), "T=%.1f°C,H=%.0f%%", tC, h);
                getMqttClient().publish(MQTT_TOPIC_STATUS, msg);
            }
        }
    }

    delay(10);
}