// Garden Sentry main board - Teensy 4.0 pin map. CHECK AGAINST SCHEMATIC.
#pragma once

// ---------- Stepper drivers (2x TMC2209) ----------
#define PIN_STEP_PAN      2
#define PIN_DIR_PAN       3
#define PIN_STEP_TILT     4
#define PIN_DIR_TILT      5
#define PIN_EN            6     // shared ENN, active LOW (10k pull-up on board)
#define TMC_SERIAL        Serial1   // pins 0 (RX) / 1 (TX), shared UART bus

// ---------- TMAG5273 hall sensors (one per axis, separate I2C buses) ----------
#define TMAG_PAN_WIRE     Wire1     // pins 17 (SDA1) / 16 (SCL1)
#define TMAG_TILT_WIRE    Wire      // pins 18 (SDA)  / 19 (SCL)

// ---------- Status LEDs ----------
#define NO_PIN            255       // LED not wired: never driven
#define PIN_LED_HEARTBEAT 11        // status / heartbeat LED
#define PIN_LED_FAULT     20        // fault LED
#define PIN_LED_READY     12        // motor ready LED
