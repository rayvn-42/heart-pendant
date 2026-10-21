#include "PulseSensor.h"
#include "Config.h"
#include <heartRate.h>

namespace {
volatile bool sensorDataReady = false;

void IRAM_ATTR sensorDataReadyInterrupt() {
  sensorDataReady = true;
}
}

bool PulseSensor::begin() {
  return initializeSensor();
}

bool PulseSensor::initializeSensor() {
  Wire.begin(PIN_SENSOR_SDA, PIN_SENSOR_SCL);
  Wire.setClock(400000);

  if (!_sensor.begin(Wire, I2C_SPEED_FAST)) {
    _ready = false;
    _lastRetryAt = millis();
    return false;
  }

  _sensor.setup(0x1F, 4, 2, 100, 411, 4096);
  _sensor.setPulseAmplitudeRed(0x00);
  _sensor.setPulseAmplitudeIR(0x1F);
  _sensor.enableDATARDY();

  pinMode(PIN_SENSOR_INT, INPUT_PULLUP);
  sensorDataReady = false;
  if (!_interruptAttached) {
    attachInterrupt(digitalPinToInterrupt(PIN_SENSOR_INT), sensorDataReadyInterrupt, FALLING);
    _interruptAttached = true;
  }

  _ready = true;
  _lastPollAt = 0;
  _lastSensorSampleAt = millis();
  _lastAcceptedBeatAt = 0;
  _fingerPresent = false;
  _bpm = 0;
  resetRateAverage();
  return true;
}

void PulseSensor::update() {
  uint32_t now = millis();

  if (!_ready) {
    if (now - _lastRetryAt >= SENSOR_RETRY_INTERVAL_MS) {
      initializeSensor();
    }
    return;
  }

  bool interruptSeen = sensorDataReady;
  sensorDataReady = false;

  if (interruptSeen || now - _lastPollAt >= SENSOR_POLL_FALLBACK_MS) {
    _lastPollAt = now;
    _sensor.getINT1();
    _sensor.check();

    while (_sensor.available()) {
      uint32_t ir = _sensor.getFIFOIR();
      _sensor.nextSample();
      processSample(ir);
    }
  }

  now = millis();
  if (_lastSensorSampleAt != 0 && now - _lastSensorSampleAt > SENSOR_SAMPLE_TIMEOUT_MS) {
    if (_interruptAttached) {
      detachInterrupt(digitalPinToInterrupt(PIN_SENSOR_INT));
      _interruptAttached = false;
    }
    _ready = false;
    _fingerPresent = false;
    _bpm = 0;
    _lastRetryAt = now;
  }
}

void PulseSensor::processSample(uint32_t ir) {
  uint32_t now = millis();
  _irValue = ir;
  _lastSensorSampleAt = now;

  bool detected = ir >= FINGER_IR_THRESHOLD;
  if (!detected) {
    if (_fingerPresent) {
      _bpm = 0;
      _lastAcceptedBeatAt = 0;
      resetRateAverage();
    }
    _fingerPresent = false;
    return;
  }

  _fingerPresent = true;
  if (!checkForBeat(ir)) {
    return;
  }

  if (_lastAcceptedBeatAt == 0) {
    _lastAcceptedBeatAt = now;
    _beatSequence++;
    _beatPending = true;
    return;
  }

  uint32_t interval = now - _lastAcceptedBeatAt;
  if (interval < MIN_BEAT_INTERVAL_MS) {
    return;
  }

  _lastAcceptedBeatAt = now;
  if (interval <= MAX_BEAT_INTERVAL_MS) {
    uint32_t instantBpm = 60000UL / interval;
    if (instantBpm >= 30 && instantBpm <= 220) {
      addRate(static_cast<uint16_t>(instantBpm));
    }
  } else {
    resetRateAverage();
    _bpm = 0;
  }

  _beatSequence++;
  _beatPending = true;
}

void PulseSensor::resetRateAverage() {
  _rateIndex = 0;
  _rateCount = 0;
  for (uint8_t i = 0; i < BPM_AVERAGE_SIZE; ++i) {
    _rates[i] = 0;
  }
}

void PulseSensor::addRate(uint16_t rate) {
  _rates[_rateIndex] = rate;
  _rateIndex = (_rateIndex + 1) % BPM_AVERAGE_SIZE;
  if (_rateCount < BPM_AVERAGE_SIZE) {
    _rateCount++;
  }

  uint32_t total = 0;
  for (uint8_t i = 0; i < _rateCount; ++i) {
    total += _rates[i];
  }
  _bpm = total / _rateCount;
}

bool PulseSensor::takeBeat(PulseBeat &beat) {
  if (!_beatPending) {
    return false;
  }
  beat.sequence = _beatSequence;
  beat.bpm = _bpm;
  _beatPending = false;
  return true;
}

bool PulseSensor::ready() const {
  return _ready;
}

bool PulseSensor::fingerPresent() const {
  return _fingerPresent;
}

uint16_t PulseSensor::bpm() const {
  return _bpm;
}

uint32_t PulseSensor::irValue() const {
  return _irValue;
}
