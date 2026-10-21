#pragma once
#include <Arduino.h>
#include "Protocol.h"
#include <esp_now.h>
#if __has_include(<esp_arduino_version.h>)
#include <esp_arduino_version.h>
#else
#define ESP_ARDUINO_VERSION_MAJOR 2
#endif

struct HeartbeatEvent {
  uint32_t sequence;
  uint16_t bpm;
  uint16_t batteryMv;
};

class RadioLink {
 public:
  bool begin();
  void update(uint16_t batteryMv, uint16_t bpm, bool sensorReady, bool fingerPresent);
  bool sendHeartbeat(uint32_t sequence, uint16_t bpm, uint16_t batteryMv, bool sensorReady, bool fingerPresent);
  bool takeHeartbeat(HeartbeatEvent &event);
  bool isPaired() const;
  bool hasRemoteStatus() const;
  uint16_t remoteBatteryMv() const;
  uint16_t remoteBpm() const;
  bool remoteSensorReady() const;
  bool remoteFingerPresent() const;

 private:
  struct IncomingFrame {
    uint8_t mac[6];
    uint16_t length;
    uint8_t data[sizeof(RadioPacket)];
  };

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  static void receiveCallbackCurrent(const esp_now_recv_info_t *info, const uint8_t *data, int length);
#else
  static void receiveCallbackLegacy(const uint8_t *mac, const uint8_t *data, int length);
#endif
  void enqueueFrame(const uint8_t *mac, const uint8_t *data, int length);
  bool popFrame(IncomingFrame &frame);
  void processFrame(const IncomingFrame &frame, uint16_t batteryMv, uint16_t bpm, bool sensorReady, bool fingerPresent);
  bool addPeer(const uint8_t *mac);
  bool adoptPeer(const uint8_t *mac);
  bool sendPacket(const uint8_t *mac, const RadioPacket &packet);
  RadioPacket makePacket(uint8_t type, uint16_t batteryMv, uint16_t bpm, bool sensorReady, bool fingerPresent, uint32_t sequence) const;

  static RadioLink *_instance;
  IncomingFrame _incoming[8];
  uint8_t _head = 0;
  uint8_t _tail = 0;
  uint8_t _peerMac[6] = {};
  bool _begun = false;
  bool _hasPeer = false;
  bool _paired = false;
  bool _hasBeacon = false;
  bool _hasSentStatus = false;
  bool _hasReceivedHeartbeat = false;
  bool _heartbeatPending = false;
  bool _hasRemoteStatus = false;
  uint32_t _lastBeaconAt = 0;
  uint32_t _lastStatusAt = 0;
  uint32_t _lastReceivedSequence = 0;
  uint16_t _remoteBatteryMv = 0;
  uint16_t _remoteBpm = 0;
  bool _remoteSensorReady = false;
  bool _remoteFingerPresent = false;
  HeartbeatEvent _heartbeatEvent = {};
  RadioPacket _retryPacket = {};
  bool _retryPending = false;
  uint32_t _retryAt = 0;
};
