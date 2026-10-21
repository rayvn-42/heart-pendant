#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Config.h"

class LedRing {
 public:
  void begin();
  void triggerBeat();
  void update();

 private:
  void render(uint32_t elapsed);
  void powerOn();
  void powerOff();

  Adafruit_NeoPixel _pixels = Adafruit_NeoPixel(LED_COUNT, PIN_LED_DATA, NEO_GRB + NEO_KHZ800);
  bool _powered = false;
  bool _animating = false;
  uint32_t _animationStartedAt = 0;
  uint32_t _lastFrameAt = 0;
};
