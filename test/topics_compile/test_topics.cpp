#include <Arduino.h>
#include <unity.h>

#include <settings.h>

void setUp() {}
void tearDown() {}

static void test_topic_macros_have_expected_values() {
    // Build expected strings at runtime using the same compile-time pieces
    String expectedTemp = String("iiot/group/") + MQTT_GROUP_NAME + "/sensor/temperature/state";
    String expectedHum  = String("iiot/group/") + MQTT_GROUP_NAME + "/sensor/humidity/state";

    TEST_ASSERT_EQUAL_STRING(expectedTemp.c_str(), MQTT_TOPIC_TEMPERATURE_STATE);
    TEST_ASSERT_EQUAL_STRING(expectedHum.c_str(),  MQTT_TOPIC_HUMIDITY_STATE);

    // Also check base topics are non-empty and contain the expected subpaths
    TEST_ASSERT_TRUE_MESSAGE(strlen(MQTT_BASE_TOPIC) > 0, "MQTT_BASE_TOPIC should not be empty");
    String statusTopic = String(MQTT_TOPIC_STATUS);
    String cmdTopic = String(MQTT_TOPIC_COMMAND);
    TEST_ASSERT_TRUE(statusTopic.endsWith("/status"));
    TEST_ASSERT_TRUE(cmdTopic.endsWith("/cmd"));
}

void setup() {
    delay(200);
    Serial.begin(115200);
    delay(100);
    UNITY_BEGIN();
    RUN_TEST(test_topic_macros_have_expected_values);
    UNITY_END();
}

void loop() {
    delay(1000);
}
