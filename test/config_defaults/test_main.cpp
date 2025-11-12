#include <Arduino.h>
#include <unity.h>

#include <settings.h>
#include <rest_api.h>

// Unity hooks
void setUp() {}
void tearDown() {}

static void test_rest_config_defaults() {
    // After initRestApi(), defaults should be seeded from settings.h
    DeviceConfig &cfg = getDeviceConfig();

    TEST_ASSERT_EQUAL_STRING(REST_DEFAULT_STATUS, cfg.status.c_str());
    TEST_ASSERT_EQUAL_UINT32(REST_DEFAULT_SEND_INTERVAL_MS, cfg.sendIntervalMs);
    TEST_ASSERT_EQUAL((bool)(REST_DEFAULT_PUBLISH_TEMPERATURE != 0), cfg.publishTemperature);
    TEST_ASSERT_EQUAL((bool)(REST_DEFAULT_PUBLISH_HUMIDITY != 0), cfg.publishHumidity);
    TEST_ASSERT_EQUAL_STRING(SENSOR_ID, cfg.tempSensorId.c_str());
    TEST_ASSERT_EQUAL_STRING(HUM_SENSOR_ID, cfg.humSensorId.c_str());
    TEST_ASSERT_FALSE_MESSAGE(cfg.statusDirty, "statusDirty should be false after init");
}

void setup() {
    delay(200);
    Serial.begin(115200);
    // Wait for Serial if running interactively
    delay(200);

    UNITY_BEGIN();

    // Initialize REST API (routes + defaults). Server start is deferred until Wi‑Fi connects, so it's safe.
    initRestApi();

    RUN_TEST(test_rest_config_defaults);

    UNITY_END();
}

void loop() {
    // Nothing to do; keep the MCU idle
    delay(1000);
}
