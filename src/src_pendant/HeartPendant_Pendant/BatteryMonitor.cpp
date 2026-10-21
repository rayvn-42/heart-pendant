#include "BatteryMonitor.h"
#include "Config.h"

void BatteryMonitor::begin() {
  pinMode(PIN_BATTERY_SENSE, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_BATTERY_SENSE, ADC_11db);
  _hasSample = false;
}

void BatteryMonitor::update() {
  uint32_t now = millis();
  if (_hasSample && now - _lastSampleAt < BATTERY_SAMPLE_INTERVAL_MS) {
    return;
  }

  uint32_t sum = 0;
  for (uint8_t i = 0; i < 8; ++i) {
    sum += analogReadMilliVolts(PIN_BATTERY_SENSE);
    delayMicroseconds(150);
  }

  uint32_t adcMillivolts = sum / 8;
  uint32_t batteryMillivolts = adcMillivolts * BATTERY_DIVIDER_RATIO;
  if (batteryMillivolts > 65535) {
    batteryMillivolts = 65535;
  }

  _millivolts = static_cast<uint16_t>(batteryMillivolts);
  _lastSampleAt = now;
  _hasSample = true;
}

uint16_t BatteryMonitor::millivolts() const {
  return _millivolts;
}

uint8_t BatteryMonitor::percent() const {
  return percentageFor(_millivolts);
}

uint8_t BatteryMonitor::percentageFor(uint16_t mv) const {
  static const uint16_t voltage[] = {3300, 3500, 3600, 3700, 3800, 3900, 4000, 4100, 4200};
  static const uint8_t charge[] = {0, 5, 10, 25, 40, 60, 75, 90, 100};
  constexpr uint8_t count = sizeof(voltage) / sizeof(voltage[0]);

  if (mv <= voltage[0]) {
    return 0;
  }
  if (mv >= voltage[count - 1]) {
    return 100;
  }

  for (uint8_t i = 1; i < count; ++i) {
    if (mv < voltage[i]) {
      uint32_t span = voltage[i] - voltage[i - 1];
      uint32_t offset = mv - voltage[i - 1];
      uint32_t rise = charge[i] - charge[i - 1];
      return charge[i - 1] + (offset * rise) / span;
    }
  }

  return 100;
}
