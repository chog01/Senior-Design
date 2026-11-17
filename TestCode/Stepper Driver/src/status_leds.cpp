#include "status_leds.h"
#include "pins.h"
#include "config.h"

static void ledInit(uint8_t pin) {
  if (pin == NO_PIN) return;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LED_OFF);
}

void ledsInit() {
  ledInit(PIN_LED_HEARTBEAT);
  ledInit(PIN_LED_FAULT);
  ledInit(PIN_LED_READY);
  ledSet(PIN_LED_HEARTBEAT, true);   // solid during setup, blinks after
}

void ledSet(uint8_t pin, bool on) {
  if (pin == NO_PIN) return;
  digitalWrite(pin, on ? LED_ON : LED_OFF);
}

void updateHeartbeat() {
  static elapsedMillis sinceToggle;
  static bool on = true;
  if (sinceToggle >= HEARTBEAT_MS) {
    sinceToggle = 0;
    on = !on;
    ledSet(PIN_LED_HEARTBEAT, on);
  }
}
