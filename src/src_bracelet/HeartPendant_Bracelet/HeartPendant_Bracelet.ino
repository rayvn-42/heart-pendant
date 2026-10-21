#include <WiFi.h>
#include "BatteryMonitor.h"
#include "PulseSensor.h"
#include "RadioLink.h"

BatteryMonitor batteryMonitor;
PulseSensor pulseSensor;
RadioLink radioLink;

uint32_t lastLogAt = 0;
bool previousPairedState = false;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("Heart Pendant -> Bracelet | its coool!!");
  batteryMonitor.begin();
  if (!pulseSensor.begin()) {
    Serial.println("MAX30102 not detected!!\nretrying automatically");
  }
  if (!radioLink.begin()) {
    Serial.println("ESP-NOW initialization failed");
  } else {
    Serial.print("MAC (& cheese): ");
    Serial.println(WiFi.macAddress());
  }
}

void loop() {
  pulseSensor.update();
  batteryMonitor.update();

  radioLink.update(
    batteryMonitor.millivolts(),
    pulseSensor.bpm(),
    pulseSensor.ready(),
    pulseSensor.fingerPresent()
  );

  PulseBeat beat;
  if (pulseSensor.takeBeat(beat)) {
    radioLink.sendHeartbeat(
      beat.sequence,
      beat.bpm,
      batteryMonitor.millivolts(),
      pulseSensor.ready(),
      pulseSensor.fingerPresent()
    );
    Serial.print("Beat ");
    Serial.print(beat.sequence);
    Serial.print(" | BPM: ");
    Serial.print(beat.bpm);
    Serial.print(" | IR: ");
    Serial.println(pulseSensor.irValue());
  }

  bool paired = radioLink.isPaired();
  if (paired != previousPairedState) {
    Serial.println(paired ? "Pendant paired" : "Pendant disconnected!!, searching");
    previousPairedState = paired;
  }

  uint32_t now = millis();
  if (now - lastLogAt >= 2500) {
    lastLogAt = now;
    Serial.print("Sensor: ");
    Serial.print(pulseSensor.ready() ? "ready" : "unavailable");
    Serial.print(" | Finger: ");
    Serial.print(pulseSensor.fingerPresent() ? "detected" : "not detected");
    Serial.print(" | Battery: ");
    Serial.print(batteryMonitor.millivolts());
    Serial.print(" mV (");
    Serial.print(batteryMonitor.percent());
    Serial.print("%) | Radio: ");
    Serial.println(paired ? "paired" : "searching");
  }
}
