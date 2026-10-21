#pragma once
#include <Arduino.h>

constexpr uint32_t RADIO_MAGIC = 0x48505431; // magic number again, 0xAA55 ;
constexpr uint8_t RADIO_PROTOCOL_VERSION = 1;

enum RadioPacketType : uint8_t {
  RADIO_PACKET_HELLO = 1,
  RADIO_PACKET_PAIR_ACK = 2,
  RADIO_PACKET_HEARTBEAT = 3,
  RADIO_PACKET_STATUS = 4
};

struct __attribute__((packed)) RadioPacket {
  uint32_t magic;
  uint8_t version;
  uint8_t type;
  uint16_t batteryMv;
  uint16_t bpm;
  uint8_t sensorReady;
  uint8_t fingerPresent;
  uint32_t sequence;
};

static_assert(sizeof(RadioPacket) == 16, "Unexpected radio packet size");
