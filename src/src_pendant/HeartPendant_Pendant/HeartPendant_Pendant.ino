#include <WiFi.h>
#include "BatteryMonitor.h"
#include "LedRing.h"
#include "RadioLink.h"

BatteryMonitor batteryMonitor;
LedRing ledRing;
RadioLink radioLink;

uint32_t lastLogAt = 0;
bool previousPairedState = false;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("Heart Pendant -> Pendant | its also cooool!!!");
  batteryMonitor.begin();
  ledRing.begin();
  if (!radioLink.begin()) {
    Serial.println("ESP-NOW initialization failed");
  } else {
    Serial.print("MAC (& cheese): ");
    Serial.println(WiFi.macAddress());
  }
}

void loop() {
  batteryMonitor.update();
  radioLink.update(batteryMonitor.millivolts(), 0, false, false);

  HeartbeatEvent beat;
  while (radioLink.takeHeartbeat(beat)) {
    ledRing.triggerBeat();
    Serial.print("Beat ");
    Serial.print(beat.sequence);
    Serial.print(" | Remote BPM: ");
    Serial.print(beat.bpm);
    Serial.print(" | Bracelet battery: ");
    Serial.print(beat.batteryMv);
    Serial.println(" mV");
  }

  ledRing.update();

  bool paired = radioLink.isPaired();
  if (paired != previousPairedState) {
    Serial.println(paired ? "Bracelet paired" : "Bracelet disconnected; searching");
    previousPairedState = paired;
  }

  uint32_t now = millis();
  if (now - lastLogAt >= 2500) {
    lastLogAt = now;
    Serial.print("Pendant battery: ");
    Serial.print(batteryMonitor.millivolts());
    Serial.print(" mV (");
    Serial.print(batteryMonitor.percent());
    Serial.print("%) | Radio: ");
    Serial.println(paired ? "paired" : "searching");
  }
}
