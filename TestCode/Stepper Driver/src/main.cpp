// Garden Sentry main board - first motor test (Teensy 4.0 + 2x TMC2209)
// Runs once at power-up: pan + tilt CW for 5s, CCW for 5s, then stop.
// Also checks the pan + tilt TMAG5273 hall sensors (I2C) and logs the Field
#include <Arduino.h>
#include "pins.h"
#include "config.h"
#include "motors.h"
#include "hall_sensor.h"
#include "status_leds.h"

// Step both motors together at a constant rate for duration Ms.
// Logs each working TMAG every TMAG_LOG_MS.
void runBoth(uint8_t dir, uint32_t durationMs) {
  digitalWriteFast(PIN_DIR_PAN, dir);
  digitalWriteFast(PIN_DIR_TILT, dir);
  delayMicroseconds(50);                   // DIR setup time

  const uint32_t stepPeriodUs = 1000000UL / STEPS_PER_SEC;
  elapsedMillis runTime;
  elapsedMicros sinceStep;
  elapsedMillis sincePanLog;
  elapsedMillis sinceTiltLog = TMAG_LOG_MS / 2;   // stagger the two sensors
  resetHallStats(hallPan);
  resetHallStats(hallTilt);

  while (runTime < durationMs) {
    updateHeartbeat();
    if (sinceStep >= stepPeriodUs) {
      sinceStep -= stepPeriodUs;
      digitalWriteFast(PIN_STEP_PAN, HIGH);
      digitalWriteFast(PIN_STEP_TILT, HIGH);
      delayMicroseconds(2);                // TMC2209 needs >100 ns
      digitalWriteFast(PIN_STEP_PAN, LOW);
      digitalWriteFast(PIN_STEP_TILT, LOW);
    }
    // One sensor read (~0.5 ms at 400 kHz) per pass, shorter than the
    // 1.25 ms step period, so at most one step is a little late.
    if (hallPan.ok && sincePanLog >= TMAG_LOG_MS) {
      sincePanLog = 0;
      logHall(hallPan, "  ");
    } else if (hallTilt.ok && sinceTiltLog >= TMAG_LOG_MS) {
      sinceTiltLog = 0;
      logHall(hallTilt, "  ");
    }
  }
}

void setup() {
  motorsInitPins();
  ledsInit();

  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}    // wait up to 3 s for serial monitor

  TMC_SERIAL.begin(TMC_BAUD);
  delay(200);                              // let 24 V / drivers settle

  Serial.println("Garden Sentry board test");
  bool panOk  = setupDriver(pan,  "PAN ");
  bool tiltOk = setupDriver(tilt, "TILT");
  if (!panOk || !tiltOk) {
    Serial.println("WARNING: driver UART failed. Is 24 V on? Motors will have");
    Serial.println("almost no current until the UART config goes through.");
  }

  // Motor test still runs if either sensor fails
  hallPan.ok  = setupHall(hallPan);
  hallTilt.ok = setupHall(hallTilt);

  // Fault LED: on (and stays on) if any check failed
  bool fault = !panOk || !tiltOk || !hallPan.ok || !hallTilt.ok;
  ledSet(PIN_LED_FAULT, fault);
  if (fault) Serial.println("FAULT LED on: see FAILED lines above");

  motorsEnable(true);
  // Ready LED: drivers enabled and both configured over UART
  ledSet(PIN_LED_READY, panOk && tiltOk);
  delay(100);

  Serial.println("CW 5 s");
  runBoth(CW, RUN_TIME_MS);
  printHallStats(hallPan,  "CW ");
  printHallStats(hallTilt, "CW ");

  Serial.println("CCW 5 s");
  runBoth(CCW, RUN_TIME_MS);
  printHallStats(hallPan,  "CCW");
  printHallStats(hallTilt, "CCW");

  motorsEnable(false);                     // stop: outputs off, motors free
  ledSet(PIN_LED_READY, false);

  Serial.println("Summary:");
  Serial.print("  PAN  UART: ");  Serial.println(panOk       ? "OK" : "FAILED");
  Serial.print("  TILT UART: ");  Serial.println(tiltOk      ? "OK" : "FAILED");
  Serial.print("  PAN  TMAG: ");  Serial.println(hallPan.ok  ? "OK" : "FAILED");
  Serial.print("  TILT TMAG: ");  Serial.println(hallTilt.ok ? "OK" : "FAILED");
  Serial.println("Done. Press the Teensy button or power-cycle to run again.");
}

void loop() {
  updateHeartbeat();                       // keeps blinking after the test
}
