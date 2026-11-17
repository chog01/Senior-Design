// Garden Sentry main board - tunable settings.
#pragma once

#include <Arduino.h>

// ---------- Stepper drivers ----------
#define TMC_BAUD          115200
#define R_SENSE           0.62f     // .62 ohms - IMPORTANT this must change if d
#define ADDR_PAN          0         // MS1 = GND, MS2 = GND
#define ADDR_TILT         1         // MS1 = 3V3, MS2 = GND
#define RUN_CURRENT_MA    250       // RMS. ~350 mA peak = motor rating (IRUN 28)
#define HOLD_MULTIPLIER   0.5f      // hold current = 50% of run
#define MICROSTEPS        16

// ---------- Motion ----------
#define STEPS_PER_SEC     800       // 800 / (200 * 16) = 0.25 rev/s
#define RUN_TIME_MS       5000
#define CW                HIGH      // swap HIGH/LOW if a motor turns the wrong way
#define CCW               LOW

// ---------- TMAG5273 hall sensors ----------
#define TMAG_ADDR         0x35      // library default (A1/A2 variant)
#define TMAG_I2C_HZ       400000
#define TMAG_LOG_MS       250       // field print interval per sensor while running

// ---------- Status LEDs ----------
#define LED_ON            LOW       // LEDs are active LOW
#define LED_OFF           (!LED_ON)
#define HEARTBEAT_MS      500       // toggle period -> 1 Hz blink
