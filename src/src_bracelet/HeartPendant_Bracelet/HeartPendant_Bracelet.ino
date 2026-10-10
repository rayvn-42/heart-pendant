#include <Wire.h>
#include "MAX30105.h"

MAX30105 sensor;

#define I2C_SDA 6
#define I2C_SCL 7

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Intialize MAX30102");
  Wire.begin(I2C_SDA, I2C_SCL, 400000);

  if (!sensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found.");
    while (1);
  }

  sensor.setup();
}

void loop() {
  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();

  if (irValue < 50000) {
    Serial.println("No pulse detected");
  } else {
    Serial.print("IR:");
    Serial.print(irValue);
    Serial.print(",");
    Serial.print("Red:");
    Serial.println(redValue);
  }
  delay(20);
}
