#pragma once
#include <Arduino.h>

class BatteryMonitor {
 public:
  void begin();
  void update();
  uint16_t millivolts() const;
  uint8_t percent() const;

 private:
  uint8_t percentageFor(uint16_t millivolts) const;
  uint32_t _lastSampleAt = 0;
  uint16_t _millivolts = 0;
  bool _hasSample = false;
};
