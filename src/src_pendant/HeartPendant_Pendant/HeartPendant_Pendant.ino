#include <Adafruit_NeoPixel.h>

#define DATA_PIN 10
#define LED_COUNT 12

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, DATA_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.show();
  strip.setBrightness(50);
}

void loop() {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0, 50));
  }
  strip.show();
  delay(1000);

  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0, 0));
  }
  strip.show();
  delay(1000);
}