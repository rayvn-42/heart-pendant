#pragma once
#include <Arduino.h>

constexpr uint8_t PIN_SENSOR_SDA = 6;
constexpr uint8_t PIN_SENSOR_SCL = 7;
constexpr uint8_t PIN_SENSOR_INT = 1;
constexpr uint8_t PIN_BATTERY_SENSE = 0;

constexpr uint8_t ESPNOW_CHANNEL = 6;
constexpr uint32_t RADIO_DISCOVERY_INTERVAL_MS = 700;
constexpr uint32_t RADIO_REANNOUNCE_INTERVAL_MS = 4000;
constexpr uint32_t RADIO_STATUS_INTERVAL_MS = 5000;
constexpr uint32_t RADIO_RETRY_DELAY_MS = 35;

constexpr uint32_t BATTERY_SAMPLE_INTERVAL_MS = 1000;
constexpr uint8_t BATTERY_DIVIDER_RATIO = 2;

constexpr uint32_t SENSOR_RETRY_INTERVAL_MS = 4000;
constexpr uint32_t SENSOR_POLL_FALLBACK_MS = 10;
constexpr uint32_t SENSOR_SAMPLE_TIMEOUT_MS = 1800;
constexpr uint32_t FINGER_IR_THRESHOLD = 50000;
constexpr uint32_t MIN_BEAT_INTERVAL_MS = 280;
constexpr uint32_t MAX_BEAT_INTERVAL_MS = 2000;
constexpr uint8_t BPM_AVERAGE_SIZE = 4;
