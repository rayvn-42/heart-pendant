#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <MAX30105.h>
#include "Config.h"

struct PulseBeat {
  uint32_t sequence;
  uint16_t bpm;
};

class PulseSensor {
 public:
  bool begin();
  void update();
  bool takeBeat(PulseBeat &beat);
  bool ready() const;
  bool fingerPresent() const;
  uint16_t bpm() const;
  uint32_t irValue() const;

 private:
  bool initializeSensor();
  void processSample(uint32_t ir);
  void resetRateAverage();
  void addRate(uint16_t rate);

  MAX30105 _sensor;
  bool _ready = false;
  bool _fingerPresent = false;
  bool _interruptAttached = false;
  bool _beatPending = false;
  uint32_t _lastRetryAt = 0;
  uint32_t _lastPollAt = 0;
  uint32_t _lastSensorSampleAt = 0;
  uint32_t _lastAcceptedBeatAt = 0;
  uint32_t _irValue = 0;
  uint32_t _beatSequence = 0;
  uint16_t _bpm = 0;
  uint16_t _rates[BPM_AVERAGE_SIZE] = {};
  uint8_t _rateIndex = 0;
  uint8_t _rateCount = 0;
};
