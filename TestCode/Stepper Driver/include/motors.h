// Pan + tilt TMC2209 drivers: pins, UART config, enable.
#pragma once

#include <Arduino.h>
#include <TMCStepper.h>

extern TMC2209Stepper pan;
extern TMC2209Stepper tilt;

void motorsInitPins();                                    // drivers disabled
bool setupDriver(TMC2209Stepper &drv, const char *name);  // prints result, returns ok
void motorsEnable(bool on);
