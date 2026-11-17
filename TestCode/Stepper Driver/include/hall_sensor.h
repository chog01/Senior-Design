// TMAG5273 hall sensor wrapper: init check, logging, |B| min/max stats.
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TMAG5273.h>

struct HallSensor {
  const char *name;
  TwoWire &wire;
  Adafruit_TMAG5273 dev;
  bool ok;
  float minMag, maxMag;   // |B| stats for the current run, in mT
};

extern HallSensor hallPan;
extern HallSensor hallTilt;

bool setupHall(HallSensor &s);                        // prints result, returns ok
void resetHallStats(HallSensor &s);
void logHall(HallSensor &s, const char *prefix);      // read X/Y/Z, print, update stats
void printHallStats(HallSensor &s, const char *label);
