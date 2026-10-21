#pragma once
#include <Arduino.h>

constexpr uint8_t PIN_LED_DATA = 10;
constexpr uint8_t PIN_LED_SWITCH = 3;
constexpr uint8_t PIN_BATTERY_SENSE = 0; // I put this here cuz idk what polarity the LEDs are so I made it configurable

constexpr uint8_t LED_SWITCH_ACTIVE_LEVEL = HIGH;
constexpr uint8_t LED_SWITCH_INACTIVE_LEVEL = LOW;
constexpr uint16_t LED_COUNT = 12;
constexpr uint8_t LED_GLOBAL_BRIGHTNESS = 72;
constexpr uint32_t LED_ANIMATION_DURATION_MS = 520;
constexpr uint32_t LED_FRAME_INTERVAL_MS = 16;

constexpr uint8_t ESPNOW_CHANNEL = 6;
constexpr uint32_t RADIO_DISCOVERY_INTERVAL_MS = 700;
constexpr uint32_t RADIO_REANNOUNCE_INTERVAL_MS = 4000;
constexpr uint32_t RADIO_STATUS_INTERVAL_MS = 5000;
constexpr uint32_t RADIO_RETRY_DELAY_MS = 35;

constexpr uint32_t BATTERY_SAMPLE_INTERVAL_MS = 1000;
constexpr uint8_t BATTERY_DIVIDER_RATIO = 2;
