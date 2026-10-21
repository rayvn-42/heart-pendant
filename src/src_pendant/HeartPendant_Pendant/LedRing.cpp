#include "LedRing.h"
#include "Config.h"
#include <math.h>

void LedRing::begin() {
  pinMode(PIN_LED_DATA, OUTPUT);
  digitalWrite(PIN_LED_DATA, LOW);
  pinMode(PIN_LED_SWITCH, OUTPUT);
  digitalWrite(PIN_LED_SWITCH, LED_SWITCH_INACTIVE_LEVEL);
  _pixels.begin();
  _pixels.setBrightness(LED_GLOBAL_BRIGHTNESS);
  _pixels.clear();
  _powered = false;
  _animating = false;
  _lastFrameAt = millis();
}

void LedRing::triggerBeat() {
  if (!_powered) {
    powerOn();
  }
  _animationStartedAt = millis();
  _animating = true;
}

void LedRing::update() {
  if (!_powered || !_animating) {
    return;
  }

  uint32_t now = millis();
  if (now - _lastFrameAt < LED_FRAME_INTERVAL_MS) {
    return;
  }
  _lastFrameAt = now;

  uint32_t elapsed = now - _animationStartedAt;
  if (elapsed >= LED_ANIMATION_DURATION_MS) {
    powerOff();
    _animating = false;
    return;
  }

  render(elapsed);
}

void LedRing::render(uint32_t elapsed) {
  float progress = static_cast<float>(elapsed) / static_cast<float>(LED_ANIMATION_DURATION_MS);
  float head = progress * LED_COUNT;
  float fade = 1.0f;
  constexpr uint32_t fadeWindowMs = 90;
  if (elapsed > LED_ANIMATION_DURATION_MS - fadeWindowMs) {
    fade = static_cast<float>(LED_ANIMATION_DURATION_MS - elapsed) / fadeWindowMs;
  }

  for (uint16_t i = 0; i < LED_COUNT; ++i) {
    float distance = head - static_cast<float>(i);
    if (distance > LED_COUNT / 2.0f) {
      distance -= LED_COUNT;
    } else if (distance < -LED_COUNT / 2.0f) {
      distance += LED_COUNT;
    }

    float intensity = 0.0f;
    if (distance >= 0.0f) {
      if (distance < 4.0f) {
        intensity = expf(-distance * 0.78f);
      }
    } else if (distance > -1.0f) {
      intensity = 1.0f + distance;
    }

    intensity *= fade;
    uint8_t red = static_cast<uint8_t>(255.0f * intensity);
    uint8_t green = static_cast<uint8_t>(10.0f * intensity);
    uint8_t blue = static_cast<uint8_t>(64.0f * intensity);
    _pixels.setPixelColor(i, _pixels.Color(red, green, blue));
  }
  _pixels.show();
}

void LedRing::powerOn() {
  digitalWrite(PIN_LED_DATA, LOW);
  digitalWrite(PIN_LED_SWITCH, LED_SWITCH_ACTIVE_LEVEL);
  delayMicroseconds(500);
  _pixels.clear();
  _pixels.show();
  _powered = true;
}

void LedRing::powerOff() {
  _pixels.clear();
  _pixels.show();
  digitalWrite(PIN_LED_DATA, LOW);
  digitalWrite(PIN_LED_SWITCH, LED_SWITCH_INACTIVE_LEVEL);
  _powered = false;
}

// I hate C++ :)
