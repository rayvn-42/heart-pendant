#include "RadioLink.h"
#include "Config.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <cstring>

namespace {
const uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
portMUX_TYPE radioQueueMux = portMUX_INITIALIZER_UNLOCKED;
constexpr uint8_t incomingCapacity = 8;
}

RadioLink *RadioLink::_instance = nullptr;

bool RadioLink::begin() {
  if (_begun) {
    return true;
  }

  _instance = this;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_ps(WIFI_PS_NONE);

  if (esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    return false;
  }
  if (esp_now_init() != ESP_OK) {
    return false;
  }

#if ESP_ARDUINO_VERSION_MAJOR >= 3
  if (esp_now_register_recv_cb(RadioLink::receiveCallbackCurrent) != ESP_OK) {
    return false;
  }
#else
  if (esp_now_register_recv_cb(RadioLink::receiveCallbackLegacy) != ESP_OK) {
    return false;
  }
#endif

  if (!addPeer(broadcastMac)) {
    return false;
  }

  _begun = true;
  return true;
}

void RadioLink::update(uint16_t batteryMv, uint16_t bpm, bool sensorReady, bool fingerPresent) {
  if (!_begun) {
    return;
  }

  IncomingFrame frame;
  while (popFrame(frame)) {
    processFrame(frame, batteryMv, bpm, sensorReady, fingerPresent);
  }

  uint32_t now = millis();
  if (_retryPending && static_cast<int32_t>(now - _retryAt) >= 0) {
    if (_paired && _hasPeer) {
      sendPacket(_peerMac, _retryPacket);
    }
    _retryPending = false;
  }

  uint32_t beaconInterval = _paired ? RADIO_REANNOUNCE_INTERVAL_MS : RADIO_DISCOVERY_INTERVAL_MS;
  if (!_hasBeacon || now - _lastBeaconAt >= beaconInterval) {
    RadioPacket hello = makePacket(RADIO_PACKET_HELLO, batteryMv, bpm, sensorReady, fingerPresent, 0);
    sendPacket(broadcastMac, hello);
    _lastBeaconAt = now;
    _hasBeacon = true;
  }

  if (_paired && (!_hasSentStatus || now - _lastStatusAt >= RADIO_STATUS_INTERVAL_MS)) {
    RadioPacket status = makePacket(RADIO_PACKET_STATUS, batteryMv, bpm, sensorReady, fingerPresent, 0);
    sendPacket(_peerMac, status);
    _lastStatusAt = now;
    _hasSentStatus = true;
  }
}

bool RadioLink::sendHeartbeat(uint32_t sequence, uint16_t bpm, uint16_t batteryMv, bool sensorReady, bool fingerPresent) {
  if (!_begun || !_paired || !_hasPeer) {
    return false;
  }

  RadioPacket packet = makePacket(RADIO_PACKET_HEARTBEAT, batteryMv, bpm, sensorReady, fingerPresent, sequence);
  bool sent = sendPacket(_peerMac, packet);
  _retryPacket = packet;
  _retryAt = millis() + RADIO_RETRY_DELAY_MS;
  _retryPending = true;
  return sent;
}

bool RadioLink::takeHeartbeat(HeartbeatEvent &event) {
  if (!_heartbeatPending) {
    return false;
  }
  event = _heartbeatEvent;
  _heartbeatPending = false;
  return true;
}

bool RadioLink::isPaired() const {
  return _paired;
}

bool RadioLink::hasRemoteStatus() const {
  return _hasRemoteStatus;
}

uint16_t RadioLink::remoteBatteryMv() const {
  return _remoteBatteryMv;
}

uint16_t RadioLink::remoteBpm() const {
  return _remoteBpm;
}

bool RadioLink::remoteSensorReady() const {
  return _remoteSensorReady;
}

bool RadioLink::remoteFingerPresent() const {
  return _remoteFingerPresent;
}

#if ESP_ARDUINO_VERSION_MAJOR >= 3
void RadioLink::receiveCallbackCurrent(const esp_now_recv_info_t *info, const uint8_t *data, int length) {
  if (_instance != nullptr && info != nullptr) {
    _instance->enqueueFrame(info->src_addr, data, length);
  }
}
#else
void RadioLink::receiveCallbackLegacy(const uint8_t *mac, const uint8_t *data, int length) {
  if (_instance != nullptr) {
    _instance->enqueueFrame(mac, data, length);
  }
}
#endif

void RadioLink::enqueueFrame(const uint8_t *mac, const uint8_t *data, int length) {
  if (mac == nullptr || data == nullptr || length != static_cast<int>(sizeof(RadioPacket))) {
    return;
  }

  portENTER_CRITICAL(&radioQueueMux);
  uint8_t next = (_head + 1) % incomingCapacity;
  if (next != _tail) {
    memcpy(_incoming[_head].mac, mac, 6);
    _incoming[_head].length = static_cast<uint16_t>(length);
    memcpy(_incoming[_head].data, data, sizeof(RadioPacket));
    _head = next;
  }
  portEXIT_CRITICAL(&radioQueueMux);
}

bool RadioLink::popFrame(IncomingFrame &frame) {
  bool available = false;
  portENTER_CRITICAL(&radioQueueMux);
  if (_tail != _head) {
    memcpy(&frame, &_incoming[_tail], sizeof(IncomingFrame));
    _tail = (_tail + 1) % incomingCapacity;
    available = true;
  }
  portEXIT_CRITICAL(&radioQueueMux);
  return available;
}

void RadioLink::processFrame(const IncomingFrame &frame, uint16_t batteryMv, uint16_t bpm, bool sensorReady, bool fingerPresent) {
  if (frame.length != sizeof(RadioPacket)) {
    return;
  }

  RadioPacket packet;
  memcpy(&packet, frame.data, sizeof(packet));
  if (packet.magic != RADIO_MAGIC || packet.version != RADIO_PROTOCOL_VERSION) {
    return;
  }

  bool samePeer = _hasPeer && memcmp(frame.mac, _peerMac, 6) == 0;

  if (packet.type == RADIO_PACKET_HELLO) {
    if (!_hasPeer) {
      if (!adoptPeer(frame.mac)) {
        return;
      }
      samePeer = true;
    } else if (!samePeer) {
      return;
    }

    _paired = true;
    RadioPacket ack = makePacket(RADIO_PACKET_PAIR_ACK, batteryMv, bpm, sensorReady, fingerPresent, 0);
    sendPacket(frame.mac, ack);
    return;
  }

  if (packet.type == RADIO_PACKET_PAIR_ACK) {
    if (!_hasPeer) {
      if (!adoptPeer(frame.mac)) {
        return;
      }
      samePeer = true;
    }
    if (samePeer) {
      _paired = true;
    }
    return;
  }

  if (!_paired || !_hasPeer || !samePeer) {
    return;
  }

  if (packet.type == RADIO_PACKET_HEARTBEAT) {
    _remoteBatteryMv = packet.batteryMv;
    _remoteBpm = packet.bpm;
    _remoteSensorReady = packet.sensorReady != 0;
    _remoteFingerPresent = packet.fingerPresent != 0;
    _hasRemoteStatus = true;

    if (!_hasReceivedHeartbeat || packet.sequence != _lastReceivedSequence) {
      _lastReceivedSequence = packet.sequence;
      _hasReceivedHeartbeat = true;
      _heartbeatEvent.sequence = packet.sequence;
      _heartbeatEvent.bpm = packet.bpm;
      _heartbeatEvent.batteryMv = packet.batteryMv;
      _heartbeatPending = true;
    }
    return;
  }

  if (packet.type == RADIO_PACKET_STATUS) {
    _remoteBatteryMv = packet.batteryMv;
    _remoteBpm = packet.bpm;
    _remoteSensorReady = packet.sensorReady != 0;
    _remoteFingerPresent = packet.fingerPresent != 0;
    _hasRemoteStatus = true;
  }
}

bool RadioLink::addPeer(const uint8_t *mac) {
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, mac, 6);
  peer.channel = ESPNOW_CHANNEL;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;

  if (esp_now_is_peer_exist(mac)) {
    return esp_now_mod_peer(&peer) == ESP_OK;
  }
  return esp_now_add_peer(&peer) == ESP_OK;
}

bool RadioLink::adoptPeer(const uint8_t *mac) {
  if (!addPeer(mac)) {
    return false;
  }
  memcpy(_peerMac, mac, 6);
  _hasPeer = true;
  return true;
}

bool RadioLink::sendPacket(const uint8_t *mac, const RadioPacket &packet) {
  if (!_begun && packet.type != RADIO_PACKET_HELLO) {
    return false;
  }
  return esp_now_send(mac, reinterpret_cast<const uint8_t *>(&packet), sizeof(packet)) == ESP_OK;
}

RadioPacket RadioLink::makePacket(uint8_t type, uint16_t batteryMv, uint16_t bpm, bool sensorReady, bool fingerPresent, uint32_t sequence) const {
  RadioPacket packet = {};
  packet.magic = RADIO_MAGIC;
  packet.version = RADIO_PROTOCOL_VERSION;
  packet.type = type;
  packet.batteryMv = batteryMv;
  packet.bpm = bpm;
  packet.sensorReady = sensorReady ? 1 : 0;
  packet.fingerPresent = fingerPresent ? 1 : 0;
  packet.sequence = sequence;
  return packet;
}
