#include <Arduino.h>
#include <FastLED.h>
#include <Encoder.h>
#include <EEPROM.h>

#define NUM_LEDS    24
#define CLK_PIN     2
#define DT_PIN      3
#define SWITCH_PIN  4
#define LED_PIN     5

CRGB leds[NUM_LEDS];
int days = 0;
unsigned long lastDaysChange = 0L;
unsigned long lastStored = 0L;
int mode = 0;

Encoder encoder(DT_PIN, CLK_PIN);
long oldPosition = 0;

void showDays() {
  CRGB colour;
  switch (mode) {
    case 0: colour = CRGB::Red; break;
    case 1: colour = CRGB::Green; break;
    case 2: colour = CRGB::Blue; break;
  }
  for (int n = 0; n < NUM_LEDS; n++) {
    if (n < days) {
      leds[n] = colour;
    } else {
      leds[n] = CRGB::Black;
    }
  }
  FastLED.show();
}

void setup() { 
  Serial.begin(115200);
  Serial.println("Starting");

  pinMode(SWITCH_PIN, INPUT_PULLUP);

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  byte stored = EEPROM[0];
  if (stored != 0xFF) {
    days = stored;
    Serial.print("Using stored days: ");
    Serial.println(days);
  } else {
    days = 0;
    EEPROM[0] = 0;
    Serial.print("Initialising days to 0");
  }
  showDays();
}

void loop() {
  long newPosition = encoder.read() / 4;
  if (newPosition != oldPosition) {
    days += newPosition - oldPosition;
    if (days < 0) {
      days = 0;
    } else if (days > NUM_LEDS) {
      days = NUM_LEDS;
    } else {
      lastDaysChange = millis();
      showDays();
    }
    oldPosition = newPosition;
  }

  // If it's 3 seconds since last time 'days' changed...
  unsigned long now = millis();
  if (now - lastDaysChange > 3000) {
    // And the last time 'days' changed is later than the last time it was stored...
    if (lastDaysChange > lastStored) {
      EEPROM[0] = days;
      lastStored = now;
      Serial.print ("Stored days: ");
      Serial.println(days);
    }
  }

  if (digitalRead(SWITCH_PIN) == 0) {
    if (++mode > 2) {
      mode = 0;
    }
    showDays();
    delay(100);
    while (digitalRead(SWITCH_PIN) == 0)
      ;
    delay(100);
  }
}