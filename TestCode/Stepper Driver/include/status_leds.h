// Heartbeat, fault and motor-ready LEDs.
#pragma once

#include <Arduino.h>

void ledsInit();                   // all LEDs off, heartbeat solid on
void ledSet(uint8_t pin, bool on); // ignores NO_PIN
void updateHeartbeat();            // non-blocking 1 Hz blink; call often
